#ifndef _EQ_DRV_H_
#define _EQ_DRV_H_

#include "typedef.h"

typedef struct {
    int *filt_44100;
    int *filt_22050;
    int *filt_11025;
    int *filt_48000;
    int *filt_24000;
    int *filt_12000;
    int *filt_32000;
    int *filt_16000;
    int *filt_8000;
    int (*freq_gain)[10];
    int *global_gain;
    int seg_num;
} HARD_EQ_FILT;

typedef struct {
    u8 ie;
    u8 nShift;
    u8 fWaitOut;
    u8 reserved;
    HARD_EQ_FILT filt;
    void (*isr_cb)(s16 *Outbuf, u16 npoint);
} HARD_EQ_PARAM;

typedef struct {
    int *filt_44100;
    int *filt_22050;
    int *filt_11025;
    int *filt_48000;
    int *filt_24000;
    int *filt_12000;
    int *filt_32000;
    int *filt_16000;
    int *filt_8000;
    int (*freq_gain)[10];
    int TargetLvl;
    int ReleaseTime;
    int AttackTime;
    int GlobalGain;
    int LimiterTog;
    int seg_num;
} SOFT_EQ_FILT;

typedef struct {
    int ReleaseTime;
    int AttackTime;
    int TargerLvl;
} LIMITER_VAR;

typedef struct {
    void (*callback)(void *priv);
    SOFT_EQ_FILT filt;
} SOFT_EQ_PARAM;


enum {
    SET_EQ_MODE		= 1	,
    SET_EQ_SR			,
    SET_EQ_BYPASS		,
    GET_EQ_STATE		,
    SET_EQ_IE			,
    SET_EQ_RESET		,
    SET_EQ_GLOBALGAIN	,
};

struct eq_driver {
    u32(*need_buf)(u8 nChannel, u8 nSection);
    s32(*init)(void *mem, void *cfg);
    s32(*on)(void *ptr);
    s32(*off)(void *ptr);
    void (*run)(s16 *in, s16 *out, u16 npoint);
    s32(*ioctl)(u32 cmd, u32 arg, void *ptr);
};
extern struct eq_driver eq_drv;
extern const struct eq_driver soft_eq_drv;

#endif // EQ_driver_h__

