#include "sdk_cfg.h"
#include "common/common.h"
#include "file_io.h"
#include "ui_con.h"
#include "dev_manage.h"

#if LCD_128X64_EN

_FILE ui_sty_file;
_FILE ui_res_file;

extern const LCD_INFO lcd_info;

static tbool ui_select_file(u8 *path, void **f_hdl_p, _FILE *p_file)
{
    u32 ret;

    if (f_hdl_p == NULL) {
        return false;
    }

    ret = file_api_creat(p_file, cache, 0);
    if (ret == false) {
        return false;
    }

    ret = fs_get_file_bypath(&p_file->fs_hdl, &p_file->file_hdl, (void *)path);
    if (ret) {
        ret = fs_drive_close(&p_file->fs_hdl);
        return false;
    }

    *f_hdl_p = &p_file->file_hdl;

    return true;
}


tbool select_sty_file_api(u8 *path, void **f_hdl_p)
{
    return ui_select_file(path, f_hdl_p, &ui_sty_file);
}

tbool select_res_file_api(u8 *path, void **f_hdl_p)
{
    return ui_select_file(path, f_hdl_p, &ui_res_file);
}

u32 ui_file_read_api(void *f_hdl, u8 *buff, u16 length)
{
    return fs_read(f_hdl, buff, length);
}

u32 ui_file_seek_api(void *f_hdl, u8 type, u32 offsize)
{
    return fs_seek(f_hdl, type, offsize);
}

const UI_FILE_IO ui_file_io = {
    .ui_select_sty_file = select_sty_file_api,
    .ui_select_res_file = select_res_file_api,
    .ui_file_read = ui_file_read_api,
    .ui_file_seek = ui_file_seek_api,
};


bool uicon_init_api(void)
{
    bool ret;
    UI_VAR_API ui_parm;
    ui_parm.file_io = (UI_FILE_IO *)&ui_file_io;
    ui_parm.lcd_parm = (LCD_INFO *)&lcd_info;

    dc_set_screen_scale(1, 1); //对屏幕显示进行同步放大

    ret = ui_init(&ui_parm);

    return ret;
}
#endif
