#ifndef __TASK_LINEIN_H__
#define __TASK_LINEIN_H__

#include "task_manager.h"

//#define AUX_IO_BIT    BIT(1)
//#define AUX_DIR_SET   (JL_PORTA->DIR |= AUX_IO_BIT)
//#define AUX_PU_SET    (JL_PORTA->PU  |= AUX_IO_BIT)
//#define AUX_IN_CHECK  (JL_PORTA->IN  &  AUX_IO_BIT)

typedef enum __AUX_STATUS {
    AUX_OFF = 0,
    AUX_ON,
    NULL_AUX,
} AUX_STATUS;

typedef struct __AUX_VAR {
    u16 pre_status;
    u16 cur_status;
    u8  status_cnt;
    u8  bDevOnline;
} AUX_VAR;

extern const TASK_APP task_linein_info;

#endif//__TASK_IDLE_H__

