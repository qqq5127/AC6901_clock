#ifndef _LADC_H_
#define _LADC_H_

#include "typedef.h"
#include "audio/ladc_api.h"

struct tws_linein_parm_t {
    u8 adc_2_dac;
    u16 rate;
};
void ladc_init(void);
void ladc_ch_open(u8 ch, u16 sr);
void ladc_ch_close(u8 ch);
void ladc_mic_gain(u8 gain, u8 gx2);
void ladc_mic_mute(u32 arg);
void ladc_set_samplerate(u16 sr);
u16 ladc_get_samplerate(void);
void microphone_open(u8 mic_gain, u8 mic_gx2);
void microphone_close(void);
void emitter_aux_open(void *priv);
void emitter_aux_close(void *priv);

#endif
