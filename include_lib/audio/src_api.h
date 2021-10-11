/*
*********************************************************************************************************
*                                             BC51
*
*                                             CODE
*
*                          (c) Copyright 2015-2016, ZHUHAI JIELI
*                                           All Rights Reserved
*
* File : *
* By   : jamin.li
* DATE : 11/11/2015 build this file
* junqian 20170329 add para inbuf_len,outbuf_len,kick_start_len,cbuf_len
*********************************************************************************************************
*/
#ifndef _SRC_DRV_H_
#define _SRC_DRV_H_

#include "hw_cpu.h"
#include "cpu.h"
#include "printf.h"
#include "typedef.h"

typedef struct {
    u8 nchannel;        //一次转换的通道个数，取舍范围(1 ~ 8)，最大支持8个通道
    u8 reserver[3];
    u16 in_rate;        ///输入采样率
    u16 out_rate;       ///输出采样率
    u16 in_chinc;       ///输入方向,多通道转换时，每通道数据的地址增量
    u16 in_spinc;       ///输入方向,同一通道后一数据相对前一数据的地址增量
    u16 out_chinc;      ///输出方向,多通道转换时，每通道数据的地址增量
    u16 out_spinc;      ///输出方向,同一通道后一数据相对前一数据的地址增量
    ///一次转换完成后，输出中断会调用此函数用于接收输出数据，数据量大小由outbuf_len决定
    void (*isr_cb)(u8 *buf, u16, u8);
} src_param_t;

enum {
    SET_SRC_PARAM	= 1	,
    CLEAR_SRC_BUF	 	,
};
struct src_driver {
    u32(*need_buf)(int inbuf_len, int outbuf_len, int ch_max);
    s32(*init)(src_param_t *parm, void *mem);
    s32(*exit)(void *ptr);
    u32(*run)(u8 *buf, u16 len);
    s32(*ioctl)(u32 cmd, u32 arg, void *ptr);
};
extern const struct src_driver src_ops;

#endif
