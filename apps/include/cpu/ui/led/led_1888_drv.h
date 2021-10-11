#ifndef	_led1888_DRV_H_
#define _led1888_DRV_H_

#include "includes.h"

#if LED_1888_EN

typedef struct _led1888_VAR {
    u8  bCoordinateX;
    u8  bFlashChar;
    u8  bFlashIcon;
    u8  bShowBuff[5];
    u8  bBrightness;
    u8  bShowBuff1[9];
} led1888_VAR;


void led1888_init(void);
void led1888_scan(void *param);
void led1888_clear_icon(void);
void led1888_flash(void);
void led1888_show_char(u8 chardata);
void led1888_show_number(u8 number);
void led1888_show_Hi(void);
void led1888_show_music_main(void);
void led1888_show_RTC_main(void);
void led1888_show_filenumber(void);
void led1888_show_volume(s32 vol);
void led1888_show_fmtx_main(void);
void led7_show_fm_main(void);
void led1888_show_IR_number(s32);
void led1888_show_pc_main(void);
void led1888_show_pc_vol_up(void);
void led1888_show_pc_vol_down(void);
void led1888_show_aux_main(void);
void led1888_show_eq(s32 arg);
void led1888_show_playmode(s32 arg);
void led1888_show_pause(void);
void led1888_show_fm_station(void);
void led1888_show_waiting(void);
void led1888_show_alarm(void);
void led1888_show_rec_main(void);
void led1888_show_linin_main(u8 menu);
void led1888_clear(void);
void led1888_show_string_menu(u8 menu);
void led1888_show_bt_main(u8 menu);
void led1888_setX(u8 X);
void led1888_show_power(void);

#define LED_STATUS  led1888_var.bShowBuff[4]

#define LED_A   BIT(0)
#define LED_B	BIT(1)
#define LED_C	BIT(2)
#define LED_D	BIT(3)
#define LED_E	BIT(4)
#define LED_F	BIT(5)
#define LED_G	BIT(6)
#define LED_H	BIT(7)

//for LED0
#define LED_PLAY	LED_A
#define LED_PAUSE	LED_B
#define LED_USB		LED_C
#define LED_SD		LED_D
#define LED_2POINT	LED_E
#define LED_MHZ	    LED_F
#define LED_DOT		LED_G
#define LED_MP3     LED_H


#define  LEDN_S0_PORT         JL_PORTC
#define  LEDN_S1_PORT         JL_PORTC
#define  LEDN_S2_PORT         JL_PORTC
#define  LEDN_S3_PORT         JL_PORTA
#define  LEDN_S4_PORT         JL_PORTA
#define  LEDN_S5_PORT         JL_PORTA


#define  LEDN_S0_BIT   5
#define  LEDN_S1_BIT   4
#define  LEDN_S2_BIT   3
#define  LEDN_S3_BIT   10
#define  LEDN_S4_BIT   6
#define  LEDN_S5_BIT   5

#endif

#endif	/*	_LED_H_	*/

