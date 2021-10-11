#ifndef __TASK_MANAGER_H__
#define __TASK_MANAGER_H__
#include "typedef.h"
#include "ui/led/led7_drv.h"
#include "key.h"
#include "sdk_cfg.h"

///app名称定义， 建议所有模式切换或者是消息发送都使用宏，不直接写字符串
// #define TASK_APP_IDLE		 "TaskAppIdle"
// #define TASK_APP_MUSIC       "TaskAppMusic"
// #define TASK_APP_IDLE_1	 	 "TaskAppIdle_1"
#define TASK_MANGER_ENABLE	1
#if TASK_MANGER_ENABLE

typedef enum {
    TASK_ID_BT = 0x0,
#if BT_HID_INDEPENDENT_MODE
    TASK_ID_BT_HID,
#endif
#if FM_RADIO_EN
    TASK_ID_FM,
#endif
#if (BT_TWS_LINEIN==0)
	#if AUX_EN
    TASK_ID_LINEIN,
    #endif
#endif
    TASK_ID_MUSIC,
#if USB_PC_EN
    TASK_ID_PC,
#endif
#if 0//FM_RADIO_EN
    TASK_ID_FM,
#endif
#if RTC_CLK_EN
    TASK_ID_RTC,
    TASK_ID_ALARM,
#endif
#if MIC_REC_EN
    TASK_ID_REC,
#endif
#if TASK_ECHO_EN
    TASK_ID_ECHO,
#endif
#if SPDIF_EN
    TASK_ID_SPDIF,
#endif
    TASK_ID_IDLE,
    TASK_ID_MAX,
    TASK_ID_TYPE_LAST,
    TASK_ID_TYPE_PREV,
    TASK_ID_TYPE_NEXT,
    TASK_ID_UNACTIVE,
} TASK_ID_TYPE;


typedef struct __TASK_APP {
    // const char *name;
    tbool(*skip_check)(void **priv);
    void *(*init)(void *priv);
    void (*exit)(void **priv);
    void (*task)(void *priv);
    const KEY_REG *key;
} TASK_APP;

tbool task_switch(TASK_ID_TYPE target, void *param);
void task_manager(void);
TASK_ID_TYPE task_get_cur(void);
TASK_ID_TYPE task_get_cur(void);

#else
#define task_switch(...)
#define task_manager(...)
#endif

#endif// __TASK_MANAGER_H__
