#include "music_decoder.h"
#include "strings.h"
#include "sdk_cfg.h"
#include "irq_api.h"
#include "common.h"
#include "clock.h"
#include "uart.h"
#include "audio/audio_stream.h"

/* #define MUSIC_DECODER_DEBUG_ENABLE */

extern int get_tws_linein_state();
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".music_decoder_bss")
#pragma data_seg(	".music_decoder_data")
#pragma const_seg(	".music_decoder_const")
#pragma code_seg(	".music_decoder_code")
#endif

#ifdef MUSIC_DECODER_DEBUG_ENABLE
#define music_decoder_printf	log_printf
#else
#define music_decoder_printf(...)
#endif


#define UPCASE(a)	\
	(((a)>='a' && (a)<='z') ? (a)-('a'-'A'):(a))



typedef enum __FORMAT_STATUS {
    FORMAT_OK = 0,
    FORMAT_OK_BUT_NO_SUPPORT,                 //格式正确但不支持的时候，不再继续检查
    FORMAT_ERR,
    FORMAT_NOT_DETECTED,
} _FORMAT_STATUS;



typedef struct __FFFR {
    u32 						output_cnt;;
    u8 							enable;
    volatile u8 				type;
} FFFR;


typedef struct __DEC_FILE {
    void 						*hdl;
    AUDIO_FILE 					*_io;
} DEC_FILE;


typedef struct __DECODER_OPS_CUR {
    void 						*hdl;
    audio_decoder_ops			*ops;
} DECODER_OPS_CUR;


typedef struct __ERR_DEAL_IO {
    void *priv;
    void (*cbk)(void *priv, u32 err);
} ERR_DEAL_IO;


typedef struct __DECODER_OPS {
    u32 						 cfg_cnt;
    DEC_CFG	 					*cfg_tab;
    DECODER_OPS_CUR				 cur;
} DECODER_OPS;

typedef struct __DECODER_CALLBACK {
    void(*dec_bf_cb)(void *);
    void(*dec_af_cb)(void *);
} DECODER_CALLBACK;

struct __MUSIC_DECODER {
    DECODER_OPS 				 dec_ops;
    DECODER_CALLBACK             dec_callback;
    struct if_decoder_io	 	 dec_io;
    AUDIO_STREAM				*output;
    ERR_DEAL_IO					 err_io;
    DEC_FILE 				 	 file;
    FFFR					 	 ff_fr;
    volatile MUSIC_DECODER_ST  	 status;
    volatile u8				  	 busy;
    u8							 read_err;
    MUSIC_DECODER_ST             ini_sta;
} ;

MUSIC_DECODER music_decoder_hdl;
static bool decoder_loop_enable = true;
static MUSIC_DECODER_ST decoder_ini_sta = MUSIC_DECODER_ST_PLAY;

static tbool music_decoder_decoder_match(const char *dec_name, const char *format_name)
{
    int i;
    if (*format_name == '\0') {
        return false;
    }
    for (i = 0; i < strlen(dec_name); i++) {
        if (UPCASE(dec_name[i]) != UPCASE(format_name[i])) {
            break;
        }
    }
    if (i == strlen(dec_name)) {
        return true;
    }

    return music_decoder_decoder_match(dec_name, format_name + strlen(dec_name));
}

static s32 music_decoder_decode_check_buf(void *priv, u32 faddr, u8 *buf)
{
    s32 read_len = 0;
    MUSIC_DECODER *dec = (MUSIC_DECODER *)priv;
    DEC_FILE *file;
    if (dec == NULL || dec->file._io == NULL) {
        return 0;
    }
    file = (DEC_FILE *) & (dec->file);
    if (file->_io->seek) {
        file->_io->seek(file->hdl, SEEK_SET, faddr);
    }
    if (file->_io->read) {
        read_len = file->_io->read(file->hdl, buf, 512);
    }

    if (read_len == (u16) - 1) {
        music_decoder_printf("\nfun = %s, line = %d\n", __func__, __LINE__);
        dec->read_err = 1;
        return 0;
    }
    return read_len;
}


static s32 music_decoder_decode_input(void *priv, u32 faddr, u8 *buf, u32 len, u8 type)
{
    MUSIC_DECODER *dec = (MUSIC_DECODER *)priv;
    DEC_FILE *file;
    if (dec == NULL || dec->file._io == NULL) {
        return 0;
    }
    file = (DEC_FILE *) & (dec->file);
    u16 read_len = 0;
    if (type == 0) {
        if (file->_io->seek) {
            file->_io->seek(file->hdl, SEEK_SET, faddr);
        }
        if (file->_io->read) {
            read_len = file->_io->read(file->hdl, buf, len);
        }
    }

    if ((u16) - 1 == read_len) {
        music_decoder_printf("\nfun = %s, line = %d\n", __func__, __LINE__);
        dec->read_err = 1;
        return 0;
    }

    return read_len;
}

static int music_decoder_decode_output(void *priv, void *buf, u32 len)
{
    MUSIC_DECODER *dec = (MUSIC_DECODER *)priv;

    int out_len = 0;
    if (dec) {
        if (dec->output) {
            if (dec->output->output) {
                if (dec->ff_fr.output_cnt > len) {
                    dec->ff_fr.output_cnt -= len;
                } else {
                    dec->ff_fr.output_cnt = 0;
                }

                out_len = dec->output->output(dec->output->priv, buf, len);
                if (out_len == len) {
                    music_decoder_loop_resume();
                }
            }
        }
    }

    return out_len;
}

static u32 music_decoder_decode_store_rev_data(void *priv, u32 addr, int len)
{
    return len;
}

static u32 music_decoder_decode_get_lslen(void *priv)
{
    MUSIC_DECODER *dec = (MUSIC_DECODER *)priv;
    DEC_FILE *file;
    if (dec == NULL || dec->file._io == NULL) {
        return 0;
    }
    file = (DEC_FILE *) & (dec->file);
    if (file->_io->get_size) {
        return file->_io->get_size(file->hdl);
    }

    return (u32) - 1;
}

SET_INTERRUPT
static void music_usbc_decoder_loop(void)
{
    irq_common_handler(IRQ_SOFT_USBC_IDX);
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
    tws_sbc_dec_co_media_run();
#endif

}
void music_usbc_decoder_loop_resume(void)
{
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
#if BT_TWS_LINEIN
    if (get_tws_linein_state()) {
        return;
    }
#endif
    MUSIC_DECODER *obj = &music_decoder_hdl;
    if (decoder_loop_enable == false) {
        return;
    }
    if (obj->status != MUSIC_DECODER_ST_STOP) {
        irq_set_pending(IRQ_SOFT_USBC_IDX);
    }
#endif
}

SET_INTERRUPT
static void music_decoder_loop(void)
{
    irq_common_handler(IRQ_SOFT_IDX);
    MUSIC_DECODER *obj = &music_decoder_hdl;
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
#if BT_TWS_LINEIN
    if (get_tws_linein_state()) {

        tws_sbc_dec_co_media_run();
    }
#endif
#endif
    music_decoder_decode_deal(obj);
}

void music_decoder_loop_resume(void)
{
    MUSIC_DECODER *obj = &music_decoder_hdl;
    if (decoder_loop_enable == false) {
        return;
    }
    if (obj->status != MUSIC_DECODER_ST_STOP) {
        irq_set_pending(IRQ_SOFT_IDX);
    }
}



MUSIC_DECODER *music_decoder_creat(void)
{
    MUSIC_DECODER *obj;

    memset((u8 *)&music_decoder_hdl, 0x0, sizeof(MUSIC_DECODER));
    obj = (MUSIC_DECODER *)&music_decoder_hdl;
    obj->ini_sta = MUSIC_DECODER_ST_PLAY;

    return obj;
}

void music_decoder_destroy(MUSIC_DECODER **hdl)
{
    if ((hdl == NULL) || (*hdl == NULL)) {
        return ;
    }
    MUSIC_DECODER *obj = *hdl;
    music_decoder_stop(obj);

    *hdl = NULL;
}

void music_decoder_set_file_interface(MUSIC_DECODER *obj, AUDIO_FILE *_io, void *hdl)
{
    if (obj == NULL) {
        return ;
    }
    obj->file.hdl = hdl;
    obj->file._io = _io;
}


void music_decoder_set_output(MUSIC_DECODER *obj, AUDIO_STREAM *output)
{
    if (obj == NULL) {
        return ;
    }

    obj->output = output;
}

void music_decoder_set_err_deal_interface(MUSIC_DECODER *obj, void (*cbk)(void *priv, u32 err), void *priv)
{
    if (obj == NULL) {
        return ;
    }
    obj->err_io.priv = priv;
    obj->err_io.cbk = cbk;
}


void music_decoder_set_configs(MUSIC_DECODER *obj, DEC_CFG cfg[], u32 cnt)
{
    if (obj == NULL) {
        return ;
    }
    if (cnt == 0) {
        return ;
    }
    obj->dec_ops.cfg_tab = cfg;
    obj->dec_ops.cfg_cnt = cnt;
}

void music_decoder_set_callback(MUSIC_DECODER *obj, void (*before_fun)(void *), void (*after_fun)(void *))
{
    if (obj == NULL) {
        return ;
    }
    if (before_fun) {
        obj->dec_callback.dec_bf_cb = before_fun;
    }

    if (after_fun) {
        obj->dec_callback.dec_af_cb = after_fun;
    }
}

void music_decoder_set_loop_en(tbool en)
{
    decoder_loop_enable = en;
}

void music_decoder_set_ini_sta(MUSIC_DECODER *obj, MUSIC_DECODER_ST sta)
{
    if (obj == NULL) {
        return ;
    }
    obj->ini_sta = sta;
}


void music_decoder_before_callback(void *priv)
{
    music_decoder_printf("%s\n", __func__);

    MUSIC_DECODER *obj = priv;

    if (obj == NULL) {
        return ;
    }
}

void music_decoder_after_callback(void *priv)
{
    music_decoder_printf("%s\n", __func__);

    MUSIC_DECODER *obj = priv;

    if (obj == NULL) {
        return ;
    }

    set_sys_freq(MUSIC_DECODE_Hz);
}


u32 music_decoder_get_decode_total_time(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return 0;
    }

    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    if (cur_ops->ops != NULL && cur_ops->hdl != NULL) {
        if (cur_ops->ops->get_dec_inf) {
            dec_inf_t *dec_inf = cur_ops->ops->get_dec_inf(cur_ops->hdl);
            if (dec_inf) {
                return dec_inf->total_time;
            }
        }
    }
    return 0;
}

u32 music_decoder_get_decode_cur_time(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return 0;
    }

    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    if (cur_ops->ops != NULL && cur_ops->hdl != NULL) {
        if (cur_ops->ops->get_playtime) {
            return cur_ops->ops->get_playtime(cur_ops->hdl);
        }
    }

    return 0;
}


MUSIC_DECODER_ST music_decoder_get_status(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return MUSIC_DECODER_ST_STOP;
    }
    return obj->status;
}

u32 music_decoder_get_break_point_size(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return 0;
    }
    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    if (cur_ops->hdl && cur_ops->ops && cur_ops->ops->need_bpbuf_size) {
        return cur_ops->ops->need_bpbuf_size(cur_ops->hdl);
    }
    return 0;
}

u8 *music_decoder_get_break_point_info(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return NULL;
    }

    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    if (cur_ops->hdl && cur_ops->ops && cur_ops->ops->get_bp_inf) {
        return (u8 *)cur_ops->ops->get_bp_inf(cur_ops->hdl);
    } else {
        return NULL;
    }
}

char *music_decoder_get_decode_name(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return NULL;
    }

    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    if (cur_ops->hdl && cur_ops->ops) {
        printf("music_decoder_get_decode_name = %s\n", cur_ops->ops->name);
        return cur_ops->ops->name;
    } else {
        printf("music_decoder_get_decode_name = NULL\n");
        return NULL;
    }
}

u32 music_decoder_play(MUSIC_DECODER *obj, DEC_BP *bp_info)
{
    u32 usbc_irq_prio = irq_index_to_prio(IRQ_SOFT_USBC_IDX);
    if (obj == NULL) {
        return MUSIC_DECODER_ERR_INIT_FAIL;
    }

    u32 i;
    u32 dec_need_buf_size;
    _FORMAT_STATUS fm_err = FORMAT_ERR;
    struct if_decoder_io *dec_io = &(obj->dec_io);

    if (obj->dec_callback.dec_bf_cb) {
        obj->dec_callback.dec_bf_cb(obj);
    }

    dec_io->priv = (void *)obj;
    dec_io->input = (void *)music_decoder_decode_input;
    dec_io->check_buf = (void *)music_decoder_decode_check_buf;
    dec_io->output = (void *)music_decoder_decode_output;
    dec_io->get_lslen = (void *)music_decoder_decode_get_lslen;
    dec_io->store_rev_data = (void *)music_decoder_decode_store_rev_data;

    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    DEC_CFG *cur_cfg = NULL;
    for (i = 0; i < obj->dec_ops.cfg_cnt;  i++) {
        cur_cfg = &(obj->dec_ops.cfg_tab[i]);
        if (cur_cfg->get_ops == NULL) {
            music_decoder_printf("no dec ops!!\n");
            continue;
        }
        cur_ops->ops = cur_cfg->get_ops();
        if (cur_ops->ops == NULL) {
            music_decoder_printf("empty dec ops!!\n");
            continue;
        }

        if (obj->file._io != NULL && obj->file._io->seek != NULL) {
            obj->file._io->seek(obj->file.hdl, SEEK_SET, 0);
        }

        dec_need_buf_size = cur_ops->ops->need_dcbuf_size();

        music_decoder_printf("dec need buf size %d\n", dec_need_buf_size);
        if (cur_cfg->dec_buf) {
            ASSERT(cur_cfg->dec_buf_size > dec_need_buf_size);
            music_decoder_printf("config ram size %d\n", cur_cfg->dec_buf_size);
            music_decoder_printf("use config ram !!\n");
            cur_ops->hdl = cur_cfg->dec_buf;
        } else {
            cur_ops->hdl = NULL;
            continue ;
        }

        if (cur_ops->ops->need_rdbuf_size) {
            music_decoder_printf("need_rdbuf_size = %d, fun = %s, line = %d\n", cur_ops->ops->need_rdbuf_size(), __func__, __LINE__);
        }

        u8 bp_enable = 0;
        if (bp_info) {
            music_decoder_printf("@@@@@@@@@@@@@@@break point size = %d\n", bp_info->bp_size);
            u32 bp_size = 0;
            if (cur_ops->ops->need_bpbuf_size) {
                bp_size = cur_ops->ops->need_bpbuf_size(cur_ops->hdl);
            }
            if (bp_size) {
                if ((bp_size == bp_info->bp_size) && (bp_info->bp_buf != NULL)) {
                    bp_enable = 1;
                }
            }
        }

        if (bp_enable) {
            cur_ops->ops->open(cur_ops->hdl, (const struct if_decoder_io *)dec_io, bp_info->bp_buf);
        } else {
            cur_ops->ops->open(cur_ops->hdl, (const struct if_decoder_io *)dec_io, NULL);
        }
        if (cur_cfg->format_check_enable) {
            if (cur_ops->ops->format_check) {
                fm_err = cur_ops->ops->format_check(cur_ops->hdl);
                if (fm_err == FORMAT_OK) {
                    music_decoder_printf("%s format check ok !!!\n", cur_ops->ops->name);
                    break;
                } else if (fm_err == FORMAT_OK_BUT_NO_SUPPORT) {
                    music_decoder_printf("%s format not support !!!\n", cur_ops->ops->name);
                    break;
                } else {
                    music_decoder_printf(" %s not support\n", cur_ops->ops->name);
                    cur_ops->hdl = NULL;
                    continue ;
                }
            } else {
                music_decoder_printf("formart check enable, but no check fun err !!\n");
                cur_ops->hdl = NULL;
                return MUSIC_DECODER_ERR_START_FAIL;
            }
        } else {
            music_decoder_printf(" %s without format check\n", cur_ops->ops->name);
            fm_err = FORMAT_OK;
            break;
        }
    }

    if (cur_cfg == NULL) {
        music_decoder_printf("decoder configs err !!\n");
        cur_ops->hdl = NULL;
        return MUSIC_DECODER_ERR_START_FAIL;
    }

    /* music_decoder_printf("fun = %s, line = %d\n", __func__, __LINE__);		 */
    if (FORMAT_OK != fm_err) {
        cur_ops->hdl = NULL;
        return MUSIC_DECODER_ERR_START_FAIL;
    }

    ///fffr enable
    obj->ff_fr.enable = cur_cfg->fffr_enable;
    ///get id3

    ///设置解码输出处理类型
    if (cur_ops->ops->dec_confing) {
        ///choose output data control
        AUDIO_DECODE_PARA dec_mode_obj;
        dec_mode_obj.mode = 1;/*1 need return writeout len when dec output, 0 not*/
        cur_ops->ops->dec_confing(cur_ops->hdl, SET_DECODE_MODE, (void *)&dec_mode_obj);
        if (cur_cfg->priv_setting) {
            cur_ops->ops->dec_confing(cur_ops->hdl, SET_SPECIFIC_PARAMETERS, (void *)(cur_cfg->priv_setting));
        }
    }

    if (obj->output != NULL) {
        if (obj->output->clear != NULL) {
            obj->output->clear(obj->output->priv);
        }

        if (obj->output->set_sr != NULL) {
            dec_inf_t *inf = cur_ops->ops->get_dec_inf(cur_ops->hdl);
            obj->output->set_sr(obj->output->priv, inf->sr, 1);
        }
    }

    obj->busy = 1;
//    obj->status = MUSIC_DECODER_ST_PLAY;
    obj->status = obj->ini_sta;
    if (obj->ini_sta != MUSIC_DECODER_ST_PLAY) {
        obj->ini_sta = MUSIC_DECODER_ST_PLAY;
    }
    if (obj->dec_callback.dec_af_cb) {
        obj->dec_callback.dec_af_cb(obj);
    }

    music_decoder_printf("music player play ok!!\n");
    irq_handler_register(IRQ_SOFT_IDX, music_decoder_loop, irq_index_to_prio(IRQ_SOFT_IDX));
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
#if ECHO_EN
    usbc_irq_prio = 1;
#endif
    if (!get_tws_linein_state()) {

        irq_handler_register(IRQ_SOFT_USBC_IDX, music_usbc_decoder_loop, usbc_irq_prio);
    }
#endif

    return MUSIC_DECODER_ERR_NONE;
}


tbool music_decoder_set_dec_confing(MUSIC_DECODER *obj, u8 cmd, void *parm)
{
    if (obj == NULL) {
        return false;
    }

    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    if (cur_ops->hdl == NULL) {
        return false;
    }

    if ((obj->status == MUSIC_DECODER_ST_STOP) || (obj->busy == 0)) {
        return false;
    }

    if (cur_ops->ops->dec_confing) {
        cur_ops->ops->dec_confing(cur_ops->hdl, cmd, (void *)parm);
        return true;
    }

    return false;
}

tbool music_decoder_set_ab_repeat(MUSIC_DECODER *obj, u8 cmd, void *parm)
{
    if (obj == NULL) {
        return false;
    }

    if (music_decoder_set_dec_confing(obj, cmd, parm)) {
        return true;
    }

    return false;
}

void music_decoder_decode_deal(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return ;
    }

    u32 res = 0;
    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    if (cur_ops->hdl == NULL) {
        return ;
    }

    if ((obj->status == MUSIC_DECODER_ST_STOP) || (obj->busy == 0)) {
        return ;
    }

    if (obj->status == MUSIC_DECODER_ST_PLAY) {
        u8 fffr = PLAY_MOD_NORMAL;
        if (obj->ff_fr.enable) {
            fffr = obj->ff_fr.type;
            obj->ff_fr.type = PLAY_MOD_NORMAL;
            if (fffr > PLAY_MOD_FB) {
                fffr = PLAY_MOD_NORMAL;
            }
        }
        res = cur_ops->ops->run(cur_ops->hdl, fffr);
        if (obj->err_io.cbk) {
            obj->err_io.cbk(obj->err_io.priv, res);
        }

        if (res == MAD_ERROR_FF_FR_FILE_START) {
            res = 0;
        }

        if (res) {
            if (music_decoder_decoder_match(cur_ops->ops->name, "AACSBCSBU") == true) {
                if (res == 99) {

                    cur_ops->ops->dec_confing(cur_ops->hdl, SET_CLEAR_STATE,  NULL);

                    //music_decoder_printf("AACSBC err!!\n");
                }
                res = 0;
            } else {
                if (obj->read_err == 0) {
                    if (res == MAD_ERROR_NODATA) {
                        res = MAD_ERROR_FILE_END;
                    }
                }
            }
        } else {
            if (music_decoder_decoder_match(cur_ops->ops->name, "MP3AACSBCSBU") == true) {
                if (cur_ops->ops->get_dec_inf) {
                    dec_inf_t *dec_inf = cur_ops->ops->get_dec_inf(cur_ops->hdl);

                    if (obj->output != NULL) {
                        if (obj->output->set_sr != NULL) {
                            if (cur_ops->ops->get_dec_inf) {
                                /* printf("*"); */
                                dec_inf_t *inf = cur_ops->ops->get_dec_inf(cur_ops->hdl);
                                obj->output->set_sr(obj->output->priv, inf->sr, 1);
                            }
                        }
                    }
                }
            }
        }


#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
        if (tws_get_reset_decode_en()) {
            tws_clear_reset_decode_en();
            extern void set_reset_hw_sync_flag(u8 flag);
            set_reset_hw_sync_flag(1);
            puts("reset decode\n");
            cur_ops->ops->dec_confing(cur_ops->hdl, SET_RESET,  NULL);
            AUDIO_DECODE_PARA dec_mode_obj;
            dec_mode_obj.mode = 1;/*1 need return writeout len when dec output, 0 not*/
            cur_ops->ops->dec_confing(cur_ops->hdl, SET_DECODE_MODE, (void *)&dec_mode_obj);
        }
#endif

    }

    if (res) {
        obj->busy = 0;
    }
}


void music_decoder_stop(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return ;
    }
    if (obj->status != MUSIC_DECODER_ST_STOP) {
        obj->status = MUSIC_DECODER_ST_STOP;

        if (obj->output != NULL) {
            if (obj->output->check != NULL) {
                while (obj->output->check(obj->output->priv) == true) {
                    /* putchar('O'); */
                }
            }
        }
    }
    music_decoder_printf("-----------------music_decoder_stop-----------------------\n");

    IRQ_RELEASE(IRQ_SOFT_IDX);
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
    IRQ_RELEASE(IRQ_SOFT_USBC_IDX);
#endif

    DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
    cur_ops->hdl = NULL;
    //clear all last play status
}

MUSIC_DECODER_ST music_decoder_pp(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return MUSIC_DECODER_ST_STOP;
    }
    MUSIC_DECODER_ST wait_st = MUSIC_DECODER_ST_STOP;
    if (obj->status == MUSIC_DECODER_ST_PLAY) {
        obj->status = MUSIC_PLAYRR_ST_PAUSE;
    } else {
        obj->status = MUSIC_DECODER_ST_PLAY;
    }
    return obj->status;
}

MUSIC_DECODER_ST music_decoder_pause(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return MUSIC_DECODER_ST_STOP;
    }
    MUSIC_DECODER_ST wait_st = MUSIC_DECODER_ST_STOP;
    //if (obj->status == MUSIC_DECODER_ST_PLAY) {
        obj->status = MUSIC_PLAYRR_ST_PAUSE;
    //} else {
    //    obj->status = MUSIC_DECODER_ST_PLAY;
    //}
    return obj->status;
}

MUSIC_DECODER_ST music_decoder_play_resume(MUSIC_DECODER *obj)
{
    if (obj == NULL) {
        return MUSIC_DECODER_ST_STOP;
    }
    MUSIC_DECODER_ST wait_st = MUSIC_DECODER_ST_STOP;
    //if (obj->status == MUSIC_DECODER_ST_PLAY) {
    //    obj->status = MUSIC_PLAYRR_ST_PAUSE;
    //} else {
        obj->status = MUSIC_DECODER_ST_PLAY;
    //}
    return obj->status;
}

void music_decoder_fffr(MUSIC_DECODER *obj, u8 type, u8 second)
{
    if (obj == NULL) {
        return ;
    }

    if (obj->ff_fr.enable == 0) {
        return;
    }

    if (type > PLAY_MOD_FB) {
        return;
    }

    if (obj->status == MUSIC_PLAYRR_ST_PAUSE) {
        obj->status = MUSIC_DECODER_ST_PLAY;
    }

    if (obj->ff_fr.output_cnt == 0) {
        DECODER_OPS_CUR *cur_ops = &(obj->dec_ops.cur);
        obj->ff_fr.type = type;
        if (cur_ops->ops->set_step) {
            cur_ops->ops->set_step(cur_ops->hdl, second);
        }
        if (cur_ops->ops->get_dec_inf) {
            dec_inf_t *dec_inf = cur_ops->ops->get_dec_inf(cur_ops->hdl);
            obj->ff_fr.output_cnt = ((dec_inf->sr * 4) / 10) * dec_inf->nch;
        }
    }
}

