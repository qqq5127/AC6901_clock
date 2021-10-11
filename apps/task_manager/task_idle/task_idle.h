#ifndef __TASK_IDLE_H__
#define __TASK_IDLE_H__
#include "task_manager.h"

extern const TASK_APP task_idle_info;
extern const TASK_APP task_idle_info_1;

#define IDLE_POWER_UP       ((void *)1)
#define IDLE_POWER_OFF      ((void *)2)
#define IDLE_POWER_UP_TONE  ((void *)3)
#define IDLE_DCIN_POWER_UP  ((void *)4)

#endif//__TASK_IDLE_H__

