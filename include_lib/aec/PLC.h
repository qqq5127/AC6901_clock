#ifndef _PLC_H_
#define _PLC_H_

#include "typedef.h"

typedef struct _BT15_REPAIR_API {
    unsigned int (*need_buf)(short max_packet_len);
    void (*open)(unsigned char *ptr, short max_packet_len);
    void (*run)(unsigned char *ptr, short *inbuf, short *oubuf, short len, short err_flag);
} BT15_REPAIR_API;
extern BT15_REPAIR_API *get_repair_api();

u32 PLC_query(void);
s8 PLC_init(void *pbuf);
void PLC_exit(void);
void PLC_run(s16 *inbuf, s16 *outbuf, u16 point, u8 repair_flag);

#endif
