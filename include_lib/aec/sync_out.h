#ifndef SYNC_OUT_H
#define SYNC_OUT_H

#include "typedef.h"


u32 sync_out_querybuf();
void sync_out_handle_register(u32(*output_cb)(void *priv, s16 *buf, u32 len), int (*getlen)(void *priv));
s32 sync_out_init(void *pbuf, u8 toggle);
s32 sync_out_run(u8 *inbuf, u32 len);
void sync_out_exit(void);

#endif
