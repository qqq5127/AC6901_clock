#ifndef __AUDIO_PARAM_H__
#define __AUDIO_PARAM_H__

#include "typedef.h"
#include "circular_buf.h"

struct sound_driver {
    const char *name;
    s32(*init)(void *ptr);
    s32(*on)(void *ptr);
    s32(*off)(void *ptr);
    s32(*ioctl)(u32 cmd, u32 arg, void *ptr);
};
#define REGISTER_SOUND_DRIVERV(drv) \
		struct sound_driver *sound_drv \
			sec(.text) = &drv

extern struct sound_driver *sound_drv;

struct SOUND_VOL {
    u8 sys_vol_l;
    u8 sys_vol_r;
    volatile u8 digital_vol_l;
    volatile u8 digital_vol_r;
    u8 max_sys_vol_l;
    u8 max_sys_vol_r;
    u8 max_digit_vol_l;
    u8 max_digit_vol_r;
};

struct SOUND_VAR {
    u8 phone_vol;
    u8 read_able;
    u8 eq_mode;
    u8 automute_ctl;
    /*digital_fade var*/
    volatile u8 digital_fade;
    u8 digital_fade_vol_l;
    u8 digital_fade_vol_r;
    /*save cur sys_vol*/
    u8 tmp_sys_vol_l;
    u8 tmp_sys_vol_r;
    cbuffer_t cbuf;
    struct SOUND_VOL vol;
};

//=============================================================
//
//					LADC PARAM DEFINE
//
//=============================================================
#define LADC_LINL_CHANNEL      		0	//LINL_2_ADC
#define LADC_LINR_CHANNEL      		1	//LINR_2_ADC
#define LADC_MIC_CHANNEL            2	//MIC_2_ADC
#define LADC_LINLR_CHANNEL      	3	//LINLR_2_ADC

//ladc_support channel
#define LADC_MAX_CHANNEL			1

//=============================================================
//
//					DAC PARAM DEFINE
//
//=============================================================
//audio samplerate define
#define SR44100    	44100
#define SR48000		48000
#define SR32000  	32000
#define SR22050    	22050
#define SR24000    	24000
#define SR16000    	16000
#define SR11025    	11025
#define SR12000    	12000
#define SR8000     	8000

enum {
    SR_441K = 0x0,
    SR_48K,
    SR_32K,
    SR_2205K = 0x4,
    SR_24K,
    SR_16K,
    SR_11025K = 0x8,
    SR_12K = 0x9,
    SR_8K = 0xA,
};

//通道最大值不能超出0x00ff，(channel_mode & 0xFF00)
enum {
    /*digital channel*/
    DAC_DIGITAL_CH = 1,       ///<数字通道
    /*analog channel*/
    DAC_AMUX0,
    DAC_AMUX1,
    DAC_AMUX2,
    DAC_AMUX0_L_ONLY,
    DAC_AMUX0_R_ONLY,
    DAC_AMUX1_L_ONLY,
    DAC_AMUX1_R_ONLY,
    DAC_AMUX2_L_ONLY,
    DAC_AMUX2_R_ONLY,
    DAC_AMUX_DAC_L,             ///<DAC_L input,DAC_R output
    DAC_AMUX_DAC_R,             ///<DAC_R input,DAC_L output
};

/*
*DAC MONO
*i.e. aux_ch = DAC_AMUX0,dac_ch = DAC_L
*linein_channel_open(DAC_AMUX0|DAC_MONO_L,1);
*/
#define DAC_MONO_L	1<<8
#define DAC_MONO_R	2<<8

#define LDO_1   1 //LDO1(default)
#define LDO_2   2 //LDO2

#define DAC_SAMPLE_CON      (0)
#define DAC_SAMPLE_POINT    (32 * (1 << DAC_SAMPLE_CON))
#define DAC_BUF_LEN         (DAC_SAMPLE_POINT*2*2)
#define DAC_DUAL_BUF_LEN    (DAC_SAMPLE_POINT * 2)


//DAC_HP
#define DAC_L_CHANNEL     1    ///Left Channel
#define DAC_R_CHANNEL     2    ///Right Channel
#define DAC_L_R_CHANNEL   3    ///Left and Right Channel

//DAC_ISEL
#define DAC_ISEL5U      BIT(2)//4mA
#define DAC_ISEL_HALF	BIT(1)//2mA
#define DAC_ISEL_THIRD  BIT(0)//1.2mA

//DAC_VSEL
#define DAC_AUDLDO_EN	1
#define DAC_AUDLDO_DIS	0
#define DAC_AUDLDO		DAC_AUDLDO_EN
#define DAC_VSEL	    6	//(0(min)~7(max))

//DAC_VCM_RSEL
#define VCM_RSEL_0		0	//power on slowly(BIG)
#define VCM_RSEL_1		1	//power on quickly(SMALL)

//DAC DELAY MODE
#define DAC_SLOW_PWR_ON_DLY		0
#define DAC_QUICK_PWR_ON_DLY	1
#define DAC_PWR_OFF_DLY			2
#define DAC_PWR_OFF_DLY_WAIT		3

#endif
