/*******************************************************************************************
 File Name: devcie.h

 Version: 1.00

 Discription:设备状态，操作函数定义和宏定义

 Author:dengyulin

 Email ：flowingfeeze@163.com

 Date:2010.09.09

 Copyright:(c) 2010 , All Rights Reserved.
*******************************************************************************************/
/*
version history

2010.09.09 v1.0 建立初始版本

*/

#ifndef __SYD_FILE_H_
#define __SYD_FILE_H_

#include "typedef.h"

typedef enum {
    SR_OK = 0,
    SR_FIND_DIR = 0x80,
    SR_FIND_FILE,
    SR_DIR_END,         //前面几个位置不能变
    SR_NO_FILE,
    SR_NO_PATH,
    SR_EXIST,
    SR_INVALID_NAME,
    SR_INVALID_DRIVE,
    SR_DENIED,
    SR_RW_ERROR,
    SR_WRITE_PROTECTED,
    SR_NO_FILESYSTEM,
    SR_DEEP_LIMIT,
    SR_END_PATH,
    SR_FILE_LIMIT,
    SR_END_FILE,
    SR_LFN_ERR,
    SR_MKFS_ABORTED,
    SR_DIR_DELETE,
    SR_DISK_ERROR,
    SR_FILE_END,
    SR_FILE_ERR,
    SR_NO_WINBUF,
    SR_INT_ERR,				/* (2) Assertion failed */
    SR_NO_SEL_DRIVE,
} SRESULT;

typedef struct _SYDWIN_BUF {
    u8  start[512];
    u32  sector;
    struct _SYDFS  *fs;
    u8   flag;
} SYDWIN_BUF;

struct _SYDFS {
    u32	fsbase;		/* FAT start sector */
    u32	file_total;
    u32(*disk_read)(void *hdev, u8 _xdata *buf, u32 lba);		/* device read function */
    u32(*disk_write)(void *hdev, u8 _xdata *buf, u32 lba);		/* device write function */
    void	*hdev;
    SYDWIN_BUF	win;		        /* Disk access window for Directory/FAT/File */
    // u8	drive_cnt;
};
typedef struct _SYDFS SYDFS;

typedef struct _SDFILE {
    u32 addr;
    u32 length;
    u32 pointer;
    u32 index;
    u16 crc;
    SYDFS *f_s;
} SDFILE, *PSDFILE, sdfile_t, *psdfile_t;

typedef struct _FLASHHEADER {
    u16 crc16;
    u16 crcfileheaddata;
    u8 info[8];
    u32 filenum;
    u32 version;
    u32 version1;
    u8 chiptype[8];

} FLASHHEADER;

typedef struct _FILEHEAD {
    u8 filetype;
    u8 reserv;
    u16 crc16;
    u32 addr;
    u32 len;
    u32 index;
    u8 name[16];

} FLASHFILEHEAD, *PFLASHPFILEHEAD ;

#define  SDDEVICE_NOR_FLASH     0x00
#define  SDDEVICE_SDCARD        0x01
#define  SDDEVICE_NAND_FLASH    0x02

void set_sydf_header_base(u32 base);
u32 check_syd(SYDFS *syd_fs, u32 sec);
s32 syd_drive_open(void **p_fs_hdl, void *p_fs_dev_info);
bool syd_get_file_total(SYDFS *syd_fs, u32 brk_info);
tbool sydf_openbyname(SYDFS *f_s, SDFILE *file, const u8 *filename);
tbool sydf_openbyindex(SYDFS *f_s, SDFILE *file, u16 index);
tbool sydf_openbysclust(SYDFS *f_s, SDFILE *file, u32 sclust);
void sydf_close(SDFILE *fp);
u32 sydf_read(SDFILE  *fp, u8  *buff, u16 length);
u32 sydf_seek(SDFILE *fp, u8 type, u32 offsize);

// tbool syd_api_init(SYDFS *syd_fs, void *p_hdev);
// u32 get_free_flash_addr(SYDFS *syd_fs);

#endif

