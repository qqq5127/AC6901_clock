#ifndef _LCD_SPI_H_
#define _LCD_SPI_H_

#include "typedef.h"

//SPI1接口选择
#if 1//
//SPI1A
#define LCD_SPI_SET_GROUP()       do{JL_IOMAP->CON1 &= ~BIT(4);}while(0)
#define LCD_SPI_SET_DATAIN()      do{JL_PORTB->DIR &= ~BIT(3);}while(0)
#define LCD_SPI_SET_CLK()	      do{JL_PORTB->DIR &= ~BIT(4);}while(0)
#define LCD_SPI_SET_DATAOUT()     do{JL_PORTB->DIR &= ~BIT(5);}while(0)

#else
//SPI1B
#define LCD_SPI_SET_GROUP()       do{JL_IOMAP->CON1  |= BIT(4);}while(0)
#define LCD_SPI_SET_DATAIN()      do{JL_PORTC->DIR &= ~BIT(3);}while(0)
#define LCD_SPI_SET_CLK()	      do{JL_PORTC->DIR &= ~BIT(4);}while(0)
#define LCD_SPI_SET_DATAOUT()     do{JL_PORTC->DIR &= ~BIT(5);}while(0)

#endif

#define LCD_DATA_OUT()
#define	LCD_CLK_OUT()
#define LCD_SPI_SET_MODE_OUT()       do{JL_SPI1->CON &= ~BIT(3);}while(0)
#define LCD_SPI_SET_MODE_INOUT()     do{JL_SPI1->CON |= BIT(3);}while(0)

extern void SPI1_init(bool is3Wrie, u8 speed);
extern void SPI1_WriteByte(u8 dat);
extern u8 SPI1_ReadByte(void);
extern void SPI1_DmaWrite(void *buf, u16 len);
extern void SPI1_DmaRead(void *buf, u16 len);

#endif/*_LCD_SPI_H_*/
