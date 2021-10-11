/*
*********************************************************************************************************
*                                             BR17
*
*                                             CODE
*
*                          (c) Copyright 2015-2016, ZHUHAI JIELI
*                                           All Rights Reserved
*
* File : *
* By   : jamin.li
* DATE : 11/11/2015 build this file
*********************************************************************************************************
*/
#include "sdk_cfg.h"
#include "circular_buf.h"
#include "audio/dac.h"
#include "audio/ladc.h"
#include "audio/dac_api.h"
#include "audio/linein_api.h"
#include "audio/alink.h"
#include "audio/audio.h"
#include "cpu/audio_param.h"
#include "aec/aec.h"
#include "audio/pdm_link.h"
#include <stdlib.h>
#include "sys_detect.h"
#include "task_manager.h"
#include "dev_pc.h"
#include "task_linein.h"
#include "rec_api.h"
#include "echo_api.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".ladc_app_bss")
#pragma data_seg(	".ladc_app_data")
#pragma const_seg(	".ladc_app_const")
#pragma code_seg(	".ladc_app_code")
#endif

AUDIO_STREAM *ladc_stream_io = NULL;
LADC_DRV *ladc_ops = NULL;
LADC_PARAM param;
static s16 ladc_buf[2 * LADC_MAX_CHANNEL][DAC_SAMPLE_POINT] __attribute__((aligned(4)));

static void sr_16k_2_8K(void *buf, u32 len)
{
    u16 i;
    s16 *res = buf;
    len >>= 2;//	x/2point 	x/2/2->16K28K

    for (i = 0; i < len; i++) {
        res[i] = res[i * 2];
    }
}
/*
 *********************************************************************************************************
 *
 * Description: calculate data energy
 * Arguments  : buffer = source_data
 *				len = data length(byte)
 *				packet_cnt = total calculate packets
 * Returns    : none
 *
 * Note:
 *********************************************************************************************************
 */
u32 data_energy_value(void *buffer, u16 len, u16 packet_cnt)
{
    static u32 total_res = 0;
    static u32 digital_energy = 0;
    static u16 cnt = 0;
    u32 res = 0;
    u32 i;

    s16 *buf = buffer;
    len >>= 1;//convert to point

    for (i = 0; i < len; i++) {
        res += abs(buf[i]);
    }
    total_res += res;

    cnt++;
    if (cnt >= packet_cnt) {
        cnt = 0;
        digital_energy = total_res;
        total_res = 0;
    }
    return digital_energy;
}

/*
 *********************************************************************************************************
 *
 * Description: digital_volume_ctl
 * Arguments  : buffer
 *				point_len
 *				ch = 1(mono),2(stereo)
 * Returns    : none
 *
 * Note:
 *********************************************************************************************************
 */
extern const u16 digital_vol_tab[MAX_DIGITAL_VOL_L + 1];
volatile u8 digital_vol = 31;
void digital_volume_ctl(void *buffer, u16 point_len, u8 ch)
{
    s32 valuetemp;
    u32 i;
    u16 curtabvol, curtabvor;

    s16 *buf = buffer;

    //get digital_vol
    curtabvol = digital_vol_tab[digital_vol];
    curtabvor = curtabvol;

    if (ch == 2) {
        //stereo
        for (i = 0; i < point_len; i += 2) {
            ///left channel
            valuetemp = buf[i];
            valuetemp = (valuetemp * curtabvol) >> 14 ;
            if (valuetemp < -32768) {
                valuetemp = -32768;
            } else if (valuetemp > 32767) {
                valuetemp = 32767;
            }
            buf[i] = (s16)valuetemp;

            ///right channel
            valuetemp = buf[i + 1];
            valuetemp = (valuetemp * curtabvor) >> 14 ;
            if (valuetemp < -32768) {
                valuetemp = -32768;
            } else if (valuetemp > 32767) {
                valuetemp = 32767;
            }
            buf[i + 1] = (s16)valuetemp;
        }
    } else {
        //mono
        for (i = 0; i < point_len; i++) {
            valuetemp = buf[i];
            valuetemp = (valuetemp * curtabvol) >> 14 ;
            if (valuetemp < -32768) {
                valuetemp = -32768;
            } else if (valuetemp > 32767) {
                valuetemp = 32767;
            }
            buf[i] = (s16)valuetemp;
        }
    }
}

/*
 *********************************************************************************************************
 *
 * Description: ladc to dac
 * Arguments  : buffer
 *				point_len
 * Returns    : none
 *
 * Note:
 *********************************************************************************************************
 */
#if 0
static const u8 sine_buf_32K[] = {

    0x00, 0x00, 0xae, 0x11, 0xad, 0x22, 0x58, 0x32,
    0x13, 0x40, 0x58, 0x4b, 0xb8, 0x53, 0xe0, 0x58,
    0x9e, 0x5a, 0xe0, 0x58, 0xb8, 0x53, 0x58, 0x4b,
    0x13, 0x40, 0x58, 0x32, 0xad, 0x22, 0xae, 0x11,
    0x00, 0x00, 0x52, 0xee, 0x53, 0xdd, 0xa8, 0xcd,
    0xed, 0xbf, 0xa8, 0xb4, 0x48, 0xac, 0x20, 0xa7,
    0x62, 0xa5, 0x20, 0xa7, 0x48, 0xac, 0xa8, 0xb4,
    0xed, 0xbf, 0xa8, 0xcd, 0x53, 0xdd, 0x52, 0xee
};
static const u8 sine_buf_8K[] = {

    0x00, 0xad, 0x13, 0xb8, 0x9e, 0xb8, 0x13, 0xad,
    0x00, 0x53, 0xed, 0x48, 0x62, 0x48, 0xed, 0x53,
    0x00, 0xad, 0x13, 0xb8, 0x9e, 0xb8, 0x13, 0xad,
    0x00, 0x53, 0xed, 0x48, 0x62, 0x48, 0xed, 0x53,
    0x00, 0xad, 0x13, 0xb8, 0x9e, 0xb8, 0x13, 0xad,
    0x00, 0x53, 0xed, 0x48, 0x62, 0x48, 0xed, 0x53,
    0x00, 0xad, 0x13, 0xb8, 0x9e, 0xb8, 0x13, 0xad,
    0x00, 0x53, 0xed, 0x48, 0x62, 0x48, 0xed, 0x53
};
#endif


void mono_to_stereo(s16 *mono_buf, u32 mono_len, s16 *stereo_buf)	//len unit: byte
{
    u32 cnt;
    for (cnt = 0; cnt < (mono_len / 2); cnt++) {
        stereo_buf[cnt * 2] = mono_buf[cnt];
        stereo_buf[cnt * 2 + 1] = mono_buf[cnt];
    }
}

void stereo_to_mono(s16 *stereo_buf, u32 stereo_len, s16 *mono_buf, u8 mode)	//len unit: byte
{
    u32 cnt;
    //mode : Lch Rch LRch
}

void ladc_to_dac(void *buf, u32 len)
{
    u8 cnt;
    s16 l2d_buf[len * 2];
    s16 *sp = buf;					/*src data	*/
    //sp = (void*)sine_buf_8K; 		/*debug		*/
    s16 *dp = (s16 *)l2d_buf;		/*dst data	*/

    mono_to_stereo(sp, len, dp);

    if (ladc_stream_io) {
        ladc_stream_io->output(ladc_stream_io->priv, dp, len << 1);
    }
}

void ladc_stereo_to_dac(void *buf_l, void *buf_r, u32 len)
{
    s16 l2d_buf[DAC_DUAL_BUF_LEN];
    s16 *sp_l = buf_l;
    s16 *sp_r = buf_r;
    s16 *dp = l2d_buf;
    u8 cnt;

    for (cnt = 0; cnt < (len / 2); cnt++) {
        dp[cnt * 2] = sp_l[cnt];
        dp[cnt * 2 + 1] = sp_r[cnt];
    }
    if (ladc_stream_io) {
        ladc_stream_io->output(ladc_stream_io->priv, dp, len << 1);
    }
}

/*
 *********************************************************************************************************
 *
 * Description: ladc_isr_callback
 * Arguments  : ladc_buf,ladc sample rate buf
 *				buf_flag,indicate the using buf
 *				buf_len,ladc buf length(default:DAC_SAMPLE_POINT * 2)
 * Returns    : none
 *
 * Note		  :
 *********************************************************************************************************
 */
void ladc_isr_callback(void *buf, u8 buf_flag, u16 buf_len)
{
    s16 *res, *ladc_mic;
    res = (s16 *)buf + DAC_SAMPLE_POINT * buf_flag;
    ladc_mic = res + 2 * DAC_DUAL_BUF_LEN;

    //calculate energy
    //data_energy_value(res,DAC_DUAL_BUF_LEN,1);
    //putchar('>');
#if BT_TWS_LINEIN
    extern s32 tws_linein_media_to_sbc_encoder(void *pbuf, u32 len, u8 ch);
    tws_linein_media_to_sbc_encoder(res, DAC_SAMPLE_POINT * 2, 1);
#endif
#if ECHO_EN
    //pitch_run....
    echo_input(res, DAC_SAMPLE_POINT * 2);
#endif

#if AUX_AD_ENABLE

    if (TASK_ID_LINEIN == task_get_cur()) {
        ladc_to_dac(res, DAC_SAMPLE_POINT * 2);
    }
#endif

    if (aec_interface.fill_adc_ref_buf) {
#if (ECHO_EN == 1)
        sr_16k_2_8K(res, DAC_DUAL_BUF_LEN);
        aec_interface.fill_adc_ref_buf(res, DAC_DUAL_BUF_LEN / 2);
#else
        aec_interface.fill_adc_ref_buf(res, DAC_DUAL_BUF_LEN);
#endif
    }
#if USB_PC_EN
    if (TASK_ID_PC == task_get_cur()) {
        usb_slave_mic_input(res, DAC_SAMPLE_POINT);
    }
#endif

#if MIC_REC_EN
    if (TASK_ID_REC == task_get_cur()) {
        rec_ladc_data_cb(res, buf_len);
    }
#endif
#if (!REC_SOURCE)&&AUX_REC_EN
    if (TASK_ID_LINEIN == task_get_cur()) {
        rec_ladc_data_cb(res, buf_len);
    }
#endif
}

//*********************************************************
//                        LADC API
//*********************************************************
void ladc_init(void)
{
    param.mic_isel = 1;
    param.adc_isel = 1;
    param.pga_isel = 1;
    param.dccs = 14;
    param.vcomo_en = VCOMO_EN;
    param.ADC_LEN = DAC_SAMPLE_POINT;
    param.ADC_ADR = (s16 *)ladc_buf;
    param.isr_cb = ladc_isr_callback;
    ladc_ops = &ladc_drv_ops;
    ladc_ops->init(&param);
}

void ladc_ch_open(u8 ch, u16 sr)
{
    if (ladc_ops) {
        if (ch == LADC_MIC_CHANNEL) {
            JL_PORTA->DIR |= BIT(0);
        }
        ladc_ops->ioctl(SET_ADC_OSC, OSC_Hz, NULL);
        ladc_ops->on(ch, sr);
        /*
         *ladc采样通道增益控制
         *范围：0x00~0x0F
         *默认为0x08
         */
        //ladc_ops->ioctl(SET_ADC_PGA,0x0F,NULL);/*0x00~0x0F*/
    }
#if AUX_AD_ENABLE
    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_NULL;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    ladc_stream_io = audio_stream_init(&stream_param, NULL);
    if (ladc_stream_io) {
        ladc_stream_io->set_sr(ladc_stream_io->priv, sr, 1);
    }
#endif
}

void ladc_ch_close(u8 ch)
{
    if (ladc_ops) {
        ladc_ops->off(ch);
    }
#if AUX_AD_ENABLE
    ladc_stream_io = NULL;
#endif
}

void ladc_mic_gain(u8 gain, u8 gx2)
{
    if (ladc_ops) {
        ladc_ops->ioctl(SET_MIC_GAIN, gain, NULL);
        ladc_ops->ioctl(SET_MIC_GX2, gx2, NULL);
    }
}

void ladc_set_samplerate(u16 sr)
{
    if (ladc_ops) {
        ladc_ops->ioctl(SET_ADC_SR, sr, NULL);
    }
}

u16 ladc_get_samplerate(void)
{
    if (ladc_ops) {
        return (u16)ladc_ops->ioctl(GET_ADC_SR, 0, NULL);
    } else {
        return 0;
    }
}

void ladc_mic_mute(u32 arg)
{
    if (ladc_ops) {
        ladc_ops->ioctl(SET_MIC_MUTE, arg, NULL);
    }
}

//*********************************************************
//                        DAA API
//*********************************************************
void microphone_open(u8 mic_gain, u8 mic_gx2)
{
    puts("microphone_open\n");
    if (ladc_ops) {
        dac_mute(1, 0);
        ladc_ops->ioctl(SET_ADC_UDE, 1, NULL);
        ladc_ops->ioctl(SET_MIC_EN, 1, NULL);
        ladc_mic_gain(mic_gain, mic_gx2);
        mic_2_dac(1, 1);
        ladc_ops->ioctl(SET_ADC_UDE, 0, NULL);
        dac_mute(0, 0);
    }
}

void microphone_close(void)
{
    puts("microphone_close\n");
    ladc_ops->ioctl(SET_MIC_EN, 0, NULL);
    mic_2_dac(0, 0);
}
//*********************************************************
//                        Emitter Aux API
//*********************************************************
void emitter_aux_open(void *priv)
{
    struct tws_linein_parm_t *tws_linein_parm = (struct tws_linein_parm_t *)priv;
    AUDIO_STREAM_PARAM stream_param;
    puts("emitter_aux_open\n");
    if (!tws_linein_parm) {
        return ;
    }
    printf("rate=0x%x\n", tws_linein_parm->rate);
    printf("adc_2_dac=0x%x\n", tws_linein_parm->adc_2_dac);
    if (LINEIN_CHANNEL == DAC_AMUX0) {
        JL_PORTB->DIR |= BIT(4) | BIT(5);
    } else if (LINEIN_CHANNEL == DAC_AMUX1) {
        JL_PORTA->DIR |= BIT(3) | BIT(4);
    } else if (LINEIN_CHANNEL == DAC_AMUX2) {
        JL_PORTB->DIR |= BIT(6) | BIT(3);
    }
    linein_channel_open(LINEIN_CHANNEL, 0);
    ladc_ch_open(LADC_LINLR_CHANNEL, tws_linein_parm->rate);
    if (tws_linein_parm->adc_2_dac) {
        stream_param.ef = AUDIO_EFFECT_NULL;
        stream_param.ch = 2;
        stream_param.sr = tws_linein_parm->rate;//SR44100;
        ladc_stream_io = audio_stream_init(&stream_param, NULL);
        if (ladc_stream_io) {
            ladc_stream_io->set_sr(ladc_stream_io->priv, stream_param.sr, 1);
        }
    }
}

void emitter_aux_close(void *priv)
{
    puts("emitter_aux_close \n");
    ladc_ch_close(LADC_LINLR_CHANNEL);
    ladc_stream_io = NULL;
}





