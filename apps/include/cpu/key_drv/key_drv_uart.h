#ifndef __KEY_DRV_URAT_H__
#define __KEY_DRV_URAT_H__

#include "sdk_cfg.h"
#include "key.h"

u8 get_uartkey_value(void);
void uart_key_init(void);

extern const key_interface_t key_uart_info;

#endif/*__KEY_DRV_IO_H__*/
