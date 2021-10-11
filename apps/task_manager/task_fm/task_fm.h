#ifndef FM_RADIO_H
#define FM_RADIO_H

#include "task_manager.h"

#define FM_DEBUG
#ifdef FM_DEBUG
#define fm_printf printf
#define fm_puts puts
#else
#define fm_printf(...)
#define fm_puts(...)
#endif

extern const TASK_APP task_fm_info;
typedef enum {
    FM_UNACTIVE = 0,
    FM_ACTIVE,
    FM_SCAN_STOP,
    FM_SCAN_BUSY,
    FM_SCAN_PREV,
    FM_SCAN_NEXT,
    FM_SCAN_ALL,
} FM_STA;

typedef struct _FM_MODE_VAR_ {
    FM_STA scan_mode;       ///当前FM的状态
    u16 wFreq;              ///<当前频点
    u16 wFreChannel; 	    ///<当前频道
    u16 wLastwTotalChannel;
    u16 wTotalChannel;      ///<总台数
    u16 bAddr;	            ///<在线的FM收音的模块指针
    u8 fm_mute;         ///当前FM的状态，1: mute/ 0:play
} FM_MODE_VAR;

typedef struct _FM_INFO_ {
    u8 dat[32];             ///FM信息保存buf
} FM_INFO;

extern FM_MODE_VAR *fm_mode_var;   ///<FM状态结构体
extern FM_INFO *fm_info;           ///<FM存台信息
extern const struct task_info fm_radio_task_info;
extern u8 fm_pp_flag;

void fm_arg_close(void);
void fm_arg_open(void);
void fm_radio_powerdown(void);
u8 fm_radio_init(void);
#endif

