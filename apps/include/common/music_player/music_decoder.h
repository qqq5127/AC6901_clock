#ifndef __MUSIC_DECODER_H__
#define __MUSIC_DECODER_H__
#include "typedef.h"
#include "dec/if_decoder_ctrl.h"
#include "audio/audio_stream.h"

enum {
    MAD_ERROR_FILE_END         = 0x40,
    MAD_ERROR_FILESYSTEM_ERR   = 0x41,              // NO USED
    MAD_ERROR_DISK_ERR         = 0x42,              // NO USED
    MAD_ERROR_SYNC_LIMIT       = 0x43,              // 文件错误
    MAD_ERROR_FF_FR_FILE_END   = 0x44,              //快进结束
    MAD_ERROR_FF_FR_END        = 0x45,              // NO USED
    MAD_ERROR_FF_FR_FILE_START = 0x46,              //快退到头
    MAD_ERROR_LIMIT            = 0x47,              // NO USED
    MAD_ERROR_NODATA           = 0x48,              // NO USED
};


typedef enum {
    MUSIC_DECODER_ST_STOP = 0x0,
    MUSIC_DECODER_ST_PLAY,
    MUSIC_PLAYRR_ST_PAUSE,
} MUSIC_DECODER_ST;


typedef enum {
    MUSIC_DECODER_ERR_NONE = 0x1000,
    MUSIC_DECODER_ERR_INIT_FAIL,
    MUSIC_DECODER_ERR_START_FAIL,
} MUSIC_DECODER_ERR;

typedef struct __MUSIC_DECODER MUSIC_DECODER;

typedef decoder_ops_t *(*DEC_OPS_TYPE)(void);

typedef struct __DEC_CFG {
    DEC_OPS_TYPE 		 get_ops;				//技术部提供的解码接口
    void		 		*priv_setting;			//私有参数配置
    void				*dec_buf;	 			//解码缓存
    u32  				 dec_buf_size;			//解码缓存空间大小
    u8					 format_check_enable;	//格式检查使能
    u8					 fffr_enable;			//快进快退使能
} DEC_CFG;

typedef struct __DEC_BP {
    void						*bp_buf;
    u32						 	 bp_size;
} DEC_BP;

typedef struct __BREAK_POINT_CBK {
    void *priv;
    tbool(*write)(void *priv, void *buf, u32 len);
    tbool(*read)(void *priv, void *buf, u32 len);
    tbool(*clear)(void *priv);
} BREAK_POINT_CBK;

void music_decoder_loop_resume(void);

MUSIC_DECODER *music_decoder_creat(void);
void music_decoder_destroy(MUSIC_DECODER **hdl);
void music_decoder_set_file_interface(MUSIC_DECODER *obj, AUDIO_FILE *_io, void *hdl);
void music_decoder_set_output(MUSIC_DECODER *obj, AUDIO_STREAM *output);
void music_decoder_set_err_deal_interface(MUSIC_DECODER *obj, void (*cbk)(void *priv, u32 err), void *priv);
void music_decoder_set_dev(MUSIC_DECODER *obj, u32 dev);
void music_decoder_set_configs(MUSIC_DECODER *obj, DEC_CFG cfg[], u32 cnt);
void music_decoder_set_callback(MUSIC_DECODER *obj, void (*before_fun)(void *), void (*after_fun)(void *));
void music_decoder_before_callback(void *priv);
void music_decoder_after_callback(void *priv);
u32 music_decoder_get_decode_total_time(MUSIC_DECODER *obj);
u32 music_decoder_get_decode_cur_time(MUSIC_DECODER *obj);
u32 music_decoder_get_break_point_size(MUSIC_DECODER *obj);
u8 *music_decoder_get_break_point_info(MUSIC_DECODER *obj);
char *music_decoder_get_decode_name(MUSIC_DECODER *obj);
MUSIC_DECODER_ST music_decoder_get_status(MUSIC_DECODER *obj);
u32 music_decoder_play(MUSIC_DECODER *obj, DEC_BP *bp_info);
void music_decoder_decode_deal(MUSIC_DECODER *obj);
void music_decoder_stop(MUSIC_DECODER *obj);
MUSIC_DECODER_ST music_decoder_pp(MUSIC_DECODER *obj);
MUSIC_DECODER_ST music_decoder_pause(MUSIC_DECODER *obj);
MUSIC_DECODER_ST music_decoder_play_resume(MUSIC_DECODER *obj);
void music_decoder_fffr(MUSIC_DECODER *obj, u8 type, u8 second);
tbool music_decoder_set_dec_confing(MUSIC_DECODER *obj, u8 cmd, void *parm);
tbool music_decoder_set_ab_repeat(MUSIC_DECODER *obj, u8 cmd, void *parm);
void music_decoder_set_loop_en(tbool en);
void music_decoder_set_ini_sta(MUSIC_DECODER *obj, MUSIC_DECODER_ST sta);
int tws_sbc_dec_co_media_run();
u8 tws_get_reset_decode_en();
u8 tws_clear_reset_decode_en();
#endif//__MUSIC_DECODER_H__

