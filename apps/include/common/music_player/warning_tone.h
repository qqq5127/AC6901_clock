#ifndef __WARNING_TONE_H__
#define __WARNING_TONE_H__

#include "if_decoder_ctrl.h"
#include "resource_manage.h"

#define MAX_WARNING_NUM		4

typedef struct _WARNING_SET {
    int freq;       //频率*512//
    int loop_len;   //持续点数：计算一下接近整数个周期，避免从峰值到静音出噪音//
    int win_type;   //0：直线衰减，1：正弦窗//
    int win_para;   //窗参数，直线衰减：百分比*32768，一般设为50%*32768=16384 正弦窗：频率*512 //
} WARNING_SET;

typedef struct _WARNING_PARM {
    u16 slot_time;				//repeat_slot_time	(ms)
    u16 set_cnt;				//number of warning_set
    WARNING_SET set_p[MAX_WARNING_NUM];
} WARNING_PARM;

extern const void *tones[];

enum {
    TONE_NWT_WARNING,
    TONE_NWT_MAX = 10, /*New Warning Tone Max*/

    TONE_BT_CONN,
    TONE_BT_CONN_LEFT,
    TONE_BT_CONN_RIGHT,
    TONE_BT_DISCON,
    TONE_BT_DISCON_TWS,
    TONE_POWER_ON,
    TONE_POWER_OFF,
    TONE_BT_PARING,
    TONE_RING,
    TONE_NUM_0,
    TONE_NUM_1,
    TONE_NUM_2,
    TONE_NUM_3,
    TONE_NUM_4,
    TONE_NUM_5,
    TONE_NUM_6,
    TONE_NUM_7,
    TONE_NUM_8,
    TONE_NUM_9,
    TONE_BT_MODE,
    TONE_MUSIC_MODE,
    TONE_SD,
    TONE_USB,
    TONE_RADIO_MODE,
    TONE_LINEIN_MODE,
    TONE_REC_MODE,
    TONE_ECHO_MODE,
    TONE_PC_MODE,
    TONE_RTC_MODE,
    TONE_WARNING,
    TONE_LOW_POWER,
    TONE_VOLMAXMIN,
    TONE_TWS,
    TONE_ALARM_RING,

    TONE_MAX,
};

typedef struct __TONE_VAR_T {
    volatile u8 status;
    u32 idx;
    u32 rpt_mode;
} TONE_VAR_T;
extern TONE_VAR_T tone_var;

u8 get_tone_status(void);
void tone_play(u8 index, u8 repeat_flag);
#endif

