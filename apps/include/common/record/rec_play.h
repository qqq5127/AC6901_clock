#ifndef _REC_PLAY_
#define _REC_PLAY_

void *rec_play_init(REC_FILE_INFO *rec_file_info);
void rec_play_exit();
u32 rec_paly_pp();
void record_play_mutex_init(void *priv);
void record_play_mutex_stop(void *priv);
void rec_play_api(REC_FILE_INFO *rec_file_info);
#endif
