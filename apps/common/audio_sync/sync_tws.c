#include "sync_software.h"
#include "string.h"
#include "audio/audio.h"
#include "audio/dac_api.h"
#include "uart.h"
#include "sdk_cfg.h"
#include "audio/dac.h"
#include "audio/syn_context_api.h"
#include "audio/syn_tws.h"
#include "bluetooth_api.h"
#include "bt_tws.h"

#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
extern void music_decoder_loop_resume(void);
extern void music_usbc_decoder_loop_resume(void);
extern u32 tws_audio_read(u32 prt, u32 len, u8 cmd);
extern u16 get_audio_stream_dac_len();
extern u16 dac_get_samplerate(void);
extern void clear_audio_stream_clear();
extern void dac_int_disable(void);
extern void dac_int_enable(void);
extern void clear_dac_dma_buf();
extern u16 get_audio_stream_total_len();
extern void tws_sync_set_timer2_us(u32 time_us, void (*fun)());
extern void reset_hw_sync_src(u32 rate);
extern bool get_esco_busy_flag();
typedef  struct _TWS_SYN_ {
    tws_sync_parm_t tws_sync_parm;
    u8 toggle;					/*a2dp sync toggle		*/
    u8 nch;
    u16 sr;
    u8 esco_sync_flag;
    AUDIO_STREAM input;
    AUDIO_STREAM *output;
    cbuffer_t *sync_buf;
} TWS_SYN;

TWS_SYN  tws_sync = {
    .toggle = 0,
    .output = NULL,
    .sync_buf = NULL,
#if 0
    .tws_sync_parm.tws_audio_read = NULL,
    .tws_sync_parm.get_audio_stream_dac_len = NULL,
    .tws_sync_parm.dac_get_samplerate = NULL,
    .tws_sync_parm.clear_audio_stream_clear = NULL,
    .tws_sync_parm.dac_int_disable = NULL,
    .tws_sync_parm.dac_int_enable = NULL,
    .tws_sync_parm.clear_dac_dma_buf = NULL,
    .tws_sync_parm.tws_sync_inc_dac_cnt = NULL,
    .tws_sync_parm.tws_sync_inc_dac_read  = NULL,
#endif
};

static SYN_API_CONTEXT  *tws_sync_ops = NULL;
static u8 twsSyncBuf[450] sec(.tws_sync_buf_sec);

void tws_sync_inc_dac_cnt()
{
    if (tws_sync.tws_sync_parm.tws_sync_inc_dac_cnt) {
        tws_sync.tws_sync_parm.tws_sync_inc_dac_cnt();
    }
}
int tws_sync_inc_dac_read()
{
    if (tws_sync.tws_sync_parm.tws_sync_inc_dac_read) {
        return tws_sync.tws_sync_parm.tws_sync_inc_dac_read();
    }
    return 0;

}
u8 get_tws_single_flag()
{
    return tws_sync.tws_sync_parm.custom[TWS_CUSTOM_ASB_LEN_FLAG];
}
u8 get_tws_ps_flag()
{
    return  tws_sync.tws_sync_parm.tws_ps_flag;
}
void clear_tws_ps_flag()
{
    tws_sync.tws_sync_parm.tws_ps_flag = 0;
}
void tws_sync_dec_packet_pcm(u32 res)
{
    tws_sync.tws_sync_parm.dec_packet_pcm_cnt += (res / 4);
}
void set_reset_hw_sync_flag(u8 flag)
{
    tws_sync.tws_sync_parm.custom[TWS_CUSTOM_RESET_HW_SYNC] = flag;
}
u8 get_reset_hw_sync_flag()
{
    return tws_sync.tws_sync_parm.custom[TWS_CUSTOM_RESET_HW_SYNC];
}

AT_AUDIO
u8 tws_get_conn_working()
{
    if (tws_sync.tws_sync_parm.tws_conn_working) {

    }
    return tws_sync.tws_sync_parm.tws_conn_working;

}

AT_AUDIO
void tws_set_dac_ide()
{
    if (tws_sync.tws_sync_parm.tws_conn_working) {
        //tws_sync.tws_sync_parm.dac_ide = 1;
    }
}
static u32 tws_sync_getlen(void *priv)
{
    if (tws_sync.sync_buf) {
        return tws_sync.sync_buf->data_len;
    } else if (tws_sync.output->free_len) {
        return tws_sync.output->data_len(tws_sync.output->priv);
    }
    return 0;
}

static u32 tws_sync_output(void *priv, u8 *buf, int len)
{
    return tws_sync.output->output(tws_sync.output->priv, buf, len);
}

u32 tws_syn_callback(unsigned int len)
{
    tws_sync_parm_t *sy_obj = &tws_sync.tws_sync_parm;
    /* STRUCT_SYN *sy_obj=XXXX;                //这里就是同步那个结构体 */
    u32 tmpv_len, tmp_pos;

    /* CPU_SR_ALLOC(); */
    CPU_INT_DIS();
    sy_obj->PCM_unsing += len;
    sy_obj->PCM_unsing = sy_obj->PCM_unsing & 0x3fffffff;
    tmpv_len = (get_audio_stream_total_len() - get_audio_stream_dac_len()) >> 2;
    tmp_pos = sy_obj->PCM_unsing + tmpv_len + 1000;
    sy_obj->pcm_pos = tmp_pos;
    CPU_INT_EN();
    return 0;
}
static void tws_sync_init(u16 ch, u16 sr)
{
    u32 bufsize;
    u32 output_total_len;
    SYNC_coef tws_sync_param;
    tws_sync_parm_t *sy_obj = &tws_sync.tws_sync_parm;

    if (tws_sync.sync_buf) {
        output_total_len = tws_sync.sync_buf->total_len;
    } else {
        output_total_len = ((AUDIO_STREAM_DAC *)tws_sync.output->priv)->buf_size;
    }

    tws_sync_param .priv = NULL;
    tws_sync_param.output = tws_sync_output;

    tws_sync_param.autosync = 0;
    tws_sync_param.speed_rate = 6;
    tws_sync_param.samplerate = SR44100;
    /* if (tws_sync.tws_sync_parm.custom[TWS_CUSTOM_ASB_LEN_FLAG] &&tws_sync.tws_sync_parm.set_ch_mode&&(!tws_sync.esco_sync_flag)) { */
    if (tws_sync.tws_sync_parm.custom[TWS_CUSTOM_ASB_LEN_FLAG] && (!tws_sync.esco_sync_flag)) {
        tws_sync_param.nch = (ch | 0x8);
    } else {
        tws_sync_param.nch = (ch);
    }
    tws_sync_param.priv = NULL;
    tws_sync_param.getlen = tws_sync_getlen;
    tws_sync_param.output = tws_sync_output;
    tws_sync_param.Limit_up = 9 * 1024 ;
    tws_sync_param.limit_down = 6 * 1024;
    tws_sync_param.more_len = 45 ; //要变多的长度，有效设置范围16-90，对应变多6ms到20ms的数据，按44100采样率就是256到900 对点
    /* tws_sync_param.tws_setflag = 0; */
    /* tws_sync_param.Limit_up = output_total_len * SW_SYNC_LIMIT_UP / SW_SYNC_LIMIT_FULL; */
    /* tws_sync_param.limit_down = output_total_len * SW_SYNC_LIMIT_DOWN / SW_SYNC_LIMIT_FULL; */


    /*******tws work***********/
    tws_sync_param.tws_pcm_cnt = &sy_obj->pcm_cnt;
    tws_sync_param.tws_flag = &sy_obj->flag;
    tws_sync_param.tws_pcm_phase = &sy_obj->pcm_phase;
    tws_sync_param.tws_callback = tws_syn_callback;
    tws_sync_param.tws_setflag = 1;

    /* printf("Limit_up:%d\n", sync_param.Limit_up); */
    /* printf("limit_down:%d\n", sync_param.limit_down); */
    tws_sync_ops = get_sync_unit_api();
    /* printf("<<<<<<tws_sync_ops=0x%x>>>>>>>",tws_sync_ops ); */
    if (tws_sync_ops) {
        bufsize = tws_sync_ops->need_size(tws_sync_param.nch & 3, tws_sync_param.samplerate);
        printf("<<<<twsSyncBuf>>>:%d,%x\n", bufsize, tws_sync_param.nch);
        if (sizeof(twsSyncBuf) >= bufsize) {
            tws_sync_ops->open(twsSyncBuf, &tws_sync_param);
            tws_sync.toggle = 1;
        } else {
            tws_sync.toggle = 0;
            puts("twsWARE_SYNC mem err\n");
        }
    } else {
        tws_sync.toggle = 0;
    }
}

static u32 tws_sync_run(void *priv, void *buf, u32 len)
{
    u8 cnt = 0;
    u32 res = 0;
    u32 out_put_frame = 0;

    if (tws_sync.toggle) {


        if (tws_sync.tws_sync_parm.pre_packet_pcm_cnt) {
            /* tws_sync.tws_sync_parm.tws_ps_flag = 0; */
        }
        extern void clear_wdt(void);
        clear_wdt();
        res = tws_sync_ops->run(twsSyncBuf, buf, len, 0);
        /* tws_sync.tws_sync_parm.tws_ps_flag = 0; */

        if (!tws_sync.esco_sync_flag) {
            tws_sync.tws_sync_parm.frame_out_cnt += (res / 4);
            out_put_frame = (tws_sync.tws_sync_parm.frame_out_cnt + tws_sync.tws_sync_parm.frame_pcm_len - 1) / (tws_sync.tws_sync_parm.frame_pcm_len) ;
            if (tws_sync.tws_sync_parm.dec_remain_frame) {
                tws_sync.tws_sync_parm.dec_remain_frame = tws_sync.tws_sync_parm.dec_in_total_frame - out_put_frame;
            }

            /* tws_sync.tws_sync_parm.dec_packet_pcm_cnt += (res / 4); */
            if ((res == len) && tws_sync.tws_sync_parm.pre_packet_pcm_cnt && (tws_sync.tws_sync_parm.dec_packet_pcm_cnt == tws_sync.tws_sync_parm.pre_packet_pcm_cnt)) {
                tws_sync.tws_sync_parm.tws_master_reset_sync_parm();
                tws_sync.tws_sync_parm.pre_packet_pcm_cnt = 0;
                reset_hw_sync_src(SR44100);
                /* puts("tws_master_reset_sync_parm\n"); */
            }

        }
        if (res != len) {
            //printf("[%d]",res);
            //putchar('.');
        }
        return res;
    } else {
        return tws_sync.output->output(tws_sync.output->priv, buf, len);
    }
}

static void tws_sync_clear(void *priv)
{
    tws_sync.output->clear(tws_sync.output->priv);
}

static tbool tws_sync_check(void *priv)
{
    return tws_sync.output->check(tws_sync.output->priv);
}

static u32 tws_sync_free_len(void *priv)
{
    if (tws_sync.output->free_len) {
        return tws_sync.output->free_len(tws_sync.output->priv);
    }
    return 0;
}

static u32 tws_sync_data_len(void *priv)
{
    if (tws_sync.output->data_len) {
        return tws_sync.output->data_len(tws_sync.output->priv);
    }
    return 0;
}

static void tws_sync_samplerate(void *priv, u16 sr, u8 wait)
{
    if ((tws_sync.sr != sr) && (sr != 0)) {
        printf("twssync SR:%d\n", sr);
        tws_sync.sr = sr;
        tws_sync_init(tws_sync.nch, tws_sync.sr);
    }
    tws_sync.output->set_sr(tws_sync.output->priv, sr, wait);
}

AUDIO_STREAM *audio_sync_tws_input(AUDIO_STREAM *output, AUDIO_STREAM_PARAM *param, void *priv)
{
    if (output == NULL) {
        return NULL;
    }

    tws_sync.output = output;
    tws_sync.input.priv  	= (void *)NULL;
    tws_sync.input.output 	= (void *)tws_sync_run;
    tws_sync.input.clear 	= (void *)tws_sync_clear;
    tws_sync.input.check 	= (void *)tws_sync_check;
    tws_sync.input.data_len 	= (void *)tws_sync_data_len;
    tws_sync.input.free_len 	= (void *)tws_sync_free_len;
    tws_sync.input.set_sr 	= (void *)tws_sync_samplerate;
    tws_sync.sync_buf = priv;
    tws_sync.nch = param->ch;
    tws_sync.sr = param->sr;

    tws_sync.esco_sync_flag	 = 0;
    tws_sync_init(tws_sync.nch, tws_sync.sr);

    return &(tws_sync.input);
}

static void __tws_sync_init()
{
    tws_sync_init(tws_sync.nch, tws_sync.sr);
}
void tws_dac_read_disble()
{
    dac_int_disable();
}
void tws_dac_read_enble()
{
    dac_int_enable();

}
u8 tws_get_reset_decode_en()
{
    return tws_sync.tws_sync_parm.reset_decode_en;
}
u8 tws_clear_reset_decode_en()
{
    tws_sync.tws_sync_parm.reset_decode_en = 0;
    return 0;
}
void debug_tws_date_state(u16 debug_lbuf_alloc_total, u16 send_packet_num, u16 get_packet_num, u32 cbuf_date_size, u16 send_sbc_list_cnt)
{
#if 0
    CPU_INT_DIS();
    u16 debug_lbuf_alloc_total_t = debug_lbuf_alloc_total;
    u16 send_packet_num_t = send_packet_num;
    u16 get_packet_num_t = get_packet_num;
    u32 cbuf_date_size_t = cbuf_date_size;
    u16 send_sbc_list_cnt_t = send_sbc_list_cnt;
    CPU_INT_EN();
    if (debug_lbuf_alloc_total_t < (send_packet_num + get_packet_num + send_sbc_list_cnt)) {
        printf(" %d-%d-%d-%d-%d ", debug_lbuf_alloc_total_t, send_packet_num_t, get_packet_num_t, cbuf_date_size_t, send_sbc_list_cnt_t);

    }
#endif

}

void tws_music_decoder_loop_resume(void)
{
    music_usbc_decoder_loop_resume();
    music_decoder_loop_resume();
}
void *get_tws_sync_parm()
{
    u16 tws_sbc_data_variable_min = (5 * 1024 - 512); //最低门限值进行变包设置，不能设置那么大，而影响音质，也不能设置那么小，会影响距离
    u16 tws_sbc_data_variable_max = (7 * 1024 + 256);
    tws_sync.tws_sync_parm.set_user_ch = 0;//0:master left  1:master right
    tws_sync.tws_sync_parm.set_ch_mode = 0;//0:声道独立   1:opL = opR = (L+R)/2
    tws_sync.tws_sync_parm.sbc_bitpool_value = 24;
    tws_sync.tws_sync_parm.tws_audio_read = tws_audio_read;
    tws_sync.tws_sync_parm.get_audio_stream_dac_len = get_audio_stream_dac_len;
    tws_sync.tws_sync_parm.dac_get_samplerate = dac_get_samplerate;
    tws_sync.tws_sync_parm.clear_audio_stream_clear = clear_audio_stream_clear;
    tws_sync.tws_sync_parm.dac_int_disable = tws_dac_read_disble;
    tws_sync.tws_sync_parm.dac_int_enable = tws_dac_read_enble;
    tws_sync.tws_sync_parm.clear_dac_dma_buf = clear_dac_dma_buf;
    tws_sync.tws_sync_parm.tws_sync_init  = __tws_sync_init ;
    tws_sync.tws_sync_parm.tws_music_decoder_loop_resume  = tws_music_decoder_loop_resume  ;
    tws_sync.tws_sync_parm.tws_sync_set_timer2_us = tws_sync_set_timer2_us ;
    tws_sync.tws_sync_parm.debug_tws_date_state  = debug_tws_date_state ;
    memcpy(&tws_sync.tws_sync_parm.custom[TWS_CUSTOM_VARIABLE_MIN_1], &tws_sbc_data_variable_min, sizeof(tws_sbc_data_variable_min));
    memcpy(&tws_sync.tws_sync_parm.custom[TWS_CUSTOM_VARIABLE_MAX_1], &tws_sbc_data_variable_max, sizeof(tws_sbc_data_variable_max));
    tws_sync.tws_sync_parm.custom[TWS_CUSTOM_VARIABLE_TIME] = 1;
    tws_sync.tws_sync_parm.custom[TWS_CUSTOM_RESTART_SBC] = 1;



    return (void *)(&tws_sync.tws_sync_parm);
}
#endif
