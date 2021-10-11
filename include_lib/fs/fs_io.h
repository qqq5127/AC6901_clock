#ifndef _FS_IO_H_
#define _FS_IO_H_

#include "typedef.h"
#include "stdarg.h"

//#define FS_IO_DEBUG

#ifdef FS_IO_DEBUG
#define fs_io_deg             printf
#define fs_io_deg_puts        puts
#define fs_io_deg_buf         printf_buf
#else
#define fs_io_deg(...)
#define fs_io_deg_puts(...)
#define fs_io_deg_buf(...)
#endif
/*----------------------------------------------------------------------------*/
/**
  								DEV_FS_TYPE
*/
/*----------------------------------------------------------------------------*/
typedef enum __DEV_FS_TYPE {
    FAT_FS_T = 1,
    SYD_FS_T,
} DEV_FS_TYPE;


/*----------------------------------------------------------------------------*/
/**
  								FILE_SYSTEM_PARM
*/
/*----------------------------------------------------------------------------*/
typedef struct __fs_dev_info {
    void *hdl;			//device handle
    void *read_p;		//
    void *write_p;	//
    u32 drive_base;		//patition info
    u32 block_size;	//device block info
} _FS_DEV_INFO;

/*----------------------------------------------------------------------------*/
/**
  								FILE_SYSTEM_STRUCT
*/
/*----------------------------------------------------------------------------*/
typedef enum {
    FS_IO_MKDIR = 0,
    FS_IO_GET_FOLDER_FILE,
    FS_IO_CHANGE_FOLDER,

    FS_IO_GET_FILE_NAME,
    FS_IO_GET_FILE_SIZE,
    FS_IO_GET_FILE_SCLUST,
    FS_IO_GET_FILE_NUMBER,
    FS_IO_GET_FILE_INFO,
} FS_IO_CMD;

typedef struct _vfs_io {
    //base_info
    u32 type;

    //base_function
    s32(*drive_open)(void **fs_pp, void *fs_dev_info_p);
    s32(*drive_close)(void **fs_pp);

    //stdio_function
    s32(*f_open)(void *fs_p, void **f_pp, char *path, u8 mode);
    s32(*f_close)(void **f_pp);
    u32(*f_seek)(void *f_p, u8 type, u32 offsize);
    u32(*f_read)(void *f_p, u8 *buff, u16 len);
    u32(*f_write)(void *f_p, u8 *buff, u16 len);
    s32(*f_sync)(void *f_p);
    s32(*f_delete)(void *f_p);

    //stdio_function
    u32(*get_file_total)(void *fs_p, const char *exit, u32 sclust, u32 *ret_p);
    u32(*get_fileinfo)(void *fs_p, char *path, char *ext, u32 *first_fn);
    s32(*get_file_byindex)(void *fs_p, void **f_pp, u32 file_number);
    s32(*get_file_bypath)(void *fs_p, void **f_pp, u8 *path);
    s32(*get_file_bysclust)(void *fs_p, void **f_pp, u32 sclust);
    s32(*get_file_byname_indir)(void *fs_p, void **f_pp, void **t_fp, void *ext_name);

    //io_ctrl_function
    s32(*io_ctrl)(void *fs_p, void *f_p, u32 cmd, va_list argptr);

} VFS_IO_T;

//file_system_handle
typedef struct __fs_hdl {
    VFS_IO_T *io;
    void *hdl;
} _FS_HDL;

//file_handle
typedef struct __fil_hdl {
    VFS_IO_T *io;
    void *hdl;
} _FIL_HDL;


/*----------------------------------------------------------------------------*/
/**
  								FILE_SYSTEM_API
*/
/*----------------------------------------------------------------------------*/
//base_init
void fs_init(void);
s32 fs_reg(VFS_IO_T *fs_io_p);
s32 fs_release(VFS_IO_T *fs_io_p);

//base_function
s32 fs_drive_open(_FS_HDL *p_fs_hdl, void *p_dev_info);
s32 fs_drive_close(_FS_HDL *p_fs_hdl);

//stdio
s32 fs_open(_FS_HDL *p_fs_hdl, _FIL_HDL *p_f_hdl, char *path, u8 mode);
s32 fs_close(_FIL_HDL *p_f_hdl);
u32 fs_seek(_FIL_HDL  *p_f_hdl, u8 type, u32 offsize);
u32 fs_tell(_FIL_HDL *p_f_hdl);
u32 fs_read(_FIL_HDL *p_f_hdl, u8 _xdata *buff, u16 len);
u32 fs_write(_FIL_HDL *p_f_hdl, u8 _xdata *buff, u16 len);
s32 fs_sync(_FIL_HDL *p_f_hdl);
s32 fs_delete(_FIL_HDL *p_f_hdl);

//open_file
u32 fs_get_file_total(_FS_HDL *p_fs_hdl, const char *file_type, u32 sclust, u32 *ret_p);
s32 fs_get_file_byindex(_FS_HDL *p_fs_hdl, _FIL_HDL *p_f_hdl, u32 file_number);
s32 fs_get_file_bypath(_FS_HDL *p_fs_hdl, _FIL_HDL *p_f_hdl, u8 *path);
s32 fs_get_file_bysclust(_FS_HDL *p_fs_hdl, _FIL_HDL *p_f_hdl, u32 sclust);
s32 fs_get_file_byname_indir(_FS_HDL *p_fs_hdl, _FIL_HDL *p_f_hdl, _FIL_HDL *p_lrc_hdl, void *ext_name);


//get_file/foler info
s32 fs_io_ctrl(_FS_HDL *p_fs_hdl, _FIL_HDL *p_f_hdl, u32 cmd, ...);

#if 0
s32 fs_mk_dir(_FS_HDL *p_fs_hdl, char *path, u8 mode);
s32 fs_folder_file(_FIL_HDL *p_f_hdl, u32 *start_file, u32 *end_file);
s32 fs_get_file_size(_FIL_HDL *p_f_hdl, u32 *parm);
s32 fs_get_file_sclust(_FIL_HDL *p_f_hdl, u32 *parm);
s32 fs_get_file_number(_FIL_HDL *p_f_hdl, u32 *parm);
#endif

#endif
