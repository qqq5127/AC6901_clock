#ifndef _MUSIC_UI_H_
#define _MUSIC_UI_H_

#include "fat/tff.h"
#include "ui/ui_api.h"
#include "music_player.h"
#include "sdk_cfg.h"

#define   MUSIC_OPT_BIT_DEL        BIT(0)
#define   MUSIC_OPT_BIT_FF         BIT(1)
#define   MUSIC_OPT_BIT_FR         BIT(2)


typedef struct _MUSIC_DIS_VAR {
    void *ui_curr_device;
    u32 ui_curr_file;
    u32 ui_total_file;
    u32 play_time;
    u32 curr_statu;
    FS_DISP_NAME *ui_file_info;
    u8  eq_mode;
    u8  play_mode;
    MUSIC_PLAYER *mapi;
    // u8  *ab_statu;
    u8 opt_state;
    bool lrc_flag;

} MUSIC_DIS_VAR;

#if UI_ENABLE
void ui_open_music(MUSIC_DIS_VAR *music_ui_api, u16 len);
void ui_close_music(MUSIC_DIS_VAR *music_ui_api);
void ui_music_update_var(MUSIC_DIS_VAR *music_ui_api);
#else
#define ui_open_music(...)
#define ui_close_music(...)
#define ui_music_update_var(...)
#endif
//
extern u32 music_curr_file;
extern u32 music_total_file;
#endif/*_MUSIC_UI_H_*/
