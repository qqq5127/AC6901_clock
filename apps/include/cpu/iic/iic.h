#ifndef	_IIC_H_
#define _IIC_H_

#include "typedef.h"
#include "common/common.h"

void iic_write(u8 chip_id, u8 iic_addr, u8 *iic_dat, u8 n);
void iic_readn(u8 chip_id, u8 iic_addr, u8 *iic_dat, u8 n);
void iic_init(void);
void iic_init_io(void);
void write_info(u8 addr, u8 dat);
tu8 read_info(u8 addr);

void write_eerom(u8 addr, u8 dat);
tu8 read_eerom(u8 iic_addr);
void eeprom_page_write_stop(void);
void eeprom_page_write_start(void);
void eeprom_verify(void);

void iic_start(void);
void iic_stop(void);
void iic_sendbyte_io(u8 byte);
void iic_sendbyte(u8 byte);
tu8 iic_revbyte(u8 para);

extern bool g_iic_busy;

#define iic_delay()      delay(3000)


#define IIC_PORT			JL_PORTB
#define IIC_DAT				1
#define IIC_CLK				2

#define iic_clk_out()    do{IIC_PORT->DIR &= ~BIT(IIC_CLK);IIC_PORT->PU &= ~BIT(IIC_CLK);}while(0)
#define iic_clk_h()      do{IIC_PORT->OUT |=  BIT(IIC_CLK);IIC_PORT->DIR &=~BIT(IIC_CLK);}while(0)
#define iic_clk_l()      do{IIC_PORT->OUT &= ~BIT(IIC_CLK);IIC_PORT->DIR &=~BIT(IIC_CLK);}while(0)


#define iic_data_out()   do{IIC_PORT->DIR &= ~BIT(IIC_DAT);IIC_PORT->PU &= ~BIT(IIC_DAT);}while(0)
#define iic_data_in()    do{IIC_PORT->DIR |=  BIT(IIC_DAT);IIC_PORT->PU |=  BIT(IIC_DAT);}while(0)
#define iic_data_r()     (IIC_PORT->IN&BIT(IIC_DAT))
#define iic_data_h()     do{IIC_PORT->OUT |=  BIT(IIC_DAT);IIC_PORT->DIR &= ~BIT(IIC_DAT);}while(0)
#define iic_data_l()     do{IIC_PORT->OUT &= ~BIT(IIC_DAT);IIC_PORT->DIR &= ~BIT(IIC_DAT);}while(0)

/*
#define iic_data_out()   do{IIC_PORT->DIR &= ~IIC_DAT;IIC_PORT->PU |= IIC_DAT;}while(0)
#define iic_data_in()    do{IIC_PORT->DIR |=  IIC_DAT;IIC_PORT->PU |= IIC_DAT;}while(0)
#define iic_data_r()     (IIC_PORT->IN & IIC_DAT)
#define iic_data_h()     do{IIC_PORT->OUT |= IIC_DAT;}while(0)
#define iic_data_l()     do{IIC_PORT->OUT &=~IIC_DAT;}while(0)

#define iic_clk_out()    do{IIC_PORT->DIR &=~IIC_CLK;IIC_PORT->PU |= IIC_CLK;}while(0)
#define iic_clk_h()      do{IIC_PORT->OUT |= IIC_CLK;}while(0)
#define iic_clk_l()      do{IIC_PORT->OUT &=~IIC_CLK;}while(0)
*/


#define app_IIC_write(a, b, c, d) \
  iic_write(a, b, c, d)
#define app_IIC_readn(a, b, c, d)  \
  iic_readn(a, b, c, d)
#define app_E2PROM_write(a, b)  \
  write_eerom(a, b)
#define app_E2PROM_read(a)   \
  read_eerom(a)


#endif






