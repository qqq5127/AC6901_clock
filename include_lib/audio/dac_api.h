#ifndef __DAC_API_H__
#define __DAC_API_H__

#include "typedef.h"
#include "audio_param.h"
#include "circular_buf.h"
#include "uart.h"

//#define DAC_DRV_DEBUG
#ifdef DAC_DRV_DEBUG
#define dac_puts           puts
#define dac_put_buf        put_buf
#define dac_printf         printf
#else
#define dac_puts(...)
#define dac_put_buf(...)
#define dac_printf(...)
#endif


#define DAC_DRV_NAME	"DAC_driver"

enum {
    //libs use
    SET_DAC_IE		= 1	,
    SET_DAC_TRIM			,
    SET_DAC_DIGITAL			,
    SET_DAC_ANALOG			,
    GET_DAC_MUTE			,
    SET_DAC_MUTE			,
    GET_DAC_GAIN			,	/*arg=0(left),arg=1(right)*/
    SET_DAC_GAIN			,	/*gain_l|(gain_r<<16)*/
    SET_DAC_LDO				,
    SET_DAC_HP				,
    SET_DAC_SPK				,
    SET_DAC_SR				,
    SET_DAC_AMUX_GAIN,
    //app use
    DAC_TOG_ON				,
    DAC_TOG_OFF_PRE			,
    DAC_TOG_OFF_POST		,
    DAC_OSC_CLK				,
};

typedef struct _DAC_PARAM {
    u8 vol_l;
    u8 vol_r;
    u8 max_vol_l;
    u8 max_vol_r;
    u8 isel;			/*LPF_ISEL*/
    u8 audldo_en;		/*AUDLDO_EN*/
    u8 audldo_vsel;		/*AUDLDO_VSEL*/
    u8 vcm_rsel;
    u8 hp_type;
    u8 ldo;				/*LDO TYPE*/
    u8 vcomo_en;		/*VCOM_OUT_EN*/
    u8 dma_point;
    u8 *dma_buf;
    void (*dac_isr)(void *adr, u8 flag);
    void (*delay_cb)(u8 mode);
} DAC_PARAM;

extern struct sound_driver dac_drv;

//********************************************************************
//
//						DAC CONFIG
//
//********************************************************************
#define FADE_ON      1                ///<通道切换使用淡入淡出
#define FADE_OFF     0                ///<通道切换不使用淡入淡出

#define AUTO_MUTE_DISABLE	  0         ///数字通道auto mute：关
#define AUTO_MUTE_ENABLE 	 BIT(0) 	///数字通道auto mute：开
#define AUTO_UNMUTE_FADE	 BIT(1)     ///有 FADE IN 效果
#define AUTO_MUTE_UMUTE_DAC	 BIT(2)     ///Only fade out when auto mute
#define AUTO_FADE_CTL		 BIT(3)
#define AUTO_MUTE_CFG		 AUTO_MUTE_ENABLE | AUTO_UNMUTE_FADE

//********************************************************************
//
//						DAC VARIABLE
//
//********************************************************************
typedef struct _DAC_CALLBACK {
    void (*sr_cb)(u16 rate); /*dac samplerate rate callback function*/
    void (*automute_cb)(u8 status); /*automute status change callback*/
} DAC_CALLBACL;

//********************************************************************
//
//           			DAC API
//
//********************************************************************
void dac_init_api(tbool vcom_outen, u8 ldo_slect, u8 hp_type, u8 dac_isel, u8 vcm_rsel);
s32 ladc_isr_register(void (*ladc_isr_reg)(void));
void dac_callback_register(DAC_CALLBACL *cb);

///dac channel api
s32 dac_channel_on(u8 channel, u8 fade_en);
s32 dac_channel_off(u8 channel, u8 fade_en);

///dac vol setting
void set_sys_vol(u32 l_vol, u32 r_vol, u8 fade_en);
void set_dac_vol(u32 l_vol, u32 r_vol, u8 fade_en);
u8 get_sys_vol(u8 l_or_r);
u8 get_max_sys_vol(u8 l_or_r);
void set_sys_max_vol(struct SOUND_VOL *vol);
void dac_vol_cb_register(void (*vol_cb)(u32 l_vol, u32 r_vol, u8 fade_en));

///dac mute api
void dac_automute_init(u8 ctl, u16 packet_cnt, u16 Threshold, u8 max_cnt);
void dac_automute_param_init(u8 ctl, u16 packet_cnt, u16 Threshold, u16 max_cnt);
void dac_auto_mute_release_immediately(void);//手动解自动mute， 可以用于解决开始播放吞字问题, add by wuxu 20190729
void dac_mute(u8 mute_flag, u8 fade_en);
bool is_dac_mute(void);
bool is_auto_mute(void);

void dac_set_samplerate(u16 sr, u8 wait);
u16 dac_get_samplerate(void);
u32 dac_digit_energy_value(void *buffer, u16 len);
void dac_fade(void *ptr);

void dac_digital_en(u8 digital_en, u8 trim_en);
void dac_ldo_set(u8 ldo);
void dac_ie_en(u8 en);
void dac_hp_en(u8 en);
u32 get_dac_energy_value(void);
void audio_sfr_dump(void);
void dac_speaker_init();
void dac_digital_lr_add(void *buffer, u32 len);
void dac_digital_lr_sub(void *buffer, u32 len);

void dac_AmuxGain_en(u8 en);

#endif  //__DAC_API_H__
