#ifndef __TASK_BT_H__
#define __TASK_BT_H__
#include "task_manager.h"

#if TASK_MANGER_ENABLE
extern const TASK_APP task_bt_info;
extern const TASK_APP task_bt_hid_info;
#endif

typedef enum {
    BT_WAITINT_INIT = 0,     //蓝牙等待初始化
    BT_INIT_OK,              //蓝牙已经初始化了
    BT_SUSPEND,              //蓝牙挂起
} BT_TASK_STATUS;

void task_bt_deal(void *hdl);
void bt_rec_exit();
// void task_bt_deal(void);
extern void no_background_suspend();
extern void background_suspend();
extern void update_bt_tws_info(u8 info);

#endif//__TASK_BT_H__

