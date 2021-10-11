#include "aec_user.h"
#include "aec/aec.h"
#include "uart.h"
#include "string.h"
#include "crc_api.h"
#include "sdk_cfg.h"
#include "audio/tone.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".aec_app_bss")
#pragma data_seg(	".aec_app_data")
#pragma const_seg(	".aec_app_const")
#pragma code_seg(	".aec_app_code")
#endif

#define AEC_EQ_NULL			0
#define AEC_EQ_NORMAL		1
#define AEC_EQ_BASS			2
#define AEC_EQ_TRE			3
#define AEC_EQ_TRE_PLUS		4
#define AEC_EQ				AEC_EQ_NULL

#if (AEC_EQ != AEC_EQ_NULL)
static const int aec_eq_tab[65] = {
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,

#if (AEC_EQ == AEC_EQ_NORMAL)
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
#elif (AEC_EQ == AEC_EQ_BASS)
    32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3, 32768 * 2 / 3,
#elif (AEC_EQ == AEC_EQ_TRE)
    32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2, 32768 * 2,
#elif (AEC_EQ == AEC_EQ_TRE_PLUS)
    32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3, 32768 * 3,
#endif

    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768
};
#endif

/*
**************************************************************************
*parNum 0:fUseAagc
*       1:DT_MIC_GAIN
*       2:MAX_NDT_MIC_GAIN
*       3:MIN_NDT_MIC_GAIN
*       4:DT_fadeout_speed
*       5:Aagc_Target
*       6:Aagc_TimeConstant
*       7:DT_EngThr
*       8:NLPClearThr
*       9:SpeechSuppress
*       10:SpeechProb_Thr(comfortable_noise param)
**************************************************************************
*/
static int parNum_advance[] = {0,	1, 	2, 	3, 	4, 	5, 		6			};
static int value_advance[]  = {1, 	0, 	60,	10,	2, 	28672, 	13110 * 4	};
void aec_post_init()
{
    set_advanced_params(parNum_advance, sizeof(parNum_advance) / 4, value_advance);
}
/*
***********************************************************************************
*						AEC PARAM CONFIG
*
*Description: aec_param_init
*
*Argument(s):
*
*Returns	:
*
*Note(s)	:1)div64 means:real_val = config_val/64
			 2)the value inside the () means default value,e.g.,(512)
			 3)mic filter:bit(15)
			 4)suppress:bit(1)
***********************************************************************************
*/
#define AEC_REDUCE		BIT(0)
#define AEC_ADVANCE		BIT(14)
AEC_ARGV aec_param = {
    //////////////////////////////////////////////////////////////////
    .dac_analog_gain			= 27,	/*range:0~30 (23)			*/
    .mic_analog_gain			= 15,	/*range:0~63 (20)			*/
    .NDT_max_gain				= 448,	/*range:64-4096,div64 (512)	*/
    .NDT_min_gain				= 64, 	/*range:64-4096,div64 (64)	*/
    .NDT_Fade_Speed				= 25, 	/*range:1-200 (20)			*/
    .suppress_rough				= 2,	/*range:0-4 (2)				*/
    .suppress_fine				= 1000,	/*range:0-8000 (1000)		*/
    .NearEnd_Begin_Threshold	= 30,	/*range:0-200 (30)			*/
    .aec_ctl					= AEC_ADVANCE,/* or AEC_REDUCE		*/
    //////////////////////////////////////////////////////////////////

    /**
      * 通常情况下，只修改以上部分参数，
      * 下面的参数保持不动，除非你看得懂
      */
    .FarEnd_Talk_Threshold		= 1000,
    .speak_detect_thr			= 50,
    .speak_detect_gain			= 64,
    .eq_gain					= 256,/*val>>8*/
#if (AEC_EQ == AEC_EQ_NULL)
    .eq_tab						= NULL,
#else
    .eq_tab						= aec_eq_tab,
#endif
    .adc_pre_delay				= 0,
    .dac_pre_delay				= 0,
    .e_slow						= 0,
    .x_slow						= 0,
    .frame_cnt					= 0,
    .em_level					= 14746,/*val>>14*/
};
u32 aec_param_init(void)
{
#if AEC_DEBUG_ONLINE
    puts("\n==========AEC PARAM=============\n");
    log_printf("dac:%d\nmic:%d\n", aec_param.dac_analog_gain, aec_param.mic_analog_gain);
    log_printf("NDT_max_gain:%d\nNDT_min_gain:%d\n", aec_param.NDT_max_gain, aec_param.NDT_min_gain);
    log_printf("NDT_Fade_Speed:%d\nNearEnd_Threshold:%d\n", aec_param.NDT_Fade_Speed, aec_param.NearEnd_Begin_Threshold);
    log_printf("suppress_rough:%d\nsuppress_fine:%d\n", aec_param.suppress_rough, aec_param.suppress_fine);
    if (aec_param.aec_ctl == AEC_REDUCE) {
        puts("aec_ctl:AEC_REDUCE\n");
    } else {
        puts("aec_ctl:AEC_ADVANCE\n");
    }
    puts("==============END=================\n");
#endif
    return AEC_ERR_NONE;
}

/*
***********************************************************************************
*						AEC PARAM CONFIG ONLINE
*
*Description: aec_config_online
*
*Argument(s):
*
*Returns	:
*
*Note(s)	:
***********************************************************************************
*/
#if AEC_DEBUG_ONLINE

enum {
    AEC_MIC_AGAIN	= 0u,
    AEC_DAC_AGAIN		,
    AEC_NDT_MAX_GAIN	,
    AEC_NDT_MIN_GAIN	,
    AEC_NDT_FADE_SPEED	,
    AEC_SUPPRESS_ROUGH	,
    AEC_SUPPRESS_FINE	,
    AEC_NEAREND_THR		,
    AEC_MODE			,/*0:disable,1:REDUCE,2:ADVANCE*/
};

struct _AEC_ONLINE {
    char tag[4];/*AEC:0x41 0x45 0x43 0x00	*/
    u16 crc;	/*crc = crc16(len+cmd+data)	*/
    u16 len;	/*data length				*/
    u16 cmd;	/*cmd index					*/
    s32 data;	/*cmd value					*/
} __attribute__((packed));
typedef struct _AEC_ONLINE AEC_ONLINE;
AEC_ONLINE aec_online;

void aec_param_deal(u16 cmd, s32 data)
{
    log_printf(">>>>aec cmd:%d \t data:%d\n", cmd, data);
    switch (cmd) {
    case AEC_DAC_AGAIN:
        aec_param.dac_analog_gain = (u16)data;
        break;
    case AEC_MIC_AGAIN:
        aec_param.mic_analog_gain = (u16)data;
        break;
    case AEC_NDT_MAX_GAIN:
        aec_param.NDT_max_gain = (u16)data;
        break;
    case AEC_NDT_MIN_GAIN:
        aec_param.NDT_min_gain = (u16)data;
        break;
    case AEC_NDT_FADE_SPEED:
        aec_param.NDT_Fade_Speed = (u16)data;
        break;
    case AEC_NEAREND_THR:
        aec_param.NearEnd_Begin_Threshold = (u16)data;
        break;
    case AEC_SUPPRESS_ROUGH:
        aec_param.suppress_rough = (u16)data;
        break;
    case AEC_SUPPRESS_FINE:
        aec_param.suppress_fine = (u16)data;
        break;
    case AEC_MODE:	/*0:disable,1:REDUCE,2:ADVANCE*/
        if (data == 0) {
            aec_interface.toggle = 0;
            aec_param.aec_ctl = 0;
        } else if (data == 1) {
            aec_param.aec_ctl = AEC_REDUCE;
        } else if (data == 2) {
            aec_param.aec_ctl = AEC_ADVANCE;
        }
        break;
    default:
        puts("unknown AEC cmd\n");
        break;
    }
}

s8 aec_config_online(void *buf, u16 size)
{
    //log_printf("AEC_rx:%d\n",size);
    //put_buf(buf,size);
    memcpy(&aec_online, buf, sizeof(AEC_ONLINE));
    if (strcmp(aec_online.tag, "AEC") == 0) {
        if (aec_online.crc == crc16(&aec_online.len, aec_online.len + sizeof(aec_online.len)))	{
            puts("aec_online OK\n");
            aec_param_deal(aec_online.cmd, aec_online.data);
            sin_tone_play(500);
            return 0;
        } else {
            puts("aec_online ERROR\n");
            put_buf((u8 *)&aec_online, sizeof(AEC_ONLINE));
            return -1;
        }
    } else {
        puts("unknown data\n");
        return -1;
    }
}
#endif
