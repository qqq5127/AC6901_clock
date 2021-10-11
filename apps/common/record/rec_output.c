#include "sdk_cfg.h"
#include "common/includes.h"
#include "common/common.h"
#include "rec_api.h"
#include "file_operate.h"
#include "rec_file_op.h"
#include "dev_mg_api.h"
#include "dev_manage.h"

void *rec_out_init()
{
    FILE_OPERATE *fop_api;
    s16 ret = 0;

    rec_api_printf("fun = %s, line = %d\n", __func__, __LINE__);
    dev_all_refurbish_part();
    if (dev_get_phydev_total(MUSIC_DEV_TYPE, DEV_ONLINE) > 0) {
        rec_api_printf("fun = %s, line = %d\n", __func__, __LINE__);
        ret = rec_fs_open(&fop_api, (void *)dev_get_fisrt(MUSIC_DEV_TYPE, DEV_ONLINE));  //打开第一个设备
        // ret = rec_fs_open(&fop_api, (void *)sd0);  //打开sd设备
        // ret = rec_fs_open(&fop_api, (void *)usb);  //打开usb设备
        if ((ret == FR_EXIST) || (ret == FR_OK)) {
            rec_api_printf("fun = %s, line = %d\n", __func__, __LINE__);
            ret = rec_file_open(fop_api);
            if (ret == FR_OK) {
                return fop_api;
            } else {
                rec_api_printf("--rec out  open file err %d\n", ret);
                rec_fs_close(&fop_api);
            }
        }
    }
    rec_api_printf("--rec out  init err %d\n", ret);
    return NULL;
}

s16 rec_out_exit(void **out_hd_p)
{
    s16 ret = 0;
    if (out_hd_p) {
        rec_api_printf("fun = %s, line = %d  %x\n", __func__, __LINE__, *out_hd_p);
        ret = rec_file_close((FILE_OPERATE *)*out_hd_p);
        rec_fs_close((FILE_OPERATE **)out_hd_p);
    }
    return ret;
}

s16 rec_out_seek(void *out_hd, u32 offsize)
{
    return rec_file_seek(out_hd, 0x0, offsize);
}


u32 rec_out_tell(void *out_hd)
{
    return rec_file_tell(out_hd);
}

s16 rec_out_put(void *out_hd, u8 *buff, u32 len)
{
    u32 size;

    size = rec_file_write(out_hd, buff, len);
    if (size > 0) {
        return size;
    }
    return 0;
}
