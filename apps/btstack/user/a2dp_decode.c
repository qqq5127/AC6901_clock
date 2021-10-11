#include "a2dp_decode.h"
#include "music_decoder.h"
#include "string.h"
#include "uart.h"
#include "audio/dac_api.h"
#include "audio/audio.h"
#include "sdk_cfg.h"
#include "common/msg.h"


/* #define MUSIC_DECODER_API_DEBUG_ENABLE */
#ifdef MUSIC_DECODER_API_DEBUG_ENABLE
#define mapi_printf log_printf
#else
#define mapi_printf(...)
#endif//MUSIC_DECODER_API_DEBUG_ENABLE



#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".a2dp_decode_bss")
#pragma data_seg(	".a2dp_decode_data")
#pragma const_seg(	".a2dp_decode_const")
#pragma code_seg(	".a2dp_decode_code")
#endif

u8 sbc_decode_ram[5 * 1024] 			sec_used(.bt_sbc_dec);

static const DEC_CFG sbc_decoder_configs[] = {
    ///sbc
    {
        get_sbcd_ops,
        NULL,
        sbc_decode_ram,
        sizeof(sbc_decode_ram),
        0,
        0,
    },
};
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
static const DEC_CFG udsbc_decoder_configs[] = {
    ///sbc
    {
        get_udsbc_dec_ops,
        NULL,
        sbc_decode_ram,
        sizeof(sbc_decode_ram),
        0,
        0,
    },
};
#endif
#if 0
static AAC_DEFAULT	aac_priv_setting 	sec_used(.bt_sbc_dec);
static const DEC_CFG aac_decoder_configs[] = {
    ///aac
    {
        get_aac_ops,
        (void *) &aac_priv_setting,
        sbc_decode_ram,
        sizeof(sbc_decode_ram),
        0,
        0,
    },
};
#endif
/*使用者自行实现， 主要是作为解码器的输入*/
AUDIO_FILE a2dp_media_file_io = {
    .seek = NULL,
    .read = NULL,
    .get_size = NULL,
};
static u8 g_bt_music_sync_vol = 30;
static u8 temp_sys_vol = 0;    //用于保存系统的音量，用于恢复
extern bool is_tws_device_slave();
u8 bt_music_sync_vol(u8 vol, u8 cmd)
{
    if (is_tws_device_slave() == TRUE) {
        return  g_bt_music_sync_vol;
    }
    printf(" bt_music_sync_vol %d-%d=%d\n", g_bt_music_sync_vol, vol, cmd);
    if (cmd == 1) {
        return g_bt_music_sync_vol;
    }
    g_bt_music_sync_vol = vol;
    return g_bt_music_sync_vol;
}
void *a2dp_media_play(void *priv, const char *format,
                      u16(*read_handle)(void *priv, u8 *buf, u16 len),
                      bool (*seek_handle)(void *priv, u8 type, u32 offsiz),
                      void *aac_setting,
                      void *inbuf)
{
    int err;
    MUSIC_DECODER *obj = NULL;
    obj = music_decoder_creat();
    if (obj == NULL) {
        mapi_printf("music player creat fail ！\n");
        return NULL;
    }
    a2dp_media_file_io.read = (void *)read_handle;
    a2dp_media_file_io.seek = (void *)seek_handle;

    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_A2DP;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    music_decoder_set_output(obj, audio_stream_init(&stream_param, inbuf));
    music_decoder_set_file_interface(obj, (AUDIO_FILE *)&a2dp_media_file_io, priv/*使用者主注册的file_io的私有属性*/);
    music_decoder_set_err_deal_interface(obj, NULL, NULL);

    if (0 == strcmp(format, "SBC")) {
        mapi_printf("is sbc, fun = %s, line = %d\n",  __func__, __LINE__);
        music_decoder_set_configs(obj, (DEC_CFG *)sbc_decoder_configs, 1);
    }
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
    else if (0 == strcmp(format, "UDSBC")) {
        mapi_printf("is udsbc, fun = %s, line = %d\n",  __func__, __LINE__);
        music_decoder_set_configs(obj, (DEC_CFG *)udsbc_decoder_configs, 1);
    }
#endif
#if 0
    else if (0 == strcmp(format, "AAC")) {
        mapi_printf("is aac, fun = %s, line = %d\n",  __func__, __LINE__);
        printf("if support aac please configs the flower !!\n");
        aac_priv_setting.samplerate = ((AAC_DEFAULT *)aac_setting)->samplerate ;
        aac_priv_setting.nchannels = ((AAC_DEFAULT *)aac_setting)->nchannels ;
        aac_priv_setting.bitrate = ((AAC_DEFAULT *)aac_setting)->bitrate ;
        music_decoder_set_configs(obj, (DEC_CFG *)aac_decoder_configs, 1);
    }
#endif
    else {
        mapi_printf("no support fomart !!\n");
        music_decoder_destroy(&obj);
        return NULL;
    }

    if (is_tws_device_slave() == FALSE) {
#if BT_MUSIC_VOL_SYNC
        if (g_bt_music_sync_vol <= 30) {
#ifndef HAVE_VOL_CONTROL_KEY
            temp_sys_vol = sound.vol.sys_vol_l;
#endif
            sound.vol.sys_vol_l = g_bt_music_sync_vol;
            sound.vol.sys_vol_r = sound.vol.sys_vol_l;
        } else {
#ifndef HAVE_VOL_CONTROL_KEY
            //要区分样机有没有音量按键控制按钮，没有音量按键的才需要加一下默认音量的赋值。
            if (SYS_DEFAULT_VOL > 0) {
                sound.vol.sys_vol_l = SYS_DEFAULT_VOL;
                sound.vol.sys_vol_r = SYS_DEFAULT_VOL;
            }
#endif
        }
#else
#ifndef HAVE_VOL_CONTROL_KEY
        //要区分样机有没有音量按键控制按钮，没有音量按键的才需要加一下默认音量的赋值。
        if (SYS_DEFAULT_VOL > 0) {
            sound.vol.sys_vol_l = SYS_DEFAULT_VOL;
            sound.vol.sys_vol_r = SYS_DEFAULT_VOL;
        }
#endif
#endif // BT_MUSIC_VOL_SYNC
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
        extern void tws_cmd_send(int msg, u8 value);
        tws_cmd_send(MSG_BT_TWS_VOL_SYNC, sound.vol.sys_vol_l);
#endif
    }
    printf("a2dp vol = %d, %d\n", sound.vol.sys_vol_l, sound.vol.sys_vol_r);
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    err = music_decoder_play(obj, NULL);
    if (err != MUSIC_DECODER_ERR_NONE) {
        music_decoder_destroy(&obj);
        mapi_printf("music_player_play dec fail %x !! fun = %s, line = %d\n", err, __func__, __LINE__);
        return NULL;
    }
    return (void *)obj;
}
void a2dp_media_stop(void **hdl)
{
    puts("a2dp_media_stop\n");
    music_decoder_destroy((MUSIC_DECODER **)&hdl);
    if (is_tws_device_slave() == FALSE) {
#if BT_MUSIC_VOL_SYNC
#ifndef HAVE_VOL_CONTROL_KEY
        if (g_bt_music_sync_vol <= 30) {
            sound.vol.sys_vol_l = temp_sys_vol;
            sound.vol.sys_vol_r = sound.vol.sys_vol_l;
            set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
        }
#endif
#endif // BT_MUSIC_VOL_SYNC
    }
    /* AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_A2DP;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    audio_stream_exit(&stream_param, NULL); */
}


