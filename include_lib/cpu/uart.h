#ifndef _UART_
#define _UART_

#include "typedef.h"


s32  uart_init(u32 baud);
void uart_module_init();
s32 uart_set_baud(u32 baud);

int puts(const char *out);
char getbyte();
void putchar(char a);
void put_buf(u8 *buf, u32 len);
void printf_buf(u8 *buf, u32 len);
void put_u8hex(u8 dat);
void put_u8hex0(u8 dat);
void put_u32hex(u32 dat);
void put_u32hex0(u32 dat);
void put_u16hex(u16 dat);
// s32 EQ_uart_init(u32 baud);

void uart_updata_io_ctrl(void **parm);
void uart_updata_receive_cmd();
extern void register_handle_printf_putchar(void (*handle)(char a));


#endif

