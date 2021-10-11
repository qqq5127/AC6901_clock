#ifndef	_LED7_DRV_H_
#define _LED7_DRV_H_

#include "includes.h"

#if LED_7_EN
typedef struct _led7_VAR {
    u8  bCoordinateX;
    u8  bFlashChar;
    u8  bFlashIcon;
    u8  bShowBuff[13];
    u8  bShowBuff_temp[13];
    u8  bBrightness;
    //u8  bShowBuff1[9];
} LED7_VAR;


//void set_LED_fade_out(void);
//void set_LED_all_on(void);

void led7_init(void);
void led7_scan(void *param);
void led7_clear_icon(void);
void led7_show_char(u8 chardata);
void led7_show_number(u8 number);
void led7_show_Hi(void);
void led7_show_music_main(void);
void led7_show_RTC_main(void);
void led7_show_filenumber(void);
void led7_show_volume(s32 vol);
void led7_show_fm_main(void);
void led7_show_IR_number(s32);
void led7_show_pc_main(void);
void led7_show_pc_vol_up(void);
void led7_show_pc_vol_down(void);
void led7_show_aux_main(void);
void led7_show_eq(s32 arg);
void led7_show_playmode(s32 arg);
void led7_show_pause(void);
void led7_show_fm_station(void);
void led7_show_waiting(void);
void led7_show_alarm(void);
void led7_show_alarm2(void);
//void led7_show_nop(void);
void led7_show_rec_main(void);
void led7_show_linin_main(u8 menu);
void led7_clear(void);
void led7_show_string_menu(u8 menu);
void led7_setX(u8 X);
void led7_show_power(void);
void led7_show_bt_main(u8 menu);
void menu_disp_bat(void);
void led7_show_RTC_time_format(void);
void led7_show_RTC_temp(void);
void led7_show_alarm_temp(void);
void led7_show_alarm2_temp(void);
void led7_show_auto_power_off(void);
void led7_show_fm_preset_station(void);
void led7_show_alarm_up(void);
void led7_show_display_test(u8 arg);
void led7_show_fm_clr(void);
void led7_show_auto_power_off_time(void);
void led7_show_alm_time(void);
void led7_show_alm2_time(void);
void led7_show_auto_power_off_icon(void);
void led7_show_display_version(void);
void led7_show_display_reset(void);
void led7_show_display_all(void);
//#define LED_STATUS  led7_var.bShowBuff[4]

#define LED_A   BIT(0)
#define LED_B	BIT(1)
#define LED_C	BIT(2)
#define LED_D	BIT(3)
#define LED_E	BIT(4)
#define LED_F	BIT(5)
#define LED_G	BIT(6)
#define LED_H	BIT(7)
#define LED_BIT_NULL	0
#if 1
///0

///1---led7_var.bShowBuff[1]
#define LED_2POINT	LED_H
///2---led7_var.bShowBuff[4]
#define LED_CD      LED_B
#define LED_USB     LED_G
#define LED_SD      LED_C
#define LED_BT      LED_H
#define LED_RANDOM  LED_D
#define LED_ONE     LED_F
#define LED_ALL     (LED_E|LED_F)
#define LED_FOLDER  LED_A
///3---led7_var.bShowBuff[5]
#define ICON_ALM1       LED_D
#define ICON_ALM2       LED_H
#define LED_FM          LED_E
#define ICON_SLEEP      LED_C
#define LED_AUX         LED_F
#define ICON_SNOOZ      LED_G
#define LED_MEMORY      LED_A
#define LED_AM          LED_B
///4---led7_var.bShowBuff[6]
#define LED_BAT	        LED_D
#define LED_MHZ_2       LED_E
#define LED_KHZ		    LED_F
#define ICON_PM         LED_A
///5---led7_var.bShowBuff[2]
#define LED_MHZ         LED_H
//
#define ICON_WDAY_1       0
#define ICON_WDAY_2       0
#define ICON_WDAY_3       0
#define ICON_WDAY_4       0
#define ICON_WDAY_5       0
#define ICON_WDAY_6       0
#define ICON_WDAY_7       0
#else
//for LED0
#define LED_PLAY	LED_BIT_NULL//LED_A
#define LED_PAUSE	LED_BIT_NULL//LED_B
#define LED_USB		LED_C
#define LED_SD		LED_BIT_NULL//LED_D
#define LED_2POINT	LED_H
#define LED_MHZ	    LED_H
#define LED_DOT		LED_BIT_NULL//LED_G
#define LED_MP3     LED_BIT_NULL//LED_H
#define LED_BT      LED_E
#define LED_FM	    LED_F
#define LED_AUX		LED_H
#define LED_MHZ_2   LED_A
#define LED_AM		LED_G
#define LED_KHZ		LED_B

//
#define LED_2POINT_ALM1    LED_H
//
#define LED_2POINT_ALM2    LED_H

#define ICON_PM         LED_H
#define ICON_DOT_UP     LED_BIT_NULL//LED_H
#define ICON_DOT_DN     LED_BIT_NULL//LED_H
#define ICON_ALM1       LED_H
#define ICON_ALM1_PM    LED_H
#define ICON_ALM2       LED_H
#define ICON_ALM2_PM    LED_H

#define LED_BAT	    LED_BIT_NULL
#define LED_BAT_LOW	LED_BIT_NULL
#define LED_BAT_1	LED_BIT_NULL
#define LED_BAT_2	LED_BIT_NULL
#define LED_BAT_3	LED_BIT_NULL

#define ICON_WDAY_1       LED_C
#define ICON_WDAY_2       LED_D
#define ICON_WDAY_3       LED_B
#define ICON_WDAY_4       LED_H
#define ICON_WDAY_5       LED_E
#define ICON_WDAY_6       LED_G
#define ICON_WDAY_7       LED_A

#define LEDSEG__	      LED_G

#define ICON_SLEEP        LED_H
#define ICON_SNOOZ        LED_H

#define ICON_1AGED        LED_G
#define ICON_5AGED        LED_G
#define ICON_11AGED       LED_G
#endif
#if 0
//占用PB3~PB9
#define  LEDN_PORT_OUT     JL_PORTB->OUT
#define  LEDN_PORT_HD      JL_PORTB->HD
#define  LEDN_PORT_DIR     JL_PORTB->DIR
#define  LEDN_PORT_PD      JL_PORTB->PD
#define  LEDN_PORT_PU      JL_PORTB->PU
//有HD1的端口才需要，PB、PD口必须要,PA、PC口可关闭
#define  LEDN_PORT_HD1      JL_PORTB->HD1


#define  LEDN_S0_BIT   0
#define  LEDN_S1_BIT   1
#define  LEDN_S2_BIT   2
#define  LEDN_S3_BIT   3
#define  LEDN_S4_BIT   4
#define  LEDN_S5_BIT   5
#define  LEDN_S6_BIT   6
#else
#if 0
extern u16 no_hd1_temp;

#define  LED_PORT0			JL_PORTB
#define  LEDN_S0_BIT   		0
#define  LEDN_PORT0_OUT     LED_PORT0->OUT
#define  LEDN_PORT0_HD      LED_PORT0->HD
#define  LEDN_PORT0_DIR     LED_PORT0->DIR
#define  LEDN_PORT0_PD      LED_PORT0->PD
#define  LEDN_PORT0_PU      LED_PORT0->PU
#define  LEDN_PORT0_HD1     LED_PORT0->HD1

#define  LED_PORT1			JL_PORTB
#define  LEDN_S1_BIT   		1
#define  LEDN_PORT1_OUT     LED_PORT1->OUT
#define  LEDN_PORT1_HD      LED_PORT1->HD
#define  LEDN_PORT1_DIR     LED_PORT1->DIR
#define  LEDN_PORT1_PD      LED_PORT1->PD
#define  LEDN_PORT1_PU      LED_PORT1->PU
#define  LEDN_PORT1_HD1     LED_PORT1->HD1

#define  LED_PORT2			JL_PORTB
#define  LEDN_S2_BIT   		2
#define  LEDN_PORT2_OUT     LED_PORT2->OUT
#define  LEDN_PORT2_HD      LED_PORT2->HD
#define  LEDN_PORT2_DIR     LED_PORT2->DIR
#define  LEDN_PORT2_PD      LED_PORT2->PD
#define  LEDN_PORT2_PU      LED_PORT2->PU
#define  LEDN_PORT2_HD1     LED_PORT2->HD1

#define  LED_PORT3			JL_PORTB
#define  LEDN_S3_BIT   		3
#define  LEDN_PORT3_OUT     LED_PORT3->OUT
#define  LEDN_PORT3_HD      LED_PORT3->HD
#define  LEDN_PORT3_DIR     LED_PORT3->DIR
#define  LEDN_PORT3_PD      LED_PORT3->PD
#define  LEDN_PORT3_PU      LED_PORT3->PU
#define  LEDN_PORT3_HD1     LED_PORT3->HD1

#define  LED_PORT4			JL_PORTB
#define  LEDN_S4_BIT   		4
#define  LEDN_PORT4_OUT     LED_PORT4->OUT
#define  LEDN_PORT4_HD      LED_PORT4->HD
#define  LEDN_PORT4_DIR     LED_PORT4->DIR
#define  LEDN_PORT4_PD      LED_PORT4->PD
#define  LEDN_PORT4_PU      LED_PORT4->PU
#define  LEDN_PORT4_HD1     LED_PORT4->HD1

#define  LED_PORT5			JL_PORTB
#define  LEDN_S5_BIT   		5
#define  LEDN_PORT5_OUT     LED_PORT5->OUT
#define  LEDN_PORT5_HD      LED_PORT5->HD
#define  LEDN_PORT5_DIR     LED_PORT5->DIR
#define  LEDN_PORT5_PD      LED_PORT5->PD
#define  LEDN_PORT5_PU      LED_PORT5->PU
#define  LEDN_PORT5_HD1     LED_PORT5->HD1

#define  LED_PORT6			JL_PORTB
#define  LEDN_S6_BIT   		6
#define  LEDN_PORT6_OUT     LED_PORT6->OUT
#define  LEDN_PORT6_HD      LED_PORT6->HD
#define  LEDN_PORT6_DIR     LED_PORT6->DIR
#define  LEDN_PORT6_PD      LED_PORT6->PD
#define  LEDN_PORT6_PU      LED_PORT6->PU
#define  LEDN_PORT6_HD1     LED_PORT6->HD1
#endif
#endif

#endif
#if 0
//显示模式
#define HT4BIT13SIG 		0x00
#define HT5BIT12SIG 		0x01
#define HT6BIT11SIG 		0x02
#define HT7BIT10SIG 		0x03
//显示辉度
#define LIGHT_OFF       	0x80
#define LIGHT_1         	0x88
#define LIGHT_2         	0x89
#define LIGHT_3         	0x8A
#define LIGHT_4         	0x8B
#define LIGHT_5         	0x8C
#define LIGHT_6         	0x8D
#define LIGHT_7         	0x8E
#define LIGHT_8         	0x8F

//数据设置
#define CONTINUITY_WRITE    0x40
#define BYTE_WRITE          0x44
//显示数据地址
#define HT1628_DIS_ADR00    	0xc0
#define HT1628_DISPLAY_MODE  	HT6BIT11SIG
#define HT1628_READ_KEY_DATA	0x42		
#define HT1628_WRITE_DATA		CONTINUITY_WRITE
#define HT1628_DISPLAY_ON		LIGHT_6//LIGHT_7		//PULSE WIDTH=14/16	
#define HT1628_DISPLAY_OFF	    LIGHT_OFF
#define HT1628_DISPLAY_ON_2		LIGHT_2

#define HT1628_DIS_BUF_NUM   14

#define HT1628_DIO_BIT      10
#define HT1628_DIO_PORT     JL_PORTA
#define HT1628_DIO_OUT()	HT1628_DIO_PORT->DIR &=~ BIT(HT1628_DIO_BIT); HT1628_DIO_PORT->PU |= BIT(HT1628_DIO_BIT)
#define HT1628_DIO_HIGH()	HT1628_DIO_PORT->OUT |= BIT(HT1628_DIO_BIT)
#define HT1628_DIO_LOW()	HT1628_DIO_PORT->OUT &=~ BIT(HT1628_DIO_BIT)
#define HT1628_DIO_IN()		HT1628_DIO_PORT->DIR |= BIT(HT1628_DIO_BIT)
#define HT1628_DIO_STA()    (HT1628_DIO_PORT->IN & BIT(HT1628_DIO_BIT))


#define HT1628_CLK_BIT      9
#define HT1628_CLK_PORT     JL_PORTA
#define HT1628_CLK_OUT() 	HT1628_CLK_PORT->DIR &=~ BIT(HT1628_CLK_BIT); HT1628_CLK_PORT->PU |= BIT(HT1628_CLK_BIT)
#define HT1628_CLK_HIGH()	HT1628_CLK_PORT->OUT |= BIT(HT1628_CLK_BIT)
#define HT1628_CLK_LOW()	HT1628_CLK_PORT->OUT &=~ BIT(HT1628_CLK_BIT)


#define HT1628_STB_BIT      8
#define HT1628_STB_PORT     JL_PORTA
#define HT1628_STB_OUT()	HT1628_STB_PORT->DIR &=~ BIT(HT1628_STB_BIT); HT1628_STB_PORT->PU |= BIT(HT1628_STB_BIT)
#define HT1628_STB_HIGH()	HT1628_STB_PORT->OUT |= BIT(HT1628_STB_BIT)
#define HT1628_STB_LOW()	HT1628_STB_PORT->OUT &=~ BIT(HT1628_STB_BIT)

enum
{
	LED7_LEVEL_MAX,
	LED7_LEVEL2,
	LED7_LEVEL1,
	LED7_LEVEL_OFF,
};

void HT1628Poweron_init (void);
void Clear1628Display(void);
void Clear1628_buf(void);
void Ht1628DateUpdate();
void HT1628_value_set(void);
void Ht1628Send(u8 sta);
void menu_batshow_ledseg(void);
void led7_clear_all_buff(void);
extern void led7_display(void);
extern void set1628Display(void);
//extern const u8 pwm_level[LED_PWM_LEVEL_MAX];
#else
enum
{
	LED7_LEVEL_MAX,
	LED7_LEVEL2,
	LED7_LEVEL1,
	LED7_LEVEL_OFF,
};
#define MAX_SEG_NUM         32
#define MAX_RAM_NUM         MAX_SEG_NUM//(MAX_SEG_NUM/2)

// 1621 Command
#define SysDis 		0x00        //turn off OSC & LCD bias generator
#define SysEn  		0x02        //turn on  OSC
#define LcdOff 		0x04        //turn off LCD bias generator
#define LcdOn  		0x06        //turn on  LCD bias generator
#define WDTDis		0x0a        //Disable WDT time-out flag output
#define TONEOFF		0X10        //turn off tone outputs
#define	ClrWDT		0x1f        //clear the contents of WDT stage
#define XTAL32K		0X28        //off chip OSC
#define RC256K		0x30	    //on chip RC OSC
#define Bias12Com4 	0x58	    //LCD 1/2 bias option
#define Bias13Com4 	0x57        //LCD 1/3 bias option

#define	DataID		0xa0    	// 3bit
#define CommandID	0x80		//3bit
#define StartReg    0x00		//BIT的存储位置

enum
{
    DISPLAY_L0=0,
    DISPLAY_L1,
    DISPLAY_L2,
    DISPLAY_L3,
    DISPLAY_L4,
    DISPLAY_L5,
    DISPLAY_L6,
    DISPLAY_L7,
    DISPLAY_L8,
    DISPLAY_L9,
    DISPLAY_L10,
    DISPLAY_L11,
    DISPLAY_FULL
};

#define SDA_1621_BIT        10
#define SDA_1621_PORT       JL_PORTA
#define HT1621DATEOUT()		SDA_1621_PORT->DIR &=~ BIT(SDA_1621_BIT);SDA_1621_PORT->PU |= BIT(SDA_1621_BIT)
#define HT1621DATEHIGH()	SDA_1621_PORT->OUT |= BIT(SDA_1621_BIT)
#define HT1621DATELOW()		SDA_1621_PORT->OUT &=~ BIT(SDA_1621_BIT)
#define HT1621DATEIN()		SDA_1621_PORT->DIR |= BIT(SDA_1621_BIT)
#define GET_1621_DATA()     (SDA_1621_PORT->IN & BIT(SDA_1621_BIT))

#define CLK_1621_BIT        9
#define CLK_1621_PORT       JL_PORTA
#define HT1621CLKOUT()		CLK_1621_PORT->DIR &=~ BIT(CLK_1621_BIT);CLK_1621_PORT->PU |= BIT(CLK_1621_BIT)
#define HT1621CLKHIGH()		CLK_1621_PORT->OUT |= BIT(CLK_1621_BIT)
#define HT1621CLKLOW()		CLK_1621_PORT->OUT &=~ BIT(CLK_1621_BIT)

#define CS_1621_BIT        	8
#define CS_1621_PORT        JL_PORTA
#define HT1621CSOUT()		CS_1621_PORT->DIR &=~ BIT(CS_1621_BIT);CS_1621_PORT->PU |= BIT(CS_1621_BIT)
#define HT1621CSHIGH()		CS_1621_PORT->OUT |= BIT(CS_1621_BIT)
#define HT1621CSLOW()		CS_1621_PORT->OUT &=~ BIT(CS_1621_BIT)
#endif
void Ht1621DateUpdate(void);
void HT1621LCDInit(void);
extern void set1628Display(void);
extern u16 power_external_value;
extern u16 power_external_2_value;

#endif	/*	_LED_H_	*/
