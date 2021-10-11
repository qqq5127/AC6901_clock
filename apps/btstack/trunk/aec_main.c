#include "aec_main.h"
#include "aec_user.h"
#include "audio/ladc_api.h"
#include "audio/ladc.h"
#include "audio/dac_api.h"
#include "audio/audio.h"
#include "audio/pdm_link.h"
#include "audio/dac.h"
#include "PLC_main.h"
#include "string.h"
#include "sdk_cfg.h"
#include "clock.h"
#include "bluetooth/avctp_user.h"
#include "msg.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".aec_app_bss")
#pragma data_seg(	".aec_app_data")
#pragma const_seg(	".aec_app_const")
#pragma code_seg(	".aec_app_code")
#endif

AEC_INTERFACE aec_interface = {
    .fill_dac_echo_buf 	= NULL,
    .fill_adc_ref_buf  	= NULL,
    .aec_set_mic_gain  	= ladc_mic_gain,
    .aec_run 			= NULL,
    .kick_start 		= 0,
#if (ECHO_EN==1)
    .toggle				= 0,
#else
    .toggle				= 1,
#endif
};

static void sco_2_output(s16 *data, int point);
/*
***********************************************************************************
*						AEC MEM MODULE
*
*Description:
*
*Argument(s):
*
*Returns	:
*
*Note(s)	:
***********************************************************************************
*/
static u8 AecBuf[21188] AT(.bt_aec) ALIGNED(4);/*aec(20704) sync_in(484)*/
static u8 *p_AecBuf = NULL;
static u8 *p_SyncInBuf = NULL;

#if ECHO_EN
#define AEC_EN_BITMAP	(AEC_BITMAP_SYNC_IN)
#else
#define AEC_EN_BITMAP	(AEC_BITMAP_BT_CALLING | AEC_BITMAP_SYNC_IN)
#endif

static void aec_buf_init()
{
    u32 aec_bufsize = 0;
    u32 sync_in_bufsize = 0;

    aec_bufsize  = aec_query_bufsize(AEC_EN_BITMAP);/*26748 bytes*/
    sync_in_bufsize = sync_in_querybuf();			/*484 bytes*/
    log_printf("aec_bufsize:%d\n", aec_bufsize);
    log_printf("sync_in_bufsize:%d\n", sync_in_bufsize);
    log_printf("AEC_total_buf:%d\n", aec_bufsize + sync_in_bufsize);
    p_AecBuf = (u8 *)AecBuf;
    p_SyncInBuf = (u8 *)p_AecBuf + aec_bufsize;
}

static u32 aec_buf_free()
{
    p_AecBuf = NULL;
    p_SyncInBuf = NULL;
    return AEC_ERR_NONE;
}
/*
***********************************************************************************
*						AEC TASK MODULE
*
*Description:
*
*Argument(s):
*
*Returns	:
*
*Note(s)	:
***********************************************************************************
*/
static int resume_max = 0;
static void aec_task_resume()
{
    aec_interface.kick_start++;
    if (aec_interface.kick_start > resume_max) {
        resume_max = aec_interface.kick_start;
    }
}
static void aec_task_run(void *priv)
{
    if (aec_interface.kick_start) {
        aec_interface.kick_start--;
        aec_run(priv);
    }
}

static void aec_task_start()
{
    resume_max = 0;
    aec_handle_register(aec_task_resume);
    aec_interface.kick_start = 0;
    aec_interface.aec_run = aec_task_run;
    aec_interface.fill_adc_ref_buf = fill_adc_ref_buf;
    aec_interface.fill_dac_echo_buf = fill_dac_echo_buf;
}

static void aec_task_stop()
{
    log_printf("resume_max:%d\n", resume_max);
    aec_interface.aec_run = NULL;
    aec_interface.fill_adc_ref_buf = NULL;
    aec_interface.fill_dac_echo_buf = NULL;
}

void aec_task_main(void)
{
    if (aec_interface.aec_run) {
        aec_interface.aec_run(NULL);
    }
}

/*
***********************************************************************************
*						AEC MODULE OPEN
*
*Description: This function is called when SCO/eSCO connect
*
*Argument(s): none
*
*Returns	: AEC_ERR_NONE	success
*			  others		faild
*
*Note(s)	:
***********************************************************************************
*/
static u32 aec_start_api()
{
    u32 err = AEC_ERR_NONE;

    //puts("aec_open_api\n");

    if (p_AecBuf) {
        return AEC_ERR_EXIST;
    }

    aec_buf_init();
    if (p_AecBuf == NULL) {
        puts("aec_err0\n");
        return AEC_ERR_MEMORY;
    }

    err = aec_param_init();
    if (err != AEC_ERR_NONE) {
        puts("aec_err1\n");
        return err;
    }

    err = aec_init(p_AecBuf, &aec_param);
    if (err != AEC_ERR_NONE) {
        puts("aec_err3\n");
        return err;
    }

    err = sync_in_init(p_SyncInBuf);
    if (err != AEC_ERR_NONE) {
        puts("aec_err4\n");
        return err;
    }

    aec_task_start();

    return err;
}

/*
***********************************************************************************
*						AEC MODULE CLOSE
*
*Description: This function is called when SCO/eSCO disconnect
*
*Argument(s): none
*
*Returns	: AEC_ERR_NONE/TH_ERR_NONE	success
*			  others					failed
*
*Note(s)	:
***********************************************************************************
*/
static u32 aec_stop_api()
{
    puts("aec_stop_api\n");
    aec_close();
    aec_task_stop();
    aec_buf_free();
    return 0;
}

/*
***********************************************************************************
*
*							SPD MODULE
*
***********************************************************************************
*/
static u8 SPD_Buf[6056] sec(.bt_aec) ALIGNED(4);
SPD_PARAM spd_param;
static void SPD_output(s16 *data, u16 npoint)
{
    sco_2_output(data, npoint);
}

static s32 SPD_init(void)
{
    u32 buf_size = SpeechDetectQuery();
    log_printf("SPD need buf:%d\n", buf_size);
    spd_param.spd_thr = aec_param.speak_detect_thr;
    spd_param.spd_gain = aec_param.speak_detect_gain;
    spd_param.output = SPD_output;
    SpeechDetectInit(SPD_Buf, &spd_param);
    return 0;
}

/*
***********************************************************************************
*						PHONE_AUDIO_INIT
*
*Description: This function is called when SCO/eSCO change state
*
*Argument(s): en = 1,phone audio enable
*			  en = 0,phone audio disable
*
*Returns	: none
*
*Note(s)	: 1)
***********************************************************************************
*/
static u8 tmp_sys_vol = 0xff;
AUDIO_STREAM *sco_stream_io = NULL;
static void phone_audio_init(u8 en)
{
#if (ECHO_EN == 1)
    u16 phone_sample_rate = SR16000;
    /* u16 phone_sample_rate = SR8000; */

    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_KTV;
    stream_param.ch = 2;
    stream_param.sr = SR8000;
    sco_stream_io = audio_stream_init(&stream_param, &phone_sample_rate);
    if (sco_stream_io) {
        sco_stream_io->clear(sco_stream_io->priv);
        sco_stream_io->set_sr(sco_stream_io->priv, phone_sample_rate, 1);
    }
#else
    u16 phone_sample_rate = SR8000;
    if (en) {
        if (en == 3) {
            phone_sample_rate = SR16000;
        }
        ladc_ch_open(LADC_MIC_CHANNEL, phone_sample_rate); /*open mic_adc*/
        ladc_mic_gain(aec_param.mic_analog_gain, 0);/*set mic gain*/

        AUDIO_STREAM_PARAM stream_param;
        stream_param.ef = AUDIO_EFFECT_SCO;
        stream_param.ch = 2;
        stream_param.sr = SR8000;
        sco_stream_io = audio_stream_init(&stream_param, NULL);
        if (sco_stream_io) {
            sco_stream_io->clear(sco_stream_io->priv);
            sco_stream_io->set_sr(sco_stream_io->priv, phone_sample_rate, 1);
        }
        sound_automute_set(0, -1, -1, -1); // 关自动mute
        /*save current system volume*/
        tmp_sys_vol = get_sys_vol(1);
        /*limit phone_vol max*/
        sound.vol.max_sys_vol_l = aec_param.dac_analog_gain;
        sound.vol.max_sys_vol_r = aec_param.dac_analog_gain;
        set_sys_max_vol(&sound.vol);
        /*set current phone_vol*/
#if (BT_PHONE_VOL_SYNC==0)
        sound.phone_vol = 15;
#endif
        sound.vol.sys_vol_l = (sound.phone_vol * aec_param.dac_analog_gain / 15) ;
        sound.vol.sys_vol_r = sound.vol.sys_vol_l;
        set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);

    } else {
        ladc_ch_close(LADC_MIC_CHANNEL); /*close mic_adc*/

        sound.vol.max_sys_vol_l = MAX_SYS_VOL_L;
        sound.vol.max_sys_vol_r = MAX_SYS_VOL_R;
        set_sys_max_vol(&sound.vol);
        sound.vol.sys_vol_l = tmp_sys_vol;
        sound.vol.sys_vol_r = sound.vol.sys_vol_l;
        set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
        //统一到a2dp_media_play处理
        /* #if BT_TWS */
        /* extern void tws_cmd_send(int msg, u8 value); */
        /* tws_cmd_send(MSG_BT_TWS_VOL_SYNC, sound.vol.sys_vol_l); */
        /* #endif */
        //要在设置完音量支持才开auto mute ，否则可能mute了之后设置音量无效
        task_post_event(NULL, 1, EVENT_AUTOMUTE_ON);
    }
#if USE_16_LEVEL_VOLUME
	u8 i;
	for (i=0; i<=MAX_SYS_VOL_TEMP; i++)
	{
		if (sound.vol.sys_vol_l <= volume_table[i])
		{
			volume_temp = i;
			break;
		}
	}
#endif
#endif
}

/*
***********************************************************************************
*						SCO	CALLBACK
*
*Description: This function is called when SCO/eSCO connect or disconnect
*
*Argument(s):
*
*Returns	:
*
*Note(s)	:
***********************************************************************************
*/
static u16 packets_dump;
static volatile u8 aec_sco_init = 0;
#define VCM_OUT_EN(x)       SFR(JL_AUDIO->DAA_CON1, 14,1, x)
static s32 aec_sco_conn(void *priv)
{
    audio_repair_init();
    aec_start_api();
    if (AEC_EN_BITMAP & AEC_BITMAP_SPEECH_DET) {
        SPD_init();
    }
    if (priv == NULL) {
        phone_audio_init(1);
    } else {
        phone_audio_init(3);
    }
    aec_buf_clear();
#if (VCOMO_EN && CALL_USE_DIFF_OUTPUT)
    VCM_OUT_EN(0);
#endif
    packets_dump = 0;
    aec_sco_init = 1;
    return 0;

}

static s32 aec_sco_disconn(void *priv)
{
    aec_sco_init = 0;
    aec_stop_api();
    audio_repair_exit();
    phone_audio_init(0);
    packets_dump = 0;
    if (sco_stream_io) {
        sco_stream_io->clear(sco_stream_io->priv);
    }
#if (VCOMO_EN && CALL_USE_DIFF_OUTPUT)
    VCM_OUT_EN(1);
#endif
    return 0;
}

s32 hook_sco_conn(void *priv)
{
#if BT_HFP_EN_SCO_DIS
    user_send_cmd_prepare(USER_CTRL_DISCONN_SCO, 0, NULL);
    return 0;
#endif
    puts("sco_conn_cb\n");
    set_sys_freq(BT_CALL_Hz);
    return aec_sco_conn(priv);
}

s32 hook_sco_disconn(void *priv)
{
#if BT_HFP_EN_SCO_DIS
    return 0;
#endif
    puts("sco_disconn_cb\n");
    set_sys_freq(SYS_Hz);
    return aec_sco_disconn(priv);
}

static void aec_sco_rx(s16 *data, u16 point, u8 sco_flags)
{
    if (!aec_sco_init) {
        //puts("\naec no initialized,return!!\n");
        return;
    }

    if (packets_dump < 60) {
        packets_dump++;
        memset(data, 0, point << 1);
        sco_flags = 0;
    }

    audio_repair_run(data, data, point, sco_flags);

    if (AEC_EN_BITMAP & AEC_BITMAP_SPEECH_DET) {
        SpeechDetectRun(data, point);
    } else {
        sco_2_output(data, point);
    }
}

void hook_sco_rx(s16 *data, u16 point, u8 sco_flags)
{
    if (data != NULL && point > 0) {
        aec_sco_rx(data, point, sco_flags);
    }
}

void hook_sco_tx(s16 *data, u16 point)
{
    get_bt_send_data(data, point);
}
#define rad (int)(0.9*32768)
#define den (int)(0.817*32768)
static void dc_notch(short *dat, int npoint, int *mem)
{
    int i;
    int vin, vout;
    int tmp32;
    for (i = 0; i < npoint; i++) {
        vin = (int) * dat << 15;
        vout = vin + mem[0];
        MUL(vout, rad);
        MLS(vin, 32768);
        MLA(mem[1], 16384);
        MRSIS(mem[0], 14);
        MUL(den, vout);
        MRSIS(tmp32, 15);
        mem[1] = vin - tmp32;
        vout >>= 15;
        if (vout > 32767) {
            vout = 32767;
        }
        if (vout < - 32768) {
            vout = -32768;
        }
        *dat++ = vout;
    }
}
int mem[2] = {0, 0};
short state = 0;
static void sco_2_output(s16 *buf, int point)
{
    s16 *data = (s16 *)buf;
    s16 temp_buf[64];
    s32 temp, i;

    dc_notch(data, point, mem);

    for (int i = 0; i < point; i++) {
        buf[i] = buf[i] - ((int)state * (int)(0.4 * 16384) >> 14);
        state = buf[i];
    }
    int eng;
    MUL((int)data[0], (int)data[0]);
    for (int i = 1; i < point; i++) {
        MLA((int)data[i], (int)data[i]);
    }
    asm volatile("macc /= %1;%0 = maccl":"=r"(eng):"r"(point));
    //printf("%d\n",eng);
    if (eng < 200) {
        memset((u8 *)buf, 0,  point * 2);
    }
    while (point) {
        temp = (point > 32) ? 32 : point;
        point = point - temp;
        for (i = 0; i < temp; i++) {
            temp_buf[2 * i] = *data;
            temp_buf[2 * i + 1] = *data++;
        }
        if (sco_stream_io) {
            sco_stream_io->output(sco_stream_io->priv, temp_buf, temp * 4);
        }
    }
}


