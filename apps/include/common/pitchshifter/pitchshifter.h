#ifndef __PITCHSHIFTER_H__
#define __PITCHSHIFTER_H__
#include "typedef.h"
#include "audio/audio_stream.h"


#define PS_PITCHT_DEFAULT_VAL (32768L) ///变调参数说明：>32768音调高，<32768音调低，变调比例pitchV/32768
#define PS_SPEED_DEFAULT_VAL (80L) ///变数参数说明，>80变快，<80变慢,建议范围：40_160, 但是20-200也有效


typedef struct __PITCHSHIFTER PITCHSHIFTER;

AUDIO_STREAM *pitchshifter_input(AUDIO_STREAM *output);
void pitchshifter_set_speed_val(u16 val);
void pitchshifter_set_pitch_val(u16 val);
u16 pitchshifter_get_cur_pitch(void);
u16 pitchshifter_get_cur_speed(void);


#endif//__PITCHSHIFTER_H__
