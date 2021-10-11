#ifndef _ECHO_API_H_
#define _ECHO_API_H_
#include "sdk_cfg.h"
#include "ladc.h"

enum {
    FIRST_ECHO_DIGITAL = 0,
    FIRST_ECHO_ANALOG = 1,
};

typedef struct _ECHO_CONTROL_VAR_ {
    u16 music_vol;
    u16 mic_vol_digital;
    u16 deep;
    u16 music_vol_bk;
    u8 mic_vol;
    s8 pitch;
    u8 decay;
    u8 volatile run_flag;
    u8 first_echo_sel;    //第一声(直达声)选则, 可以选则 数字通路 或 模拟通路.
} ECHO_CONTROL_VAR;


void echo_start(void);
void echo_stop(void);
void echo_dac_adc_sr_sync(void);  //LADC和DAC采样率同步
void echo_input(s16 *buf, u32 len);	//unit:byte
void echo_output_mix_dac(s16 *buf, u32 len);	//len unit byte
tbool echo_msg_deal(void *p, u32 msg);

void echo_music_vol_set(u16 vol); //0~ 16384   //背景音乐音量大小. 16384(0 db)
void echo_mic_vol_set(u8 vol);    //0 ~ 63      //MIC模拟音量(放大)大小
void echo_pitch_set(s8 pitch);    //-127 ~ 127  //变调, 0为正常声音, 负值越小频率越低, 正值越大频率越高
void echo_decay_set(u8 decay);    //0 ~ 128     //混响强度衰减: 值越大, 混响的后一声比前一声衰减越多.
void echo_deep_set(u16 deep);     //-127 ~ 127  //变调, 0为正常声音, 负值越小频率越低, 正值越大频率越高
bool echo_open_flag_get(void);
void echo_open_flag_set(bool flag);

#endif

