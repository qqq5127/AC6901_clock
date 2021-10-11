#ifndef _REC_FILE_OP_
#define _REC_FILE_OP_

#include "file_operate.h"


s16 rec_file_write(FILE_OPERATE *rec_fop_api, u8 *buf, u32 len);
s16 rec_file_close(FILE_OPERATE *rec_fop_api);
s16 rec_file_seek(FILE_OPERATE *rec_fop_api, u8 type, u32 offsize);
u32 rec_file_tell(FILE_OPERATE *rec_fop_api);
s16 rec_file_open(FILE_OPERATE *rec_fop_api);
s16 rec_fs_open(FILE_OPERATE **fop_api_p, void *rec_dev);
u32 rec_fop_del_recfile(REC_FILE_INFO *rec_file_info);
u32 rec_fop_update_lastfile(REC_FILE_INFO *rec_file_info);
void rec_fs_close(FILE_OPERATE **fop_api_p);
u32 get_rec_fname_cnt();
bool rec_get_file_info(void *fop_api, REC_FILE_INFO *rec_file_info);
void *rec_fop_get_dev(FILE_OPERATE *fop_api);
#endif

