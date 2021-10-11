#include "includes.h"
#include "alink.h"
#include "cpu/audio_param.h"
#include "audio/dac.h"
#include "audio/audio.h"
#include "iis/wm8978.h"
#include "uart.h"
#include "audio/audio_link.h"
#include "aec_main.h"
#include "board_init.h"

ALINK_PARM alink_parm;
#define ALINK_DBG
#ifdef ALINK_DBG
AUDIO_STREAM *alink_stream_io = NULL;
#endif

void alink_isr_handle(void *buf, u16 len)
{
#ifdef ALINK_DBG
    //putchar('a');
    if (alink_stream_io) {
        alink_stream_io->output(alink_stream_io->priv, buf, len);
    }
#else
    s16 tmp_buf[128] = {0};
    s16 *pbuf;
    u16 i;
    pbuf = buf;
    for (i = 0; i < (len >> 1); i++) {
        tmp_buf[i] = pbuf[2 * i];
        //memset(tmp_buf,0,sizeof(tmp_buf));
    }
    if (aec_interface.fill_adc_ref_buf) {
        aec_interface.fill_adc_ref_buf(tmp_buf, len >> 1);
    }
#endif
}

void audio_link_init(void)
{
    alink_parm.ch_io = ALINK_IOMAP_PB;
    alink_parm.ch_num = ALINK_CH0;
    alink_parm.ch_dir = ALINK_DIR_RX;
    alink_parm.ch_mode = ALINK_MD_IIS;
    alink_parm.ch_moe = 1;
    alink_parm.ch_soe = 1;
    alink_parm.ch_sr = SR11025;
    alink_parm.dma_len = DAC_SAMPLE_POINT;
    alink_parm.isr_cb = alink_isr_handle;

#ifdef ALINK_DBG
    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_NULL;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    alink_stream_io = audio_stream_init(&stream_param, NULL);
    if (alink_stream_io) {
        alink_stream_io->set_sr(alink_stream_io->priv, alink_parm.ch_sr, 0);
    }
#endif

    WM8978_Init();/*init IIS module*/
    dac_set_samplerate(alink_parm.ch_sr, 0);
    alink_init(&alink_parm);
}
//no_sequence_initcall(audio_link_init);

void audio_link_exit()
{
    alink_exit();
}
