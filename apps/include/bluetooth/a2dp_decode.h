#ifndef __A2DP_DECODE_H__
#define __A2DP_DECODE_H__
#include "typedef.h"

void *a2dp_media_play(void *priv, const char *format,
                      u16(*read_handle)(void *priv, u8 *buf, u16 len),
                      bool (*seek_handle)(void *priv, u8 type, u32 offsiz),
                      void *aac_setting,
                      void *infbuf);

void a2dp_media_stop(void **hdl);

#endif// __A2DP_DECODE_H__
