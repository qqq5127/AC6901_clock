#ifndef _PWM_LED_H_
#define _PWM_LED_H_

#include "typedef.h"

extern void P33_CON_SET(u16 addr, u8 start, u8 len, u8 data);
extern u8 P33_CON_GET(u8 addr);


#define LED_PWM_FRE_SLOW_SPEED            100	//慢闪的速度，值越大闪烁越慢 1-256
#define LED_PWM_FRE_FAST_SPEED            30	//快闪的速度，值越小闪烁越快 1-256
#define LED_BREATHE_BRIGHTNESS_LEVEL	  100	//呼吸灯亮度级数 0-255
#define LED_BREATHE_BRIGHTNESS_MAX_DELAY  0		//呼吸灯最高亮度延时 0-255
#define LED_BREATHE_BRIGHTNESS_MIN_DELAY  250	//呼吸灯灭灯延时 0-255

#define P3_PWM_CON0                       0x10
#define P3_PWM_CON1                       0x11
#define P3_BRI_PRD                        0x12
#define P3_BRI_DUTY                       0x13
#define P3_PRD_DIV                        0x14
#define P3_PWM_DUTY0                      0x15
#define P3_PWM_DUTY1                      0x16
#define P3_PWM_DUTY2                      0x17

//----------------------------P3_PWM_CON0----------------------------------//
#define RESET_P3_PWM_CON0()			P33_CON_SET(P3_PWM_CON0, 0, 8, 0)
//0:moudle close 1:moudle open
#define LED_PWM_EN(x)				P33_CON_SET(P3_PWM_CON0, 0, 1, x)
//0:breathe close 1:breathe open
#define LED_PWM_BREATHE_EN(x)		P33_CON_SET(P3_PWM_CON0, 1, 1, x)
//0:rclk(250kHz) 1:btosc(24MHz) 2:osc32k(32kHz) 3:rc32k(32kHz)
#define LED_PWM_SSEL(x)			    P33_CON_SET(P3_PWM_CON0, 2, 2, x)

//----------------------------P3_PWM_CON1----------------------------------//
#define RESET_P3_PWM_CON1()			P33_CON_SET(P3_PWM_CON1, 0, 8, 0)
//0:pwm 不取反    1:pwm 取反
#define LED_PWM_EDGE0(x)			P33_CON_SET(P3_PWM_CON1, 0, 1, x)
//0:pwm 不取反    1:pwm 取反
#define LED_PWM_EDGE1(x)			P33_CON_SET(P3_PWM_CON1, 1, 1, x)
//00:不变色  01:一个周期变色 10:两个周期变色 11:三个周期变色
#define LED_SHIFT_DUTY(x)			P33_CON_SET(P3_PWM_CON1, 2, 2, x)
//0:低电平亮 1:高电平亮
#define LED_OUT_LOGIC(x)			P33_CON_SET(P3_PWM_CON1, 4, 1, x)
//灯亮灭控制0/呼吸灯最高亮度延时开关
#define LED_DUTY0_EN(x)				P33_CON_SET(P3_PWM_CON1, 5, 1, x)
//灯亮灭控制1/呼吸灯最高亮度延时开关
#define LED_DUTY1_EN(x)				P33_CON_SET(P3_PWM_CON1, 6, 1, x)
//灯亮灭控制2
#define LED_DUTY2_EN(x)				P33_CON_SET(P3_PWM_CON1, 7, 1, x)


//----------------------------LED PWM0 SET----------------------------------//
#define LED_DUTY_CYCLE(x)			P33_CON_SET(P3_BRI_PRD, 0, 8, x)
#define LED_DUTY_SET(x)				P33_CON_SET(P3_BRI_DUTY, 0, 8, x)
//[1:0] 00:div1  01:div4  10:div16  11:div64
//[2]:  0:x1     1:x2
//[3]:  0:x1     1:x256
//DIV = [1:0] * [2] * [3]
#define LED_PWM0_CLK_DIV(x)			P33_CON_SET(P3_PWM_CON0, 4, 4, x)

//----------------------------LED PWM1 SET----------------------------------//
#define LED_DUTY0_SET(x)			P33_CON_SET(P3_PWM_DUTY0, 0, 8, x)
#define LED_DUTY1_SET(x)			P33_CON_SET(P3_PWM_DUTY1, 0, 8, x)
#define LED_DUTY2_SET(x)			P33_CON_SET(P3_PWM_DUTY2, 0, 8, x)

//[7:0]
#define LED_PWM1_CLK_DIV(x)	   	    P33_CON_SET(P3_PRD_DIV, 0, 8, x-1)


#define LED_PWM_PORTR_INIT()     	do{PORTR_DIR(LED_PORTR, 0);PORTR_PU(LED_PORTR, 1);PORTR_PD(LED_PORTR, 1);\
							   			PORTR_DIE(LED_PORTR, 1);PORTR_PWM_OE(LED_PORTR, 1);\
							  		}while(0)

//PWM0 CLK DIV
#define	CLK_DIV_1 			0
#define CLK_DIV_4           1
#define CLK_DIV_16          2
#define CLK_DIV_64          3

#define CLK_DIV_x2(x)       (0x04 | x)
#define CLK_DIV_x256(x)		(0x08 | x)
#define CLK_DIV_x2_x256(x)	(0x0c | x)


enum {
    LED_RED,
    LED_BLUE,
};

enum {
    LED_RCLK,
    LED_BTOSC,
    LED_OSC32K,
    LED_LRC32K,
};


void led_pwm_portr_init(void);
void led_pwm_portr_close(void);
void led_pwm_portr_fre_set(u8 mode);
void led_pwm_demo(void);


#endif //_PWM_LED_H_
