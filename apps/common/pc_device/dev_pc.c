#include "dev_manage.h"
#include "dev_pc.h"
#include "usb_slave_api.h"
#include "card_reader_io.h"
#include "audio/audio.h"
#include "dac.h"
#include "audio/dac_api.h"
#include "sdk_cfg.h"
#include "ladc.h"
#include "msg.h"
#include "flash_api.h"
#include "audio/audio_stream.h"
#include "common/common.h"

#define CLASS_CONFIG        (AUDIO_CLASS|MASSSTORAGE_CLASS|HID_CLASS)

#if USB_PC_EN

#define USB_PC_PROTECT_EN	0
#define PC_INDEPEND_VOL     1   //独立音量

#define PC_ALLOC_POOL_SIZE  (5*1024 + 512)

#if 0 //def HAVE_MALLOC
static u8 *pc_alloc_pool;
#else
static u8 pc_alloc_pool[PC_ALLOC_POOL_SIZE] sec(.pc_mem);
#endif



LOOP_DETECT_REGISTER(pc_detect_loop) = {
    .time = 5,
    .fun  = pc_check_api,
};

void usb_pc_mem_init(void)
{
#if 0 //def HAVE_MALLOC
    pc_alloc_pool = malloc(PC_ALLOC_POOL_SIZE);
    ASSERT(pc_alloc_pool);
#endif
}

void usb_pc_mem_free(void)
{
#if 0 //def HAVE_MALLOC
    if (pc_alloc_pool) {
        free(pc_alloc_pool);
    }
#endif
}

#if USB_PC_PROTECT_EN

#define PROTECT_NUM		1

#define PROTECT_BUFF_MAX    (sizeof(usb_pc_protect) + (PROTECT_NUM * 2 * sizeof(u32)))
static u8 protect_buffer[PROTECT_BUFF_MAX] sec(.pc_mem);
static usb_pc_protect *protect_p = NULL;
static void usb_pc_protect_open(void)
{
#if USB_MALLOC_EN
    u32 need_buf = sizeof(usb_pc_protect) + (PROTECT_NUM * 2 * sizeof(u32));
    printf("---need_buf = %d\n", need_buf);
    protect_p = malloc(need_buf);
#else
    protect_p = (usb_pc_protect *)&protect_buffer[0];
#endif
    printf("protect_p = %x\n", protect_p);
    if (protect_p) {
        protect_p->gc_ProtectFATNum = PROTECT_NUM;
        protect_p->ProtectedFATSectorIndex[0] = 0x00;
        protect_p->ProtectedFATSectorIndex[1] = 0x04;
        //printf_buf(protect_p,3*4);
        usb_slave_ioctrl_api((void *)protect_p, USB_SLAVE_MD_PROTECT);
    } else {
        puts("usb_pc_protect_malloc_err\n");
    }
}

static void usb_pc_protect_close(void)
{
    usb_slave_ioctrl_api((void *)NULL, USB_SLAVE_MD_PROTECT);

    if (protect_p) {
#if USB_MALLOC_EN
        free(protect_p);
#endif
        protect_p = NULL;
    }
}
#endif


//用户自定义字符串描述
#if USER_PC_DESCRIPTOR

const u8 DISK_DEVNAME_STR[] = {
    'J', 'L', ' ', ' ', 'D', 'E', 'V', 'I', 'C', 'E', 'V',
    '1', '.', '0', '0', ' ', ' ', ' ',  ' ', ' ', ' ',  ' ', ' ', ' ',
    0x00,
};

const u8 USER_IMANUFACTURE_STR[] = {
    0x30, 0x03,
    'Z', 0, 'h', 0, 'u', 0, 'H', 0, 'a', 0, 'i', 0, '-', 0,
    'J', 0, 'i', 0, 'e', 0, 'L', 0, 'i', 0, '-', 0,
    'T', 0, 'e', 0, 'c', 0, 'h', 0, 'n', 0, 'o', 0, 'l', 0, 'o', 0, 'g', 0, 'y', 0,
};

const u8 USER_RIPRODUCT_STR[] = {
    0x22, 0x03,
    'J', 0, 'i', 0, 'e', 0, 'L', 0, 'i', 0, '-', 0,
    'B', 0, 'R', 0, '2', 0, '1', 0, ' ', 0, 's', 0, 'p', 0, 'k', 0, 'e', 0, 'r', 0
};

static void user_descriptor_init()
{
    user_set_descriptor(IMANUFACTURE_STRING, (void *)USER_IMANUFACTURE_STR, USER_IMANUFACTURE_STR[0]);

    user_set_descriptor(IPRODUCT_STRING, (void *)USER_RIPRODUCT_STR, USER_RIPRODUCT_STR[0]);
}
#endif

/*----------------------------------------------------------------------------*/
/**@brief  PC音频输出回调函数
   @param  NULL
   @return 数据长度
   @note
*/
/*----------------------------------------------------------------------------*/
static AUDIO_STREAM *pc_output;
static u8 pc_speak_out_sw_flag = 0;
static u32 pc_output_cb(s16 *buffer, u32 buffer_len)
{
    if (!pc_speak_out_sw_flag) {
        return 0;
    }
    if (pc_output) {
        return pc_output->output(pc_output->priv, buffer, buffer_len);
    }
    return 0;
}

static u32 get_pc_output_cbuf_len()
{
    if (pc_output) {
        return pc_output->data_len(pc_output->priv);
    }
    return 0;
}
static u32 get_pc_output_cbuf_size()
{
    if (pc_output) {
        AUDIO_STREAM_DAC *p_dac_stream = pc_output->priv;
        return p_dac_stream->buf_size;
    }
    return 0;
}

void pc_speak_out_sw(u8 sw)
{
    pc_speak_out_sw_flag = sw;
}
/*----------------------------------------------------------------------------*/
/**@brief  PC音频输出回调注册函数
   @param  dev_io usb 从机操作io
   @return 无
   @note
*/
/*----------------------------------------------------------------------------*/
static void usb_slave_spk_output_io_reg(void)
{
    u32 parm;
    AUDIO_OUTPUT_IO spk_output_io;

    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_PC;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    pc_output = audio_stream_init(&stream_param, NULL);

    spk_output_io.get_output_cbuf_len = get_pc_output_cbuf_len;
    spk_output_io.get_output_cbuf_size = get_pc_output_cbuf_size;
    spk_output_io.output_func = pc_output_cb;
    parm = (u32)&spk_output_io;

    usb_slave_ioctrl_api((void *)&parm, USB_SLAVE_SPK_OUTPUT_IO_REG);
}
/*----------------------------------------------------------------------------*/
/**@brief  PC MIC输入接口
   @param  buffer :mic采集的数据 buffer_len:数据长度
   @return 无
   @note
*/
/*----------------------------------------------------------------------------*/
void usb_slave_mic_input(s16 *buffer, u32 buffer_len)
{
    usb_mic_ladc_input(buffer, buffer_len);
}

/*----------------------------------------------------------------------------*/
/**@brief  PC 模式静音状态设置
   @param  mute_status：mute状态控制；fade_en：淡入淡出设置
   @return 无
   @note   void pc_dac_mute(bool mute_status, u8 fade_en)
*/
/*----------------------------------------------------------------------------*/
void pc_dac_mute(bool mute_status, u8 fade_en)
{
    dac_mute(mute_status, fade_en);
}


/*----------------------------------------------------------------------------*/
/**@brief  PC 模式DAC通道开启和音量设置
   @param  无
   @return 无
   @note   void pc_dac_on(void)
*/
/*----------------------------------------------------------------------------*/
void pc_dac_channel_on(void)
{
    dac_channel_on(UDISK_CHANNEL, FADE_ON);

#if PC_INDEPEND_VOL
    sound.tmp_sys_vol_l = sound.vol.sys_vol_l;
    sound.tmp_sys_vol_r = sound.vol.sys_vol_r;
//    sound.vol.sys_vol_l = PC_DEFAULT_VOL;
//    sound.vol.sys_vol_r = PC_DEFAULT_VOL;
#endif

//    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);//resume sys vol
}

/*----------------------------------------------------------------------------*/
/**@brief  PC 模式DAC通道关闭
   @param  无
   @return 无
   @note   void pc_dac_off(void)
*/
/*----------------------------------------------------------------------------*/
void pc_dac_channel_off(void)
{

#if PC_INDEPEND_VOL
    sound.vol.sys_vol_l = sound.tmp_sys_vol_l;
    sound.vol.sys_vol_r = sound.tmp_sys_vol_r;
#endif
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);

    dac_channel_off(UDISK_CHANNEL, FADE_ON);
//    dac_var.bMute = 0;

#if USB_PC_PROTECT_EN
    usb_pc_protect_close();	//close at last
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief  PC 模式AUDIO音量设置消息钩子函数
   @param  msg：需要投递的消息
   @return 无
   @note   void hook_susb_msg(u32 msg)
*/
/*----------------------------------------------------------------------------*/
static void hook_susb_msg(u32 msg)
{
    if (msg == USB_SLAVE_CMD_UPDATA) {
        msg = MSG_PC_UPDATA;
    } else if (msg == USB_SLAVE_CMD_VOL) {
        msg = MSG_PC_SPK_VOL;
    } else if (msg == USB_SLAVE_CMD_MUTE) {
        msg = MSG_PC_SPK_MUTE;
    } else if (msg == USB_SLAVE_CMD_UNMUTE) {
        msg = MSG_PC_SPK_UNMUTE;
    } else {
        return ;
    }

    task_post_msg(NULL, 1, msg);
}

/*----------------------------------------------------------------------------*/
/**@brief  从VM获取记忆音量值
   @param  none
   @return 记忆值
   @note   u8 get_usb_slave_volume_from_vm(void)
*/
/*----------------------------------------------------------------------------*/
static u8 get_usb_slave_volume_from_vm(void)
{
    u8 	pc_vol;

    //取出VM保存的上次PC音量值
    if (VM_PC_VOL_LEN != vm_read(VM_PC_VOL, &pc_vol, VM_PC_VOL_LEN)) {
        pc_vol = MAX_SYS_VOL_L;
    }
    printf("vm_pc_vol=%d\n", pc_vol);
    return pc_vol;
}

/*----------------------------------------------------------------------------*/
/**@brief  get left volume from vm
   @param  none
   @return left volume
   @note   u8 get_usb_slave_audio_left_volume(void)
*/
/*----------------------------------------------------------------------------*/
static u8 get_usb_slave_audio_left_volume(void)
{
    u8 spk_vol;

    spk_vol = get_usb_slave_volume_from_vm();  ///SYS_DEFAULT_VOL;
    spk_vol = (spk_vol >= MAX_SYS_VOL_L) ? 0xff : spk_vol << 3;

    return spk_vol;
}

/*----------------------------------------------------------------------------*/
/**@brief  get right volume from vm
   @param  none
   @return right volume
   @note   u8 get_usb_slave_audio_right_volume(void)
*/
/*----------------------------------------------------------------------------*/
static u8 get_usb_slave_audio_right_volume(void)
{
    u8 spk_vol;

    spk_vol = get_usb_slave_volume_from_vm();
    spk_vol = (spk_vol >= MAX_SYS_VOL_R) ? 0xff : spk_vol << 3;

    return spk_vol;
}
/*----------------------------------------------------------------------------*/
/**@brief  reset class config
   @param  NULL
   @return none
   @note   void usb_slave_class_config_reset(void)
*/
/*----------------------------------------------------------------------------*/
static void usb_slave_class_config_reset(void)
{
    u32 parm = CLASS_CONFIG;

    usb_slave_ioctrl_api(&parm, USB_SLAVE_CLASS_CONFIG_RESET);
}

/*----------------------------------------------------------------------------*/
/**@brief  register card reader
   @param  NULL
   @return none
   @note   void usb_slave_card_reader_reg(void)
*/
/*----------------------------------------------------------------------------*/
static void usb_slave_card_reader_reg(void)
{
    sUSB_DEV_IO *reg_parm = NULL;

    /* memset(reg_parm, 0x00, sizeof(card_reader_parm)); */
    /*注册读卡器的SD控制器接口*/
#if SDMMC0_EN
    reg_parm = get_card_read_io(DEV_SDCRAD_0);
#if USER_PC_DESCRIPTOR
    if (reg_parm) {
        reg_parm->card_r_parm.sDevName = (void *)DISK_DEVNAME_STR;  //用户自定义磁盘驱动器名
    }
#endif
    usb_slave_ioctrl_api(reg_parm, USB_SLAVE_CARD_READER0_REG);
#endif

#if SDMMC1_EN
    reg_parm = get_card_read_io(DEV_SDCRAD_1);
#if USER_PC_DESCRIPTOR
    if (reg_parm) {
        reg_parm->card_r_parm.sDevName = (void *)DISK_DEVNAME_STR; //用户自定义磁盘驱动器名
    }
#endif
    usb_slave_ioctrl_api(reg_parm, USB_SLAVE_CARD_READER1_REG);
#endif
    recover_cardreader_popup_api();//init popup
}



/*----------------------------------------------------------------------------*/
/**@brief  init usb slave parm
   @param  NULL
   @return init status
   @note   s32 usb_slave_var_init(void)
*/
/*----------------------------------------------------------------------------*/
static s32 usb_slave_var_init(void)
{
    usb_slave_init_parm susb_init_parm;

    susb_init_parm.os_msg_post_init = (void (*)(void))hook_susb_msg;
    susb_init_parm.vol_left 		= get_usb_slave_audio_left_volume();
    susb_init_parm.vol_right 		= get_usb_slave_audio_right_volume();

    return usb_slave_init_api(&susb_init_parm);
}

/*----------------------------------------------------------------------------*/
/**@brief  init usb slave moudle
   @param  none
   @return init status
   @note   s32 app_usb_slave_init(void)
*/
/*----------------------------------------------------------------------------*/
s32 app_usb_slave_init(void)
{
    s32 res;

    usb_pc_mem_init();

    //PC 模块所需RAM分配初始化
    usb_slave_alloc_init(pc_alloc_pool, PC_ALLOC_POOL_SIZE);
    /*usb slave var init*/
    res = usb_slave_var_init();
    if (res == NULL) {
        puts("susb var malloc error\n");
        return 0;
    }

#if USER_PC_DESCRIPTOR
    user_descriptor_init();    //用户自定义描述字符串注册
#endif
    /*audio dac init*/
    //for speaker
    if (CLASS_CONFIG & SPEAKER_CLASS) {
        dac_set_samplerate(48000, 0);     //DAC采样率设置为48K
        pc_dac_channel_on(); //开PC DAC模拟通道
        pc_speak_out_sw(1);
    }

    /*MASS STORAGE、AUDIO和HID功能重新设置:默认全开*/
    usb_slave_class_config_reset();

    /*register card_reader 属性*/
    usb_slave_card_reader_reg();
    /* 注册SPK USB中断回调  */
    usb_slave_spk_output_io_reg();


    /*挂载USB SLAVE设备*/
    usb_slave_open_api(NULL);

    //for mic
    if (CLASS_CONFIG & MIC_CLASS) {
        ladc_ch_open(LADC_MIC_CHANNEL, SR48000);
    }

#if USB_PC_PROTECT_EN
    usb_pc_protect_open();
#endif

    return res;
}

/*----------------------------------------------------------------------------*/
/**@brief  close usb slave moudle
   @param  none
   @return close status
   @note   s32 app_usb_slave_close(void)
*/
/*----------------------------------------------------------------------------*/
s32 app_usb_slave_close(void)
{
    s32 res;

    res =  usb_slave_close_api(NULL);

    if (CLASS_CONFIG & SPEAKER_CLASS) {
        pc_dac_channel_off(); //关PC DAC通道
    }
    if (CLASS_CONFIG & MIC_CLASS) {
        ladc_ch_close(LADC_MIC_CHANNEL);	//关闭ladc模块
    }

    usb_pc_mem_free();

    return res;
}

/*----------------------------------------------------------------------------*/
/**@brief  run card reader moudle
   @param  none
   @return run status
   @note   s32 app_usb_slave_card_reader(u32 cmd)
*/
/*----------------------------------------------------------------------------*/
s32 app_usb_slave_card_reader(u32 cmd)
{
    return usb_slave_ioctrl_api(&cmd, USB_SLAVE_RUN_CARD_READER);
}

/*----------------------------------------------------------------------------*/
/**@brief  run hid moudle
   @param  none
   @return run status
   @note   s32 app_usb_slave_hid(u32 hid_cmd)
*/
/*----------------------------------------------------------------------------*/
s32 app_usb_slave_hid(u32 hid_cmd)
{
    return usb_slave_ioctrl_api(&hid_cmd, USB_SLAVE_HID);
}

/*----------------------------------------------------------------------------*/
/**@brief  PC 模式AUDIO音量设置
   @param  pc_mute_status：mute状态
   @return AUDIO音量
   @note   u8 app_pc_set_speaker_vol(u32 pc_mute_status)
*/
/*----------------------------------------------------------------------------*/
u8 app_pc_set_speaker_vol(u32 pc_mute_status)
{
    u32 spk_vol_l, spk_vol_r;

    usb_slave_ioctrl_api(&spk_vol_l, USB_SLAVE_GET_SPEAKER_VOL);
    spk_vol_l &= 0xff;
    //pc_printf("api_vol=%x, %x\n", ep0_var->usb_class_info.bAudioCurL[0], ep0_var->usb_class_info.bAudioCurR[0]);
    if (spk_vol_l) {
        spk_vol_l >>= 3;
        spk_vol_l = (spk_vol_l == 0) ? 1 : spk_vol_l;
        spk_vol_l = (spk_vol_l >= MAX_SYS_VOL_L) ? MAX_SYS_VOL_L : spk_vol_l;
    }

    usb_slave_ioctrl_api(&spk_vol_r, USB_SLAVE_GET_SPEAKER_VOL);
    spk_vol_r >>= 8;
    spk_vol_r &= 0xff;
    if (spk_vol_r) {
        spk_vol_r >>= 3;
        spk_vol_r = (spk_vol_r == 0) ? 1 : spk_vol_r;
        spk_vol_r = (spk_vol_r >= MAX_SYS_VOL_L) ? MAX_SYS_VOL_L : spk_vol_r;
    }

    pc_dac_mute(pc_mute_status, FADE_ON);

    // set_sys_vol(spk_vol_l, spk_vol_r, FADE_ON);

    sound.vol.sys_vol_r = spk_vol_r;
    sound.vol.sys_vol_l = spk_vol_l;
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);

    vm_cache_write(VM_PC_VOL, &spk_vol_l, 4);

    return spk_vol_l;
}

/*----------------------------------------------------------------------------*/
/**@brief  get usb slave online status
   @param  none
   @return run online status
   @note   u32 app_usb_slave_online_status(void)
*/
/*----------------------------------------------------------------------------*/
u32 app_usb_slave_online_status(void)
{
    u32 online_status;

    return usb_slave_ioctrl_api(&online_status, USB_SLAVE_GET_ONLINE_STATUS) ;  //USB_SLAVE_GET_ONLINE_STATUS)
}
/*----------------------------------------------------------------------------*/
/**@brief  PC 在线列表
   @note   const u8 pc_event_tab[]
*/
/*----------------------------------------------------------------------------*/
static const u8 pc_event_tab[] = {
    EVENT_PC_OFFLINE, //PC拔出
    EVENT_PC_ONLINE,  //PC插入
};


/*----------------------------------------------------------------------------*/
/**@brief  PC 在线检测API函数
   @param  无
   @return 无
   @note   void pc_check_api(void)
*/
/*----------------------------------------------------------------------------*/
void pc_check_api()
{
    s32 dev_status;
    u32 parm;

    parm = USB_DISK_EN;  //如果打开U盘检测，PC不需要再重复检测
    dev_status = usb_slave_ioctrl_api(&parm, USB_SLAVE_ONLINE_DETECT);

    if (dev_status != DEV_HOLD) {
        task_post_event(NULL, 1, pc_event_tab[dev_status]);
//        printf("pc event %d\n", dev_status);
    }
}
#undef	NULL

#endif
