/*--------------------------------------------------------------------------*/
/**@file     common.c
   @brief    常用公共函数
   @details
   @author
   @date    2011-3-7
   @note    CD003
*/
/*----------------------------------------------------------------------------*/
#include "typedef.h"
#include "string.h"
#include "common/common.h"
#include "sdk_cfg.h"
#include "irq_api.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif

system_global_t sys_global_value = {
    .fast_test_flag = 0,
    .going_to_pwr_off = 0,
    .mask_task_switch_flag = 0,
};

AT_POWER
void delay(u32 d)
{
    while (d--) {
        __builtin_pi32_nop();
    }
}


void system_enter_critical(void)
{
    struct sys_critical_handler *h;

    list_for_each_loop_sys_critical(h) {
        if (h->enter) {
            h->enter();
        }
    }
}

void system_exit_critical(void)
{
    struct sys_critical_handler *h;

    list_for_each_loop_sys_critical(h) {
        if (h->exit) {
            h->exit();
        }
    }
}

/*
***********************************************************************************
*					DELAY_2MS_COUNT
*
*Description: This function is called by timer_isr
*
*Argument(s): none
*
*Returns	: none
*
*Note(s)	:
***********************************************************************************
*/
static volatile u32 delay2ms_cnt = 0;
static void delay_2ms_count()
{
    if (delay2ms_cnt) {
        delay2ms_cnt--;
    }
}
LOOP_DETECT_REGISTER(delay_2m_cnt) = {
    .time = 1,
    .fun  = delay_2ms_count,
};
/*
***********************************************************************************
*					DELAY_2MS
*
*Description: This function is called by application to block delay
*
*Argument(s): delay_time	wait time = delay_time * 2ms
*
*Returns	: none
*
*Note(s)	: 1)call this function will pend process until delay time decrease to 0
***********************************************************************************
*/
void delay_2ms(u32 delay_time)
{
    delay2ms_cnt = delay_time;
    while (delay2ms_cnt);
}


u8 get_going_to_pwr_off()
{
    return (sys_global_value.going_to_pwr_off);
}

void set_going_to_pwr_off(u8 cnt)
{
    sys_global_value.going_to_pwr_off = cnt;
}


#if ADKEY_SD_MULT_EN
static u8 soft_loop_start;
extern void adc_scan();
SET_INTERRUPT
void soft_loop_irq_loop()
{
    irq_common_handler(IRQ_SOFT_LOOP_IDX);
    adc_scan();
}

static void adc_soft_irq_resume()
{
    if (soft_loop_start) {
        irq_set_pending(IRQ_SOFT_LOOP_IDX);
    } else {
        soft_loop_start = 1;
        irq_handler_register(IRQ_SOFT_LOOP_IDX, soft_loop_irq_loop, irq_index_to_prio(IRQ_SOFT_LOOP_IDX));
    }
}

LOOP_DETECT_REGISTER(adc_scan_detect) = {
    .time = 5,
    .fun  = adc_soft_irq_resume,
};
#endif
//*************************stack detect function*************************//
#ifdef __DEBUG

#include "uart.h"

extern u32 STACK_START[];
extern u32 STACK_END[];

#define STACK_SHIFT		0x100
#define STACK_MAGIC		0x55555555

#define STACK_PROTECT_EN	0
extern void cpu_protect_ram(u32 start, u32 end, u8 range);
extern void prp_protect_ram(u32 begin, u32 end, u8 range);

void stack_detect_init(void)
{
    memset((void *)STACK_START, STACK_MAGIC, ((u32)STACK_END - (u32)STACK_START - STACK_SHIFT));
    memset((void *)((u32)STACK_END - 0x20), STACK_MAGIC, 0x20);
    memset((void *)((u32)STACK_START), 0xAA, 0x10);
#if STACK_PROTECT_EN
    cpu_protect_ram((u32)STACK_START, (u32)STACK_START + 0x40, 1);
    prp_protect_ram((u32)STACK_START, (u32)STACK_START + 0x40, 1);
#endif

}

static int word_cmp(int *p, int ch, u32 num)
{
    int ret = num;
    while (num-- > 0) {
        if (*p++ != ch) {
            return ret - num;
        }
    }
    return 0;
}
u8 do_one_time = 1;
void stack_detect_api(void)
{
    /* printf("stack_start = 0x%x\n", STACK_START); */
    /* printf("stack_end = 0x%x\n", STACK_END); */
    /* printf_buf((void *)STACK_START, ((u32)STACK_END - (u32)STACK_START)); */

    int ret = 0;

    //usp check
    ret = word_cmp((void *)(STACK_START + 0x10), STACK_MAGIC, ((u32)STACK_END - (u32)STACK_START - 0x10) / sizeof(int));
    ret *= sizeof(int);		//word to byte
    if (ret < 0x100) {	//0x100 byte
        printf("warning: stack_reath = 0x%x\n", ret);
        if (do_one_time) {
            do_one_time = 0;
            put_buf((void *)(STACK_START + 0x10), 0x200);
        }
    }

    //ssp check
    ret = word_cmp((void *)((u32)STACK_END - 0x20), STACK_MAGIC, 0x10 / sizeof(int));
    if (ret > 0) {
        printf("warning: sys_stack err = 0x%x\n", ret);
        printf_buf((void *)((u32)STACK_END - 0x20), 0x20);
    }

    /* extern void printf_buf(u8 *buf, u32 len); */
    /* printf_buf((void *)STACK_START, (ret * 4) + 32); */
}

LOOP_DETECT_REGISTER(stack_detect) = {
    .time = 2000,
    .fun  = stack_detect_api,
};

#else
void stack_detect_init(void) {}
#endif
