#ifndef THREAD_H
#define THREAD_H

#include "typedef.h"
#include "common/list.h"


#define PRIORITY_NUM   4

struct thread_owner {
    struct list_head head;
    int (*create)(void (*fun)(struct thread_owner *), u8);
    int (*delete)(u8);
    int (*suspend)(u8, u8);
    int (*resume)(void);
};


struct thread {
    struct list_head entry;
    struct thread_owner *owner;
    char *name;
    void (*fun)(struct thread *);
    u8 priority;
    //resume
    u8 resume_cnt;
    //suspend
    u8 suspend_timeout;
};
int thread_owner_init(struct thread_owner *owner);

int thread_create(struct thread *th, char *name, void (*fun)(struct thread *), struct thread_owner *owner);

void thread_suspend(struct thread *th, int timeout);

void thread_resume(struct thread *th);

int thread_delete(struct thread *th);

void thread_run(struct thread_owner *owner);

extern struct thread_owner bt_task_thread;
#endif

