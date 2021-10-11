#include "fs/fs_io.h"
//#include "ffile_io.h"
#include "rec_api.h"
#include "string.h"
#include "rec_file_op.h"
#include "fs_io.h"
#include "file_io.h"

const char rec_folder_name[] = "/JL_REC";                   //录音文件夹  //仅支持一层目录

#if REC_FILE
const char rec_file_name[] =   "/JL_REC/FILE0000.MP3";      //MP3录音文件名（含路径）
#else
const char rec_file_name[] =   "/JL_REC/FILE0000.WAV";      //ADPCM录音文件名（含路径）
#endif


static char rec_file_name_last[sizeof(rec_file_name)];
static u32 rec_fname_cnt = 0;       //录音文件名计数

u32 get_rec_fname_cnt()
{
    return rec_fname_cnt;
}

static char *get_rec_filename(u32 rec_num)
{
    u32 tmp;

    if (rec_num > 9999) {
        rec_num = 0;
    }

    memcpy(rec_file_name_last, rec_file_name, sizeof(rec_file_name));
    tmp = rec_num / 1000;
    rec_file_name_last[FILE_NUM_LOCA] = (char)(tmp + '0');

    tmp = rec_num % 1000 / 100;
    rec_file_name_last[FILE_NUM_LOCA + 1] = (char)(tmp + '0');

    tmp = rec_num % 100 / 10;
    rec_file_name_last[FILE_NUM_LOCA + 2] = (char)(tmp + '0');

    tmp = rec_num % 10;
    rec_file_name_last[FILE_NUM_LOCA + 3] = (char)(tmp + '0');
    return rec_file_name_last;
}

s16 rec_file_write(FILE_OPERATE *rec_fop_api, u8 *buf, u32 len)
{
    if (rec_fop_api) {
        s16 res;
        _FIL_HDL *f_p = file_operate_get_file_hdl(rec_fop_api);
        res = fs_write(f_p, buf, len);
        printf("w");
        return res;
    }
    return -FILE_OP_ERR_OP_HDL;
}

u32 rec_file_tell(FILE_OPERATE *rec_fop_api)
{
    rec_api_printf("--rec_file_seek\n");
    if (rec_fop_api) {
        _FIL_HDL *f_p = file_operate_get_file_hdl(rec_fop_api);
        return fs_tell(f_p);
    }
    return -FILE_OP_ERR_OP_HDL;
}

s16 rec_file_close(FILE_OPERATE *rec_fop_api)
{
    rec_api_printf("--rec_file_close\n");
    if (rec_fop_api) {
        u32 file_size;
        _FIL_HDL *f_p = file_operate_get_file_hdl(rec_fop_api);
        fs_sync(f_p);
        file_size = fs_tell(f_p);
        rec_api_printf("--rec_file_size %d\n", file_size);
        if (file_size <= 512) {
            fs_delete(f_p);
        }
        return fs_close(f_p);
    }
    return -FILE_OP_ERR_OP_HDL;
}

s16 rec_file_seek(FILE_OPERATE *rec_fop_api, u8 type, u32 offsize)
{
    rec_api_printf("--rec_file_seek\n");
    if (rec_fop_api) {
        _FIL_HDL *f_p = file_operate_get_file_hdl(rec_fop_api);
        return fs_seek(f_p, type, offsize);
    }
    return -FILE_OP_ERR_OP_HDL;
}

s16 rec_file_open(FILE_OPERATE *rec_fop_api)
{
    rec_api_printf("--rec_file_open\n");
    if (rec_fop_api) {
        s16 ret;
        _FIL_HDL *f_p = file_operate_get_file_hdl(rec_fop_api);
        _FS_HDL  *fs_p = file_operate_get_fs_hdl(rec_fop_api);
        char *file_name;

start_open_rec_file:
        file_name = get_rec_filename(rec_fname_cnt);
        ret = fs_open(fs_p, f_p, file_name, FA_CREATE_NEW | FA_WRITE);
        if (ret == FR_OK) {
            printf("--rec_file_new:%s\n", file_name);
            fs_sync(f_p);
            return FR_OK;
        } else if (ret == FR_EXIST) {
            rec_api_printf("--rec_file_exist:%s\n", file_name);
            rec_fname_cnt++;
            goto start_open_rec_file;
        } else { //其他错误
            rec_api_printf("--open rec file err:%d\n", ret);
            return ret;
        }
    }
    return -FILE_OP_ERR_OP_HDL;
}

s16 rec_fs_open(FILE_OPERATE **fop_api_p, void *rec_dev)
{
    u32 err = 0;
    FILE_OPERATE *fop_api;

    if (fop_api_p == NULL) {
        return -FILE_OP_ERR_OP_HDL;
    }


    rec_api_printf("fun = %s, line = %d\n", __func__, __LINE__);
    *fop_api_p = file_operate_creat();
    if (*fop_api_p == NULL) {
        return -FILE_OP_ERR_NO_MEM;
    }

    fop_api = *fop_api_p;
    rec_api_printf("fun = %s, line = %d\n", __func__, __LINE__);
    if (rec_dev == NULL) {
        file_operate_set_dev_sel_mode(fop_api, DEV_SEL_FIRST);
    } else {
        file_operate_set_dev_sel_mode(fop_api, DEV_SEL_SPEC);
        file_operate_set_dev(fop_api, (u32)rec_dev);
    }
    err = file_operate_dev_sel(fop_api);
    if (err) {
        rec_api_printf("rec dev sel err = %x\n", err);
        return -err;
    }

    err = file_operate_rec_file_api_creat(fop_api);
    rec_api_printf("fun = %s, line = %d\n", __func__, __LINE__);
    if (!err) {
        bool ret;
        s16 make_dir_status;
        make_dir_status = file_operate_op(fop_api, FOP_CREAT_FOLDER, (void *)rec_folder_name, NULL);
        if (make_dir_status < 0) {
            rec_api_printf("FILE_OP_ERR_NOT_INIT\n");
            return -FILE_OP_ERR_NOT_INIT;
        } else {
            if (FR_EXIST == make_dir_status) {
                rec_api_printf("FR_EXIST\n");
                return FR_EXIST;
            }
            rec_api_printf("make_dir_status\n");
            return make_dir_status;
        }
    }
    return err;
}

void rec_fs_close(FILE_OPERATE **fop_api_p)
{
    if (fop_api_p) {
        file_operate_destroy(fop_api_p);
    }
}

bool rec_get_file_info(void *fop_api, REC_FILE_INFO *rec_file_info)
{
    REC_FILE_INFO file_info_tmp;
    if (rec_file_info == NULL) {
        return false;
    }
    if (fop_api == NULL) {
        return false;
    }
    memset(&file_info_tmp, 0x00, sizeof(REC_FILE_INFO));

    file_info_tmp.rec_dev = file_operate_get_dev(fop_api);
    file_info_tmp.file_number = file_operate_get_file_number(fop_api);
    file_info_tmp.rec_fname_cnt = get_rec_fname_cnt();

    _FIL_HDL *f_p = (_FIL_HDL *)file_operate_get_file_hdl(fop_api);
    FIL *f_h = (FIL *)f_p->hdl;
    file_info_tmp.rec_file_sclust = f_h->dir_info.sclust;
    if (file_info_tmp.rec_file_sclust == 0) {
        return false;
    }
    memcpy(rec_file_info, &file_info_tmp, sizeof(REC_FILE_INFO));
    return true;
}

#if REC_DEL
u32 rec_fop_update_lastfile(REC_FILE_INFO *rec_file_info)
{
    FILE_OPERATE *rec_fop_api;
    s16 ret;
    if (rec_file_info == NULL) {
        return FR_FILE_ERR;
    }
    ret = rec_fs_open(&rec_fop_api, rec_file_info->rec_dev);
    if ((ret != FR_EXIST) && (ret != FR_OK)) {
        return FR_NO_FILESYSTEM;
    }

    do {
        file_operate_set_path(rec_fop_api, (void *)get_rec_filename(rec_fname_cnt), 0);
        ret = file_operate_op(rec_fop_api, FOP_OPEN_FILE_BYPATH, NULL, NULL);
        if (ret) {
            if (rec_fname_cnt == 0) {
                rec_fs_close(&rec_fop_api);
                return FR_NO_FILE;
            }
            rec_fname_cnt--;
        } else {
            rec_get_file_info(rec_fop_api, rec_file_info);
        }
    } while (ret);
    rec_fs_close(&rec_fop_api);
    return FR_OK;
}


u32 rec_fop_del_recfile(REC_FILE_INFO *rec_file_info)
{
    FILE_OPERATE *rec_fop_api;
    s16 ret;
    if (rec_file_info == NULL) {
        return FR_FILE_ERR;
    }
    ret = rec_fs_open(&rec_fop_api, rec_file_info->rec_dev);
    if ((ret != FR_EXIST) && (ret != FR_OK)) {
        return FR_NO_FILESYSTEM;
    }

    file_operate_set_file_sclust(rec_fop_api, rec_file_info->rec_file_sclust);
    ret = file_operate_op(rec_fop_api, FOP_OPEN_FILE_BYSCLUCT, NULL, NULL);
    if (ret) {
        ret = FR_NO_FILE;
        goto exit;
    }

    ret = file_operate_op(rec_fop_api, FOP_DEL_FILE, NULL, NULL);
exit:
    rec_fs_close(&rec_fop_api);
    return ret;
}

#endif
void *rec_fop_get_dev(FILE_OPERATE *fop_api)
{
    return file_operate_get_dev(fop_api);
}
