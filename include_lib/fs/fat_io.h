#ifndef _FAT_IO_H_
#define _FAT_IO_H_

#include "typedef.h"
#include "stdarg.h"

//#define FAT_IO_DEBUG

#ifdef FAT_IO_DEBUG
#define fat_io_deg             printf
#define fat_io_deg_puts        puts
#define fat_io_deg_buf         printf_buf
#else
#define fat_io_deg(...)
#define fat_io_deg_puts(...)
#define fat_io_deg_buf(...)
#endif


//init
s32 fat_drive_open(void **p_fs_hdl, void *p_fs_dev_info);
s32 fat_drive_close(void **p_fs_hdl);

/******io_function******/
//file_function
s32 fat_open(void *p_fs_hdl, void **p_f_hdl, char *path, u8 mode);
s32 fat_close(void **pp_f_hdl);
u32 fat_seek(void *p_f_hdl, u8 type, u32 offsize);
u32 fat_read(void *p_f_hdl, u8 _xdata *buff, u16 len);
u32 fat_write(void *p_f_hdl, u8 _xdata *buff, u16 len);
s32 fat_write_close(void *p_f_hdl);
s32 fat_file_delete(void *p_f_hdl);

//folder_function
s32 fat_mk_dir(void *p_fs_hdl, char *path, u32 mode);
s32 fat_folder_file(void *p_f_hdl, u32 *start_file, u32 *end_file);
s32 fat_get_filenum_byfolder(void *p_f_hdl, u8 mode, u32 *total_file, u32 *curr_file);

//file_function
s32 fat_get_file_byindex(void *p_fs_hdl, void **p_f_hdl, u32 file_number);
s32 fat_get_file_bypath(void *p_fs_hdl, void **p_f_hdl, u8 *path);
s32 fat_get_file_bysclust(void *p_fs_hdl, void **p_f_hdl, u32 sclust);

//file_info_function
u32 fat_get_file_total(void *p_fs_hdl, const char *file_type, u32 sclust, u32 *ret_p);
s32 fat_get_file_name(void *p_f_hdl, void *n_buf);
s32 fat_get_file_size(void *p_f_hdl, u32 *parm);
s32 fat_get_file_sclust(void *p_f_hdl, u32 *parm);
s32 fat_get_file_number(void *p_f_hdl, u32 *parm);

s32 fat_io_ctrl(void *fs_p, void *f_p, u32 cmd, va_list argptr);

//base init
void fat_init(void);
void fat_del(void);
void fat_set_lfn_buf(void *buf);
//0成功  n:需要的ram大小, 如果membuf不符合条件将使用默认的RAM
u32 fat_mem_init(u8 *membuf, u32 size);
#endif
