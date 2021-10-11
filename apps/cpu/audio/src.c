#include "audio/src.h"
#include "audio/audio.h"
#include "circular_buf.h"
#include "uart.h"
#include "sdk_cfg.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".dac_app_bss")
#pragma data_seg(	".dac_app_data")
#pragma const_seg(	".dac_app_const")
#pragma code_seg(	".dac_app_code")
#endif

u8 src_buffer[SRC_IDAT_LEN_MAX + SRC_ODAT_LEN_MAX + SRC_FLTB_MAX * 48] AT(.dac_buf_sec) __attribute__((aligned(4)));

#define SRC_MODULE_EN	1
#if SRC_MODULE_EN

u8 src_cbuffer[SRC_CBUFF_SIZE] AT(.dac_buf_sec) __attribute__((aligned(4)));
AUDIO_STREAM *src_stream_io = NULL;

typedef struct __SRC_T_ {
    cbuffer_t src_cbuf;
    volatile u8 toggle;
    volatile u8 empty;
} SRC_T;
SRC_T src_t;

void src_enable(src_param_t *arg)
{
    u32 size;
    size = src_ops.need_buf(SRC_IDAT_LEN, SRC_ODAT_LEN, SRC_FLTB_MAX);
    printf("src_enable src mem:%d\n", size);
    cbuf_init(&src_t.src_cbuf, src_cbuffer, SRC_CBUFF_SIZE);
    memset(src_buffer, 0, sizeof(src_buffer));
    src_ops.init(arg, src_buffer);
    src_t.toggle = 1;
    src_t.empty = 1;
}

void src_disable()
{
    src_ops.exit(NULL);
    src_t.toggle = 0;
    src_t.empty = 1;
}

u32 src_run(u8 *buf, u16 len)
{
    u32 wlen = 0;
    u8 tmp_buf[SRC_IDAT_LEN];

    if (src_t.toggle) {
        wlen = cbuf_write(&src_t.src_cbuf, buf, len);
        if (wlen != len) {
            putchar('f');
        }
        if (src_t.empty) {
            if (cbuf_get_data_size(&src_t.src_cbuf) >= SRC_KICK_START_LEN) {
                src_t.empty = 0;
                cbuf_read(&src_t.src_cbuf, tmp_buf, SRC_IDAT_LEN);
                //putchar('K');
                src_ops.run(tmp_buf, SRC_IDAT_LEN);
            }
        }
    }
    return wlen;
}

void src_kick_start(u8 start_flag)
{
    u8 tmp_buf[SRC_IDAT_LEN];
    //putchar('k');
    if (src_t.toggle && (src_t.empty == 0)) {
        if (start_flag) {
            src_ops.run(NULL, SRC_IDAT_LEN);
            return;
        }
        if (cbuf_get_data_size(&src_t.src_cbuf) >= SRC_IDAT_LEN) {
            /* putchar('.'); */
            cbuf_read(&src_t.src_cbuf, tmp_buf, SRC_IDAT_LEN);
            src_ops.run(tmp_buf, SRC_IDAT_LEN);
        } else {
            src_t.empty = 1;
        }
    }
}

static void src_output_cb(u8 *buf, u16 len, u8 flag)
{
    u32 wlen;
    if (flag & BIT(0)) {
        if (src_stream_io) {
            src_stream_io->output(src_stream_io->priv, buf, len);
        }
        //output
    }

    //kick start
    if (flag == BIT(0)) {
        src_kick_start(1);/*idat remaind,need not read data */
    } else {
        src_kick_start(0);/*idat done,need read data again*/
    }
}

void src_clear(void)
{
    memset(src_buffer, 0, sizeof(src_buffer));
    //src_ops.ioctl(CLEAR_SRC_BUF,0,NULL);
}

void spdif_src_init()
{
    src_param_t src_parm;

    src_parm.in_chinc = 1;
    src_parm.in_spinc = 2;
    src_parm.out_chinc = 1;
    src_parm.out_spinc = 2;
    src_parm.in_rate = 480;
    src_parm.out_rate = 480;
    src_parm.nchannel = 2;
    src_parm.isr_cb = (void *)src_output_cb;

    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_NULL;
    stream_param.ch = 2;
    stream_param.sr = SR48000;
    src_stream_io = audio_stream_init(&stream_param, NULL);
    if (src_stream_io) {
        src_stream_io->set_sr(src_stream_io->priv, SR48000, 1);
    }

    src_enable(&src_parm);
    //src_run(NULL, 0);
    //src_disable();

}

void spdif_src_set_input_samplerate(u32 sample_rate)
{
    src_param_t src_parm;

    src_parm.in_chinc = 1;
    src_parm.in_spinc = 2;
    src_parm.out_chinc = 1;
    src_parm.out_spinc = 2;
    src_parm.in_rate = sample_rate;  // 882;    // 44100;
    src_parm.out_rate = 480;
    src_parm.nchannel = 2;
    src_parm.isr_cb = (void *)src_output_cb;

    src_ops.ioctl(SET_SRC_PARAM, 0, &src_parm);
}
#endif


#define SRC_CBUFF_MODE	0
#define SRC_DMA_MODE	1
#define SRC_MODE	SRC_CBUFF_MODE

typedef struct {
    volatile u8 busy;
    AUDIO_STREAM input;
    AUDIO_STREAM *output;
} HW_SRC;
HW_SRC hw_src;

static void hw_src_output_cb(u8 *buf, u16 len, u8 flag)
{
    u32 wlen = 0;

    if (flag & BIT(0)) {
        wlen = hw_src.output->output(hw_src.output->priv, buf, len);

        if (wlen != len) {
            putchar('f');
        }
    }
#if (SRC_MODE == SRC_CBUFF_MODE)
    if (flag == BIT(0)) {
        src_kick_start(1);/*idat remaind,need not read data */
    } else {
        src_kick_start(0);/*idat done,need read data again*/
    }
#else
    if (flag == BIT(0)) {
        /*src indat remaind,contine*/
        hw_src.busy = 1;
        src_ops.run(NULL, SRC_IDAT_LEN_MAX);
    } else {
        hw_src.busy = 0;
    }
#endif
}

void hw_src_init(u16 in_sr, u16 out_sr)
{
    src_param_t src_parm;

    src_parm.in_chinc = 1;
    src_parm.in_spinc = 2;
    src_parm.out_chinc = 1;
    src_parm.out_spinc = 2;
    src_parm.in_rate = in_sr;
    src_parm.out_rate = out_sr;
    src_parm.nchannel = 2;
    src_parm.isr_cb = (void *)hw_src_output_cb;
    puts("HW_src init\n");
    log_printf("in_sr:%d \nout_sr:%d\n", in_sr, out_sr);

    src_enable(&src_parm);
}

static u32 hw_src_run(void *priv, void *buf, u32 len)
{
    u32 wlen = 0;
#if (SRC_MODE == SRC_CBUFF_MODE)
    wlen = src_run(buf, len);
    if (wlen != len) {
        //putchar('F');
    }
#else
    if (hw_src.busy) {
        //putchar('1');
    }

    if (hw_src.output->free_len(hw_src.output->priv) < (len * 2)) {
        //putchar('2');
    }
    hw_src.busy = 1;
    if (len > SRC_IDAT_LEN_MAX) {
        wlen = SRC_IDAT_LEN_MAX;
    } else {
        wlen = len;
    }
    src_ops.run(buf, wlen);
#endif
    return wlen;
}

static void hw_src_clear(void *priv)
{
    if (hw_src.output) {
        hw_src.output->clear(hw_src.output->priv);
    }
}

static tbool hw_src_check(void *priv)
{
    if (hw_src.output) {
        return hw_src.output->check(hw_src.output->priv);
    } else {
        return false;
    }
}
static u32 hw_src_data_len(void *priv)
{
    if (hw_src.output->data_len) {
        return hw_src.output->data_len(hw_src.output->priv);
    }
    return 0;
}

static u32 hw_src_free_len(void *priv)
{
    if (hw_src.output->free_len) {
        return hw_src.output->free_len(hw_src.output->priv);
    }
    return 0;
}

static void hw_src_samplerate(void *priv, u16 sr, u8 wait)
{
    if (hw_src.output->set_sr) {
        hw_src.output->set_sr(hw_src.output->priv, sr, wait);
    }
}

AUDIO_STREAM *audio_src_hw_input(AUDIO_STREAM *output, AUDIO_STREAM_PARAM *param, void *priv)
{
    if (output == NULL) {
        return NULL;
    }

    memset(&hw_src, 0, sizeof(hw_src));

    hw_src.output 			= output;
    hw_src.input.priv  		= (void *)NULL;
    hw_src.input.output 	= (void *)hw_src_run;
    hw_src.input.clear 		= (void *)hw_src_clear;
    hw_src.input.check 		= (void *)hw_src_check;
    hw_src.input.data_len 	= (void *)hw_src_data_len;
    hw_src.input.free_len 	= (void *)hw_src_free_len;
    hw_src.input.set_sr 	= (void *)hw_src_samplerate;

    hw_src_init(param->sr, *((u16 *)priv));

    return &(hw_src.input);
}

