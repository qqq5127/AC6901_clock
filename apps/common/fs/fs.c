#include "fs.h"

#include "fs_io.h"
#include "mbr.h"

#include "syd_io.h"
#include "fat_io.h"

#include "dev_usb.h"

#include "string.h"
#include "uart.h"
#include "board_init.h"
#include "sdk_cfg.h"

//#define FS_LOG
#ifdef FS_LOG
#define fs_puts		puts
#define fs_printf	printf
#else
#define fs_puts(...)
#define fs_printf(...)
#endif

u32 fs_open_file_bypath(void *dev_hdl, void *buf, u16 len, char *path)
{
    u32 special_part = 0;
    u32 block_size = 0;
    u32 rlen;
    s32 err;
    //u8 buf[512];
    u32 parm;

    _FS_DEV_INFO dev_info;
    _FS_HDL fs_hdl;
    _FIL_HDL f_hdl;

    memset(&fs_hdl, 0x00, sizeof(fs_hdl));
    memset(&f_hdl, 0x00, sizeof(f_hdl));

    /* fs_printf("++++++++++++++++++++++++++++vfs_test++++++++++++++++++++++++++++++++\n"); */
    err = dev_part_get_base_addr(dev_hdl, 0, &special_part);

    if (err) {
        fs_printf("dev_part_get_base_addr err	fun = %s, line = %d\n", __func__, __LINE__);
        return false;
    }
    fs_printf("special_part = 0x%x\n", special_part);

    err = dev_io_ctrl(dev_hdl, DEV_GET_BLOCK_SIZE, &block_size);
    if (err) {
        fs_printf("dev_io_ctrl err	fun = %s, line = %d\n", __func__, __LINE__);
        return false;
    }
    /* ASSERT(0 == err); */
    fs_printf("block_size = 0x%x\n", block_size);

    dev_info.hdl			= dev_hdl;
    dev_info.read_p			= read_api;
    dev_info.write_p		= write_api;
    dev_info.drive_base		= special_part;
    dev_info.block_size 	= block_size / 512;

    err = fs_drive_open(&fs_hdl, &dev_info);
    /* fs_printf("file_total = 0x%x\n",fs_get_file_total(&fs_hdl, 0, 0)); */
    fs_printf("fs_drive_open ret = 0x%x\n", -err);
    ASSERT(!err);

    /* fs_printf("file_total = 0x%x\n", fs_get_file_total(&fs_hdl, 0, 0)); */

#if 0
    fs_puts("fs_get_file_byindex\n");
    err = fs_get_file_byindex(&fs_hdl, &f_hdl, 1, NULL);
    if (err) {
        fs_puts("fs_get_file_byindex err\n");
        return;
    }
    /* ASSERT(!err); */
#else
    fs_puts("fs_get_file_bypath\n");
    /* err = fs_get_file_bypath(&fs_hdl, &f_hdl, (void*)"test.iso", NULL); */
    err = fs_get_file_bypath(&fs_hdl, &f_hdl, (void *)path);//SLEEP.lrc
    if (err) {
        fs_printf("fs_get_file_bypath err = %d\n", err);
        return false;
    }
    /* ASSERT(!err); */
#endif

    fs_puts("fs_read\n");
    err = fs_read(&f_hdl, buf, len);
    /* while (1) {
        err = fs_read(&f_hdl, buf, 512);
        put_buf(buf, err);
        if (err != 512) {
            fs_puts("fs_read end\n");
            break;
        }
    } */

    fs_puts("fs_close\n");
    err = fs_close(&f_hdl);
    ASSERT(err == 0);

    fs_puts("fs_drive_close\n");
    err = fs_drive_close(&fs_hdl);
    ASSERT(err == 0);

    return true;
}

u32 read_api(void *dev, u8 *buf, u32 addr)
{
    if (512 == dev_read(dev, buf, addr, 1)) {
        return 0;//SUCC
    } else {
        return 1;//err
    }
}

u32 write_api(void *dev, u8 *buf, u32 addr)
{
    if (512 == dev_write(dev, buf, addr, 1)) {
        return 0;//SUCC
    } else {
        return 1;//err
    }
}

void fs_init_all(void)
{
    fs_init();

    syd_init();
    /* fat_init(); */
}
no_sequence_initcall(fs_init_all);

