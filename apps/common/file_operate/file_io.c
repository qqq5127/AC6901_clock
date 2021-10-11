#include "file_io.h"
#include "string.h"
#include "dev_mg_api.h"
#include "fs_io.h"
#include "mbr.h"
#include "syd_io.h"
#include "fat_io.h"
#include "dev_usb.h"

extern u32 read_api(DEV_HANDLE hdev, u8 *buf, u32 addr);
extern u32 write_api(DEV_HANDLE hdev, u8 *buf, u32 addr);

extern int mbr_scan_parition(MBR_DRIVE_INFO *mbr_inf, void *hdev, void *read_fun, void *write_fun);
extern int mbr_scan_end(MBR_DRIVE_INFO *mbr);

tbool file_api_creat(_FILE *obj, void *dev_hdl, u32 dev_part_index)
{
    s32 err;
    MBR_DRIVE_INFO mbr_inf;
    _FS_DEV_INFO dev_info;
    tbool ret = false;
    u32 special_part = 0;
    u32 block_size = 0;

    if (obj == NULL) {
        printf("obj == NULL	fun = %s, line = %d\n", __func__, __LINE__);
        return false;
    }

    memset((u8 *)obj, 0x0, sizeof(_FILE));

    err = dev_part_get_base_addr(dev_hdl, dev_part_index, &special_part);
    if (err) {
        printf("dev_part_get_base_addr err	fun = %s, line = %d\n", __func__, __LINE__);
        return false;
    }
    //printf("special_part = 0x%x\n", special_part);

    err = dev_io_ctrl(dev_hdl, DEV_GET_BLOCK_SIZE, &block_size);
    if (err) {
        printf("dev_io_ctrl err	fun = %s, line = %d\n", __func__, __LINE__);
        return false;
    }
    /* ASSERT(0 == err); */
    //printf("block_size = 0x%x\n", block_size);

    dev_info.hdl			= dev_hdl;
    dev_info.read_p			= read_api;
    dev_info.write_p		= write_api;
    dev_info.drive_base		= special_part;
    dev_info.block_size 	= block_size / 512;

    err = fs_drive_open(&obj->fs_hdl, &dev_info);
    /* printf("file_total = 0x%x\n",fs_get_file_total(&fs_hdl, 0, 0)); */
    //printf("fs_drive_open ret = 0x%x\n", -err);

    if (err) {
        printf("dev_io_ctrl err	fun = %s, line = %d\n", __func__, __LINE__);
        return false;
    }

    return true;
}


void file_api_destroy(_FILE *obj)
{
    if (obj == NULL) {
        return ;
    }
    fs_close(&obj->file_hdl);
    fs_close(&obj->lrc_hdl);
    fs_drive_close(&obj->fs_hdl);
}


