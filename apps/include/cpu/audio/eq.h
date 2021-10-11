/*******************************************************************************************
  File Name: dac.h

Version: 1.00

Discription:


Author:yulin deng

Email :flowingfeeze@163.com

Date:2014-01-13 17:09:41

Copyright:(c)JIELI  2011  @ , All Rights Reserved.
 *******************************************************************************************/
#ifndef _EQ_H_
#define _EQ_H_

#include "typedef.h"
#include "audio/eq_api.h"
#include "audio/audio.h"

#define EQ_CHANNEL_NUM			2
#define EQ_SECTION_NUM			10
#define SOFT_EQ_SECTION_NUM		7
#define EQ_MODE_MAX				5	/*0~5*/

typedef enum {

    EQ_NORMAL = 0,
    EQ_ROCK,
    EQ_POP,
    EQ_CLASSIC,
    EQ_JAZZ,
    EQ_COUNTRY,
    EQ_CUSTOM,   //User_defined
} EQ_TYPE;

typedef struct {
    u8 mode;
    void *pEQ;
} EQ_ARG;


void eq_init(void);
void eq_enable(void);
void eq_disable(void);
void eq_bypass_en(u8 en);
void eq_run(short *in, short *out, int npoint);
void eq_mode_set(u8 mode);
void eq_samplerate(u16 sr);
void eq_mode_switch(u8 eq_mode);
void eq_mode_switch_bt_sync(u8 eq_mode);
u8 get_eq_default_mode(void);
AUDIO_STREAM *audio_eq_input(AUDIO_STREAM *output);
void soft_eq_SegmentCoeff_Modify(int seg_idx, int tab_idx);
s32 eq_cfg_read(void);

#endif
