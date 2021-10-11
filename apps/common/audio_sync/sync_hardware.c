#include "sync_hardware.h"
#include "string.h"
#include "audio/src.h"
#include "audio/audio.h"
#include "audio/dac_api.h"
#include "uart.h"
#include "sdk_cfg.h"
#include "audio/dac.h"
#include "audio/audio_stream.h"
#include "music_decoder.h"

#define HW_SYNC_LIMIT_UP	9
#define HW_SYNC_LIMIT_DOWN	7
#define HW_SYNC_LIMIT_FULL	10

#define DUMP_PACKETS_NUM	0
typedef struct _HARDWARE_SYNC {
    volatile u8 toggle;			/*a2dp sync toggle		*/
    volatile u8 busy;			/*src state				*/
    u8 dump_cnt;
    u8 nch;
    volatile u16 dump_packets;
    u16 sr_upper_limit;			/*src upper limit value	*/
    u16 sr_lower_limit;			/*src lower limit value	*/
    u16 sr_normal;				/*src normal sr value	*/
    u16 sr_step;				/*src fade step			*/
    u16 sr_step_normal;			/*src recover step		*/
    u16 input_sr;				/*src in_rate			*/
    u16 output_sr;				/*src out_rate			*/
    cbuffer_t *sync_buf;
    AUDIO_STREAM input;
    AUDIO_STREAM *output;
} HARDWARE_SYNC;
static HARDWARE_SYNC hw_sync;


/*
***********************************************************************************
*					A2DP SYNC OUTPUT
*
*Description: This function is called when src converted finish
*
*Argument(s): buf = src output
*			  len = src output length
*			  flag = src dma flag
*Returns	: none
*
*Note(s)	:
***********************************************************************************
*/
void hw_sync_output(u8 *buf, u16 len, u8 flag)
{
    u32 wlen = 0;

    if (flag & BIT(0)) {
        wlen = hw_sync.output->output(hw_sync.output->priv, buf, len);

        //log_printf("[%d",get_dac_cbuf_len());
        if (wlen != len) {
            log_printf("[%d--%d]", hw_sync.output->free_len(NULL), len);
        }
    }

    if (flag == BIT(0)) {
        /*src indat remaind,contine*/
        hw_sync.busy = 1;
        src_ops.run(NULL, SRC_IDAT_LEN_MAX);
    } else {
        hw_sync.busy = 0;
    }

}
/*
***********************************************************************************
*					SRC INIT
*
*Description: This function is called to init SRC module
*
*Argument(s):
*
*Returns	:
*
*Note(s)	:
***********************************************************************************
*/
s32 src_init(void)
{
    s32 err = 0;
    u32 size;
    src_param_t src_p;

    src_p.in_rate = hw_sync.input_sr;
    src_p.out_rate = hw_sync.output_sr;
    if (hw_sync.nch == 2) {
        src_p.nchannel = 2;
        src_p.in_chinc = 1;
        src_p.in_spinc = 2;
        src_p.out_chinc = 1;
        src_p.out_spinc = 2;
    } else {
        src_p.nchannel = 1;
        src_p.in_chinc = 1;
        src_p.in_spinc = 1;
        src_p.out_chinc = 1;
        src_p.out_spinc = 1;
    }
    src_p.isr_cb = (void *)hw_sync_output;

    size = src_ops.need_buf(SRC_IDAT_LEN_MAX, SRC_ODAT_LEN_MAX, SRC_FLTB_MAX);
    /* log_printf("src_need_buf:%d\n", size); */
    err = src_ops.init(&src_p, src_buffer);
    return err;
}
void reset_hw_sync_src(u32 rate)
{
    src_param_t src_p;
    src_p.in_rate = rate;
    src_p.out_rate = rate;
    hw_sync.output_sr = rate;
    //hw_sync.sr_step = 25;
    if (hw_sync.nch == 2) {
        src_p.nchannel = 2;
        src_p.in_chinc = 1;
        src_p.in_spinc = 2;
        src_p.out_chinc = 1;
        src_p.out_spinc = 2;
    } else {
        src_p.nchannel = 1;
        src_p.in_chinc = 1;
        src_p.in_spinc = 1;
        src_p.out_chinc = 1;
        src_p.out_spinc = 1;
    }
    src_p.isr_cb = (void *)hw_sync_output;
    src_ops.ioctl(SET_SRC_PARAM, 0, &src_p);

}
void clear_hw_sync_buf()
{
    hw_sync.output->clear(hw_sync.output->priv);
    src_clear();
}

/*
***********************************************************************************
*					A2DP SYNC INIT
*
*Description: This function is called init sync module
*
*Argument(s):
*
*Returns	:
*
*Note(s)	:
***********************************************************************************
*/
static s32 hw_sync_init(u16 sr, u8 ch)
{
    s32 err = 0;

    while (hw_sync.busy) {
        putchar('B');
    };

    if (sr < SR8000) {
        hw_sync.toggle = 0;
        puts("HW_sync SR err\n");
        return -1;
    }

    memset(&hw_sync, 0, sizeof(hw_sync));
    hw_sync.nch = ch;
    hw_sync.sr_normal = sr;
    hw_sync.input_sr = sr;
    hw_sync.output_sr = sr;
    hw_sync.sr_upper_limit = sr + 80;
    hw_sync.sr_lower_limit = sr - 80;
    hw_sync.sr_step = 5;
    hw_sync.sr_step_normal = 1;
    hw_sync.dump_packets = DUMP_PACKETS_NUM;
    hw_sync.dump_cnt = 0;

    err = src_init();
    src_clear();
    if (err == 0) {
        hw_sync.toggle = 1;
        /* puts("hw_sync_init_OK\n"); */
    }
    return err;
}

/*
***********************************************************************************
*					A2DP SYNC AUTO CONTROL(AC)
*
*Description: This function is called to reset auto control SRC param
*
*Argument(s): input_sr = src input sample_rate
*
*Returns	: none
*
*Note(s)	:
***********************************************************************************
*/
static void hw_sync_AC(u16 input_sr)
{
    src_param_t src_p;
    u32 data_len, total_len;
    static u32 put_cnt = 0;
    static u32 put_cnt1 = 0;
    if (hw_sync.sync_buf) {
        data_len = hw_sync.sync_buf->data_len;
        total_len = hw_sync.sync_buf->total_len;
    } else {
        data_len = hw_sync.output->data_len(hw_sync.output->priv);
        total_len = ((AUDIO_STREAM_DAC *)hw_sync.output->priv)->buf_size;
    }

    put_cnt++;
    put_cnt1++;
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
    extern u8 get_tws_single_flag();
    extern u8 get_tws_ps_flag();
    extern void clear_tws_ps_flag();
    u8 tws_ps_flag = get_tws_ps_flag();
    clear_tws_ps_flag();
    if (get_tws_single_flag() == 1) {
        if (tws_ps_flag == 2) {
            hw_sync.sr_step = 10;
            hw_sync.output_sr = ((hw_sync.output_sr - hw_sync.sr_step) < hw_sync.sr_lower_limit) ? \
                                hw_sync.sr_lower_limit : (hw_sync.output_sr - hw_sync.sr_step);
            /* putchar('b'); */
            /* put_u16hex(hw_sync.output_sr); */
        } else if (tws_ps_flag == 1) {
            if (put_cnt > 50) {
                put_cnt = 0;
                /* putchar('s'); */
            }

            hw_sync.sr_step = 30;
            hw_sync.output_sr = ((hw_sync.output_sr + hw_sync.sr_step) > hw_sync.sr_upper_limit) ? \
                                hw_sync.sr_upper_limit : (hw_sync.output_sr + hw_sync.sr_step);
        } else {
            if (put_cnt1 > 50) {
                put_cnt1 = 0;
                /* putchar('n'); */
            }
            if (hw_sync.output_sr > hw_sync.sr_normal) {
                hw_sync.output_sr = ((hw_sync.output_sr - hw_sync.sr_step_normal) < hw_sync.sr_normal) ? \
                                    hw_sync.sr_normal : (hw_sync.output_sr - hw_sync.sr_step_normal);
            } else {
                hw_sync.output_sr = ((hw_sync.output_sr + hw_sync.sr_step_normal) > hw_sync.sr_normal) ? \
                                    hw_sync.sr_normal : (hw_sync.output_sr + hw_sync.sr_step_normal);
            }
        }

    } else
#endif
    {

        if (data_len > (total_len * HW_SYNC_LIMIT_UP / HW_SYNC_LIMIT_FULL)) {
            hw_sync.output_sr = ((hw_sync.output_sr - hw_sync.sr_step) < hw_sync.sr_lower_limit) ? \
                                hw_sync.sr_lower_limit : (hw_sync.output_sr - hw_sync.sr_step);
            // putchar('b');
            /* put_u16hex(hw_sync.output_sr); */
        } else if (data_len < (total_len * HW_SYNC_LIMIT_DOWN / HW_SYNC_LIMIT_FULL)) {
            if (put_cnt > 50) {
                put_cnt = 0;
                //  putchar('s');
            }
            hw_sync.output_sr = ((hw_sync.output_sr + hw_sync.sr_step) > hw_sync.sr_upper_limit) ? \
                                hw_sync.sr_upper_limit : (hw_sync.output_sr + hw_sync.sr_step);
        } else {
            if (put_cnt1 > 50) {
                put_cnt1 = 0;
                //  putchar('n');
            }
            if (hw_sync.output_sr > hw_sync.sr_normal) {
                hw_sync.output_sr = ((hw_sync.output_sr - hw_sync.sr_step_normal) < hw_sync.sr_normal) ? \
                                    hw_sync.sr_normal : (hw_sync.output_sr - hw_sync.sr_step_normal);
            } else {
                hw_sync.output_sr = ((hw_sync.output_sr + hw_sync.sr_step_normal) > hw_sync.sr_normal) ? \
                                    hw_sync.sr_normal : (hw_sync.output_sr + hw_sync.sr_step_normal);
            }
            if (data_len < 1024) {
                putchar('D');
                /* log_printf("[sbc_data:%d]", data_len); */
            }
        }
    }


    src_p.in_rate = input_sr;
    src_p.out_rate = hw_sync.output_sr;
    //printf("[%d-%d]",input_sr,hw_sync.output_sr);
    src_ops.ioctl(SET_SRC_PARAM, 0, &src_p);
}

/*
***********************************************************************************
*					A2DP SYNC RUN
*
*Description: This function is called kick start SRC convertion
*
*Argument(s): buf:indat
*			  len:indat_len
*			  sr :a2dp decodec sample_rate
*
*Returns	: wlen:convert len
*
*Note(s)	:
***********************************************************************************
*/
u32 hw_sync_run(void *priv, void *buf, u32 len)
{
    s32 wlen = 0;

    if (hw_sync.dump_packets) {
        hw_sync.dump_packets--;
        memset(buf, 0x00, len);
    }

    if (hw_sync.toggle) {
        if (hw_sync.busy) {
            //putchar('#');
            return 0;
        };
        if (hw_sync.output->free_len(hw_sync.output->priv) < 1024) {
            return 0;
        }
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
        extern u8 get_reset_hw_sync_flag();
        extern void set_reset_hw_sync_flag(u8 flag);
        extern void clear_audio_stream_clear();
        extern void tws_sync_dec_packet_pcm(u32 res);
        if (get_reset_hw_sync_flag()) {
            puts("hw_sync_output clear\n");
            set_reset_hw_sync_flag(0);
            clear_hw_sync_buf();
            clear_audio_stream_clear();
            reset_hw_sync_src(SR44100);
            return 0;
        }
#endif

        hw_sync_AC(hw_sync.input_sr);
        hw_sync.busy = 1;
        if (len > SRC_IDAT_LEN_MAX) {
            wlen = SRC_IDAT_LEN_MAX;
        } else {
            wlen = len;
        }
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
        tws_sync_dec_packet_pcm(wlen);
#endif
        src_ops.run(buf, wlen);

    }
    return wlen;
}

/*
***********************************************************************************
*					HW SYNC EXIT
*
*Description: This function is called when dec end
*
*Argument(s): none
*
*Returns	: 0 = success
*			 !0 = false
*
*Note(s)	:
***********************************************************************************
*/
s32 hw_sync_exit(void)
{
    src_ops.exit(NULL);
    hw_sync.toggle = 0;
    puts("hw_sync_exit_OK\n");
    return 0;
}

static void hw_sync_clear(void *priv)
{
    if (hw_sync.output) {
        hw_sync.output->clear(hw_sync.output->priv);
    }
}

static tbool hw_sync_check(void *priv)
{
    if (hw_sync.output) {
        return hw_sync.output->check(hw_sync.output->priv);
    } else {
        return false;
    }
}
static u32 hw_sync_free_len(void *priv)
{
    if (hw_sync.output->free_len) {
        return hw_sync.output->free_len(hw_sync.output->priv);
    }
    return 0;
}

static u32 hw_sync_data_len(void *priv)
{
    if (hw_sync.output->data_len) {
        return hw_sync.output->data_len(hw_sync.output->priv);
    }
    return 0;
}

static void hw_sync_samplerate(void *priv, u16 sr, u8 wait)
{
    if (sr) {
        hw_sync.input_sr = sr;
    }
    hw_sync.output->set_sr(hw_sync.output->priv, hw_sync.sr_normal, wait);
}

AUDIO_STREAM *audio_sync_hw_input(AUDIO_STREAM *output, AUDIO_STREAM_PARAM *param, void *priv)
{
    if (output == NULL) {
        return NULL;
    }

    hw_sync_init(param->sr, param->ch);
    hw_sync.sync_buf = priv;
    /* if (hw_sync.sync_buf) {
        log_printf("total_len:%d\n", hw_sync.sync_buf->total_len);
        log_printf("data_len:%d\n", hw_sync.sync_buf->data_len);
    } */

    hw_sync.output = output;
    hw_sync.input.priv  	= (void *)NULL;
    hw_sync.input.output 	= (void *)hw_sync_run;
    hw_sync.input.clear 	= (void *)hw_sync_clear;
    hw_sync.input.check 	= (void *)hw_sync_check;
    hw_sync.input.data_len 	= (void *)hw_sync_data_len;
    hw_sync.input.free_len 	= (void *)hw_sync_free_len;
    hw_sync.input.set_sr = (void *)hw_sync_samplerate;


    return &(hw_sync.input);
}
