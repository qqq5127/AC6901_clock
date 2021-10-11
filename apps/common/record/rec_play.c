#include "sdk_cfg.h"
#include "rec_api.h"
#include "rec_file_op.h"
#include "audio/audio.h"
#include "music_player.h"
#include "audio/dac_api.h"
#include "dac.h"
#include "rec_play.h"
#include "resource_manage.h"

#if REC_PLAY_EN
void *rec_play_hdl = NULL;
void *rec_play_init(REC_FILE_INFO *rec_file_info)
{
    MUSIC_PLAYER *obj = NULL;
    tbool ret;

    if (rec_file_info == NULL) {
        rec_api_printf("no rec file info \n");
        return NULL;
    }
    if (rec_file_info->rec_dev == NULL) {
        rec_api_printf("no rec dev \n");
        return NULL;
    }
    rec_api_printf("rec play dev %x  file num %d sclust %d\n", (u32)rec_file_info->rec_dev, rec_file_info->file_number, rec_file_info->rec_file_sclust);
    obj = rec_play((u32)rec_file_info->rec_dev, rec_file_info->rec_file_sclust);
    REC_ASSERT(obj, __music_creat_err);

//    music_player_destroy(&obj);
    return obj;
__music_creat_err:
    printf("music creat err \n");
    rec_play_stop();
    return NULL;
}

void rec_play_exit()
{
    rec_play_stop();
}

u32 rec_paly_pp()
{
    return 0;
}


void record_play_mutex_init(void *priv)
{
    if (priv) {
        rec_play_hdl = rec_play_init(priv);
        if (rec_play_hdl == NULL) {
            mutex_resource_release("record_play");
        }
    }
}

void record_play_mutex_stop(void *priv)
{
    if (rec_play_hdl) {
        rec_play_exit();
        rec_play_hdl = NULL;
    }
    mutex_resource_release("record_play");
}

void rec_play_api(REC_FILE_INFO *rec_file_info)
{
    record_play_mutex_stop(NULL);
    resource_manage_schedule();
    mutex_resource_apply("record_play", 4, record_play_mutex_init, record_play_mutex_stop, rec_file_info);
}

#endif
