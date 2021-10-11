#ifndef _LED_H_
#define _LED_H_

#include "includes.h"
#include "sys_detect.h"
#include "sdk_cfg.h"
#include "led/pwm_led.h"
#include "rtc_api.h"

void led_pwm_init(void);
void led_pwm_close(void);


enum {
    C_ALL_OFF = 1,         ///全灭
    C_ALL_ON,              ///全亮

    C_BLED_ON,             ///蓝亮
    C_BLED_OFF,            ///蓝灭
    C_BLED_SLOW,           ///蓝慢闪
    C_BLED_FAST,           ///蓝快闪
    C_BLED_FAST_DOBLE_5S,  ///蓝灯5秒闪连闪两下
    C_BLED_FAST_ONE_5S,    ///蓝灯5秒闪连闪1下

    C_RLED_ON,             ///红亮
    C_RLED_OFF,            ///红灭
    C_RLED_SLOW,           ///红慢闪
    C_RLED_FAST,           ///红快闪
    C_RLED_FAST_DOBLE_5S,  //////红灯5秒闪连闪两下
    C_RLED_FAST_ONE_5S,    //////红灯5秒闪连闪1下

    C_RB_FAST,              ///红蓝交替闪（快闪）
    C_RB_SLOW,              ///红蓝交替闪（慢闪）

    C_BLED_BREATHE,			///蓝灯呼吸灯模式,目前用PR口推灯才支持
    C_RLED_BREATHE,			///红灯呼吸灯模式,目前用PR口推灯才支持
    C_RB_BREATHE,			///红蓝交替呼吸灯模式,目前用PR口推灯才支持
};

#define C_ALL_OFF_MODE               ((0<<8)|(C_ALL_OFF))             ///全灭
#define C_ALL_ON_MODE                ((0<<8)|(C_ALL_ON))              ///全亮

#define C_BLED_ON_MODE               ((0<<8)|(C_BLED_ON))             ///蓝亮
#define C_BLED_OFF_MODE              ((0<<8)|(C_BLED_OFF))            ///蓝灭
#define C_BLED_SLOW_MODE             ((50<<8)|(C_BLED_SLOW))          ///蓝慢闪
#define C_BLED_FAST_MODE             ((20<<8)|(C_BLED_FAST))          ///蓝快闪
#define C_BLED_FAST_DOBLE_5S_MODE    ((500<<8)|(C_BLED_FAST_DOBLE_5S))   ///蓝灯5秒闪连闪两下
#define C_BLED_FAST_ONE_5S_MODE      ((500<<8)|(C_BLED_FAST_ONE_5S))   ///蓝灯5秒闪连闪两下


#define C_RLED_ON_MODE               ((0<<8)|(C_RLED_ON))             ///红亮
#define C_RLED_OFF_MODE              ((0<<8)|(C_RLED_OFF))            ///红灭
#define C_RLED_SLOW_MODE             ((50<<8)|(C_RLED_SLOW))          ///红慢闪
#define C_RLED_FAST_MODE             ((20<<8)|(C_RLED_FAST))          ///红快闪
#define C_RLED_FAST_DOBLE_5S_MODE    ((500<<8)|(C_RLED_FAST_DOBLE_5S))///红灯5秒闪连闪两下
#define C_RLED_FAST_ONE_5S_MODE      ((500<<8)|(C_RLED_FAST_ONE_5S))  ///红灯5秒闪连闪1下

#define C_RB_FAST_MODE               ((80<<8)|(C_RB_FAST))          ///红蓝交替闪（快闪）
#define C_RB_SLOW_MODE               ((160<<8)|(C_RB_SLOW))          ///红蓝交替闪（慢闪）

#define C_RLED_LOWER_POWER_MODE        C_RLED_SLOW_MODE               ///低电红慢闪

//LED端口选择
#define    LED_DM_TWO_LED       0                     //用usb 的DM同时推两个灯R_LED, B_LED,推灯注意开关灯的顺序
#define    LED_DM_DP            1                     //用UBS DM/DP分别推两盏灯
#define    LED_PRX              2                     //用PR口推灯
#define    LED_PXX              3                     //PORTA/B/C/D推灯
#define    LED_NULL             4

//LED驱动方式选择选择
#define    LED_NORMAL_TYPE      0                     //普通方式推灯
#define    LED_PWM_TYPE         1                     //PWM方式推灯
#define    LED_SNIFF_TYPE       2                     //sniff方式推灯



#if (BT_LOW_POWER_MODE || SNIFF_MODE_CONF)            //进低功耗模式只能用PR口推灯,一个PR口同时推两个灯

#define LED_TYPE_SEL            LED_PXX
#define LED_CONTROL_SEL         LED_SNIFF_TYPE

// #define LED_TYPE_SEL            LED_PRX
// #define LED_CONTROL_SEL         LED_PWM_TYPE
#else
#define LED_TYPE_SEL            LED_PXX                 //LED_DM_DP
#define LED_CONTROL_SEL         LED_NORMAL_TYPE
#endif  //endif BT_LOW_POWER_MODE


#if(LED_TYPE_SEL == LED_DM_TWO_LED)                   //用pwm推灯设置,用usb 的DM同时推两个灯R_LED, B_LED,只能用普通IO口推
#undef  LED_CONTROL_SEL
#define LED_CONTROL_SEL         LED_NORMAL_TYPE
#endif

#if(LED_TYPE_SEL == LED_PRX)                          //PR口推灯端口选择
#define LED_PORTR 		        PORTR1
#elif(LED_TYPE_SEL == LED_PXX)                        //普通IO推灯端口选择
#define LED_B_PORTX               //JL_PORTA
#define LED_BLUE                  //BIT(4)
#define LED_R_PORTX               //JL_PORTA
#define LED_RED                   //BIT(3)
#endif


#if(LED_CONTROL_SEL == LED_PWM_TYPE)            //使用PWM方式推灯
#define LED_INIT_EN()     	    do{led_pwm_init();}while(0)
#define LED_INIT_DIS()		    do{led_pwm_close();}while(0)
#if(LED_TYPE_SEL == LED_DM_DP)
#define R_LED_ON()              do{USB_DP_PU(1);USB_DP_PD(1);USB_DP_DIR(0);}while(0)
#define R_LED_OFF()             do{USB_DP_PU(0);USB_DP_PD(0);USB_DP_DIR(1);}while(0)
#define B_LED_ON()              do{USB_DM_PU(1);USB_DM_PD(1);USB_DM_DIR(0);}while(0)
#define B_LED_OFF()             do{USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(1);}while(0)
#elif(LED_TYPE_SEL == LED_PRX)
#undef  LED_INIT_EN
#undef  LED_INIT_DIS
#define LED_INIT_EN()     	    do{led_pwm_portr_init();}while(0)
#define LED_INIT_DIS()		    do{led_pwm_portr_close();}while(0)
#define R_LED_ON()              do{led_fre_set(C_RLED_ON_MODE);}while(0)
#define R_LED_OFF()      	    do{led_fre_set(C_RLED_OFF_MODE);}while(0)
#define B_LED_ON()       	    do{led_fre_set(C_BLED_ON_MODE);}while(0)
#define B_LED_OFF()      	    do{led_fre_set(C_BLED_OFF_MODE);}while(0)
#elif(LED_TYPE_SEL == LED_PXX)
#define B_LED_ON()			    do{LED_PORTX->PU |=  LED_BLUE;LED_PORTX->PD |=  LED_BLUE;LED_PORTX->DIR &= ~LED_BLUE;LED_PORTX->OUT &= ~LED_BLUE;LED_PORTX->DIE &= ~LED_BLUE;}while(0)
#define B_LED_OFF()		    	do{LED_PORTX->PU &= ~LED_BLUE;LED_PORTX->PD &= ~LED_BLUE;LED_PORTX->DIR |=  LED_BLUE;}while(0)
#define R_LED_ON()			    do{LED_PORTX->PU |=  LED_RED;LED_PORTX->PD |=  LED_RED;LED_PORTX->DIR &= ~LED_RED;LED_PORTX->OUT &= ~LED_RED;LED_PORTX->DIE &= ~LED_RED;}while(0)
#define R_LED_OFF()			    do{LED_PORTX->PU &= ~LED_RED;LED_PORTX->PD &= ~LED_RED;LED_PORTX->DIR |=  LED_RED;}while(0)
#else
#error "PLEASE CHECK CONFIG"
#endif          //LED_DM_DP
#else                                          //使用非PWM方式推灯
#if(LED_TYPE_SEL == LED_DM_DP)                 //非PWM方式下用DM/DP推灯
#define LED_INIT_EN()           do{USB_DP_PU(0);USB_DP_PD(0);USB_DP_DIR(0);USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(0);}while(0)
#define LED_INIT_DIS()          do{USB_DP_PU(0);USB_DP_PD(0);USB_DP_DIR(1);USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(1);}while(0)
#define R_LED_ON()              do{USB_DP_OUT(1);}while(0)
#define R_LED_OFF()             do{USB_DP_OUT(0);}while(0)
#define B_LED_ON()              do{USB_DM_OUT(1);}while(0)
#define B_LED_OFF()             do{USB_DM_OUT(0);}while(0)
#elif(LED_TYPE_SEL == LED_DM_TWO_LED)          //非PWM方式使用DM推2个灯
#define LED_INIT_EN(...)
#define LED_INIT_DIS()          do{USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(1);}while(0)
#define R_LED_ON()              do{USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(0);USB_DM_OUT(0);}while(0)
#define R_LED_OFF()             do{USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(1);}while(0)
#define B_LED_ON()              do{USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(0);USB_DM_OUT(1);}while(0)
#define B_LED_OFF()             do{USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(1);}while(0)
#elif(LED_TYPE_SEL == LED_PRX)                 //非PWM方式PR口推灯
#define LED_INIT_EN()           do{PORTR_PU(LED_PORTR,0);PORTR_PD(LED_PORTR,0);PORTR_DIR(LED_PORTR,0);}while(0)
#define LED_INIT_DIS()          do{PORTR_PU(LED_PORTR,0);PORTR_PD(LED_PORTR,0);PORTR_DIR(LED_PORTR,1);}}while(0)
#define B_LED_ON()              do{PORTR_OUT(LED_PORTR,1);}while(0)
#define B_LED_OFF()             do{PORTR_OUT(LED_PORTR,0);}while(0)
#define R_LED_ON(...)
#define R_LED_OFF(...)
#elif(LED_TYPE_SEL == LED_PXX)                 //非PWM方式下使用普通IO口推灯
#define B_LED_ON()			    //LED_B_PORTX->OUT |= LED_BLUE;LED_B_PORTX->DIR &= ~LED_BLUE;//do{LED_PORTX->OUT |=  LED_BLUE;}while(0)
#define B_LED_OFF()			    //LED_B_PORTX->OUT &= ~LED_BLUE;LED_B_PORTX->DIR &= ~LED_BLUE;//do{LED_PORTX->OUT &= ~LED_BLUE;}while(0)
#define B_LED_INIT()     	    //B_LED_OFF()//do{LED_PORTX->PU &= ~LED_BLUE;LED_PORTX->PD &= ~LED_BLUE;LED_PORTX->DIR &= ~LED_BLUE;LED_PORTX->PU &= ~LED_RED;LED_PORTX->PD &= ~LED_RED;LED_PORTX->DIR &= ~LED_RED;}while(0)
#define R_LED_ON()			    //LED_R_PORTX->OUT |= LED_RED;LED_R_PORTX->DIR &= ~LED_RED;//do{LED_PORTX->OUT |=  LED_RED;}while(0)
#define R_LED_OFF()			    //LED_R_PORTX->OUT &= ~LED_RED;LED_R_PORTX->DIR &= ~LED_RED;//do{LED_PORTX->OUT &= ~LED_RED;}while(0)
#define R_LED_INIT()		    //R_LED_OFF()//do{LED_PORTX->PU &= ~LED_BLUE;LED_PORTX->PD &= ~LED_BLUE;LED_PORTX->DIR |=  LED_BLUE;LED_PORTX->PU &= ~LED_RED;LED_PORTX->PD &= ~LED_RED;LED_PORTX->DIR |=  LED_RED;}while(0)
#else
#error "LED NORMAL MODE DON‘T SUPPORT OTHER LED_TYPE"
#endif  //endif LED_DM_DP
#endif  //endif LED_PWM_TYPE
//#endif  //endif LED_PRX

void led_fre_set(u32 fre_type);

void led_bt_sniff_init(void);

void set_led_scan(u8 en);

void led_test(void);
void set_r_led_on_cnt(u8 cnt);
void lower_power_led_flash(u8 control, u32 led_flash);
void clear_led_cnt(void);
void clear_led_rb_flag(void);

#if (LED_RED_BLINK||LED_BLUE_BLINK)
void led_mode_on(void);
void led_mode_off(void);
//void led_bt_none(void);
//void led_bt_voice(void);
void led_bt_idle(void);
void led_bt_connect(void);
void led_bt_play(void);
void led_bt_update_name_end(void);
void led_idle(void);
//void led_busy(void);
void led_music_play(void);
void led_music_pause(void);
void led_music_idle(void);
void led_aux_play(void);
void led_aux_pause(void);
void led_fm_play(void);
void led_fm_pause(void);
void led_fm_scan(void);
#else
#define led_mode_on()
#define led_mode_off()
//#define led_bt_none()
//#define led_bt_voice()
#define led_bt_idle()
#define led_bt_connect()
#define led_bt_play()
#define led_bt_update_name_end()
#define led_idle()
//#define led_busy()
#define led_music_play()
#define led_music_pause()
#define led_music_idle()
#define led_aux_play()
#define led_aux_pause()
#define led_fm_play()
#define led_fm_pause()
#define led_fm_scan()
#endif

#endif

//#endif/*_LED_H_*/

