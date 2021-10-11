#include "audio/audio_stream.h"
#include "audio/audio.h"
#include "audio/dac_api.h"
#include "audio/eq.h"
#include "uart.h"
#include "string.h"
#include "sync_hardware.h"
#include "sync_software.h"
#include "pitchshifter.h"
#include "src.h"

AUDIO_STREAM *audio_stream_init(AUDIO_STREAM_PARAM *param, void *priv)
{
    AUDIO_STREAM *output = (AUDIO_STREAM *)&audio_stream_io;

#if (AUDIO_EFFECT & AUDIO_EFFECT_HW_EQ)
    if (param->ef & AUDIO_EFFECT_HW_EQ) {
        puts("AUDIO_EFFECT:HW_eq\n");
        eq_init();
        eq_bypass_en(0);
    } else {
        eq_bypass_en(1);
    }
#endif

#if (AUDIO_EFFECT & AUDIO_EFFECT_SW_EQ)
    if (param->ef & AUDIO_EFFECT_SW_EQ) {
        puts("AUDIO_EFFECT:SW_eq\n");
        output = audio_eq_input((AUDIO_STREAM *)output);
        eq_bypass_en(0);
        if (output == NULL) {
            puts("audio_eq error\n");
        }
    } else {
        eq_bypass_en(1);
    }
#endif




#if (AUDIO_EFFECT & AUDIO_EFFECT_SW_SYNC)
    if (param->ef & AUDIO_EFFECT_SW_SYNC) {
        puts("AUDIO_EFFECT:SW_sync\n");
        output = audio_sync_sw_input(output, param, priv);
        if (output == NULL) {
            puts("SW_sync error\n");
        }
    }
#endif

#if (AUDIO_EFFECT & AUDIO_EFFECT_PITCH)
    if (param->ef & AUDIO_EFFECT_PITCH) {
        puts("AUDIO_EFFECT:pitch_speed\n");
        output = pitchshifter_input(output);
        if (output == NULL) {
            puts("pitch_speed error\n");
        }
    }
#endif
#if (AUDIO_EFFECT & AUDIO_EFFECT_TWS_SYNC)
    extern AUDIO_STREAM *audio_sync_tws_input(AUDIO_STREAM * output, AUDIO_STREAM_PARAM * param, void *priv);
    extern u8 get_tws_single_flag();
    if (param->ef & AUDIO_EFFECT_TWS_SYNC && (get_tws_single_flag() == 1)) {
        puts("AUDIO_EFFECT:tws_sync\n");
        output = audio_sync_tws_input(output, param, priv);
        if (output == NULL) {
            puts("SW_sync error\n");
        }
    }
#endif

#if (AUDIO_EFFECT & AUDIO_EFFECT_HW_SYNC)
    if (param->ef & AUDIO_EFFECT_HW_SYNC) {
        puts("AUDIO_EFFECT:HW_sync\n");
        output = audio_sync_hw_input(output, param, priv);
        if (output == NULL) {
            puts("HW_sync error\n");
        }
    }
#endif
#if (AUDIO_EFFECT & AUDIO_EFFECT_HW_SRC)
    if (param->ef & AUDIO_EFFECT_HW_SRC) {
        puts("AUDIO_EFFECT:HW_SRC\n");
        output = audio_src_hw_input(output, param, priv);
        if (output == NULL) {
            puts("HW_src error\n");
        }
    }
#endif

    return output;
}


s32 audio_stream_exit(AUDIO_STREAM_PARAM *param, void *priv)
{
#if (AUDIO_EFFECT & AUDIO_EFFECT_HW_SYNC)
    if (param->ef & AUDIO_EFFECT_HW_SYNC) {
        puts("AUDIO_EFFECT:HW_sync_EXIT\n");
        hw_sync_exit();
    }
#endif
    return 0;
}







