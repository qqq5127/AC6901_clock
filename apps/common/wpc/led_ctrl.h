#ifndef _WPC_LED_H_
#define _WPC_LED_H_

#include "typedef.h"
#include "sdk_cfg.h"

#define LED_STATUS_OFF          0
#define LED_STATUS_ON           1
#define LED_STATUS_BREATH       2
#define LED_STATUS_BLINK_S      3
#define LED_STATUS_BLINK_F      4

#define LED_RED_PORT            JL_PORTA
#define LED_RED_BIT             5
#define LED_BLUE_PORT           JL_PORTA
#define LED_BLUE_BIT            6
#define LED_GREEN_PORT          JL_PORTA
#define LED_GREEN_BIT           7

enum {
    WPC_LED_RED = 0,
#if (LED_LIGHT_NUM > 1)
    WPC_LED_BLUE,
#if (LED_LIGHT_NUM > 2)
    WPC_LED_GREEN,
#endif
#endif
    WPC_LED_MAX,
};
extern void wpc_led_struct_init(void);
extern void wpc_led_struct_set(u8 group, u8 set);

#endif

