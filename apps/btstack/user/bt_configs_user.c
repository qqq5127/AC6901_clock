#include "bluetooth/avctp_user.h"
#include "task_bt.h"
#include "sdk_cfg.h"
#include "fcc_test.h"
#include "audio/ladc.h"
#include "audio/dac_api.h"
#include "audio/tone.h"
#include "aec_main.h"
#include "PLC_main.h"
#include "crc_api.h"
#include "power.h"
#include "aec_user.h"
#include "msg.h"
#include "audio/dac_api.h"
#include "audio/dac.h"
#include "audio/audio.h"
#include "a2dp_decode.h"
#include "clock_interface.h"
#include "flash_api.h"
#include "cpu/rand64.h"
#include "dev_manage.h"
#include "fs.h"
#include "bt_tws.h"
#include "ble_api.h"
#include "common/common.h"
#include "warning_tone.h"
#include "wdt.h"
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".bt_app_bss")
#pragma data_seg(	".bt_app_data")
#pragma const_seg(	".bt_app_const")
#pragma code_seg(	".bt_app_code")
#endif

//#define CFG_DEBUG_EN
#ifdef CFG_DEBUG_EN
#define cfg_puts 		puts
#define cfg_put_buf 	put_buf
#define cfg_printf		log_printf
#else
#define cfg_puts(...)
#define cfg_put_buf(...)
#define cfg_printf(...)
#endif

///---sdp service record profile- 用户选择支持协议--///
#define USER_SUPPORT_PROFILE_SPP    0
#define USER_SUPPORT_PROFILE_HFP    1
#define USER_SUPPORT_PROFILE_A2DP   1
#define USER_SUPPORT_PROFILE_AVCTP  1
#define USER_SUPPORT_PROFILE_HID    0
#define USER_SUPPORT_PROFILE_PBAP   0

static void bt_profile_select_init(void)
{
    u8 support_profile;
    support_profile = SPP_CH | HFP_CH | A2DP_CH | AVCTP_CH | HID_CH | AVDTP_CH | PBAP_CH ;
#if (USER_SUPPORT_PROFILE_HFP==0)
    support_profile &= ~HFP_CH;
#else
    set_supprot_hfp();
#endif
#if (USER_SUPPORT_PROFILE_AVCTP==0)
    support_profile &= ~AVCTP_CH;
#else
    set_supprot_avctp();
#endif
#if (USER_SUPPORT_PROFILE_A2DP==0)
    support_profile &= ~A2DP_CH;
    support_profile &= ~AVCTP_CH;
    support_profile &= ~AVDTP_CH;
#else
    set_supprot_a2dp();
#endif
#if (USER_SUPPORT_PROFILE_SPP==0)
    support_profile &= ~SPP_CH;
#else
    set_supprot_spp();
#endif
#if (USER_SUPPORT_PROFILE_HID== 0)
    support_profile &= ~HID_CH;
#else
    set_supprot_hid();
#endif
#if (USER_SUPPORT_PROFILE_PBAP== 0)
    support_profile &= ~PBAP_CH;
#else
    set_supprot_pbap();
#endif
#if BT_TWS
    set_supprot_a2dp_source();

#endif

    bt_cfg_default_init(support_profile);/*外部设置支持什么协议*/
#if BT_HID_INDEPENDENT_MODE
    set_hid_independent_info();
#endif
}

/*
 ******************************************************************************
 *					BT SETUP INIT
 *
 *Description: This function is called to init bt config infomation
 *
 *Argument(s): addr:dst bd_addr
 *			    name:dst bd_name
 *             name_idx:host_name index(0~(max-1))
 *Returns	 : none
 *
 *Note(s)	 : 1)more name,you can select one of them through name_idx
 *			   2)single name,select the newest one
 ******************************************************************************
 */
#define LOCAL_NAME_NUM	20	/*BD_NAME NUM_MAX*/
#define LOCAL_NAME_LEN	30	/*BD_NAME_LEN_MAX*/

typedef struct _BT_ADDR {
    u16 crc;
    u8 data[6];
} bt_addr;

typedef struct _BT_NAME {
    u16 crc;
    u8 data[LOCAL_NAME_LEN];
} bt_name;

struct _BT_CFG {
    //cfg info
    u16 cfg_crc;
    u16 cfg_len;
    /* 蓝牙2.1信息配置 */
    u8 name_num_max;
    bt_name name[LOCAL_NAME_NUM];
    char pin_code[4];
    u8 addr[6];
    u8 rf_power;
    /* bt aec */
    u8 dac_analog_gain;
    u8 mic_analog_gain;
    /* 蓝牙4.0信息配置 */
    /* u8 ble_name[16]; */
    bt_name ble_name;
    u8 ble_public_addr[6];
    u8 ble_random_addr_type;
    u8 ble_random_addr[6];
    u8 ble_identity_addr_type;
} _GNU_PACKED_;
typedef struct _BT_CFG BT_CFG;

static s32 bt_cfg_read(BT_CFG *cfg)
{
    DEV_HANDLE dev = cache;
    fs_open_file_bypath(dev, cfg, sizeof(BT_CFG), "/bt_cfg.bin");
    //put_buf((u8 *)cfg, sizeof(BT_CFG));
    if ((cfg->cfg_crc == crc16(&cfg->cfg_len, cfg->cfg_len - 2)) && (cfg->cfg_crc != 0x0)) {
        puts("read cfg file ok\n");
        return 0;
    } else {
        puts("read cfg file faild\n");
        return -1;
    }
}

#define ADDR_UPDATE_MAX		2
extern void bd_set_max_pwr(u8 value);
extern void bd_set_min_pwr(u8 value);
static void bt_setup_init(u8 *adr, char *name, u8 idx, char *pin_code)
{
    u32 bd_addr_base = app_use_flash_cfg.btif_addr;
    u32 bd_cfg_base = bd_addr_base + (ADDR_UPDATE_MAX * 8);
    u32 btif_len = app_use_flash_cfg.btif_len;
    bt_addr addr_cfg;
    u8 update_idx = 0;
    u8 cfg_status = 0; /*1 means open cfg file success*/
    u8 All_FF[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 rf_power_max = 7; /*default rf tx power*/
    BT_CFG bt_cfg;

    log_printf("btif_addr:0x%x\n", app_use_flash_cfg.btif_addr);
    log_printf("btif_len:0x%x\n", app_use_flash_cfg.btif_len);


    vm_read_by_addr((u8 *)&bt_cfg, bd_cfg_base, sizeof(bt_cfg));
    /**没升级过len是0xffff，这里加一个0xffff的判断是为了防止crc不要算太久了***/
    if ((bt_cfg.cfg_len != 0xffff) &&
        (bt_cfg.cfg_crc == crc16(&bt_cfg.cfg_len, bt_cfg.cfg_len - 2)) &&
        (bt_cfg.cfg_crc != 0x0)) {
        /*read updata cfg ok*/
        cfg_status = 1;
    } else if (bt_cfg_read(&bt_cfg) == 0) {
        /*read default cfg ok*/
        cfg_status = 2;
    }

    if (cfg_status) {
        log_printf("bt_cfg %d OK\n", cfg_status);
        memcpy(name, bt_cfg.name[idx].data, LOCAL_NAME_LEN);
        memcpy(pin_code, bt_cfg.pin_code, 4);
        aec_param.dac_analog_gain = bt_cfg.dac_analog_gain;
        aec_param.mic_analog_gain = bt_cfg.mic_analog_gain;
        rf_power_max = bt_cfg.rf_power;

        /**
         * bd addr update_area
         * check update_area have valid bd_addr  or not.
         */
        if ((bt_cfg.addr[0] == 0xFF) &&
            (bt_cfg.addr[1] == 0xFF) &&
            (bt_cfg.addr[2] == 0xFF) &&
            (bt_cfg.addr[3] == 0xFF) &&
            (bt_cfg.addr[4] == 0xFF) &&
            (bt_cfg.addr[5] == 0xFF)) {
            while (update_idx < ADDR_UPDATE_MAX) {
                vm_read_by_addr((u8 *)&addr_cfg, bd_addr_base + update_idx * (sizeof(bt_addr)), sizeof(bt_addr));
                printf("addr %d:", update_idx);
                put_buf((u8 *)&addr_cfg, sizeof(bt_addr));
                if (addr_cfg.crc == crc16(addr_cfg.data, 6)) {
                    memcpy(adr, addr_cfg.data, 6);
                    break;
                }
                update_idx++;
            }
            if (update_idx >= ADDR_UPDATE_MAX) {
                puts("cfg_addr invalid\n");
                update_idx = 0;
                while (update_idx < ADDR_UPDATE_MAX) {
                    vm_read_by_addr((u8 *)&addr_cfg, bd_addr_base + update_idx * (sizeof(bt_addr)), sizeof(bt_addr));
                    if (memcmp(All_FF, &addr_cfg, 8) == 0) {
                        get_random_number(addr_cfg.data, 6);
                        addr_cfg.crc = crc16(addr_cfg.data, 6);
                        printf("cfg_addr new %d:", update_idx);
                        put_buf((u8 *)&addr_cfg, sizeof(bt_addr));
                        vm_write_by_addr((u8 *)&addr_cfg, bd_addr_base + update_idx * (sizeof(bt_addr)), sizeof(bt_addr));
                        memcpy(adr, addr_cfg.data, 6);
                        break;
                    }
                    update_idx++;
                }
            }
        } else {
            puts("BD_addr Fixed:");
            memcpy(adr, bt_cfg.addr, 6);
            put_buf(adr, 6);
        }
#if 1
        puts("pin_code:");
        put_buf((u8 *)pin_code, 4);
        log_printf("rf_power:%d\n", bt_cfg.rf_power);
        log_printf("dac_analog_gain:%d\n", aec_param.dac_analog_gain);
        log_printf("mic_analog_gain:%d\n", aec_param.mic_analog_gain);
#endif
    }
    bd_set_max_pwr(rf_power_max);
    bd_set_min_pwr(0);
    printf("LocalName:%s\n", name);
}

#if(BT_MODE == NORMAL_MODE)
static char host_name[LOCAL_NAME_LEN] = "BR21_lhh";
#else
static char host_name[LOCAL_NAME_LEN] = "BR21-RAM";
#endif
static const char hid_name[] = "JL_HID_NAME";
/*设置参数统一在这个函数内设置*/
static void bt_function_select_init()
{
    char pin_code[4] = {'0', '0', '0', '0'};
    /*蓝牙功能流程选择配置*/
    cfg_test_box_and_work_mode(NON_TEST, BT_MODE);
    cfg_bt_pll_para(OSC_Hz, SYS_Hz, BT_ANALOG_CFG, BT_XOSC);
    cfg_rf_rxtx_status_enable(0);
    __set_ble_bredr_mode(BLE_BREDR_MODE);    /*bt enble BT_BLE_EN|BT_BREDR_EN */
#if(BT_MODE==NORMAL_MODE)
    u8 debug_addr[6] = {0x78, 0x82, 0x71, 0x19, 0x30, 0x48};
    bt_setup_init(debug_addr, host_name, 0, pin_code);
#else
    u8 debug_addr[6] = {0x11, 0x22, 0x33, 0x33, 0x22, 0x11};
    u8 unuse_addr[6];
    char unuse_name[LOCAL_NAME_LEN];
    bt_setup_init(unuse_addr, unuse_name, 0, pin_code);
#endif
    __set_host_name(host_name, sizeof(host_name));
    __set_hid_name(hid_name, sizeof(hid_name));
    __set_pin_code(pin_code);
    __set_bt_mac_addr(debug_addr);
    bt_profile_select_init();

    __set_user_ctrl_conn_num(BT_CONNTCT_NUM);     /*用户设置支持连接的个数，1*/
    __set_auto_conn_device_num(BT_CONNTCT_NUM);   /*该接口用于设置上电回连需要依次搜索设备的个数。大于9无效，直到连上一个*/

    __bt_set_sniff(SNIFF_MODE_CONF);  /*设置进入sniff是进入poweroff还是powerdown*/
    __bt_set_update_battery_time(0); /*设置电量显示发送更新的周期时间，为0表示关闭电量显示功能，单位秒，u32, 不能小于10s*/
    __bt_set_a2dp_auto_play_flag(0); /*高级音频连接完成自动播放歌曲使能, 0不使能，1使能*/
    __set_simple_pair_flag(1);       /*提供接口外部设置配对方式,1使能简易配对。0使用pin code, 会使用配置文件的值*/
    __set_sbc_cap_bitpool(53);
    __set_page_timeout_value(8000); /*回连搜索时间长度设置,可使用该函数注册使用，ms单位,u16*/
    __set_super_timeout_value(8000); /*回连时超时参数设置。ms单位。做主机有效*/
    __set_phone_vol_sync_en(BT_PHONE_VOL_SYNC);		/*通话时音量同步,0不使能，1使能*/
    __set_search_timeout_value(0x06);/*搜索设备时间,Range:0x01-0x30,Time = N * 1.28s,0为不超时*/
    __set_music_break_in_flag(1);  /* 音频抢断开关，0：不抢断，1：抢断*/
    __set_auto_pause_flag(1);		/*播歌/通话抢断自动暂停，0：不暂停，1：自动暂停*/
    __set_hfp_switch(1);             /*通话抢断开关，0：不抢断，1：抢断*/

    __bt_set_music_back_play_flag(1); /*设置后台时可以通过音乐播放返回蓝牙 */
    __set_esco_packet_type(1);
#if BT_BACKGROUND_EN
    set_sbc_detect_module(1);         /*设置后台预处理数据模块，用于检测是否为静音数据*/
    __set_back_to_last_mode_flag(1);  /*若音乐返回蓝牙，为1时设置音乐暂停或静音一段时间后返回原来的模式*/
    __set_a2dp_sound_detect_counter(5, 150); /*第一个参数是后台检测返回蓝牙的包数目，第二个参数是退出回到原来模式静音的包数目*/
#endif

    /*1拖2需要设置的参数*/
#if ((BT_TWS == 0) && (BT_CONNTCT_NUM == 2))
    set_sbc_detect_module(1);
    /*
     *default:
     *sound_come_counter:80
     *sound_go_counter:260
     */
    __set_a2dp_sound_detect_counter(80, 260);

    /*
     *该配置用在1拖2的功能中，两台手机一起打电话:
     *case 1:第二台打电话的手机打断第一台：通话结束恢复第一台(如果通话中)的通话
     *case 2:第二台打电话的手机不打断第一台：通话结束恢复第二台(如果通话中)的通话
     */
    __set_hfp_restore(1);            /*通话恢复开关，0：不恢复，1：恢复*/
#endif

#if BT_TWS
#if BT_TWS&BT_TWS_BROADCAST
    __set_tws_start_conn(0); /*对箱回连，0：不回连，1：回连(手机优先),2：回连(TWS优先)*/
#else
    u8 get_tws_mem_role(void);
    if (get_tws_mem_role() == TWS_ROLE_SLAVE) {
        /*TWS从机不回连对箱*/
        __set_tws_start_conn(0); /*对箱回连，0：不回连，1：回连(手机优先),2：回连(TWS优先)*/
        __set_auto_conn_device_num(1);/*该接口用于设置上电回连需要依次搜索设备的个数。大于9无效，直到连上一个*/
    } else {
        __set_tws_start_conn(1); /*对箱回连，0：不回连，1：回连(手机优先),2：回连(TWS优先)*/
    }
#endif
    //__set_phone_conn_filt(1); /*连接对箱之前过滤手机的连接*/
    __set_device_role(S_DEVICE_ROLE_SLAVE | M_DEVICE_ROLE_MASTER_TO_SLAVE); /*设置设备role*/
#else
    __set_device_role(S_DEVICE_ROLE_SLAVE | M_DEVICE_ROLE_MASTER_TO_SLAVE); /*设置设备role*/
#endif
    __change_hci_class_type(BD_CLASS_PHONEBOOK);


    /**********************************************************************

    		              fcc srrc 测试

    void bredr_fcc_srrc_reset(s16 ofsi, s16 ofsq, s16 cmpuk1, s16 cmpsk2, s16 smpsk3)

    1、ac691x_sdk_cfg.h 里面把BT_MODE 配置  TEST_FRE_OFF_MODE
       开启串口打印
    2、fcc_srrc_trim_step 函数 传参数  1   检测 ofsi  ofsq 值 通过串口打印
    3、fcc_srrc_trim_step 函数 传参数  2   检测 cmpuk1 cmpsk2 smpsk3 值  通过串口打印
    4、把2、3检测到的值 填到 bredr_fcc_srrc_reset 函数对应参数
    5、fcc_srrc_trim_step 函数 传参数  0   关闭检测


    ************************************************************************** */

    //fcc_srrc_trim_step(0);
    //bredr_fcc_srrc_reset(-14, -14, 128, 0, -5);

    // __set_simple_pair_param(2, 0, 1);  ///<通过输入配对码的方式鉴权

}


#if (BLE_BREDR_MODE&BT_BLE_EN)

#define BLE_LOCAL_NAME_LEN    30
static char ble_local_name[BLE_LOCAL_NAME_LEN] = "BR21_BLE";
static const u8 ble_debug_addr[6] = {0x11, 0x22, 0x77, 0x19, 0x30, 0x99};
static int ble_cfg_read(BT_CFG *bt_cfg)
{
    DEV_HANDLE dev = cache;
    fs_open_file_bypath(dev, bt_cfg, sizeof(BT_CFG), "/bt_cfg.bin");
    //put_buf((u8 *)&bt_cfg, sizeof(BT_CFG));
    if (bt_cfg->cfg_crc == crc16(&bt_cfg->cfg_len, bt_cfg->cfg_len - 2)) {
        //puts("ble read cfg file ok\n");
        return 1;
    } else {
        //puts("ble read cfg file faild\n");
        memcpy(bt_cfg->ble_name.data, ble_local_name, BLE_LOCAL_NAME_LEN);
        memset(bt_cfg->ble_public_addr, 0xFF, 6);
        memset(bt_cfg->ble_random_addr, 0xFF, 6);
        bt_cfg->ble_identity_addr_type = 0;
        bt_cfg->ble_random_addr_type = 0;
        return 0;
    }
}

/*设置参数统一在这个函数内设置*/
static void ble_config_select_init(void)
{
    BT_CFG bt_cfg;
    u8 *adr;
    u32 bd_addr_base = app_use_flash_cfg.btif_addr;
    u32 bd_cfg_base = bd_addr_base + (ADDR_UPDATE_MAX * 8);
    u32 btif_len = app_use_flash_cfg.btif_len;
    u8 addr_len = 8;
    bt_addr addr_cfg;
    u8 update_idx = 0;

    /*蓝牙功能流程选择配置*/

    vm_read_by_addr((u8 *)&bt_cfg, bd_cfg_base, sizeof(bt_cfg));
    if ((bt_cfg.cfg_crc != crc16(&bt_cfg.cfg_len, bt_cfg.cfg_len - 2)) && (bt_cfg.cfg_crc != 0x0)) {
        ble_cfg_read(&bt_cfg);
    }

    adr = bt_cfg.ble_public_addr;

    if ((adr[0] == 0xFF) &&
        (adr[1] == 0xFF) &&
        (adr[2] == 0xFF) &&
        (adr[3] == 0xFF) &&
        (adr[4] == 0xFF) &&
        (adr[5] == 0xFF)) {
        //user edr address
        while (update_idx < ADDR_UPDATE_MAX) {
            vm_read_by_addr((u8 *)&addr_cfg, bd_addr_base + update_idx * (sizeof(bt_addr)), sizeof(bt_addr));
            printf("addr %d:", update_idx);
            put_buf((u8 *)&addr_cfg, sizeof(bt_addr));
            if (addr_cfg.crc == crc16(addr_cfg.data, 6)) {
                memcpy(adr, addr_cfg.data, 6);
                //NOT BIT0
                if (adr[5] & BIT(0)) {
                    adr[5] &= 0xFE;
                } else {
                    adr[5] |= BIT(0);
                }
                break;
            }
            update_idx++;
        }

        if (update_idx >= ADDR_UPDATE_MAX) {
            memcpy(bt_cfg.ble_public_addr, ble_debug_addr, 6);
        }
    }

    memcpy(ble_local_name, bt_cfg.ble_name.data, BLE_LOCAL_NAME_LEN);

    ble_set_mac_addr(bt_cfg.ble_random_addr_type, bt_cfg.ble_public_addr);

    printf("---ble_config-----name %s,address:", ble_local_name);
    printf_buf(bt_cfg.ble_public_addr, 6);
    puts("\n");

}

u8 get_ble_local_name_len()
{
    u8 len = strlen(ble_local_name);
    return len;
}

u8 get_ble_local_name(u8 *name_buf)
{
    u8 len = strlen(ble_local_name);
    memcpy(name_buf, ble_local_name, len + 1);
    return len;
}

//lbuf,不可改小，会导致BLE运行不正常
extern void ble_set_gap_role(u8 role);
static u8 ble_btstack_memory[0x1000] __attribute__((aligned(4)));
static void ble_before_init_set(void)
{
    int ret;
    ble_set_gap_role(BLE_GAP_ROLE);
    ret = ble_btstack_set_ram((u32)&ble_btstack_memory, sizeof(ble_btstack_memory));

    if (ret) {
        printf(">>>ble init fail,ram not good!!!, need ram = 0x%x !!!!!!!!!\n", ret);
        while (1);
    }

    if (BLE_GAP_ROLE) {
        puts("\n---ble gap role is client---\n");
    } else {
        puts("\n---ble gap role is server---\n");
    }

}


#endif

extern u8 bt_music_sync_vol(u8 vol, u8 cmd);
extern u8 get_a2dp_decoder_status();
/*
蓝牙库用到的一系列可设置参数配置，通过handle的形式。
这样用户就不用考虑调用配置函数的具体位置。需要改变可以注册，不需要设置有默认参数
*/
static void set_device_volume(int volume)
{
    /*
     *以下情况不同步高级音频音量
     *1:播提示音的时候
     *2:通话的时候
     *3:蓝牙处于后台模式
     */

    if (is_tws_device_slave() == TRUE) {
        puts("TWS slave,not support music sync\n");
        return ;
    }

    if (volume > 127) {
        puts("not support music sync\n");
        bt_music_sync_vol(0xff, 0);
        return;
    }
    u8 change_to_dac_level = volume *  MAX_SYS_VOL_L / 127;
    printf("phone:%d\tdac:%d\n", volume, change_to_dac_level);
    bt_music_sync_vol(change_to_dac_level, 0);
    if (get_a2dp_decoder_status() == 0) {
        puts("It's not smart to sync a2dp vol now\n");
        return;
    }
    sound.vol.sys_vol_l = change_to_dac_level;
#if BT_TWS
    //send sync_vol to tws_slave
    tws_cmd_send(MSG_BT_TWS_VOL_SYNC, change_to_dac_level);
#endif
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_l, FADE_OFF);
    printf("phone:%d\tdac:%d\n", volume, sound.vol.sys_vol_l);
#if USE_16_LEVEL_VOLUME
	for (u8 i=0; i<=MAX_SYS_VOL_TEMP; i++)
	{
		if (sound.vol.sys_vol_l <= volume_table[i])
		{
			volume_temp = i;
			break;
		}
	}
#endif
}

static int get_dac_vol()
{
    int a2dp_vol = bt_music_sync_vol(0xff, 1);
    if (a2dp_vol > 30) {
        a2dp_vol = sound.vol.sys_vol_l;
    }
    bt_music_sync_vol(a2dp_vol, 0);
    return (a2dp_vol * 127 / MAX_SYS_VOL_L) ;
}
static void spp_data_deal(u8 packet_type, u16 channel, u8 *packet, u16 size)
{
    switch (packet_type) {
    case 1:
        /* puts("spp connect\n"); */
        break;
    case 2:
        /* puts("spp disconnect\n"); */
        break;
    case 7:
        /* puts("spp data\n"); */
#if AEC_DEBUG_ONLINE
        aec_config_online(packet, size);
#endif
        break;
    }
}


static void bt_fast_test_api(void)
{
    log_printf("---bt_fast_test_api---\n");
    sys_global_value.fast_test_flag = 0x1A;
    sin_tone_toggle(1);//enable key_tone

    //不用宏包住
    sound_automute_set(0, -1, -1, -1); // 关自动mute
    dac_toggle(1);
    set_sys_vol(30, 30, FADE_OFF);
    microphone_open(30, 0);
}
void mic_test(void)
{
    //不用宏包住
    printf("mic test\n");
    sound_automute_set(0, -1, -1, -1); // 关自动mute
    dac_toggle(1);
    set_sys_vol(30, 30, FADE_OFF);
    microphone_open(30, 0);
}
static void bt_updata_start_api(u32 parm)
{
    task_post_msg(NULL, 1, MSG_BT_UPDATA_START);
}

static void bt_updata_end_api(u32 parm)
{

    u8 updata_type = (u8)((parm >> 8) & 0xFF);
    u8 updata_ret  = (u8)(parm & 0xFF);

    printf("bt_updata_end_api ret = %d\n", updata_type);
    printf("bt_updata_end_api ret = %d\n", updata_ret);


    if (updata_type == 0x02) {		//updata_name
        if (!updata_ret) {
            task_post_msg(NULL, 1, MSG_BT_UPDATA_END);
        }
    } else if (updata_type == 0x01) {
        //updata_osc
    }
}

static void read_name_handle(u8 *name)
{
    putchar('\n');
    puts((const char *)name);
    putchar('\n');
}
/*电量等级变化*/
#if USE_AD_CHECK_VOLTAGE
	#define POWER_FLOOR_LEVEL   32	//根据实际分压填写
#else
    #define POWER_FLOOR_LEVEL    32
#endif
static int bt_get_battery_value()
{
    //只支持1-9直接的值，9个等级
    u16 battery_level;
    u16 battery_value = get_battery_level() / 10;
    if (battery_value <= POWER_FLOOR_LEVEL) {
        battery_level = 1;
    } else {
        battery_level = (battery_value - POWER_FLOOR_LEVEL);
    }
    /* printf("battery_level=0x%x\n",battery_level ); */
    return battery_level;
}

static void user_get_bt_music_info(u8 type, u32 time, u8 *info, u16 len)
{
    //profile define type: 1-title 2-artist name 3-album names 4-track number 5-total number of tracks 6-genre  7-playing time
    //JL define 0x10-total time , 0x11 current play position
    u8  min, sec;
    //printf("type %d\n", type );
    if ((info != NULL) && (len != 0)) {
        puts((const char *)info);
        putchar('\n');
    }
    if (time != 0) {
        min = time / 1000 / 60;
        sec = time / 1000 - (min * 60);
        printf(" time %d %d\n ", min, sec);
    }
}
static u8 user_inf_buf[50];
u8 osc_internal_data[4];
void set_osc_internal_info(u32 bt_osc_internal)
{
    u16 crc;
    osc_internal_data[2] = (bt_osc_internal & 0x1f);
    osc_internal_data[3] = (bt_osc_internal >> 5) & 0x1f;
    crc = crc16((void *)&osc_internal_data[2], 2);
    osc_internal_data[0] = crc & 0xff;
    osc_internal_data[1] = (crc >> 8) & 0xff;
    vm_write(VM_BT_OSC_INT_INFO, osc_internal_data, VM_OSC_INT_INFO_LEN);
    puts("set_osc_internal_info\n");
    printf_buf((void *)osc_internal_data, 4);
    /* set_bt_update_data(bfu_head.filename, UPDATE_OSC_TRIM, bt_updata_ram, 4); */
}

bool get_osc_internal_info(u8 *bt_osc_internal_l, u8 *bt_osc_internal_r)
{
    u16 crc = 0;
    osc_internal_data[0] = crc & 0xff;
    osc_internal_data[1] = (crc >> 8) & 0xff;
    if (VM_OSC_INT_INFO_LEN != vm_read(VM_BT_OSC_INT_INFO, osc_internal_data, VM_OSC_INT_INFO_LEN)) {
        puts("get_osc_internal_info  is null xx\n");
        memset(osc_internal_data, 0x00, sizeof(osc_internal_data));
        return FALSE;
    }
    crc |= osc_internal_data[0] & 0xff;
    crc |= (osc_internal_data[1] << 8) & 0xff00;
    if (crc == crc16((void *)&osc_internal_data[2], 2)) {
        *bt_osc_internal_l = osc_internal_data[2];
        *bt_osc_internal_r = osc_internal_data[3];
        return  TRUE;
    }
    return FALSE;
}
void bt_osc_init()
{
    u8 bt_osc_internal_l;
    u8 bt_osc_internal_r;
    if ((get_osc_internal_info(&bt_osc_internal_l, &bt_osc_internal_r)) && (bt_osc_internal_l <= 0x1f) && (bt_osc_internal_r <= 0x1f)) {
        //printf("<<<get_osc_internal_info ok,bt_osc_internal_l=0x%x,bt_osc_internal_r=0x%x>>\n", bt_osc_internal_l, bt_osc_internal_r);
    } else {
        puts("get_osc_internal_info err\n");
        bt_osc_internal_l = BT_OSC_INTERNAL_L;
        bt_osc_internal_r = BT_OSC_INTERNAL_R;
    }

    bt_osc_internal_cfg(bt_osc_internal_l, bt_osc_internal_r);
}

/**
  *保存对箱连接角色
  *用来判断下次开机是否发起回连
  */
void bt_tws_info_save(u8 info)
{
    u8 tws_info = 0;
    vm_read(VM_TWS_INFO, &tws_info, VM_TWS_INFO_LEN);
    if (tws_info == info) {
        puts("same tws_info,return\n");
    }
    tws_info = info;
    update_bt_tws_info(info);
    log_printf("TWS_info W:%x\n", info);
    vm_write(VM_TWS_INFO, &tws_info, VM_TWS_INFO_LEN);
}
/**
  *获取对箱上次连接角色，用来判断开机是否发起回连
  *主机发起回连，从机等代连接
  */
u8 get_tws_mem_role(void)
{
    u8 info;
    vm_read(VM_TWS_INFO, &info, VM_TWS_INFO_LEN);
    log_printf("TWS_info R:%x\n", info);
    //return 0;
    return info;
}
/**
  *对箱连接成功，同步一些必要的配置，如eq模式，音量级数等
  *如果不想同步，不注册该接口或者直接返回即可
  */
static void bt_tws_info_sync(void)
{
    tws_cmd_send(MSG_BT_TWS_EQ_SYNC, sound.eq_mode);
	if (get_tone_status())
	{
        tws_cmd_send(MSG_BT_TWS_VOL_SYNC, sound.tmp_sys_vol_l);
	}
	else
	{
        tws_cmd_send(MSG_BT_TWS_VOL_SYNC, sound.vol.sys_vol_l);
	}
}

extern void pbap_register_packet_handler(void (*handler)(u8 type, const u8 *name, const u8 *number, const u8 *date));
static void phonebook_packet_handler(u8 type, const u8 *name, const u8 *number, const u8 *date)
{
    static u16 number_cnt = 0;
    printf("NO.%d:", number_cnt);
    number_cnt++;
    printf("type:%d ", type);
    if (type == 0xff) {
        number_cnt = 0;
    }
    if (name) {
        printf(" NAME:%s  ", name);
    }
    if (number) {
        printf("number:%s  ", number);
    }
    if (date) {
        printf("date:%s ", date);
    }
    putchar('\n');
}

void bt_iq_trim_info(s16 i_dc, s16 q_dc)
{
    printf("---bt_fcc_srrc_info --- %d %d \n", i_dc, q_dc);
}

/*注册函数统一在下面这个函数调用 */
static void bredr_handle_register()
{
    set_bt_vm_interface(VM_REMOTE_DB, bt_vm_interface());
    infor_2_user_handle_register(btstack_status_update_deal, user_inf_buf);
    discon_complete_handle_register(bt_discon_complete_handle);/*断开或者连接上会调用的函数，给客户反馈信息*/
    spp_data_deal_handle_register(spp_data_deal);
#if BT_MUSIC_VOL_SYNC
    music_vol_change_handle_register(set_device_volume, get_dac_vol);
#endif // BT_MUSIC_VOL_SYNC
    bt_a2dp_audio_handle_register(a2dp_media_play, a2dp_media_stop, 2);
#if (USER_SUPPORT_PROFILE_HFP==1)
    bt_phone_audio_handle_register(hook_sco_conn, hook_sco_disconn, hook_sco_rx, hook_sco_tx);
#endif
    bt_fast_test_handle_register(bt_fast_test_api);
    bt_updata_handle_register(bt_updata_start_api, bt_updata_end_api);
    set_osc_internal_info_register(set_osc_internal_info);
    read_remote_name_handle_register(read_name_handle);
    bt_noconn_pwr_handle_register(bt_noconn_pwr_down_in, bt_noconn_pwr_down_out);
    get_battery_value_register(bt_get_battery_value);   /*电量显示获取电量的接口*/
#if BT_TWS
    bt_tws_register(bt_tws_info_save, bt_tws_info_sync, tws_deal_cmd, NULL);
    bt_tws_resgister_set_irq_prio(tws_inc_bt_irq_prio);
#endif
#if (SNIFF_MODE_CONF&SNIFF_EN)
    resgister_check_connect_enter_sniff_status(check_connect_enter_sniff_status);
#endif
#if (USER_SUPPORT_PROFILE_PBAP== 1)
    pbap_register_packet_handler(phonebook_packet_handler); //获取电话号码的回调函数注册
#endif
    iq_trim_info_handle_register(bt_iq_trim_info);
}

#define BT_CTRL_BUF_SIZE      (10*1024)
#if ((BT_MODE != NORMAL_MODE))
#define TX_MEM_SIZE       (4*1024)
#elif (BT_TWS != 0)
#define TX_MEM_SIZE       (4*1024 - 300)
#else
#define TX_MEM_SIZE       (2*1024)
#endif

static u8 bt_ctrl_buf[ BT_CTRL_BUF_SIZE] sec(.bt_lmp_mem) __attribute__((aligned(4)));
u8 tx_mem[TX_MEM_SIZE] sec(.bt_classic_data) __attribute__((aligned(4)));

void bt_mode_init()
{
    ASSERT(sizeof(bt_ctrl_buf) > controller_need_buf());
    controller_buf_init(bt_ctrl_buf, tx_mem, sizeof(tx_mem));

    classic_controller_init(BT_MODE);

    bredr_stack_mem_init();

    bt_config_default_init();

    bt_function_select_init();

#if (BLE_BREDR_MODE&BT_BREDR_EN)
    register_edr_init_handle(1);
    bredr_handle_register();
#endif   //SUPPORT BREDR
#if BT_TWS
    user_tws_init();
#endif

    extern void bd_hci_firmware_init(void *var);
    bd_hci_firmware_init(NULL);

#if (BLE_BREDR_MODE&BT_BLE_EN)
    extern void ble_stack_config_init(void);
    ble_btstack_init_cbk_handler_register(ble_before_init_set, ble_stack_config_init);
    ble_config_select_init();
#if (BT_MODE == NORMAL_MODE)
    register_ble_init_handle(1);
#else
    //BQB_MODE FCC_MODE
    register_ble_init_handle(2);
#endif

#endif

    bt_stack_init(NULL);
    bt_osc_init();


}

//demo
u8 hci_reset_cmd[] = {0x3, 0xc, 0x0};
void demo_packet_handler(u8 packet_type, u8 *packet, u16 size)
{
    switch (packet_type) {
    case 0x04:
        puts("HCI_EVENT_PACKET\n");
        break;
    case 0x02:
        break;
    default:
        break;
    }

}
void controller_mode_init()
{
    {
        ASSERT(sizeof(bt_ctrl_buf) > controller_need_buf());
        controller_buf_init(bt_ctrl_buf, tx_mem, sizeof(tx_mem));

        classic_controller_init(BT_MODE);

        bt_config_default_init();
    }

    bt_function_select_init();

#if (BLE_BREDR_MODE&BT_BREDR_EN)
    register_edr_init_handle(0);
    bredr_handle_register();
#endif   //SUPPORT BREDR

    /* extern void bd_hci_firmware_init(void *var); */
    /* bd_hci_firmware_init(NULL); */

    bt_stack_init(NULL);

    controller_mode_packet_handler(demo_packet_handler);
    controller_mode_send_packet(0x01, hci_reset_cmd, sizeof(hci_reset_cmd));

}

