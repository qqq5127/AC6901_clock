#ifndef _SYD_IO_H_
#define _SYD_IO_H_

#include "typedef.h"
#include "stdarg.h"

//init
s32 syd_drive_open(void **p_fs_hdl, void *p_dev_info);
s32 syd_drive_close(void **p_fs_hdl);

/******io_function******/
//file_function
s32 syd_open(void *p_fs_hdl, void **p_f_hdl, char *path, u8 mode);
s32 syd_file_close(void **p_f_hdl);
u32 syd_seek(void *p_f_hdl, u8 type, u32 offsize);
u32 syd_read(void *p_f_hdl, u8 _xdata *buff, u16 len);
u32 syd_write(void *p_f_hdl, u8 _xdata *buff, u16 len);
s32 syd_write_close(void *p_f_hdl);
s32 syd_file_delete(void *p_f_hdl);

//folder_function
s32 syd_mk_dir(void *p_fs_hdl, char *path, u8 mode);
s32 syd_folder_file(void *p_f_hdl, u32 *start_file, u32 *end_file);

//file_function
s32 syd_get_file_byindex(void *p_fs_hdl, void **p_f_hdl, u32 file_number);
s32 syd_get_file_bypath(void *p_fs_hdl, void **p_f_hdl, u8 *path);
s32 syd_get_file_bysclust(void *p_fs_hdl, void **p_f_hdl, u32 sclust);

//file_info_function
u32 syd_getfile_total(void *p_fs_hdl, const char *file_type, u32 sclust, u32 *ret_p);
s32 syd_get_file_name(void *p_f_hdl, void *n_buf);
s32 syd_get_file_size(void *p_f_hdl, u32 *parm);
s32 syd_get_file_sclust(void *p_f_hdl, u32 *parm);
s32 syd_get_file_number(void *p_f_hdl, u32 *parm);

s32 syd_enter_dir(void *p_fs_hdl, void **p_f_hdl, void *dir_info, const char *file_type);
s32 syd_exit_dir(void *p_fs_hdl, void **p_f_hdl);
s32 syd_get_dir(void *p_fs_hdl, void *p_f_hdl, u32 start_num, u32 total_num, void *buf_info);

s32 syd_io_ctrl(void *fs_p, void *f_p, u32 cmd, va_list argptr);

//base_init
void set_sydf_header_base(u32 base);
void syd_init(void);

#endif
