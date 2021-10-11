#ifndef __DEBUG_LOG_H_
#define __DEBUG_LOG_H_

#include "cpu.h"

void printf_buf(u8 *buf, u32 len);

#ifdef DEBUG_ENABLE

#define PRINTF(format, ...)         printf(format, ## __VA_ARGS__)

#define log_info(format, ...)       PRINTF(format "\n", ## __VA_ARGS__)
#define log_error(format, ...)      PRINTF(format "\n", ## __VA_ARGS__)
#define log_info_hexdump(x,y)       printf_buf(x,y)

#else
#define log_info(...)
#define log_error(...)
#define log_info_hexdump(...)

#endif

#endif//__DEBUG_LOG_H_
