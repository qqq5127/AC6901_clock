#include "audio/dac.h"
#include "audio/tone.h"
#include "sdk_cfg.h"
#include "cpu/audio_param.h"
#include "audio/dac_api.h"
#include "audio.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".tone_app_bss")
#pragma data_seg(	".tone_app_data")
#pragma const_seg(	".tone_app_const")
#pragma code_seg(	".tone_app_code")
#endif

#if TONE_EN
static u8 temp_automute_ctl = 0;
#define sin_tone_44K_TAB_MAX	  44
const s16 sin_tone_44100Hz_tab[sin_tone_44K_TAB_MAX] = {
    0,
    4663,
    9231,
    13611,
    17715,
    21457,
    24763,
    27565,
    29805,
    31439,
    32433,
    32766,
    32433,
    31439,
    29805,
    27565,
    24763,
    21457,
    17715,
    13611,
    9231,
    4663,
    0,
    -4663,
    -9231,
    -13611,
    -17715,
    -21457,
    -24763,
    -27565,
    -29805,
    -31439,
    -32433,
    -32766,
    -32433,
    -31439,
    -29805,
    -27565,
    -24763,
    -21457,
    -17715,
    -13611,
    -9231,
    -4663,
};

#define sin_tone_48K_TAB_MAX	  48
const s16 sin_tone_48000Hz_tab[sin_tone_48K_TAB_MAX] = {
    0,
    4276,
    8480,
    12539,
    16383,
    19947,
    23169,
    25995,
    28377,
    30272,
    31650,
    32486,
    32766,
    32486,
    31650,
    30272,
    28377,
    25995,
    23169,
    19947,
    16383,
    12539,
    8480,
    4276,
    0,
    -4276,
    -8480,
    -12539,
    -16383,
    -19947,
    -23169,
    -25995,
    -28377,
    -30272,
    -31650,
    -32486,
    -32766,
    -32486,
    -31650,
    -30272,
    -28377,
    -25995,
    -23169,
    -19947,
    -16383,
    -12539,
    -8480,
    -4276,
};

#define sin_tone_32K_TAB_MAX	  32
const s16 sin_tone_32000Hz_tab[sin_tone_32K_TAB_MAX] = {
    0,
    6392,
    12539,
    18204,
    23169,
    27244,
    30272,
    32137,
    32766,
    32137,
    30272,
    27244,
    23169,
    18204,
    12539,
    6392,
    0,
    -6392,
    -12539,
    -18204,
    -23169,
    -27244,
    -30272,
    -32137,
    -32766,
    -32137,
    -30272,
    -27244,
    -23169,
    -18204,
    -12539,
    -6392,
};

SIN_TONE_VAR sin_tone_var;

void sin_tone_init(void)
{
    memset(&sin_tone_var, 0, sizeof(SIN_TONE_VAR));
    sin_tone_var.vol = 1500;	/*default tone vol*/
    sin_tone_toggle(TONE_EN);
}

void sin_tone_toggle(u16 en)
{
    sin_tone_var.toggle = en;	/*default tone vol*/
}

u16 is_sin_tone_busy(void)
{
    return sin_tone_var.tab_cnt;
}

void sin_tone_sr_set(u16 sr)
{
    sin_tone_var.dac_sr = sr;
}

void sin_tone_play(u16 cnt)
{
    u8 step = 0;

    if (0 == sin_tone_var.toggle) {
        return;
    }


    if (sin_tone_var.tab_cnt) {
        return;
    }
    dac_toggle(1);
    temp_automute_ctl = sound.automute_ctl;
    sound_automute_set(AUTO_MUTE_ENABLE, -1, -1, -1);

    switch (sin_tone_var.dac_sr) {
    case 12000://step = 4
        step += 2;
    case 24000://step = 2
        step++;
    case 48000://step = 1
        step++;
        sin_tone_var.sr_tab = sin_tone_48000Hz_tab;
        sin_tone_var.sr_tab_size = sin_tone_48K_TAB_MAX;
        break ;

    case 11025:
        step += 2;
    case 22050:
        step++;
    case 44100:
        step++;
        sin_tone_var.sr_tab = sin_tone_44100Hz_tab;
        sin_tone_var.sr_tab_size = sin_tone_44K_TAB_MAX;
        break ;

    case 8000:
        step += 2;
    case 16000:
        step++;
    case 32000:
        step++;
        sin_tone_var.sr_tab = sin_tone_32000Hz_tab;
        sin_tone_var.sr_tab_size = sin_tone_32K_TAB_MAX;
        break ;

    default:
        log_printf("tone_dac_sr:%d\n", sin_tone_var.dac_sr);
        return ;
    }

    sin_tone_var.step = step;
    sin_tone_var.tab_cnt = cnt;
    sin_tone_var.point_cnt = 0;
}

/*
***********************************************************************************
*							ADD TONE TO DAC
*
*Description: This function called to add tone to dac buf
*
*Argument(s): buff	dac raw data
*			  len	dac raw data len
*
*Returns	: none
*
*Note(s)	:
***********************************************************************************
*/
void sin_tone_2_dac(s16 *buff, u32 len)
{
    if (sin_tone_var.tab_cnt) {
        u32 i = 0;
        while (i < len) {
            s32 tmp32;
            s16 tmp16;

            tmp16  = buff[i];
            tmp32  = (sin_tone_var.sr_tab[sin_tone_var.point_cnt] * sin_tone_var.vol) >> 14;/*16384*/
            tmp32 += tmp16;

            if (tmp32 < -32768) {
                tmp32 = -32768;
            } else if (tmp32 > 32767) {
                tmp32 = 32767;
            }
            buff[i++] = tmp32;

            sin_tone_var.dac_lr++;
            if (sin_tone_var.dac_lr > 1) {
                sin_tone_var.dac_lr = 0;
                sin_tone_var.point_cnt += sin_tone_var.step;
                if (sin_tone_var.point_cnt >= sin_tone_var.sr_tab_size) {
                    sin_tone_var.tab_cnt--;
                    sin_tone_var.point_cnt = sin_tone_var.point_cnt % sin_tone_var.sr_tab_size;
                    if (0 == sin_tone_var.tab_cnt) {
                        break;
                    }
                }
            }
        }/*end of while*/

        if (sin_tone_var.tab_cnt == 0) {
            puts("tone_play_end\n");
            sound_automute_set(temp_automute_ctl, -1, -1, -1);
        }
    }
}

#else

void sin_tone_init(void) {}
void sin_tone_toggle(u16 en) {}
void sin_tone_sr_set(u16 sr) {}
void sin_tone_play(u16 cnt) {}
void sin_tone_2_dac(s16 *buff, u32 len) {}
u16 is_sin_tone_busy(void)
{
    return 0;
}

#endif

