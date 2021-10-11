#ifndef _CLOCK_API_H_
#define _CLOCK_API_H_

#include "typedef.h"
#include "clock_interface.h"

/********************************************************************************/
/*
 *      API
 */
void clock_init(SYS_CLOCK_INPUT sys_in, u32 input_freq, u32 out_freq);

u32 clock_get_usb_freq(void);

u32 clock_get_dac_freq(void);

u32 clock_get_apc_freq(void);

u32 clock_get_uart_freq(void);

u32 clock_get_bt_freq(void);

u32 clock_get_sfc_freq(void);

u32 clock_get_hsb_freq(void);

u32 clock_get_lsb_freq(void);

u32 clock_get_iosc_freq(void);

u32 clock_get_osc_freq(void);

u32 clock_get_sys_freq(void);
void clock_dump(void);
void clock_set_apc_freq(u32 freq, u8 apc_src);

#define USB_CLK         clock_get_usb_freq()
#define DAC_CLK         clock_get_dac_freq()
#define APC_CLK         clock_get_apc_freq()
#define UART_CLK        clock_get_uart_freq()
#define BT_CLK          clock_get_bt_freq()


#define SFC_CLK         clock_get_sfc_freq()
#define SYS_CLK         clock_get_hsb_freq()
#define LSB_CLK         clock_get_lsb_freq()

#define IOSC_CLK        clock_get_iosc_freq()
#define OSC_CLK         clock_get_osc_freq()

void set_spi_speed_auto(void);
#endif
