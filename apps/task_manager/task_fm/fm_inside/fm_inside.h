/*--------------------------------------------------------------------------*/
/**@file     fm_inside.c
   @brief    692x内部收音底层驱动
   @details
   @author junqian
   @date   2018-3-20
   @note
*/
/*----------------------------------------------------------------------------*/
#ifndef _FM_INSIDE_H_
#define _FM_INSIDE_H_
#include "sdk_cfg.h"
#include "audio_stream.h"
#if DAC2IIS_EN
#define FM_DAC_OUT_SAMPLERATE  44100L
#else
#define FM_DAC_OUT_SAMPLERATE  44100L
#endif

//----------------------------------------
//内部FM 收台参数调节.
#define FMSCAN_SEEK_CNT_MIN  400
#define FMSCAN_SEEK_CNT_MAX  600
#define FMSCAN_CNR           3

//----------------------------------------

struct FM_INSIDE_DAT {
    AUDIO_STREAM *stream_io;
    volatile u8 src_toggle;
};

void fm_inside_init(void);
bool fm_inside_set_fre(u16 fre);
bool fm_inside_read_id(void);
void fm_inside_powerdown(void);
void fm_inside_mute(u8 flag);

void fm_inside_mutex_init(void);
void fm_inside_mutex_stop(void);
void fm_insice_scan_info_printf(u16 freq_start, u16 freq_end);
#endif // _FM_INSIDE_H_
