#ifndef AUDIO_IMPROVE_H
#define AUDIO_IMPROVE_H

#include "typedef.h"

void audio_repair_init();
void audio_repair_run(s16 *inbuf, s16 *output, u16 point, u8 repair_flag);
void audio_repair_exit();
void audio_sync_out_init();
void audio_sync_out_run(u8 *inbuf, u32 len);
void audio_sync_out_exit();

#endif
