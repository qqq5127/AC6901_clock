#ifndef __TASK_COMMON_H__
#define __TASK_COMMON_H__
#include "typedef.h"

tbool task_common_msg_deal(void *hdl, u32 msg);
u32 get_input_number(u32 *num);

void loop_task(void);

#endif// __TASK_COMMON_H__
