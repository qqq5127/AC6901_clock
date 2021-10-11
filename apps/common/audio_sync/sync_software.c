#include "sync_software.h"
#include "string.h"
#include "audio/audio.h"
#include "audio/dac_api.h"
#include "uart.h"
#include "sdk_cfg.h"
#include "audio/dac.h"
#include "audio/syn_context_api.h"

#define SW_SYNC_LIMIT_UP	8
#define SW_SYNC_LIMIT_DOWN	6
#define SW_SYNC_LIMIT_FULL	10
typedef struct _SOFTWARE_SYNC {
    u8 toggle;					/*a2dp sync toggle		*/
    u8 nch;
    u16 sr;
    AUDIO_STREAM input;
    AUDIO_STREAM *output;
    cbuffer_t *sync_buf;
} SOFTWARE_SYNC;
SOFTWARE_SYNC sw_sync = {
    .toggle = 0,
    .output = NULL,
    .sync_buf = NULL,
};

static SYN_API_CONTEXT *sync_ops = NULL;
static u8 SoftSyncBuf[2964] sec(.dac_buf_sec);

static u32 sw_sync_getlen(void *priv)
{
    if (sw_sync.sync_buf) {
        return sw_sync.sync_buf->data_len;
    } else if (sw_sync.output->free_len) {
        return sw_sync.output->data_len(sw_sync.output->priv);
    }
    return 0;
}

static u32 sw_sync_output(void *priv, u8 *buf, int len)
{
    return sw_sync.output->output(sw_sync.output->priv, buf, len);
}

static void sw_sync_init(u16 ch, u16 sr)
{
    u32 bufsize;
    u32 output_total_len;
    SYNC_coef sync_param;

    if (sw_sync.sync_buf) {
        output_total_len = sw_sync.sync_buf->total_len;
    } else {
        output_total_len = ((AUDIO_STREAM_DAC *)sw_sync.output->priv)->buf_size;
    }

    sync_param.autosync = 1;
    sync_param.speed_rate = 6;
    if (sr >= SR32000) {
        sync_param.samplerate = SR48000;
    } else if (sr >= SR16000) {
        sync_param.samplerate = SR16000;
    } else {
        sync_param.samplerate = SR8000;
    }
    sync_param.nch = ch ;
    sync_param.priv = NULL;
    sync_param.getlen = sw_sync_getlen;
    sync_param.output = sw_sync_output;
    sync_param.Limit_up = output_total_len * SW_SYNC_LIMIT_UP / SW_SYNC_LIMIT_FULL;
    sync_param.limit_down = output_total_len * SW_SYNC_LIMIT_DOWN / SW_SYNC_LIMIT_FULL;
    sync_param.tws_setflag = 0;

    printf("total_len:%d\n", output_total_len);
    printf("Limit_up:%d\n", sync_param.Limit_up);
    printf("limit_down:%d\n", sync_param.limit_down);
    sync_ops = get_sync_unit_api();
    if (sync_ops) {
        bufsize = sync_ops->need_size(sync_param.nch, sync_param.samplerate);
        printf("SoftSyncBuf:%d\n", bufsize);
        if (sizeof(SoftSyncBuf) >= bufsize) {
            sync_ops->open(SoftSyncBuf, &sync_param);
            sw_sync.toggle = 1;
        } else {
            sw_sync.toggle = 0;
            puts("SOFTWARE_SYNC mem err\n");
        }
    } else {
        sw_sync.toggle = 0;
    }
}

static u32 sw_sync_run(void *priv, void *buf, u32 len)
{
    u8 cnt = 0;
    u32 res;
    if (sw_sync.toggle) {
        res = sync_ops->run(SoftSyncBuf, buf, len, SYNC_OUTPUT_NORMAL);
        if (res != len) {
            //printf("[%d]",res);
            //putchar('.');
        }
        return res;
    } else {
        return sw_sync.output->output(sw_sync.output->priv, buf, len);
    }
}

static void sw_sync_clear(void *priv)
{
    sw_sync.output->clear(sw_sync.output->priv);
}

static tbool sw_sync_check(void *priv)
{
    return sw_sync.output->check(sw_sync.output->priv);
}

static u32 sw_sync_free_len(void *priv)
{
    if (sw_sync.output->free_len) {
        return sw_sync.output->free_len(sw_sync.output->priv);
    }
    return 0;
}

static u32 sw_sync_data_len(void *priv)
{
    if (sw_sync.output->data_len) {
        return sw_sync.output->data_len(sw_sync.output->priv);
    }
    return 0;
}

static void sw_sync_samplerate(void *priv, u16 sr, u8 wait)
{
    if ((sw_sync.sr != sr) && (sr != 0)) {
        printf("sw_sync SR:%d\n", sr);
        sw_sync.sr = sr;
        sw_sync_init(sw_sync.nch, sw_sync.sr);
    }
    sw_sync.output->set_sr(sw_sync.output->priv, sr, wait);
}

AUDIO_STREAM *audio_sync_sw_input(AUDIO_STREAM *output, AUDIO_STREAM_PARAM *param, void *priv)
{
    if (output == NULL) {
        return NULL;
    }

    sw_sync.output = output;
    sw_sync.input.priv  	= (void *)NULL;
    sw_sync.input.output 	= (void *)sw_sync_run;
    sw_sync.input.clear 	= (void *)sw_sync_clear;
    sw_sync.input.check 	= (void *)sw_sync_check;
    sw_sync.input.data_len 	= (void *)sw_sync_data_len;
    sw_sync.input.free_len 	= (void *)sw_sync_free_len;
    sw_sync.input.set_sr 	= (void *)sw_sync_samplerate;
    sw_sync.sync_buf = priv;
    sw_sync.nch = param->ch;
    sw_sync.sr = param->sr;

    sw_sync_init(sw_sync.nch, sw_sync.sr);

    return &(sw_sync.input);
}
