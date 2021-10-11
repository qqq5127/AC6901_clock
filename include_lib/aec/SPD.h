#ifndef _SPD_H_
#define _SPD_H_

#include "typedef.h"

typedef struct {
    u16 spd_thr;
    u16 spd_gain;
    void (*output)(s16 *data, u16 point);
} SPD_PARAM;

u32 SpeechDetectQuery(void);
s32 SpeechDetectInit(void *mem, SPD_PARAM *param);
void SpeechDetectRun(short *data, int npoint);

#endif

