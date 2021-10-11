#ifndef __TASK_SPDIF_H__
#define __TASK_SPDIF_H__
#include "task_manager.h"

extern const TASK_APP task_spdif_info;

extern void clear_wdt(void);
extern void spdif_close();

enum {
    NONE_LINEAR_STATUS_STOP,
    NONE_LINEAR_STATUS_WAIT_INIT,
    NONE_LINEAR_STATUS_WORKING,
    NONE_LINEAR_STATUS_WAIT_STOP,
};

int spdif_none_linear_init(u16 burst_info);
int spdif_none_linear_stop(void *priv);
int spdif_data_write(void *data, u32 len);
int get_spdif_none_linear_sta();



#endif//__TASK_SPDIF_H__

