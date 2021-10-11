#include "sdk_cfg.h"
#include "common/common.h"
#include "power.h"
#include "board_init.h"
#include "music_decoder.h"
#include "string.h"
#include "msg.h"
#include "uart_param.h"
#include "uart.h"

#include "audio/dac_api.h"
#include "audio/dac.h"
#include "audio/audio.h"


#include "dec/if_decoder_ctrl.h"

#include "warning_tone.h"
#include "res_file.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif


typedef struct __TONE_INFO_T {
    u32 addr;
    u32 len;
} TONE_INFO_T;


typedef struct __SBC_NOTICE_CTRL {
    MUSIC_DECODER *dec;
    TONE_INFO_T info;
    u32 read_ptr;
    u8  index;
    u8  repeat_flag;
} SBC_NOTICE_CTRL;

static SBC_NOTICE_CTRL sbc_notice_hdl;
TONE_VAR_T tone_var;


extern decoder_ops_t *get_wt_ops(void);
extern u8 tone_display_flag;
extern u8 alarm_tone_flag;

decoder_ops_t nwt_ops;			//重写get_wt_ops
WARNING_PARM curr_tone_parm;	//记录当前播放的tone_parm
struct if_decoder_io *decoder_io_p;

const WARNING_PARM tone_disconn = {
    .slot_time = 40,
    .set_cnt = 2,
    .set_p[0] = {194648, 4924, 0, 32767,},
    .set_p[1] = {194648, 4924, 0, 16384,},
};

const WARNING_PARM tone_connect = {
    .slot_time = 0,
    .set_cnt = 1,
    .set_p[0] = {389297, 4797, 0, 16384,},
};

const WARNING_PARM tone_paring = {
    .slot_time = 0,
    .set_cnt = 1,
    .set_p[0] = {256582, 4789, 0, 16384,},
};

const WARNING_PARM tone_warning = {
    .slot_time = 50,
    .set_cnt = 2,
    .set_p[0] = {327235, 9988, 0, 16384,},
    .set_p[1] = {434215, 9961, 0, 16384,},
};

const WARNING_PARM tone_poweron = {
    .slot_time = 20,
    .set_cnt = 4,
    .set_p[0] = {256582, 7566, 0, 32767,},
    .set_p[1] = {205265, 7542, 0, 16384,},
    .set_p[2] = {154652, 7627, 0, 16384,},
    .set_p[3] = {102633, 7662, 0, 16384,},
};

const WARNING_PARM tone_poweroff = {
    .slot_time = 20,
    .set_cnt = 4,
    .set_p[0] = {102633, 7662, 0, 16384,},
    .set_p[1] = {154652, 7627, 0, 16384,},
    .set_p[2] = {205265, 7542, 0, 16384,},
    .set_p[3] = {256582, 7566, 0, 16384,},
};

const WARNING_PARM tone_ring = {
    .slot_time = 0,
    .set_cnt = 1,
    .set_p[0] = {230400, 69122, 1,  8533,},
};

const void *tones[] = {
    &tone_warning,
    &tone_connect,
    &tone_disconn,
    &tone_poweron,
    &tone_poweroff,
    &tone_paring,
    &tone_ring,
};


#define MS_2_BYTE		(48*2*2)

static void slot_delay(u32 time)
{
    u8 silent[MS_2_BYTE];

    memset(silent, 0x00, sizeof(silent));

    while (time--) {
        /* puts("*"); */
        decoder_io_p->output(decoder_io_p->priv, silent, sizeof(silent));
    }
}

static u32 nwt_run(void *work_buf, u32 type)
{
    u32 run_ret;
    u32 ret;

    decoder_ops_t *ops_hd = get_wt_ops();

    run_ret = ops_hd->run(work_buf, type);

    if (0 != run_ret) { //deocde_end
        if (curr_tone_parm.set_cnt) {
            curr_tone_parm.set_cnt--;
            if (curr_tone_parm.set_cnt) {
                slot_delay(curr_tone_parm.slot_time);
                ret = ops_hd->open(work_buf, decoder_io_p, NULL);
                if (ops_hd->dec_confing) {
                    ///choose output data control
                    ops_hd->dec_confing(work_buf, SET_SPECIFIC_PARAMETERS, (void *)&curr_tone_parm.set_p[curr_tone_parm.set_cnt - 1]);
                }
                return 0;
            } else {
                goto __EXIT;
            }
        }
        goto __EXIT;
    } else {
        /* puts("r"); */
    }

    return 0;

__EXIT:
    return run_ret;
}

static u32 nwt_open(void *work_buf, const struct if_decoder_io *decoder_io, u8 *bk_point_ptr)
{
    decoder_ops_t *ops_hd = get_wt_ops();
    u32 temp_res = 0 ;

    memcpy(&curr_tone_parm, (void *)tones[sbc_notice_hdl.index], sizeof(curr_tone_parm));

    decoder_io_p = (struct if_decoder_io *)decoder_io;

    temp_res = ops_hd->open(work_buf, decoder_io, NULL);
    if (ops_hd->dec_confing) {
        ///choose output data control
        ops_hd->dec_confing(work_buf, SET_SPECIFIC_PARAMETERS, (void *)&curr_tone_parm.set_p[curr_tone_parm.set_cnt - 1]);
    }
    return temp_res;
}

decoder_ops_t *get_nwt_ops(void)
{
    memcpy(&nwt_ops, get_wt_ops(), sizeof(nwt_ops));

    //rewrite run and open function
    nwt_ops.run = nwt_run;
    nwt_ops.open = nwt_open;

    return &nwt_ops;
}


extern u32 sbc_decode_ram[3 * 1024];
static const DEC_CFG notice_nwt_decoder_configs[] = {
    ///sbc
    {
        get_nwt_ops,
        NULL,
        sbc_decode_ram,
        sizeof(sbc_decode_ram),
        0,
        0,
    },
};

static void sbc_notice_decoder_err_deal(void *priv, u32 err)
{
    u32 msg;
    if (err && !sbc_notice_hdl.repeat_flag) {
        /* log_printf("sbc notice dec err = %x\n", err); */
        mutex_resource_release("tone");
    }
}



static u32 sbc_notice_file_read(void *priv, u8 *buf, u16 len)
{
    SBC_NOTICE_CTRL *hdl = (SBC_NOTICE_CTRL *)priv;
    if (hdl == NULL) {
        return (u32) - 1;
    }

    if (hdl->read_ptr >= hdl->info.len) {
        return (u32) 0;
    }

    memcpy(buf, (u8 *)(hdl->info.addr + hdl->read_ptr), len);
    hdl->read_ptr += len;

    /* log_printf_buf(buf, len); */

    return len;
}

static u32 sbc_notice_file_get_size(void *priv)
{
    SBC_NOTICE_CTRL *hdl = (SBC_NOTICE_CTRL *)priv;
    if (hdl == NULL) {
        return 0;
    }

    return hdl->info.len;
}

const AUDIO_FILE sbc_notice_file_io = {
    .seek = NULL,
    .read = sbc_notice_file_read,
    .get_size = sbc_notice_file_get_size,
};

static void notice_play(void *priv)
{

    u8 tone_index = (u8)priv;
    MUSIC_DECODER *obj = NULL;

    sbc_notice_hdl.index = tone_var.idx;
    sbc_notice_hdl.repeat_flag = tone_var.rpt_mode;
    sbc_notice_hdl.read_ptr = 0;

    tone_var.status = 1;
    obj = music_decoder_creat();
    if (obj == NULL) {
        return ;
    }

    sound_automute_set(0, -1, -1, -1); // 关自动mute
    dac_mute(0, 1);
    /* set_sys_vol(SYS_DEFAULT_VOL, SYS_DEFAULT_VOL, FADE_ON);//固定音量播提示音 */
    sbc_notice_hdl.dec = obj;

    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_NULL;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    music_decoder_set_output(obj, audio_stream_init(&stream_param, NULL));
    music_decoder_set_file_interface(obj, (AUDIO_FILE *)&sbc_notice_file_io, &sbc_notice_hdl/*使用者主注册的file_io的私有属性*/);
    music_decoder_set_err_deal_interface(obj, sbc_notice_decoder_err_deal, NULL);

    music_decoder_set_configs(obj, (DEC_CFG *)notice_nwt_decoder_configs, 1);

    tbool err = music_decoder_play(obj, NULL);
    if (err == MUSIC_DECODER_ERR_NONE) {
    } else {
    }
}

static void notice_stop(void *hdl)
{
    puts("notice_stop\n");
    music_decoder_destroy((MUSIC_DECODER **) & (sbc_notice_hdl.dec));
    //dac_cbuf_clear();
    sound_automute_set(AUTO_MUTE_CFG, -1, -1, -1); // 开自动mute
    tone_var.status = 0;
}


u8 get_tone_status(void)
{
    return tone_var.status;
}

#include "music_player.h"
void *get_number_tone(u32 num)
{
    switch (num) {
    case 0:
        return RES_0_MP3;
    case 1:
        return RES_1_MP3;
    case 2:
        return RES_2_MP3;
    case 3:
        return RES_3_MP3;
    case 4:
        return RES_4_MP3;
    case 5:
        return RES_5_MP3;
    case 6:
        return RES_6_MP3;
    case 7:
        return RES_7_MP3;
    case 8:
        return RES_8_MP3;
    case 9:
        return RES_9_MP3;
    }
    return NULL;
}

static void mp3_tone_mutex_play(void *priv)
{
    void *tone_name = NULL;
    //printf("idx:%d\n", tone_var.idx);
	tone_display_flag = 0;
    alarm_tone_flag = 0;
    switch (tone_var.idx) {
    case TONE_POWER_ON:
        tone_name = RES_START_MP3;
        break;
    case TONE_BT_MODE:
        tone_name = RES_BT_MP3;
        break;
    case TONE_POWER_OFF:
        tone_name = RES_POWER_OFF_MP3;
        break;
    case TONE_BT_CONN:
        tone_name = RES_CONNECT_MP3;
        break;
    case TONE_BT_CONN_LEFT:
        tone_name = RES_CONNECT_LEFT_MP3;
        break;
    case TONE_BT_CONN_RIGHT:
        tone_name = RES_CONNECT_RIGHT_MP3;
        break;
    case TONE_BT_DISCON:
        tone_name = RES_DISCONNECT_MP3;
        break;
    case TONE_BT_DISCON_TWS:
        tone_name = RES_DISCONNECT_TWS_MP3;
        break;
    case TONE_BT_PARING:
        break;
    case TONE_RING:
        tone_name = RES_RING_MP3;
        break;
    case TONE_WARNING:
        tone_name = RES_WARNING_MP3;
        break;
    case TONE_MUSIC_MODE:
        tone_name = RES_MUSIC_MP3;
        break;
    case TONE_SD:
        tone_name = RES_SD_MP3;
        break;
    case TONE_USB:
        tone_name = RES_USB_MP3;
        break;
    case TONE_RADIO_MODE:
        tone_name = RES_RADIO_MP3;
        break;
    case TONE_LINEIN_MODE:
        tone_name = RES_LINEIN_MP3;
        break;
    case TONE_REC_MODE:
        tone_name = RES_REC_MP3;
        break;
    case TONE_ECHO_MODE:
        tone_name = RES_ECHO_MP3;
        break;
    case TONE_PC_MODE:
        tone_name = RES_PC_MP3;
        break;
    case TONE_RTC_MODE:
        tone_name = RES_RTC_MP3;
        break;
    case TONE_LOW_POWER:
        tone_name = RES_LOW_POWER_MP3;
        break;
	case TONE_VOLMAXMIN:
		tone_display_flag = 1;
        tone_name = RES_VOLMAXMIN_MP3;
		break;
	case TONE_TWS:
        tone_name = RES_TWS_MP3;
		break;
	case TONE_ALARM_RING:
	    alarm_tone_flag = 1;
        tone_name = RES_ALARM_RING_MP3;
		break;
    case TONE_NUM_0:
    case TONE_NUM_1:
    case TONE_NUM_2:
    case TONE_NUM_3:
    case TONE_NUM_4:
    case TONE_NUM_5:
    case TONE_NUM_6:
    case TONE_NUM_7:
    case TONE_NUM_8:
    case TONE_NUM_9:
        tone_name = get_number_tone(tone_var.idx - TONE_NUM_0);
        break;
    }


    if (tone_name) {
        music_tone_play(tone_name);
    } else {
        puts("tone NULL,release mutex\n");
		tone_display_flag = 0;
	    alarm_tone_flag = 0;
        mutex_resource_release("tone");
    }
}

static void mp3_tone_mutex_end(void *priv)
{
    puts("mp3_tone_end\n");
    music_tone_end();
}
void tone_play(u8 index, u8 repeat_flag)
{
    u32 res;
#if (BT_MODE != NORMAL_MODE)
    return;
#endif
    if (get_going_to_pwr_off() && (index != TONE_POWER_OFF)) { //关机过程不播其它提示音
        return;
    }
    log_printf("tone_index:%d\n", index);
    tone_var.idx = index;
    tone_var.rpt_mode = repeat_flag;
    if (index < TONE_NWT_MAX) {
        res = mutex_resource_apply("tone", 5, notice_play, notice_stop, NULL);
    } else {
        res = mutex_resource_apply("tone", 5, mp3_tone_mutex_play, mp3_tone_mutex_end, NULL);
    }
    if (res == FALSE) {
        //puts("tone_play mutex apply error\n");
    }
}

