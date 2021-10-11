#include "sdk_cfg.h"
#include "common/common.h"
#include "file_io.h"
#include "font_out.h"
#include "fs.h"
#include "fs_io.h"
#include "dev_manage.h"
#include "font_area_ctl.h"
#include "lcd_drv_interface.h"

#if LCD_128X64_EN

/* #define FONT_DEBUG */
#ifdef FONT_DEBUG
#define font_puts		puts
#define font_printf	printf
#else
#define font_puts(...)
#define font_printf(...)
#endif

typedef struct __font_flashdata_t {
    u8 buf[512];
    u32 sector;
} font_flashdata_t;

static font_flashdata_t flashdata = {
    .sector = (u32) - 1,
};

static int move_window_flashdata(u32 sector, font_flashdata_t *win_buf)
{
    int res = 0;
    if (win_buf->sector != sector) {
        res = read_api(cache, win_buf->buf, sector);
        if (res != 0) {
            return res;
        }
        win_buf->sector = sector;
    }
    return res;
}

tbool font_read_api(u8 _xdata *buf, u32 addr, u32 length)
{
    u64 t_addr = 0;
    u32 sector;
    u32 sec_offset;
    u32 len0;
    u32 len = length;
    t_addr = addr;
    u8 _xdata *ptr;
    ptr = buf;
    bool ret = true;

    //font_printf("read flash %d\n", addr);
    while (len) {
        sector = t_addr / 512;
        ret = move_window_flashdata(sector, &flashdata);
        sec_offset = t_addr % 512;
        if (len > (512 - sec_offset)) {
            len0 = 512 - sec_offset;
        } else {
            len0 = len;
        }
        memcpy(ptr, &(flashdata.buf[sec_offset]), len0);
        len -= len0;
        t_addr += len0;
        ptr += len0;
    }
    return ret;
}

tbool font_get_file_addr_bypath_api(u8 *path, u32 *addr)
{
    s32 err;
    _FILE font_file;

    memset(&font_file, 0x00, sizeof(_FILE));

    err = file_api_creat(&font_file, cache, 0);
    if (err == false) {
        return false;
    }

    font_printf("fs_get_file_bypath %s\n", path);
    err = fs_get_file_bypath(&(font_file.fs_hdl), &(font_file.file_hdl), (void *)path);
    if (err) {
        font_printf("fs_get_file_bypath err = %d\n", err);
        err = fs_drive_close(&(font_file.fs_hdl));
        ASSERT(err == 0);
        return false;
    }

    if (addr) {
        fs_io_ctrl(NULL, &font_file.file_hdl, FS_IO_GET_FILE_SCLUST, addr);
    } else {
        return false;
    }

    font_puts("fs_close\n");
    fs_close(&(font_file.file_hdl));

    font_puts("fs_drive_close\n");
    fs_drive_close(&(font_file.fs_hdl));

    return true;
}

const FONT_IO font_io = {
    .get_file_addr_bypath = font_get_file_addr_bypath_api,
    .flash_read = font_read_api,
};

bool font_init_api(u8 language)
{
    return font_init(language, (FONT_IO *)&font_io);
}


#endif
