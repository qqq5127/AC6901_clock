#ifndef _PLINK_H_
#define _PLINK_H_

#include "typedef.h"

typedef struct _PLNK_DRV {
    s32(*init)(void *dma_adr, u16 dma_len, u16 sclk_div, void (*isr_cb)(s16 *buf, u8 buf_flag, u16 len));
    void (*on)();
    void (*off)();
    s32(*ioctl)(u32 cmd, u32 arg, void *ptr);
} PLNK_DRV;
extern struct _PLNK_DRV plnk_drv_ops;

#endif
