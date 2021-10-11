#ifndef __DEV_CACHE_H__
#define __DEV_CACHE_H__

#include "typedef.h"

#include "dev_mg_api.h"


#define FLASH_BASE_ADDR		0x1000000

enum {
    CACHE_SUCC = 0x0,
    CACHE_OFFLINE,
};

/*****************************
        Function Declare
*****************************/
// s32 cache_mount_api(void *pram);
// s32 cache_unmount_api(void);
// s32 cache_read_api(u8 *buf, u32 lba, u32 len);
// s32 cache_write_api(u8 *buf, u32 lba, u32 len);
// s32 cache_det_api(void);
// s32 cache_var_api(void *var_p, void *parm);

const struct DEV_IO *dev_reg_cache(void *parm);

#endif
