/***********************************Jieli tech************************************************
  File : timer.c
  By   : Juntham
  date : 2014-07-04 11:21
********************************************************************************************/
#include "sdk_cfg.h"
#include "timer.h"
#include "clock_api.h"
#include "irq_api.h"
#include "clock.h"
#include "dac.h"
#include "msg.h"
#include "common/sys_timer.h"
#include "common/common.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".timer_app_bss")
#pragma data_seg(	".timer_app_data")
#pragma const_seg(	".timer_app_const")
#pragma code_seg(	".timer_app_code")
#endif

#define SOURCE_CLK	1	//0:LSB		1:OSC
#define MAX_TIME_CNT 0x7fff
#define MIN_TIME_CNT 0x0100
#define MIN_TIME_CNT1 0x0010
#if SOURCE_CLK
#define TIMER_CLK        IOSC_CLK
#else
#define TIMER_CLK        LSB_CLK
#endif

void (*timer2_caback_fun)() = NULL;

static const u16 timer_div[] = {
    /*0000*/    1,
    /*0001*/    4,
    /*0010*/    16,
    /*0011*/    64,
    /*0100*/    2,
    /*0101*/    8,
    /*0110*/    32,
    /*0111*/    128,
    /*1000*/    256,
    /*1001*/    4 * 256,
    /*1010*/    16 * 256,
    /*1011*/    64 * 256,
    /*1100*/    2 * 256,
    /*1101*/    8 * 256,
    /*1110*/    32 * 256,
    /*1111*/    128 * 256,
};

u32 os_time_get(void)
{
    return get_jiffies(0, 0);
}
/*----------------------------------------------------------------------------*/
/**@brief  get halfsec flag
   @param
   @return
   @note

 */
/*----------------------------------------------------------------------------*/
u8 get_sys_halfsec(void)
{
    return sys_global_value.sys_halfsec;
}

/*----------------------------------------------------------------------------*/
/**@brief  get 2ms's count
   @param
   @return counkt
   @note

 */
/*----------------------------------------------------------------------------*/
extern u8 cancel_mute_cnt;
extern u8 cancel_mute_flag;
static void timer0_isr_fun(void)
{
    struct loop_detect_handler *detect;
    JL_TIMER0->CON |= BIT(14);

    sys_global_value.t0_cnt1++;

    list_for_each_loop_detect(detect) {
        if ((sys_global_value.t0_cnt1 %  detect->time) == 0) {
            if (detect->fun) {
                detect->fun();
            }
        }
    }
#if WARNING_SD_USB
	if (cancel_mute_cnt)
	{
		cancel_mute_cnt--;
		if (cancel_mute_cnt == 0)
			cancel_mute_flag = 1;
	}
#endif
#if USE_AD_TUNER_VOLUME
	static u8 cnt=0;
	cnt++;
	if (cnt >= 20)
	{
		cnt = 0;
		flag_100ms = 1;
	}
#endif
}
IRQ_REGISTER(IRQ_TIME0_IDX, timer0_isr_fun);

static void timer0_init(void)
{
    u32 prd_cnt, clk, tmp_tick;
    u8 index;
    u8 clk_src;
    u8 catch_flag = 0;

    //resrt_sfr
    JL_TIMER0->CON = 0;

    clk = TIMER_CLK;
    clk /= 1000;
    clk *= 2; ///2ms
    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = clk / timer_div[index];
        if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT) {
            catch_flag = 1;
            break;
        }
    }

#if SOURCE_CLK
    clk_src = TIMER_CLK_SRC_OSC;
#else
    clk_src = TIMER_CLK_SRC_SYSCLK;
#endif

    if (catch_flag == 0) {
        puts("warning:timer_err\n");
        return;
    }

    IRQ_REQUEST(IRQ_TIME0_IDX, timer0_isr_fun);

    JL_TIMER0->CNT = 0;
    JL_TIMER0->PRD = prd_cnt - 1;
    JL_TIMER0->CON = BIT(0) | (clk_src << 2) | (index << 4);

    /* log_printf("JL_TIMER0->CNT = 0x%x\n", JL_TIMER0->CNT); */
    /* log_printf("JL_TIMER0->PRD = 0x%x\n", JL_TIMER0->PRD); */
    /* log_printf("JL_TIMER0->CON = 0x%x\n", JL_TIMER0->CON); */
}
void timer2_isr_fun(void)
{
    struct loop_detect_handler *detect;
    JL_TIMER2->CON |= BIT(14);
    sys_global_value.t2_cnt1++;
    IRQ_RELEASE(IRQ_TIME2_IDX);
    if (timer2_caback_fun) {
        timer2_caback_fun();
        timer2_caback_fun = NULL;
    }
    /* puts("timer2_isr_fun\n"); */
#if 0
    list_for_each_tws_detect(detect) {
        /* putchar('@'); */
        if ((sys_global_value.t2_cnt1 %  detect->time) == 0) {
            if (detect->fun) {
                detect->fun();
            }
        }
    }
#endif
}

IRQ_REGISTER(IRQ_TIME2_IDX, timer2_isr_fun);
void tws_sync_set_timer2_us(u32 time_us, void (*fun)())
{
    u32 tem_tick = time_us;
    u32 tem_tick1;
    s32 timer_num;
    u32 prd_cnt, clk, tmp_tick;
    u8 index;
    u8 clk_src;
    u8 catch_flag = 0;
    if (tem_tick == 0) {
        IRQ_RELEASE(IRQ_TIME2_IDX);
        timer2_caback_fun = NULL;
    }
    if (tem_tick < 4) {
        tem_tick = 4;
    }
    //resrt_sfr
    JL_TIMER2->CON = 0;

    clk = TIMER_CLK;
    clk /= 1000;
    tem_tick1 = (tem_tick * 100) / 100;
    clk = clk * tem_tick1 / 1000;
    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = clk / timer_div[index];
        /* printf("----prd_cnt=0x%x\n",prd_cnt ); */
        if (prd_cnt > MIN_TIME_CNT1 && prd_cnt < MAX_TIME_CNT) {
            catch_flag = 1;
            break;
        }
    }

#if SOURCE_CLK
    clk_src = TIMER_CLK_SRC_OSC;
#else
    clk_src = TIMER_CLK_SRC_SYSCLK;
#endif

    if (catch_flag == 0) {
        puts("warning:timer_err\n");
        return;
    }

    timer2_caback_fun = fun;
    IRQ_REQUEST(IRQ_TIME2_IDX, timer2_isr_fun);

    JL_TIMER2->CNT = 0;
    JL_TIMER2->PRD = prd_cnt - 1;
    JL_TIMER2->CON = BIT(0) | (clk_src << 2) | (index << 4);

    /* log_printf("JL_TIMER0->CNT = 0x%x\n", JL_TIMER0->CNT); */
    /* log_printf("JL_TIMER0->PRD = 0x%x\n", JL_TIMER0->PRD); */
    /* log_printf("JL_TIMER0->CON = 0x%x\n", JL_TIMER0->CON); */
}

/****为了处理闪屏问题。默认独立使用timer3设置成最高优先级使用。
 *   如果有其他调度要认真考虑能不能放在最高优先级里面
 * ****/
static void timer3_isr_fun(void)
{
    struct loop_detect_handler *detect;
    JL_TIMER3->CON |= BIT(14);

    sys_global_value.t3_cnt1++;

    list_for_each_loop_ui(detect) {
        if ((sys_global_value.t3_cnt1 %  detect->time) == 0) {
            if (detect->fun) {
                detect->fun();
            }
        }
    }
    if (0 == (sys_global_value.t3_cnt1 % 250)) {
        sys_global_value.sys_halfsec = !sys_global_value.sys_halfsec;//led7 driver
    }
}
IRQ_REGISTER(IRQ_TIME3_IDX, timer3_isr_fun);

static void timer3_init(void)
{
    u32 prd_cnt, clk, tmp_tick;
    u8 index;
    u8 clk_src;
    u8 catch_flag = 0;

    //resrt_sfr
    JL_TIMER3->CON = 0;

    clk = TIMER_CLK;
    clk /= 1000;
    clk *= 2; ///2ms
    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = clk / timer_div[index];
        if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT) {
            catch_flag = 1;
            break;
        }
    }

#if SOURCE_CLK
    clk_src = TIMER_CLK_SRC_OSC;
#else
    clk_src = TIMER_CLK_SRC_SYSCLK;
#endif

    if (catch_flag == 0) {
        puts("warning:timer_err\n");
        return;
    }

    IRQ_REQUEST(IRQ_TIME3_IDX, timer3_isr_fun);

    JL_TIMER3->CNT = 0;
    JL_TIMER3->PRD = prd_cnt - 1;
    JL_TIMER3->CON = BIT(0) | (clk_src << 2) | (index << 4);

    /* log_printf("JL_TIMER3->CNT = 0x%x\n", JL_TIMER3->CNT); */
    /* log_printf("JL_TIMER3->PRD = 0x%x\n", JL_TIMER3->PRD); */
    /* log_printf("JL_TIMER3->CON = 0x%x\n", JL_TIMER3->CON); */
}

void timer_init(void (*isr_fun)(void))
{
    sys_timer_var_init() ;  //major use in bluetooth
    timer0_init();
    timer3_init();
}

