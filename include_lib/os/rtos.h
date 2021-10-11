#ifndef _RTOS_H_
#define _RTOS_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "typedef.h"

#ifdef HAVE_OS
#include "os/ucos/os_api.h"
#else
#include "os/embedded/embedded_os.h"
#endif

#define RTOS_SEM    OS_SEM
#define RTOS_MSGQ   OS_MSGQ

#include "thread.h"



typedef struct rtos {
    //base
    void (*init)(void);
    void (*start)(void);
    u8(*task_create)(void (*task)(void *priv), void *priv, u8 prio, int qsize, char *name);
    u8(*task_delete)(char *name);

    //  semaphore
    u8(*sem_create)(void *priv, u16 cnt);
    u8(*sem_delete)(void *priv, u8 opt);
    u8(*sem_post)(void *priv);
    u8(*sem_pend)(void *priv, u16 timeout);

    //message queue
    u8(*q_create)(void *priv, void **start, u8 size);
    u8(*q_post)(void *priv, void *msg);
    u8(*q_pend)(void *priv, u16 timeout, void *msg);

    void (*time_tick)(u32);
    u32(*time_get)(void);
} rtos_t;

/* API_START */

void rtos_init(const rtos_t *os);

void rtos_start(void);

u8 rtos_task_create(void (*task)(void *priv), void *priv, u8 prio, int qsize, char *name);

u8 rtos_task_delete(char *name);

u8 rtos_sem_create(void *priv, u16 cnt);

u8 rtos_sem_delete(void *priv, u8 opt);

u8 rtos_sem_post(void *priv);

u8 rtos_sem_pend(void *priv, u16 timeout);

u8 rtos_q_create(void *priv, void **start, u8 size);

u8 rtos_q_post(void *priv, void *msg);

u8 rtos_q_pend(void *priv, u16 timeout, void *msg);

void rtos_time_tick(u32 unit);

u32 rtos_time_get(void);

const rtos_t *rtos_embedded_get_instance(void);

#ifdef __cplusplus
}
#endif

#endif
