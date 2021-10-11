#ifndef __MUSIC_DECRYPT_H__
#define __MUSIC_DECRYPT_H__

#include "typedef.h"


typedef struct _CIPHER {
    void *file_type;		  //文件后缀
    u32   cipher_code;        ///>解密key
    u8    cipher_enable;      ///>解密读使能
} CIPHER;

tbool  cipher_obj_creat(u32 key,  CIPHER *obj, const char *file_type);
tbool cipher_obj_destroy(CIPHER *obj);
void cipher_analysis_buff(CIPHER *obj, void *buf, u32 faddr, u32 len);
tbool cipher_check_file_type(CIPHER *obj, void *file);

#endif //__MUSIC_DECRYPT_H__

