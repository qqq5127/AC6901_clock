/*******************************************************************************************
 File Name: dac.c

 Version: 1.00

 Discription:


 Author:yulin deng

 Email :flowingfeeze@163.com

 Date:2014-01-13 17:00:21

 Copyright:(c)JIELI  2011  @ , All Rights Reserved.
 *******************************************************************************************/
#include "sdk_cfg.h"
#include "circular_buf.h"
#include "common/app_cfg.h"
#include "common/common.h"
#include "cpu/audio_param.h"
#include "audio/dac_api.h"
#include "audio/dac.h"
#include "audio/eq.h"
#include "audio/digital_vol.h"
#include "audio/sin_tab.h"
#include "audio/audio.h"
#include "audio/tone.h"
#include "audio/src.h"
#include "audio/audio_stream.h"
#include "timer.h"
#include "aec/aec.h"
#include "music_decoder.h"
#include "warning_tone.h"
#include "flash_api.h"
#include "echo_api.h"
#include "rec_api.h"
#include "task_manager.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".dac_app_bss")
#pragma data_seg(	".dac_app_data")
#pragma const_seg(	".dac_app_const")
#pragma code_seg(	".dac_app_code")
#endif

#define DAC_DEBUG	0

struct DAC_CTL dac;
DAC_PARAM dac_param;

/*----------------------------------------------------------------------------*/
/** @brief:  DAC IE control
    @param:
    @return: none
    @author:
    @note:
*/
/*----------------------------------------------------------------------------*/
void dac_int_enable(void)
{
    dac_ie_en(1);
#if BT_TWS
    dac_mute(0, 0);
#else
    dac_mute(0, 1);
#endif
}
void dac_int_disable(void)
{
#if BT_TWS
    dac_mute(0, 0);
#else
    dac_mute(0, 1);
#endif
    dac_ie_en(0);
}

/*----------------------------------------------------------------------------*/
/** @brief:  DAC SR control
    @param:
    @return: none
    @author:
    @note:
*/
/*----------------------------------------------------------------------------*/
void dac_sr_cb(u16 rate)
{
    log_printf("dac_sr = %d\n", rate);
    eq_samplerate(rate);
    sin_tone_sr_set(rate);
#if ECHO_EN
    echo_dac_adc_sr_sync();
#endif
}

/*----------------------------------------------------------------------------*/
/** @brief:  DAC power on delay
    @param:	 mode=0,power on slowly,about 600ms
			 mode=1,power on quickly,about 140ms
			 mode=2,power off delay,about 120ms
    @return: none
    @author:
    @note:
*/
/*----------------------------------------------------------------------------*/
void dac_delay_cb(u8 mode)
{
    switch (mode) {
    case DAC_SLOW_PWR_ON_DLY:
        puts("dac_on slowly\n");
        delay_2ms(300);
        break;
    case DAC_QUICK_PWR_ON_DLY:
        puts("dac_on quickly\n");
        delay_2ms(70);
        break;
    case DAC_PWR_OFF_DLY:
        puts("dac_off_delay\n");
        dac.dac_off_delay = 60;
        break;
    case DAC_PWR_OFF_DLY_WAIT:
        puts("dac_off_delay_wait\n");
        delay_2ms(60);
        break;
    default:
        break;
    }
}
/*----------------------------------------------------------------------------*/
/** @brief : dac_automute_cb
    @param : statue,automute status
    @return: none
    @author:
    @note  : 1)This function called to get automute status
*/
/*----------------------------------------------------------------------------*/
void dac_automute_cb(u8 status)
{
    if (status) {
        puts(">>auto_mute\n");
    } else {
        puts(">>auto_umute\n");
    }
}

/*----------------------------------------------------------------------------*/
/** @brief:  DAC Differential output
    @param:
    @return:
    @author:
    @note:
*/
/*----------------------------------------------------------------------------*/
AT_AUDIO
void dac_different_output(s16 *buf, u16 len)
{
    u32 i = 0;
    s16 *dat = buf;
    s16 tmp1, tmp2;

    while (i < len) {
        tmp1 = dat[i];
        tmp2 = dat[i + 1];
        tmp1 = ((s32)tmp1 + tmp2) >> 1;
        dat[i] = tmp1;
        dat[i + 1] = (tmp1 == -32768) ? 32767 : -tmp1;
        i += 2;
    }
}

/*----------------------------------------------------------------------------*/
/** @brief:  DAC DEBUG
    @param:
    @return:
    @author:
    @note:
*/
/*----------------------------------------------------------------------------*/
void dac_debug(s16 *dac_buf)
{
#if 0
    //32k
    memcpy(dac_buf, sin_32K, 128);
    return;
#else
    //44.1k
    static u16 sincnt;
    unsigned char i;
    s16 *buffer;
    buffer = dac_buf;
    for (i = 0; i <= 63;) {
        buffer[i] = sin_441k[sincnt];
        i ++;
        buffer[i] = sin_441k[sincnt];
        i ++;
        JL_PORTA->DIR |= BIT(9);
        JL_PORTA->PU |= BIT(9);
        if (!(JL_PORTA->IN & BIT(9))) {
            buffer[i - 2] = 0;  //-220
            buffer[i - 1] = 0;  //-220
        }
        if ((++sincnt) >= 441) {
            sincnt = 0;
        }
    }
#endif
}


/*----------------------------------------------------------------------------*/
/** @brief:  DAC isr callback function
    @param:
    @return: none
    @author:

    @WARNING:
	vm_init_api函数中，如果使能允许DAC工作，dac_isr_cb内调用得所有函数和常数都必须
	放在audio_text段中（使用AT_AUDIO定义即可）

*/
/*----------------------------------------------------------------------------*/
s16 read_buf[DAC_DUAL_BUF_LEN] AT(.dac_buf_sec)  __attribute__((aligned(4)));
AT_AUDIO
void dac_isr_cb(void *dac_buf, u8 buf_flag)
{
#if DAC_DEBUG
    dac_debug(dac_buf);
    return;
#endif

    /*******************************************************************/
    /* WARNING: dac_isr_cb内调用得所有函数和常数都必须使用AT_AUDIO定义 */
    /*******************************************************************/

#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
    if (0 == get_vm_statu()) {
        tws_sync_inc_dac_cnt();
        if (tws_sync_inc_dac_read()) {
            goto __dac_skip_read;
        }
    }
#endif
    if (audio_read(read_buf, DAC_BUF_LEN) == 0) {
        memset(read_buf, 0x00, DAC_BUF_LEN);
    } else {
        dac_hp_en(DAC_CHANNEL_SLECT);
    }
__dac_skip_read:
    if (0 == get_vm_statu()) {
        sin_tone_2_dac((s16 *)read_buf, DAC_SAMPLE_POINT * 2);
    }

    if (is_dac_mute()) {
        memset(read_buf, 0x00, DAC_BUF_LEN);
    }

#if DAC_SOUNDTRACK_COMPOUND
    dac_digital_lr_add(read_buf, DAC_BUF_LEN);
#endif

#if DAC_SOUNDTRACK_SUBTRACT
    dac_digital_lr_sub(read_buf, DAC_BUF_LEN);
#endif

#if ECHO_EN
    if (0 == get_vm_statu()) {
        echo_output_mix_dac(read_buf, DAC_BUF_LEN);
    }
#endif

    /*******************************************************************/
    /* WARNING: dac_isr_cb内调用得所有函数和常数都必须使用AT_AUDIO定义 */
    /*******************************************************************/

#if DAC_DIFF_OUTPUT
    dac_different_output(read_buf, DAC_DUAL_BUF_LEN);
#endif
    dac_digit_energy_value(read_buf, DAC_DUAL_BUF_LEN);
#if (VCOMO_EN && CALL_USE_DIFF_OUTPUT)
    if (aec_interface.aec_run) {
        dac_different_output(read_buf, DAC_DUAL_BUF_LEN);
    }
#endif
    digital_vol_ctrl(read_buf, DAC_BUF_LEN, sound.vol.sys_vol_l);
    //ad_vol_mix(read_buf,DAC_BUF_LEN,sound.vol.sys_vol_l);

    if (aec_interface.fill_dac_echo_buf && (0 == get_vm_statu())) {
        aec_interface.fill_dac_echo_buf((s16 *)read_buf, DAC_SAMPLE_POINT << 1);
    }

#if (AUDIO_EFFECT & AUDIO_EFFECT_HW_EQ)
    eq_run((short *)read_buf, (short *)dac_buf, DAC_SAMPLE_POINT);
#else
    memcpy(dac_buf, read_buf, DAC_BUF_LEN);
#endif

    /*******************************************************************/
    /* WARNING: dac_isr_cb内调用得所有函数和常数都必须使用AT_AUDIO定义 */
    /*******************************************************************/
#if (BT_REC_EN||FM_REC_EN||AUX_REC_EN)
    if (0 == get_vm_statu()) {
        switch (task_get_cur()) {
#if BT_REC_EN
        case TASK_ID_BT:
            rec_dac_data_input(read_buf, DAC_BUF_LEN, 2);
            break;
#endif
#if REC_SOURCE
        case TASK_ID_FM:
        case TASK_ID_LINEIN:
            rec_dac_data_input(read_buf, DAC_BUF_LEN, 1);
            break;
#endif
        default:
            break;
        }
    }
#endif

}

/*----------------------------------------------------------------------------*/
/** @brief: 音量调节接口回调函数
    @param:
    @return:
    @author:
    @note: 适用于对音量调节有特定需求的用户
*/
/*----------------------------------------------------------------------------*/
static void dac_vol_cb(u32 l_vol, u32 r_vol, u8 fade_en)
{
    //printf("[%d-%d]",l_vol,r_vol);
#if 1
    if (!l_vol && !r_vol) {
        dac_mute(1, fade_en);
    } else if (is_dac_mute()) {
        dac_mute(0, fade_en);
    }
#endif

#if SYS_VOL_EXT
    set_dac_vol_ext(l_vol, r_vol, fade_en);
#else
    set_dac_vol(l_vol, r_vol, fade_en);
#endif
}
/*----------------------------------------------------------------------------*/
/** @brief: Audio Moudule initial
    @param:
    @return:
    @author:Juntham
    @note:
*/
/*----------------------------------------------------------------------------*/
static u32 dac_buf[2][DAC_SAMPLE_POINT] 	AT(.dac_buf_sec);	/* dac DMA buf	*/
DAC_CALLBACL dac_cb_fun = {
    .sr_cb 			= dac_sr_cb,
    .automute_cb 	= dac_automute_cb,
};

void dac_init(void)
{
    puts("dac_init\n");
    dac_param.isel = DAC_ISEL_THIRD;
    dac_param.audldo_en = DAC_AUDLDO;
    dac_param.audldo_vsel = DAC_VSEL;
    dac_param.vcm_rsel = VCM_RSEL_0;
    dac_param.hp_type = DAC_CHANNEL_SLECT;
    dac_param.ldo = LDO_SLECT;
    dac_param.vcomo_en = VCOMO_EN;
    dac_param.dma_buf = (u8 *)dac_buf;
    dac_param.dma_point = DAC_SAMPLE_POINT;
    dac_param.dac_isr = dac_isr_cb;
    dac_param.delay_cb = dac_delay_cb;
    dac_param.vol_l = sound.vol.sys_vol_l;
    dac_param.vol_r = sound.vol.sys_vol_r;
    dac_param.max_vol_l = sound.vol.max_sys_vol_l;
    dac_param.max_vol_r = sound.vol.max_sys_vol_r;
    sound_drv->ioctl(DAC_OSC_CLK, OSC_Hz, NULL);
    sound_drv->init(&dac_param);
    sound_drv->on(NULL);

    dac_callback_register(&dac_cb_fun);
#if SYS_VOL_EXT
    dac_vol_cb_register(dac_vol_cb);
#endif
    dac_set_samplerate(SR8000, 1);
    dac_channel_on(DAC_DIGITAL_CH, FADE_ON);
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    dac_mute(0, FADE_ON);
    dac.toggle = 1;
}
u32 tws_audio_read(u32 prt, u32 len, u8 cmd)
{
    u32 read_len = 0;
    if (cmd) {
        read_len = audio_read(&read_buf[prt], len);
    } else {
        memset(read_buf, 0, sizeof(read_buf));

    }
    return read_len;
}
void clear_dac_dma_buf()
{
    memset(read_buf, 0x00, DAC_BUF_LEN);
    memset(dac_buf, 0, sizeof(dac_buf));

}


/*----------------------------------------------------------------------------*/
/** @brief: DAC Moudule Toggle
    @param: toggle = 0,dac off
			toggle = 1,dac on
    @return:
    @author:
    @note:
*/
/*----------------------------------------------------------------------------*/
void dac_toggle(u8 toggle)
{
    if (sound_drv && (strcmp(sound_drv->name, DAC_DRV_NAME) == 0)) {
        if (toggle && dac.dac_off_delay) {
            puts("dac_offing,but open now\n");
            dac.dac_off_delay = 0;
            dac.toggle = 0;
        }

        if (!toggle && dac.dac_off_delay) {
            puts("dac_offing,need't off repeat\n");
            return;
        }

        if (!toggle && get_tone_status()) {
            puts("tone_busy,can't close dac\n");
            return;
        }

        if (toggle && !dac.toggle) {
            puts(">dac_on<\n");
            dac.toggle = 1;
            sound_drv->ioctl(DAC_TOG_ON, BIT(0), NULL); /*BIT(0):analog,BIT(1):digital*/
            set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
        } else if (!toggle && dac.toggle) {
            puts(">dac_off_pre<\n");
            sound_drv->ioctl(DAC_TOG_OFF_PRE, 0, NULL);
        }
    }
}

static void dac_delay_count(void)
{
    if (dac.dac_off_delay) {
        dac.dac_off_delay--;
        if (dac.dac_off_delay == 0) {
            dac.toggle = 0;
            sound_drv->ioctl(DAC_TOG_OFF_POST, 0, NULL);
            puts(">dac_off_post<\n");
        }
    }
}

LOOP_DETECT_REGISTER(dac_delay) = {
    .time = 1,
    .fun  = dac_delay_count,
};
LOOP_DETECT_REGISTER(dac_fade_loop) = {
    .time = 1,
    .fun  = dac_fade,
};

//
#if USE_16_LEVEL_VOLUME
u8 volume_temp=0;
const u8 volume_table[MAX_SYS_VOL_TEMP+1] = 
{
#if 1
	0,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11,12,13,14,15,16,17,18,19,20,
	21,21,22,22,23,23,24,/*24,25,*/25,
	26,27
#else
	0,
	5, 8, 10,12,14,16,18,20,
	22,24,26,27,28,29,MAX_SYS_VOL_L
#endif
};
const u8 volume_table_2[MAX_SYS_VOL_TEMP+1] = 
{
	0,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11,12,13,14,15,16,17,18,19,20,
	21,22,23,24,25,26,26,27,/*27,28,*/
	28,MAX_SYS_VOL_L
};
#endif

