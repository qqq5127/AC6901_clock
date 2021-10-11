#ifndef __MUSIC_PLAYER_H__
#define __MUSIC_PLAYER_H__
#include "typedef.h"
#include "music_decoder.h"
#include "dev_manage.h"
#include "file_operate.h"

//以最大的断点信息存放, flac:68byte, wma:68byte, 其他:8byte
#define MUSIC_DECODER_BREAK_POINT_SIZE (680L)

//过滤时间为0的文件处理
#define FILTER_NULL_TIME_MUSIC      1


typedef struct __MMUSIC_PLAYER_BP {
    // DEV_HANDLE dev;
    FOP_BP f_info;
    u32 bp_size;
    u8	buf[MUSIC_DECODER_BREAK_POINT_SIZE];
} MUSIC_PLAYER_BP;

typedef struct __MUSIC_PLAYER MUSIC_PLAYER;

typedef struct _PARM_RECOVERMODE_ {
    u32  RECOVER_MODE_value;
} PARM_RECOVERMODE;

typedef enum {
    AB_RPT_NON,
    AB_RPT_ASTA,
    AB_RPT_BSTA,
} AB_RPT_STA;

enum {
    SET_BREAKPOINT_A = 0x08,	//设置复读起始位置A
    SET_BREAKPOINT_B,			//设置复读停止位置B
    SET_RECOVER_MODE,			//停止复读,设置恢复点
};

enum {
    RECOVER_MODE_BP_A = 0x01,	//停止复读,跳到A点正常播放
    RECOVER_MODE_BP_B,			//停止复读,跳到B点正常播放
    RECOVER_MODE_CUR,			//停止复读,在当前点正常播放
};

void music_player_ab_repeat_close(MUSIC_PLAYER *obj);
tbool music_player_ab_repeat_set(MUSIC_PLAYER *obj, u8 cmd, void *parm);
void music_player_ab_repeat_switch(MUSIC_PLAYER *obj);
void music_player_run(MUSIC_PLAYER *obj);
void music_player_destroy(MUSIC_PLAYER **hdl);
MUSIC_PLAYER *music_player_creat(void);
tbool music_player_play(MUSIC_PLAYER *obj, MUSIC_PLAYER_BP *bp_info,  u8 is_auto);
tbool music_player_operation(MUSIC_PLAYER *obj, ENUM_FILE_SELECT_MODE op);
tbool music_player_play_spec_num_file(MUSIC_PLAYER *obj, u32 filenum);
tbool music_player_play_path_file(MUSIC_PLAYER *obj, u8 *path, u32 index);
tbool music_player_play_prev_folder_file(MUSIC_PLAYER *obj);
tbool music_player_play_spec_dev(MUSIC_PLAYER *obj, u32 dev);
tbool music_player_play_next_dev(MUSIC_PLAYER *obj);
tbool music_player_play_prev_dev(MUSIC_PLAYER *obj);
tbool music_player_play_first_dev(MUSIC_PLAYER *obj);
tbool music_player_play_last_dev(MUSIC_PLAYER *obj);
tbool music_player_play_spec_break_point_file(MUSIC_PLAYER *obj, MUSIC_PLAYER_BP *bp_info);

void music_player_set_repeat_mode(MUSIC_PLAYER *obj, ENUM_PLAY_MODE mode);
void music_player_set_auto_next(MUSIC_PLAYER *obj, u8 auto_next_flag);
void music_player_set_decoder_init_sta(MUSIC_PLAYER *obj, MUSIC_DECODER_ST sta);
tbool music_player_delete_playing_file(MUSIC_PLAYER *obj);
u32 music_player_get_file_number(MUSIC_PLAYER *obj);

void music_player_ff(MUSIC_PLAYER *obj, u8 second);
void music_player_fr(MUSIC_PLAYER *obj, u8 second);
MUSIC_DECODER_ST music_player_pp(MUSIC_PLAYER *obj);
MUSIC_DECODER_ST music_player_pause(MUSIC_PLAYER *obj);
MUSIC_DECODER_ST music_player_play_resume(MUSIC_PLAYER *obj);

MUSIC_DECODER_ST music_player_get_status(MUSIC_PLAYER *obj);
u32 music_player_get_cur_time(MUSIC_PLAYER *obj);
u32 music_player_get_total_time(MUSIC_PLAYER *obj);
u32 music_player_get_total_file(MUSIC_PLAYER *obj);
tbool music_player_get_break_point_info(MUSIC_PLAYER *obj, MUSIC_PLAYER_BP *bp_info);
void *music_player_get_cur_dev(MUSIC_PLAYER *obj);
MUSIC_DECODER *music_player_get_dop(MUSIC_PLAYER *obj);
FILE_OPERATE *music_player_get_fop(MUSIC_PLAYER *obj);
AUDIO_FILE *music_player_get_fs_hdl(u8 cipher_enable);
u8 music_player_get_auto_next_flag(MUSIC_PLAYER *obj);
FS_DISP_NAME *music_player_get_file_info(void);
void music_tone_play(void *name);
void music_tone_end(void);
void music_tone_stop(void);

void *rec_play(u32 dev, u32 file_number);
void rec_play_stop(void);
#endif// __MUSIC_PLAYER_H__
