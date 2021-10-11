#ifndef __ADC_API_H__
#define __ADC_API_H__

#include "typedef.h"
#include "key_drv/key.h"


//AD channel define
#define AD_CH_PA3    (0x0<<8)
#define AD_CH_PA4    (0x1<<8)
#define AD_CH_PA5    (0x2<<8)
#define AD_CH_PA6    (0x3<<8)
#define AD_CH_PC4    (0x4<<8)
#define AD_CH_PA10   (0x5<<8)
#define AD_CH_PB0    (0x6<<8)
#define AD_CH_PB1    (0x7<<8)
#define AD_CH_PB4    (0x8<<8)
#define AD_CH_PB5    (0x9<<8)
#define AD_CH_PC3    (0xA<<8)
#define AD_CH_DM     (0xB<<8)
#define AD_CH_RTC    (0xC<<8)
#define AD_CH_PMU    (0xD<<8)
#define AD_CH_BT     (0xE<<8)
#define AD_CH_AUDIO  (0xF<<8)

#define ADC_RTC_MUX_PR1       (0x1<<16)
#define ADC_RTC_MUX_PR2       (0x2<<16)
#define ADC_RTC_MUX_RTC_HOSC  (0x3<<16)
#define ADC_RTC_MUX_RTC_LOSC  (0x4<<16)
#define ADC_RTC_MUX_RTCVDD    (0x5<<16)

#define ADC_PMU_MUX_VBG       (0x0<<16)
#define ADC_PMU_MUX_VDD       (0x1<<16)
#define ADC_PMU_MUX_VDDDA     (0x2<<16)
#define ADC_PMU_MUX_VDC13     (0x3<<16)
#define ADC_PMU_MUX_VBAT      (0x4<<16)
#define ADC_PMU_MUX_LDOIN     (0x5<<16)
#define ADC_PMU_MUX_RTCLDO33  (0x6<<16)
#define ADC_PMU_MUX_CHARGE    (0x7<<16)

#define AD_CH_PR1       (AD_CH_RTC | ADC_RTC_MUX_PR1)
#define AD_CH_PR2       (AD_CH_RTC | ADC_RTC_MUX_PR2)
#define AD_CH_RTC_HOSC  (AD_CH_RTC | ADC_RTC_MUX_RTC_HOSC)
#define AD_CH_RTC_LOSC  (AD_CH_RTC | ADC_RTC_MUX_RTC_LOSC)
#define AD_CH_RTCVDD    (AD_CH_RTC | ADC_RTC_MUX_RTCVDD)

#define AD_CH_LDOREF    (AD_CH_PMU | ADC_PMU_MUX_VBG)
#define AD_CH_VDD       (AD_CH_PMU | ADC_PMU_MUX_VDD)
#define AD_CH_VDDDA     (AD_CH_PMU | ADC_PMU_MUX_VDDDA)
#define AD_CH_VDC13     (AD_CH_PMU | ADC_PMU_MUX_VDC13)
#define AD_CH_VBAT      (AD_CH_PMU | ADC_PMU_MUX_VBAT)
#define AD_CH_LDOIN     (AD_CH_PMU | ADC_PMU_MUX_LDOIN)
#define AD_CH_RTCLDO33  (AD_CH_PMU | ADC_PMU_MUX_RTCLDO33)
#define AD_CH_CHARGE    (AD_CH_PMU | ADC_PMU_MUX_CHARGE)


/*AD通道定义*/
enum {
#if KEY_AD_RTCVDD_EN
    R_AD_CH_KEY = 0,
    R_AD_CH_RTCVDD,
#elif KEY_AD_VDDIO_EN
    R_AD_CH_KEY = 0,
    //R_AD_CH_KEY2,
#endif
#if POWER_EXTERN_DETECT_EN
    R_AD_CH_EXTERN_POWER,
#endif
#if USE_AD_CHECK_VOLTAGE
    R_AD_CH_CHECK_VOLTAGE,
#endif
#if USE_AD_TUNER_VOLUME
    R_AD_CH_TUNER_VOLUME,
#endif
    R_AD_CH_VBAT,
    //R_AD_CH_CHARGE,
    // R_AD_CH_LDOIN,
    //R_AD_CH_LDOREF,
    R_AD_CH_BT,
    R_MAX_AD_CHANNEL,
};


// enum {
// CH_VBG = 0,
// CH_VDD,
// CH_DVDDA,
// CH_VDC13,
// CH_QUARTER_VBAT,
// CH_QUARTER_LDO5V,
// CH_ONE_THIRD_RTC_LDO33,
// CH_CHARGE_DET,
// };

extern u16 adc_value[R_MAX_AD_CHANNEL];
extern const u32 ad_table[];

void adc_init_api(u32 channel, u32 lsb_clk, u8 lvd_en);
void adc_off_api(void);
void adc_ch_sel(u32 ch);
void adc_ldo_ch_sel(u32 ch);
void adc_init();
u16 adc_res_api(u32 channel);
void adc_scan();

#endif
