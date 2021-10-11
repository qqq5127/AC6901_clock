#ifndef _LADC_API_H_
#define _LADC_API_H_

#include "typedef.h"
#include "circular_buf.h"

enum {
    SET_MIC_EN			= 1	,
    SET_MIC_GAIN			,
    SET_MIC_MUTE			,
    SET_MIC_GX2				,
    SET_MIC_NEG12			,
    SET_ADC_SR				,
    GET_ADC_SR				,
    SET_ADC_PGA				,
    SET_ADC_UDE				,
    SET_ADC_OSC				,
};

typedef struct _LADC_PARAM {
    u8 mic_isel;
    u8 adc_isel;
    u8 pga_isel;
    u8 dccs;
    u8 vcomo_en;
    u8 ADC_LEN;
    s16 *ADC_ADR;
    void (*isr_cb)(void *buf, u8 flag, u16 buf_len);
} LADC_PARAM;

typedef struct _LADC_DRV {
    s32(*init)(LADC_PARAM *param);
    s32(*on)(u8 ch, u16 sr);
    s32(*off)(u8 ch);
    s32(*ioctl)(u32 cmd, u32 arg, void *ptr);
} LADC_DRV;
extern struct _LADC_DRV ladc_drv_ops;

#endif
