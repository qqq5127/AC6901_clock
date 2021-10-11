#ifndef __LCD_DRV_API_H__
#define __LCD_DRV_API_H__

#include "sdk_cfg.h"
#include "rtc_api.h"

///************************************************************/
///****************配置屏幕的大小
///****************说明：点阵屏是一个点对应一个bit
///************************************************************/
#define  LCDPAGE            8
#define  LCDCOLUMN          128

#define SCR_WIDTH           LCDCOLUMN
#define SCR_HEIGHT          (LCDPAGE*8)



#if 0
#define LCD_A0_PORT         JL_PORTC
#define LCD_A0_BIT          (BIT(3))
#else
#define LCD_A0_PORT         JL_PORTA
#define LCD_A0_BIT          (BIT(11))
#endif
#define	LCD_A0_OUT()	    do{LCD_A0_PORT->DIR &= ~LCD_A0_BIT;}while(0)
#define	LCD_A0_L()	        do{LCD_A0_PORT->OUT &= ~LCD_A0_BIT;}while(0)
#define	LCD_A0_H()	        do{LCD_A0_PORT->OUT |= LCD_A0_BIT;}while(0)

#if 0
#define LCD_RES_PORT        JL_PORTC
#define LCD_RES_BIT         (BIT(2))
#else
#define LCD_RES_PORT        JL_PORTA
#define LCD_RES_BIT         (BIT(13))
#endif
#define LCD_RES_OUT()	    do{LCD_RES_PORT->DIR &= ~LCD_RES_BIT;}while(0)//
#define LCD_RES_L()	        do{LCD_RES_PORT->OUT &= ~LCD_RES_BIT;}while(0)
#define LCD_RES_H()	        do{LCD_RES_PORT->OUT |= LCD_RES_BIT;}while(0)

#if 0
#define LCD_CS_PORT         JL_PORTC
#define LCD_CS_BIT          (BIT(2))
#else
#define LCD_CS_PORT         JL_PORTA
#define LCD_CS_BIT          (BIT(9))
#endif
#define LCD_CS_OUT()	    do{LCD_CS_PORT->DIR &= ~LCD_CS_BIT;}while(0)//
#define LCD_CS_L()	        do{LCD_CS_PORT->OUT &= ~LCD_CS_BIT;}while(0)
#define LCD_CS_H()	        do{LCD_CS_PORT->OUT |= LCD_CS_BIT;}while(0)


#define LCD_BL_ON()         do{PORTR_DIR(PORTR0,0); PORTR_OUT(PORTR0,1);}while(0)
#define LCD_PORT_OUT()      do{LCD_DATA_OUT();LCD_CLK_OUT();LCD_A0_OUT();LCD_RES_OUT();LCD_CS_OUT();}while(0)
#define LCD_PORT_OUT_H()    do{LCD_DATA_OUT();LCD_CLK_OUT();LCD_A0_H();  LCD_RES_H();	LCD_CS_H();}while(0)

//全局变量声明
extern u8 disp_buf[];

//函数声明
void draw_lcd_buf(void);
void lcd_clear(void);
void lcd_init(void);

#endif/*__LCD_DRV_API_H__*/
