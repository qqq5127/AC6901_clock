#ifndef _FS_H_
#define _FS_H_

#include "typedef.h"

u32 fs_open_file_bypath(void *dev_hdl, void *buf, u16 len, char *path);

u32 read_api(void *dev, u8 *buf, u32 addr);
u32 write_api(void *dev, u8 *buf, u32 addr);

void fs_init_all(void);

#endif
