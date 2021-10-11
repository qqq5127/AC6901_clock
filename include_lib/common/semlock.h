#ifndef SEM_LOCK_H_
#define SEM_LOCK_H_

#include "cpu.h"


typedef struct {
    int counter;
} semlock_t;

static inline int semlock_add_return(int i, semlock_t *v)
{
    int val;
    CPU_SR_ALLOC();

    CPU_INT_DIS();

    val = v->counter;
    v->counter = val += i;

    CPU_INT_EN();

    return val;
}


static inline int semlock_sub_return(int i, semlock_t *v)
{
    int val;
    CPU_SR_ALLOC();

    CPU_INT_DIS();

    val = v->counter;
    v->counter = val -= i;

    CPU_INT_EN();

    return val;
}


#define semlock_add(i, v)	(void) semlock_add_return(i, v)
#define semlock_sub(i, v)	(void) semlock_sub_return(i, v)

#define semlock_read(v)	(*(volatile int *)&(v)->counter)
#define semlock_set(v,i)	(((v)->counter) = (i))

#define semlock_inc(v)		semlock_add(1, v)
#define semlock_dec(v)		semlock_sub(1, v)

#define semlock_inc_and_test(v)	(semlock_add_return(1, v) == 0)
#define semlock_dec_and_test(v)	(semlock_sub_return(1, v) == 0)
#define semlock_inc_return(v)    (semlock_add_return(1, v))
#define semlock_dec_return(v)    (semlock_sub_return(1, v))
#define semlock_sub_and_test(i, v) (semlock_sub_return(i, v) == 0)

#define semlock_add_negative(i,v) (semlock_add_return(i, v) < 0)




#endif

