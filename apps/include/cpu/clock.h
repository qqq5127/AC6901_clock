#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "typedef.h"
#include "clock_api.h"
#include "common/list.h"
#include "sdk_cfg.h"




typedef int (*switcher_t)(void);
typedef void (*init_t)(void);

struct clock_switch {
    struct list_head entry;
    switcher_t fun;
    char *user;
};

enum {
    CLOCK_SWITCH_ENTER = 0,
    CLOCK_SWITCH_EXIT,
};

extern init_t clock_switcher_begin[];
extern init_t clock_switcher_end[];

#define list_for_each_clock_switcher(switcher) \
	for (switcher = clock_switcher_begin; switcher < clock_switcher_end; \
			switcher++) \

void clock_switch_register(u8 type, struct clock_switch *switcher, switcher_t fun, char *user);

#define CLOCK_SWITCH_USER_REGISTER(a, b, c, d)      \
    static void switch_init_##c(void) \
    {\
        clock_switch_register(a, b, c, d);\
    }\
    const init_t __switch_init_##c  \
        AT(.clock_switch.fun) = switch_init_##c

void set_sys_freq(u32 out_freq);
void set_spi_speed_auto(void);
void clock_init_app(SYS_CLOCK_INPUT sys_in, u32 input_freq, u32 out_freq);
void clock_switching(u32 out_freq);
void clock_debug(void);
void set_apc_clk(u32 freq);

#endif //__CLOCK_H__
