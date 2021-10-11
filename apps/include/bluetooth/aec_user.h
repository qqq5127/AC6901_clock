#ifndef __AEC_USER_H__
#define __AEC_USER_H__

#include "typedef.h"

#define AEC_DEBUG_ONLINE	1

u32 aec_param_init();
s8 aec_config_online(void *buf, u16 size);

#endif
