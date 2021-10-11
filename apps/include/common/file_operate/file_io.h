#ifndef __FILE_IO_H__
#define __FILE_IO_H__
#include "typedef.h"
#include "fs_io.h"

typedef struct __FILE {
    _FS_HDL  fs_hdl;
    _FIL_HDL file_hdl;
    _FIL_HDL lrc_hdl;
} _FILE;

tbool file_api_creat(_FILE *obj, void *dev_hdl, u32 dev_part_index);
void file_api_destroy(_FILE *obj);

#endif// __FILE_IO_H__
