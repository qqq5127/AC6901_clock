#ifndef _SYNC_SOFTWARE_H_
#define _SYNC_SOFTWARE_H_

#include "typedef.h"
#include "audio/audio_stream.h"

AUDIO_STREAM *audio_sync_sw_input(AUDIO_STREAM *output, AUDIO_STREAM_PARAM *param, void *priv);

#endif

