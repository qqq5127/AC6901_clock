#include "music_decrypt.h"
#include "sdk_cfg.h"


#if 1
#define decrypt_puts(...)
#define decrypt_put_buf(...)
#define decrypt_printf(...)
#else
#define decrypt_puts puts
#define decrypt_put_buf put_buf
#define decrypt_printf printf
#endif

#define ALIN_SIZE	4



static tbool __cipher_match(char *fname, char const *ext)
{
    char *str = fname;
    u8   fname_len = 0;

    if (!ext) { //不匹配
        return false;
    }

    while (*str++ != '\0') {
        fname_len ++;
    }

    printf("fext_name = %s\n", fname + fname_len - 3);
    if (!memcmp(fname + fname_len - 3, ext, 3)) {
        return true;
    }

    return false;
}


tbool  cipher_obj_creat(u32 key,  CIPHER *obj, const char *file_type)
{
    if (obj == NULL) {
        return false;
    }

    obj->cipher_code = key;
    obj->file_type = (void *)file_type;

    return true;
}

tbool cipher_obj_destroy(CIPHER *obj)
{
    if (obj == NULL) {
        return false;
    }
    return true;
}

void cipher_analysis_buff(CIPHER *obj, void *buf, u32 faddr, u32 len)
{
    u32 i;
    u8 j;
    u8	head_rem;//
    u8  tail_rem;//
    u32 len_ali;
    u8  *buf_1b_ali;
    u8  *cipher_code = NULL;

    if (obj == NULL) {
        return ;
    }

    if (obj->cipher_enable == 0) {
        return;
    }

    cipher_code = (u8 *) & (obj->cipher_code); //cipher_file.cipher_code;

    head_rem = ALIN_SIZE - (faddr % ALIN_SIZE);
    if (head_rem == ALIN_SIZE) {
        head_rem = 0;
    }

    if (head_rem > len) {
        head_rem = len;
    }

    if (len - head_rem) {
        tail_rem = (faddr + len) % ALIN_SIZE;
    } else {
        tail_rem = 0;
    }

    buf_1b_ali = buf;
    j = 3;
    for (i = head_rem; i > 0; i--) {
        buf_1b_ali[i - 1] ^= cipher_code[j--];
    }

    buf_1b_ali = buf;
    buf_1b_ali = (u8 *)(buf_1b_ali + head_rem);
    len_ali = len - head_rem - tail_rem;
    {
        for (i = 0; i < (len_ali / 4); i++) {
            /* buf_4b_ali[i] = cipher_file.cipher_code; */
            buf_1b_ali[0 + i * 4] ^= cipher_code[0];
            buf_1b_ali[1 + i * 4] ^= cipher_code[1];
            buf_1b_ali[2 + i * 4] ^= cipher_code[2];
            buf_1b_ali[3 + i * 4] ^= cipher_code[3];
        }
    }

    buf_1b_ali = buf;
    buf_1b_ali += len - tail_rem;
    j = 0;
    for (i = 0 ; i < tail_rem; i++) {
        buf_1b_ali[i] ^= cipher_code[j++];
    }
}


tbool cipher_check_file_type(CIPHER *obj, void *file)
{
    if (obj == NULL || file == NULL) {
        return false;
    }

    if (true == __cipher_match((char *)file, (const char *)obj->file_type)) {
        obj->cipher_enable = 1;
        return true;
    }
    obj->cipher_enable = 0;
    return false;
}



