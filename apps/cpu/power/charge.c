#include "charge.h"
#include "power.h"
#include "sdk_cfg.h"
#include "rtc/rtc_api.h"
#include "led/led.h"
/* #include "sys_detect.h" */
#include "dac.h"
#include "key_drv/key.h"
#include "audio/dac_api.h"
#include "clock.h"
#include "timer.h"
#include "adc_api.h"
#include "irq_api.h"
#include "clock_api.h"
#include "power_manage_api.h"

//该宏配置是否开着机充电，当需要保持开机状态充电时，定义该宏
/* #define POWER_ON_CHARGE */

/* #define CHARGEE_DBG */
#ifdef  CHARGEE_DBG
#define charge_putchar        putchar
#define charge_printf         log_printf
#define charge_buf            printf_buf
#else
#define charge_putchar(...)
#define charge_printf(...)
#define charge_buf(...)
#endif    //CHARGEE_DBG


static void delay_ms()
{
    //Timer2 for delay
    JL_TIMER2->CON = BIT(14);
    JL_TIMER2->PRD = 375;
    JL_TIMER2->CNT = 0;
    SFR(JL_TIMER2->CON, 2, 2, 2); //use osc
    SFR(JL_TIMER2->CON, 4, 4, 3); //div64
    SFR(JL_TIMER2->CON, 14, 1, 1); //clr pending
    SFR(JL_TIMER2->CON, 0, 2, 1); //work mode
    while (!(JL_TIMER2->CON & BIT(15)));
    JL_TIMER2->CON = BIT(14);
}

void delay_nms(u32 n)
{
    while (n--) {
        delay_ms();
    }
}
