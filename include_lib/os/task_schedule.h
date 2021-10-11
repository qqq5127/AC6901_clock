#ifndef TASK_SCHEDULE_H
#define TASK_SCHEDULE_H


#include "typedef.h"
#include "os/rtos.h"
#include "os/thread.h"

//high -> low
enum {
    TASK_PRIO_INPUT = 0,
    TASK_PRIO_BTSTACK,
    TASK_PRIO_TASK1,
    TASK_PRIO_TASK2,
    TASK_PRIO_STREAM_DEV,
    TASK_PRIO_ASYNC_TIMER,
    TASK_PRIO_UI,
    TASK_PRIO_MAIN,
    TASK_IDLE,              //always exist
};

//Not time slice
#define TASK_DEBUG_IDLE             IO_DEBUG_TOGGLE(C, 0)

#define TASK_NAME_TASK1             "I/O wait Task"
// #define TASK_PRIO_TASK1             TASK_PRIO1
// #define TASK_DEBUG_TASK1

#define TASK_NAME_TASK2             "CPU use Task"
// #define TASK_PRIO_TASK2             TASK_PRIO2
// #define TASK_DEBUG_TASK2

#define TASK_NAME_STREAM_DEV        "stream_dev Task"
// #define TASK_PRIO_STREAM_DEV        TASK_PRIO2
// #define TASK_DEBUG_STREAM_DEV

#define TASK_NAME_ASYNC_TIMER       "async_timer Task"
// #define TASK_PRIO_ASYNC_TIMER       TASK_PRIO1
// #define TASK_DEBUG_ASYNC_TIMER

#define TASK_NAME_UI                "UI Task"
// #define TASK_PRIO_UI                TASK_PRIO1

#define TASK_NAME_MAIN              "main Task"
// #define TASK_PRIO_MAIN              TASK_PRIO1
#define TASK_DEBUG_MAIN             IO_DEBUG_TOGGLE(C, 1)
//
#define TASK_NAME_BTSTACK           "btstack Task"
#define TASK_DEBUG_BTSTACK          IO_DEBUG_TOGGLE(C, 2)
extern struct thread_owner          bt_task_thread;

#define TASK_NAME_INPUT           "Input Task"


#endif  /*  TASK_SCHEDULE_H */
