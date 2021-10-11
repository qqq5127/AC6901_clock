#ifndef SYS_TIMER_H
#define SYS_TIMER_H


#include "typedef.h"
#include "common/list.h"




struct sys_timer {
    struct list_head entry;
    void (*fun)(struct sys_timer *);
    u32 jiffies;
    u32 user;
    u32 delay_do;
    u32 loop;
};

typedef struct sys_timer timer_source_t ;

extern volatile unsigned long jiffies;


u32 get_jiffies(u8 mode, u32 timer_ms);
void __timer_insert(struct sys_timer *timer);
bool __timer_find(struct sys_timer *timer);
void sys_timer_register(struct sys_timer *timer, u32 msec,
                        void (*fun)(struct sys_timer *timer), u8 delay_do);
void sys_timer_remove(struct sys_timer *timer);
void sys_timer_reset(struct sys_timer *timer, u32 msec);

void sys_timer_init(struct sys_timer *timer);
void sys_timer_del_schedule();
void loop_timer_schedule();
void sys_timer_var_init() ;
void sys_timer_delay_handler_register(void (*handler)());
void *get_sys_timer_head();

u32 sys_timer_get_user(struct sys_timer *timer);
void sys_timer_set_user(struct sys_timer *timer, u32 user);
void sys_timer_us_register(struct sys_timer *timer, u32 us_sec,
                           void (*fun)(struct sys_timer *timer), u8 delay_do);
void sys_timer_us_remove(struct sys_timer *timer);
void sys_timer_reduce(struct sys_timer *timer, u32 msec);

#endif

