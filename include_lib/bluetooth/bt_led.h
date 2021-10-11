#ifndef _BT_LED_H_
#define _BT_LED_H_

#include"typedef.h"

void bt_led_init(void (*wk_cb)(void), u32 active_slot_time, u32 sleep_slot_time);
void powndown_led_fre_set(u32 active_ms_time, u32 sleep_ms_time);

void bt_led_close(void);

void bt_led_restart(void);

#endif
