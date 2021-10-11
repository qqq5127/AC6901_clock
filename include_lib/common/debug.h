/*********************************************************************************************
    *   Filename        : debug.h

    *   Description     :

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2017-01-17 15:14

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

/*
 *  debug.h
 *
 *  allow to funnel debug & error messages
 */

#ifndef __DEBUG_H
#define __DEBUG_H


#include "typedef.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

/**
 * @brief Log Security Manager key via log_info
 * @param key to log
 */
//void log_info_key(const char *name, sm_key_t key);

/**
 * @brief Hexdump via log_info
 * @param data
 * @param size
 */
void log_info_hexdump(const void *data, int size);


void printf_buf(u8 *buf, u32 len);

//#define DEBUG_INFO_ENABLE


#ifdef DEBUG_INFO_ENABLE

#ifdef ENABLE_LOG_DEBUG
#ifdef HAVE_HCI_DUMP
#define log_debug(format, ...)  	printf(format,  ## __VA_ARGS__)
#else
#define log_debug   //printf
#endif
#else
#define log_debug(...)
#endif

#define PRINTF(format, ...)         printf(format, ## __VA_ARGS__)
#ifdef LOG_INFO_ENABLE
#define log_info(format, ...)       PRINTF("[info] :" format "\n", ## __VA_ARGS__)
#else
#define log_info(...)
#endif

#ifdef LOG_ERROR_ENABLE
#define log_error(format, ...)      PRINTF("<error> :" format "\n", ## __VA_ARGS__)
#define log_error_hexdump(x, y)     printf_buf(x, y)
#else
#define log_error(...)
#define log_error_hexdump(...)
#endif

#ifdef LOG_DUMP_ENABLE
#define log_info_hexdump(x,y)       printf_buf(x,y)
#else
#define log_info_hexdump(...)
#endif

#else

#define log_info(...)
#define log_error(...)
#define log_error_hexdump(...)
#define log_info_hexdump(...)
#define log_debug(...)

#endif


#endif // __DEBUG_H
