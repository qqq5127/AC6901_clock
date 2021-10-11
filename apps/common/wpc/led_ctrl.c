#include "sdk_cfg.h"
#include "led_ctrl.h"
#include "common/common.h"

#define LED_DO_PERIOD           100                         //uS 执行一次的时间
#define LED_BLINK_SLOW_PERIOD   (3250000/LED_DO_PERIOD)     //uS 闪烁的周期
#define LED_BLINK_SLOW_ON_TIME  (250000/LED_DO_PERIOD)      //uS 闪烁时亮灯的时间
#define LED_BLINK_FAST_PERIOD   (500000/LED_DO_PERIOD)      //uS 闪烁的周期
#define LED_BLINK_FAST_ON_TIME  (250000/LED_DO_PERIOD)      //uS 闪烁时亮灯的时间
#define LED_BREATH_PERIOD       (1000000/LED_DO_PERIOD/100) //uS 呼吸灯的周期

typedef struct {
    int status;
    void (*led_init)(void);
    void (*led_on)(void);
    void (*led_off)(void);
} led_struct;

static led_struct led[WPC_LED_MAX];
static u8 led_flag;

AT_RAM
static void led_red_off(void)
{
#if (LED_LIGHT_LVL == 0)
    LED_RED_PORT->OUT |=  BIT(LED_RED_BIT);
#else
    LED_RED_PORT->OUT &= ~BIT(LED_RED_BIT);
#endif
}

AT_RAM
static void led_red_on(void)
{
#if (LED_LIGHT_LVL == 0)
    LED_RED_PORT->OUT &= ~BIT(LED_RED_BIT);
#else
    LED_RED_PORT->OUT |=  BIT(LED_RED_BIT);
#endif
}

AT_RAM
static void led_red_init(void)
{
    LED_RED_PORT->DIR &= ~BIT(LED_RED_BIT);
    LED_RED_PORT->PU  &= ~BIT(LED_RED_BIT);
    LED_RED_PORT->PD  &= ~BIT(LED_RED_BIT);
    led_red_off();
}

AT_RAM
static void led_blue_off(void)
{
#if (LED_LIGHT_LVL == 0)
    LED_RED_PORT->OUT |=  BIT(LED_BLUE_BIT);
#else
    LED_RED_PORT->OUT &= ~BIT(LED_BLUE_BIT);
#endif
}

AT_RAM
static void led_blue_on(void)
{
#if (LED_LIGHT_LVL == 0)
    LED_BLUE_PORT->OUT &= ~BIT(LED_BLUE_BIT);
#else
    LED_BLUE_PORT->OUT |=  BIT(LED_BLUE_BIT);
#endif
}

AT_RAM
static void led_blue_init(void)
{
    LED_BLUE_PORT->DIR &= ~BIT(LED_BLUE_BIT);
    LED_BLUE_PORT->PU  &= ~BIT(LED_BLUE_BIT);
    LED_BLUE_PORT->PD  &= ~BIT(LED_BLUE_BIT);
    led_blue_off();
}

AT_RAM
static void led_green_off(void)
{
#if (LED_LIGHT_LVL == 0)
    LED_GREEN_PORT->OUT |=  BIT(LED_GREEN_BIT);
#else
    LED_GREEN_PORT->OUT &= ~BIT(LED_GREEN_BIT);
#endif
}

AT_RAM
static void led_green_on(void)
{
#if (LED_LIGHT_LVL == 0)
    LED_GREEN_PORT->OUT &= ~BIT(LED_GREEN_BIT);
#else
    LED_GREEN_PORT->OUT |=  BIT(LED_GREEN_BIT);
#endif
}

AT_RAM
static void led_green_init(void)
{
    LED_GREEN_PORT->DIR &= ~BIT(LED_GREEN_BIT);
    LED_GREEN_PORT->PU  &= ~BIT(LED_GREEN_BIT);
    LED_GREEN_PORT->PD  &= ~BIT(LED_GREEN_BIT);
    led_green_off();
}

AT_RAM
static void led_control_on(u8 status)
{
    u8 i;
    for (i = 0; i < WPC_LED_MAX; i ++) {
        if (led[i].status == status) {
            led[i].led_on();
        }
    }
}

AT_RAM
static void led_control_off(u8 status)
{
    u8 i;
    for (i = 0; i < WPC_LED_MAX; i ++) {
        if (led[i].status == status) {
            led[i].led_off();
        }
    }
}

AT_RAM
static void led_breath_do(void)
{
    static char led_count = 0;
    static char duty_cycle = 0;
    static char flag = 0;
    u8 i;
    led_count++;
    if (led_count >= LED_BREATH_PERIOD) {
        led_count = 0;
        if (flag) {
            duty_cycle++;
        } else {
            duty_cycle--;
        }
        if ((duty_cycle >= 99) || (duty_cycle <= -1)) {
            flag = !flag;
        }
    }
    if (led_count < duty_cycle) {
        led_control_on(LED_STATUS_BREATH);
    } else {
        led_control_off(LED_STATUS_BREATH);
    }
}

AT_RAM
static void led_blink_slow_do(void)
{
    static u16 led_count;
    led_count++;
    if (led_count >= LED_BLINK_SLOW_PERIOD) {
        led_count = 0;
    }
    if (led_count >= LED_BLINK_SLOW_ON_TIME) {
        led_control_on(LED_STATUS_BLINK_S);
    } else {
        led_control_off(LED_STATUS_BLINK_S);
    }
}

AT_RAM
static void led_blink_fast_do(void)
{
    static u16 led_count;
    led_count++;
    if (led_count >= LED_BLINK_FAST_PERIOD) {
        led_count = 0;
    }
    if (led_count >= LED_BLINK_FAST_ON_TIME) {
        led_control_on(LED_STATUS_BLINK_F);
    } else {
        led_control_off(LED_STATUS_BLINK_F);
    }
}

AT_RAM
static void led_on_off_do(void)
{
    led_control_on(LED_STATUS_ON);
    led_control_off(LED_STATUS_OFF);
}

AT_RAM
static void led_tmr_callback(void *parm)
{
    if (!led_flag) {
        return;
    }
    led_breath_do();
    led_blink_slow_do();
    led_blink_fast_do();
    led_on_off_do();
}

USLOOP_DETECT_REGISTER(wpc_led) = {
    .time = LED_DO_PERIOD,
    .fun  = led_tmr_callback,
};

void wpc_led_struct_init(void)
{
    u8 i;
    led[WPC_LED_RED].status   = LED_STATUS_OFF;
    led[WPC_LED_RED].led_init = led_red_init;
    led[WPC_LED_RED].led_on   = led_red_on;
    led[WPC_LED_RED].led_off  = led_red_off;
#if (LED_LIGHT_NUM > 1)
    led[WPC_LED_BLUE].status   = LED_STATUS_OFF;
    led[WPC_LED_BLUE].led_init = led_blue_init;
    led[WPC_LED_BLUE].led_on   = led_blue_on;
    led[WPC_LED_BLUE].led_off  = led_blue_off;
#if (LED_LIGHT_NUM > 2)
    led[WPC_LED_GREEN].status   = LED_STATUS_OFF;
    led[WPC_LED_GREEN].led_init = led_green_init;
    led[WPC_LED_GREEN].led_on   = led_green_on;
    led[WPC_LED_GREEN].led_off  = led_green_off;
#endif
#endif

    for (i = 0; i < WPC_LED_MAX; i ++) {
        if (led[i].led_init) {
            led[i].led_init();
        }
    }
    led_flag = 1;
}

void wpc_led_struct_set(u8 group, u8 set)
{
    led[group].status = set;
}




