#ifndef _CHARGE_H_
#define _CHARGE_H_

#include "typedef.h"
#include "common/common.h"

enum {
    POWER_ON = 1,
    POWER_OFF,
};

//可配置选项
//充电时间过长，可以调整关机的充电阀值POWEROFF_THRESHOLD_VALUE和开机充电阀值POWERON_THRESHOLD_VALUE宏，值越大，充电时间越短，充满电压越低
#define POWEROFF_THRESHOLD_VALUE        500L
#define POWERON_THRESHOLD_VALUE         580L

#define C_POWER_BAT_CHECK_CNT         50
#define C_POWER_KEY_CHECK_CNT         300


#endif    //_CHARGE_H_
