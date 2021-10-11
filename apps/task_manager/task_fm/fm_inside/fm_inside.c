/*--------------------------------------------------------------------------*/
/**@file     fm_inside.c
   @brief    692x 内部收音底层驱动
   @details
   @author wangjunqian
   @date   2018-3-18
   @note
*/
/*----------------------------------------------------------------------------*/
#include "sdk_cfg.h"
#include "clock_api.h"
#include "common.h"
#include "task_fm/fm_api.h"
#include "fm_inside.h"
#include "task_fm/task_fm.h"
#include "fm_inside/fm_inside.h"
#include "audio/src.h"
#include "audio/src_api.h"
#include "audio/dac_api.h"
#include "audio/dac.h"
#include "iis/dac2iis.h"
#include "fm/fm_inside_api.h"
#include "rec_api.h"
#if	FM_INSIDE
struct FM_INSIDE_DAT fm_inside_dat  sec_used(.fm_mem) __attribute__((aligned(4)));

extern void cfg_bt_pll_para(u32 osc, u32 sys, u8 low_power, u8 xosc);

static void fm_src_input(void *buf, u32 len)
{
    src_run(buf, len);
}

static void fm_src_output_cb(u8 *buf, u16 len, u8 flag)
{
    u32 wlen;
	u16 i;
	s16 *fm_data = (s16*)buf;
	s32 valuetemp;
    if (flag & BIT(0)) {
        if (fm_inside_dat.stream_io) {
			for (i = 0; i < len/2; i++) {
				valuetemp = fm_data[i];
				valuetemp = (valuetemp * 5616) >> 14 ;
				fm_data[i] = (s16)valuetemp;
			}
            fm_inside_dat.stream_io->output(fm_inside_dat.stream_io->priv, buf, len);
        }
#if !REC_SOURCE
        rec_dac_data_input((s16 *)buf, len, 1);
#endif
    }

    if (flag == BIT(0)) {
        src_kick_start(1);/*idat remaind,need not read data */
    } else {
        src_kick_start(0);/*idat done,need read data again*/
    }
}

void fm_src_enable(bool flag)
{
    s32 err = 0;
    u32 size;
    src_param_t src_p;
    if (fm_inside_dat.src_toggle == flag) {
        fm_puts("fm src_toggle same\n");
        return;
    }
    fm_inside_dat.src_toggle = flag;

    if (flag) {
        fm_puts("fm_src_start\n");
        src_p.in_chinc  = 1;
        src_p.in_spinc  = 2;
        src_p.out_chinc = 1;
        src_p.out_spinc = 2;
        src_p.in_rate  = 48000000 / FM_DAC_OUT_SAMPLERATE;   //输入输出均以48M为基准，防止产生误差
        src_p.out_rate = 48000000 / 37500;
        src_p.nchannel = 2;
        src_p.isr_cb = (void *)fm_src_output_cb;

        AUDIO_STREAM_PARAM stream_param;
        stream_param.ef = AUDIO_EFFECT_NULL;
        stream_param.ch = 2;
        stream_param.sr = FM_DAC_OUT_SAMPLERATE;

        fm_inside_dat.stream_io = audio_stream_init(&stream_param, NULL);
        if (fm_inside_dat.stream_io) {
            fm_inside_dat.stream_io->set_sr(fm_inside_dat.stream_io->priv, stream_param.sr, 1);
        } else {
            fm_puts("fm_stream fail\n");
        }
        src_enable(&src_p);

    } else {
        src_disable();
        fm_puts("fm_src_stop\n");
    }
}
///--------------------------FM_INSIDE_API------------------------
void fm_inside_init(void)
{
    bool mute_flag;
    memset(&fm_inside_dat, 0x00, sizeof(struct FM_INSIDE_DAT));
    mute_flag = is_dac_mute();
    if (mute_flag == 0) {
        fm_puts("notmute\n");
        dac_mute(1, 1);
    }
    fm_puts("fm_insice_init\n");
    set_fm_pcm_out_fun(fm_src_input);
    fm_src_enable(1);
    dac_channel_on(FM_INSI_CHANNEL, FADE_ON);
    fm_inside_default_config();
    fm_inside_io_ctrl(SET_FM_INSIDE_SYS_CLK, SYS_Hz / 1000000L);
    cfg_bt_pll_para(OSC_Hz, SYS_Hz, BT_ANALOG_CFG, BT_XOSC);  //fm/bt analog reuse,// rf.delay_sys need

    fm_inside_io_ctrl(SET_FM_INSIDE_SCAN_ARG1, FMSCAN_SEEK_CNT_MIN, FMSCAN_SEEK_CNT_MAX, FMSCAN_CNR);

#ifdef __DEBUG
    fm_inside_io_ctrl(SET_FM_INSIDE_PRINTF, 1);   //搜台时,库中是否打印相关信息.
#endif

    fm_inside_on();  //fm analog init

//    fm_inside_set_mono();  //inside_fm, default is stereo
    fm_inside_set_stereo(0);  //set[0,127], 0 mono, 127 stereo.
//    fm_inside_set_abw(127);  //audio bandwidth set //set[0,127] <=>2k~16k
//    fm_inside_deempasis_set(1);// set[0/1]

    dac_mute(mute_flag, 1); //recover mute status
}


bool fm_inside_set_fre(u16 fre)
{
    u8 ret;
    //fm_printf("[%d]", fre);
    u32 freq = fre * FM_STEP;
    fm_inside_int_set(false);
    //ret = fm_set_freq(freq, 3);
    ret =  fm_inside_freq_set(freq);
    //fm_inside_int_set(true);
    return ret;
}

bool fm_inside_read_id(void)
{
    return (bool)fm_inside_id_read();
}

void fm_inside_powerdown(void)
{
    fm_inside_off();
    fm_src_enable(0);
    dac_channel_off(FM_INSI_CHANNEL, FADE_ON);
}

void fm_inside_mutex_init(void)
{
    fm_inside_int_set(false);
    fm_src_enable(1);
    fm_inside_int_set(true);
}

void fm_inside_mutex_stop(void)
{
    fm_inside_int_set(false);
    fm_src_enable(0);
}

void fm_inside_mute(u8 flag)
{
    fm_inside_int_set(!flag);
    fm_mode_var->fm_mute = flag;
}

#endif // FM_INSIDE
