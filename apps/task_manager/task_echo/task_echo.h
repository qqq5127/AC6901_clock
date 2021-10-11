#ifndef __TASK_ECHO__
#define __TASK_ECHO__

#include "task_manager.h"


#if 1   //echo printf en
#define echo_printf(x)   printf(x)
#else
#define echo_printf(...)
#endif


extern const TASK_APP task_echo_info;




#endif

