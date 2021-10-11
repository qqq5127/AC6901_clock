#include "led/pwm_led.h"
#include "led/led.h"
#include "sdk_cfg.h"
#include "rtc_api.h"


#if((LED_TYPE_SEL == LED_PRX) && (LED_CONTROL_SEL == LED_PWM_TYPE))
void led_pwm_portr_init(void)
{
    u8 osc = 0;

    led_use_portr_config(1, LED_PORTR);

#if (LED_PORTR == PORTR0)
    X32EN(0);
#endif

    LED_PWM_EN(0);
    LED_PWM_BREATHE_EN(0);  //呼吸灯使能
    RESET_P3_PWM_CON1();

#if (LOWPOWER_OSC_TYPE == BT_OSC)
    osc = LED_BTOSC;
#elif (LOWPOWER_OSC_TYPE == LRC_32K)
    osc = LED_LRC32K;
#elif (LOWPOWER_OSC_TYPE == RTC_OSCH || LOWPOWER_OSC_TYPE == RTC_OSCL)
    osc = LED_RCLK;
#else
    osc = LED_OSC32K;
#endif
    LED_PWM_SSEL(osc);

    switch (osc) {
    case LED_RCLK:		//250KHz
        LED_PWM0_CLK_DIV(CLK_DIV_16);
        break;
    case LED_BTOSC: 	//24MHz
        LED_PWM0_CLK_DIV(CLK_DIV_x2_x256(CLK_DIV_4));
        break;
    case LED_OSC32K:	//32KHz
        LED_PWM0_CLK_DIV(CLK_DIV_x2(CLK_DIV_1));
        break;
    case LED_LRC32K:	//32KHz
        LED_PWM0_CLK_DIV(CLK_DIV_x2(CLK_DIV_1));
        break;
    default:
        break;
    }

    //pwm0 set,set brightness
    LED_DUTY_CYCLE(255);
    LED_DUTY_SET(255 / 2);

    LED_OUT_LOGIC(1);       //亮灯电平

    LED_PWM_PORTR_INIT();
}

void led_pwm_portr_close(void)
{
    LED_PWM_EN(0);
}

void led_pwm_portr_fre_set(u8 fre_mode)
{
#define LED_CLOSE_CNT 					  230   //值越大灯灭的时间越长，0-255

    u8 duty = 0;
    u8 duty1 = (256 - LED_CLOSE_CNT) / 3;
    u8 led_type = LED_RED;

    LED_PWM_EN(0);

    switch (fre_mode) {
    case C_BLED_ON:
    case C_BLED_SLOW:
    case C_BLED_FAST:
    case C_BLED_FAST_DOBLE_5S:
    case C_BLED_FAST_ONE_5S:
    case C_BLED_BREATHE:
        led_type = LED_BLUE;
        break;

    case C_RLED_ON:
    case C_RLED_SLOW:
    case C_RLED_FAST:
    case C_RLED_FAST_DOBLE_5S:
    case C_RLED_FAST_ONE_5S:
    case C_RLED_BREATHE:
        led_type = LED_RED;
        break;

    default:
        break;
    }

    //pwm1 set,set flicker fre
    /* LED_PWM1_CLK_DIV(256); */

    switch (fre_mode) {
    case C_BLED_ON:
    case C_RLED_ON:
        LED_PWM_EDGE1(1);
        PORTR_DIE(LED_PORTR, 0);
        PORTR_OUT(LED_PORTR, led_type);
        break;
    case C_ALL_ON:
        PORTR_DIE(LED_PORTR, 1);
        PORTR_PWM_OE(LED_PORTR, 0);
        LED_PWM_EDGE1(1);
        LED_PWM1_CLK_DIV(1);
        break;
    case C_BLED_OFF:
    case C_RLED_OFF:
    case C_ALL_OFF:
        LED_PWM_EDGE1(0);
        PORTR_DIE(LED_PORTR, 0);
        PORTR_OUT(LED_PORTR, 0);
        break;
    case C_BLED_SLOW:
    case C_RLED_SLOW:
        /* LED_PWM0_CLK_DIV(CLK_DIV_x2_x256(CLK_DIV_1)); */
        LED_PWM1_CLK_DIV(LED_PWM_FRE_SLOW_SPEED);
        LED_DUTY0_SET(220);
        LED_DUTY0_EN(1);
        PORTR_DIE(LED_PORTR, 0);
        PORTR_OUT(LED_PORTR, led_type);
        break;
    case C_BLED_FAST:
    case C_RLED_FAST:
        /* LED_PWM0_CLK_DIV(CLK_DIV_x2(CLK_DIV_64)); */
        LED_PWM1_CLK_DIV(LED_PWM_FRE_FAST_SPEED);
        LED_DUTY0_SET(220);
        LED_DUTY0_EN(1);
        PORTR_DIE(LED_PORTR, 0);
        PORTR_OUT(LED_PORTR, led_type);
        break;
    case C_BLED_FAST_DOBLE_5S:
    case C_RLED_FAST_DOBLE_5S:
        /* LED_PWM0_CLK_DIV(CLK_DIV_x2_x256(CLK_DIV_4)); */
        LED_PWM1_CLK_DIV(256);
        duty += (LED_CLOSE_CNT - 1);
        LED_DUTY0_SET(duty);
        LED_DUTY0_EN(1);

        duty += duty1;
        LED_DUTY1_SET(duty);
        LED_DUTY1_EN(1);

        duty += duty1;
        LED_DUTY2_SET(duty);
        LED_DUTY2_EN(1);

        PORTR_DIE(LED_PORTR, 0);
        PORTR_OUT(LED_PORTR, led_type);
        break;
    case C_BLED_FAST_ONE_5S:
    case C_RLED_FAST_ONE_5S:
        /* LED_PWM0_CLK_DIV(CLK_DIV_x2_x256(CLK_DIV_4)); */
        LED_PWM1_CLK_DIV(256);
        LED_DUTY0_SET(250);
        LED_DUTY0_EN(1);
        PORTR_DIE(LED_PORTR, 0);
        PORTR_OUT(LED_PORTR, led_type);
        break;
    case C_RB_SLOW:
        /* LED_PWM0_CLK_DIV(CLK_DIV_x256(CLK_DIV_1)); */
        LED_PWM1_CLK_DIV(LED_PWM_FRE_SLOW_SPEED);
        LED_DUTY0_SET(230);
        LED_DUTY0_EN(1);
        LED_SHIFT_DUTY(1);
        break;
    case C_RB_FAST:
        LED_PWM1_CLK_DIV(LED_PWM_FRE_FAST_SPEED);
        LED_DUTY0_SET(180);
        LED_DUTY0_EN(1);
        LED_SHIFT_DUTY(1);
        break;
    case C_BLED_BREATHE:
    case C_RLED_BREATHE:
        LED_DUTY0_SET(LED_BREATHE_BRIGHTNESS_MAX_DELAY);
        LED_DUTY0_EN(1);
        LED_DUTY1_SET(LED_BREATHE_BRIGHTNESS_MIN_DELAY);
        LED_DUTY1_EN(1);
        LED_DUTY_CYCLE(LED_BREATHE_BRIGHTNESS_LEVEL);
        /* LED_PWM0_CLK_DIV(CLK_DIV_x2_x256(CLK_DIV_1)); */
        LED_PWM1_CLK_DIV(150);

        LED_PWM_BREATHE_EN(1);  //呼吸灯使能
        LED_PWM_EDGE0(1);
        LED_PWM_EDGE1(1);
        PORTR_DIE(LED_PORTR, 0);
        PORTR_OUT(LED_PORTR, led_type);
        break;
    case C_RB_BREATHE:
        LED_DUTY0_SET(LED_BREATHE_BRIGHTNESS_MAX_DELAY);
        LED_DUTY0_EN(1);
        LED_DUTY1_SET(LED_BREATHE_BRIGHTNESS_MIN_DELAY);
        LED_DUTY1_EN(1);

        LED_DUTY_CYCLE(LED_BREATHE_BRIGHTNESS_LEVEL);
        /* LED_PWM0_CLK_DIV(CLK_DIV_x2_x256(CLK_DIV_1)); */
        LED_PWM1_CLK_DIV(100);

        LED_PWM_BREATHE_EN(1);  //呼吸灯使能
        LED_SHIFT_DUTY(2);
        LED_PWM_EDGE0(1);
        LED_PWM_EDGE1(1);
        break;
    default:
        return;
    }

    LED_PWM_EN(1);
}

void led_pwm_demo(void)
{
    printf("led_pwm_demo...\n");

    led_pwm_portr_init();

    /* led_pwm_portr_fre_set(C_ALL_OFF); */
    /* led_pwm_portr_fre_set(C_ALL_ON); */
    /* led_pwm_portr_fre_set(C_BLED_ON); */
    /* led_pwm_portr_fre_set(C_BLED_OFF); */
    led_pwm_portr_fre_set(C_BLED_SLOW);
    /* led_pwm_portr_fre_set(C_BLED_FAST); */
    /* led_pwm_portr_fre_set(C_BLED_FAST_DOBLE_5S); */
    /* led_pwm_portr_fre_set(C_BLED_FAST_ONE_5S); */
    /* led_pwm_portr_fre_set(C_RLED_ON); */
    /* led_pwm_portr_fre_set(C_RLED_OFF); */
    /* led_pwm_portr_fre_set(C_RLED_SLOW); */
    /* led_pwm_portr_fre_set(C_RLED_FAST); */
    /* led_pwm_portr_fre_set(C_RLED_FAST_DOBLE_5S); */
    /* led_pwm_portr_fre_set(C_RLED_FAST_ONE_5S); */
    /* led_pwm_portr_fre_set(C_RB_SLOW); */
    /* led_pwm_portr_fre_set(C_RB_FAST); */
    /* led_pwm_portr_fre_set(C_RLED_BREATHE); */
    /* led_pwm_portr_fre_set(C_BLED_BREATHE); */
    /* led_pwm_portr_fre_set(C_RB_BREATHE); */

    extern void clear_wdt(void);
    while (1) {
        clear_wdt();
    }
}
#endif
