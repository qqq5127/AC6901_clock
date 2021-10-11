#ifndef _SYNC_HARDWARE_H_
#define _SYNC_HARDWARE_H_

#include "typedef.h"
#include "circular_buf.h"
#include "audio/audio_stream.h"


enum {
    AUDIO_SYNC_HW	= 1,/*Hardware_Sync*/
    AUDIO_SYNC_SW,		/*Software_Sync*/
};

typedef struct _AUDIO_SYNC {
    s32(*init)(u16 sr);
    s32(*exit)(void *ptr);
    u32(*run)(void *buf, u32 len);
} AUDIO_SYNC;
extern AUDIO_SYNC *audio_sync;

AUDIO_STREAM *audio_sync_hw_input(AUDIO_STREAM *output, AUDIO_STREAM_PARAM *param, void *priv);
s32 hw_sync_exit(void);

#endif
