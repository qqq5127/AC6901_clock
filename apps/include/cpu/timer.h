#ifndef TIMER_H
#define TIMER_H

#include "typedef.h"

void timer_init();
u32 os_time_get(void);

enum {
    TIMER_CLK_SRC_SYSCLK          = 0,
    TIMER_CLK_SRC_IOSIGN,
    TIMER_CLK_SRC_OSC,
    TIMER_CLK_SRC_RC,
};

#endif

