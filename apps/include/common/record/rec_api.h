#ifndef _REC_API_
#define _REC_API_
#include "typedef.h"
#include "encode.h"
#include "file_operate.h"
#include "file_op_err.h"

//#define REC_API_DEBUG_ENABLE
#define REC_FILE     0//1：录MP3文件       0：录WAV文件
#define REC_DEL      0 //录音删除功能
#define REC_SOURCE   1 //录音源  1：dac  0:other
#define SET_ENC_BUF  1 //是否设置编码缓存，为0是使用默认最小缓存

#if SET_ENC_BUF

#if REC_FILE
#define ENC_INPUT_BUF_SIZE      2*1024  //设置编码输入BUFF大小
#define ENC_OUTPUT_BUF_SIZE     9*1024  //设置编码输出大小
#define REC_POOL_SIZE   (ENC_OUTPUT_BUF_SIZE + ENC_INPUT_BUF_SIZE + 10*1024)
#else
#define ENC_INPUT_BUF_SIZE      2*1024  //设置编码输入BUFF大小
#define ENC_OUTPUT_BUF_SIZE     11*1024  //设置编码输出大小
#define REC_POOL_SIZE   (ENC_OUTPUT_BUF_SIZE + ENC_INPUT_BUF_SIZE + 2*1024)
#endif

#else

#if REC_FILE
#define REC_POOL_SIZE   16*1024   //16K 给rec使用
#else
#define REC_POOL_SIZE   8*1024   //8K 给rec使用
#endif

#endif

#ifdef REC_API_DEBUG_ENABLE
#define rec_api_printf	printf
#else
#define rec_api_printf(...)
#endif

#define FILE_NUM_LOCA	12		//文件名序号位置

#define REC_ASSERT(x,y)    do{if(NULL == x){goto y;}}while(0)

typedef struct _REC_FILE_INFO {
    void *rec_dev;
    u32 rec_fname_cnt;
    u32 file_number;
    u32 rec_file_sclust;
} REC_FILE_INFO;

typedef struct _RECORD_OP_API_ {
    ENC_CTL   *enc_ctl;
    void *output_hd;
    void *input_hd;
} RECORD_OP_API;

typedef enum {
    REC_MIC_CHANNEL = 0,
    REC_LINEIN_CHANNEL,
    REC_FM_CHANNEL,
    REC_BT_CHANNEL,
} REC_CHANNEL;

void *rec_input_init(void *enc_input_io, u8 ch, u32 sr);
void rec_input_exit(void **input_hd_p);
void rec_input(void *rec_input_hdl, void *buff, u32 buffer_len);
void rec_ladc_data_cb(s16 *buff, u32 buffer_len);
void rec_dac_data_input(s16 *buff, u32 buffer_len, u8 nch);
void *rec_out_init();
s16 rec_out_exit(void **out_hd_p);
s16 rec_out_seek(void *out_hd, u32 offsize);
u32 rec_out_tell(void *out_hd);
s16 rec_out_put(void *out_hd, u8 *buff, u32 len);

RECORD_OP_API  *rec_init(u8 ch);
void rec_exit(RECORD_OP_API **rec_api_p);
void rec_msg_deal(RECORD_OP_API **rec_api_p, int msg);
u32 rec_get_enc_time(RECORD_OP_API *rec_api);
u32 rec_get_enc_sta(RECORD_OP_API *rec_api);
void *rec_get_cur_dev(RECORD_OP_API *rec_api);
void rec_set_loop_en(bool en);

#if REC_EN
#define rec_msg_deal_api(x,y)       rec_msg_deal(x,y)
#else
#define rec_msg_deal_api(...)
#endif

#endif
