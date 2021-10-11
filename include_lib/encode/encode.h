#ifndef __ENCODE_H__
#define __ENCODE_H__

#include "typedef.h"
#include "audio_enc_api.h"
#include "circular_buf.h"

#define REC_MALLOC          0   //录音模块是否使用内存分配
#define ENCODE_ADPCM_EN     1
#define ENCODE_MP2_EN       1

typedef enum {
    UNUSED_FORMAT = 0,      //标记

#if ENCODE_MP2_EN
    MP2_FORMAT,
#endif

#if ENCODE_ADPCM_EN
    ADPCM_FORMAT,
#endif

    MAX_FORMAT,
} ENC_FORMAT;

typedef enum {
    ENC_STOP = 0,
    ENC_STAR,
    ENC_PAUS,
} ENC_STA;      ///录音状态

typedef struct _ADPCM_HEAD {
    u8 head_buf[512];
    u16 flag;
} ADPCM_HEAD;

typedef struct _REC_LOST_FRAME {
    u32 front_lost_frame;
    u32 black_lost_frame;
} REC_LOST_FRAME;

typedef struct _ENC_FILE_INFO {
    u32 file_size;
    u32 file_del_size;
    u32 enc_frame_cnt;
    u32 enc_time_cnt;
    u32 cut_head_ms;	//单位0.1ms
    u32 cut_tail_ms;	//单位0.1ms
} ENC_FILE_INFO;

typedef struct _ENC_OUTPUT_IO {
    void *priv;
    s16(*output)(void *priv, u8 *buff, u32 len);
    s16(*seek)(void *priv, u32 offsize);
    u32(*tell)(void *priv);
} ENC_OUTPUT_IO;

typedef struct _ENC_INPUT_IO {
    void *priv;
    void (*input)(void *priv, void *buff, u32 len);
} ENC_INPUT_IO;

typedef struct _ENC_CTL {
    ENC_INPUT_IO input_io;
    cbuffer_t *input_cbuf;
    u8 *input_buf;
    ENC_OUTPUT_IO output_io;
    cbuffer_t *output_cbuf;
    u8 *output_buf;
    u8 *buf_ptr;
    ENC_DATA_INFO *enc_info;
    EN_FILE_IO    *en_io;
    ENC_OPS   *enc_ops;
    ADPCM_HEAD *file_head_info;
    volatile ENC_STA enable;
    //volatile bool rec_busy;
    volatile bool wf_busy;
    ENC_FILE_INFO file_info;
    REC_LOST_FRAME lost_frame;
    u16 enc_fram_size;///>录音帧大小，单位（s16），用于FM录音
    u8  enc_format;
} ENC_CTL;

enum {
    ERR_ENCODE_NO_ERR = 0x00,
    ERR_ENCODE_API_NULL,
    ERR_ENCODE_IO_NULL,
    ERR_ENCODE_OUT_ERR,
    ERR_ENCODE_STOP,
    ERR_ENCODE_IN_LOST_FRAME,
    ERR_ENCODE_OUT_LOST_FRAME,
    ERR_ENCODE_RUN_ERR,
};



bool enc_info_init(ENC_CTL *enc_ctl, u16 ch_cnt, u16 br, u16 sr);
u32 get_enc_time(ENC_CTL *enc_ctl);   //获取时间
u32 updata_enc_time(ENC_CTL *enc_ctl);//建议半秒更新
void encode_err_deal_cbk_register(void (*cbk)(u32 err));
void encode_input_end_cbk_register(void (*cbk)(void *priv));
void enc_run_start(ENC_CTL *enc_ctl);
void encode_input_buf_len(u32 len);//默认值：DAC_DUAL_BUF_LEN*50
void encode_output_buf_len(u32 len);//默认值： 2.5K(other)

void encode_run();
u32 enc_out_run();
void encode_run_loop();
ENC_CTL *encode_open(u8 ch_cnt, u16 br, u16 sr, ENC_FORMAT format);
void encode_close(ENC_CTL *enc_ctl);
void enc_run_start(ENC_CTL *enc_ctl);
ENC_STA encode_pp(ENC_CTL *enc_ctl);
ENC_STA encode_get_status(ENC_CTL *enc_ctl);

#if (REC_MALLOC == 1)
#include "mem/malloc.h"
#define rec_malloc		malloc
#define	rec_free		free
#define	rec_free_fun	free_fun
void free_fun(void **ptr);
#else
void  rec_alloc_init(void *alloc_buffer, u32 buffer_len);
void *rec_malloc(u32 size);
void rec_free_fun(void **ptr);
#endif

#endif/*__ENCODE_H__*/
