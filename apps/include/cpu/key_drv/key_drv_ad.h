#ifndef __KEY_DRV_AD_H__
#define __KEY_DRV_AD_H__

#include "key_drv/key.h"
#include "adc_api.h"

#define ADC_KEY_NUMBER 10

#if KEY_AD_RTCVDD_EN
#define AD_KEY_IO_PRX  PORTR2   // PORTR1 PORTR2
#define AD_KEY_CH      AD_CH_PR2
#elif KEY_AD_VDDIO_EN
#if 0
#define AD_KEY_IO_PRX  PORTR2   // PORTR1 PORTR2
#define AD_KEY_CH      AD_CH_PR2
//#define AD_KEY2_IO_PRX  PORTR1
//#define AD_KEY2_CH      AD_CH_PR1
#define AD_KEY2_IO_PAX  3
#define AD_KEY2_IO_PX   JL_PORTA
#define AD_KEY2_CH      AD_CH_PA3
#else
#define AD_KEY_IO_PAX  3
#define AD_KEY_IO_PX   JL_PORTA
#define AD_KEY_CH      AD_CH_PA3
#endif
#endif
#define  EXTERN_R_UP       220  //外部上拉22K 使用芯片内部上拉设为0

extern u8 get_adkey_value();

void adkey_sd_mult_set_sd_io();

extern const key_interface_t key_ad_info;
#endif/*__KEY_DRV_AD_H__*/
