/*--------------------------------------------------------------------------*/
/**@file     common.h
   @brief
   @details
   @author
   @date    2011-3-7
   @note    CD003
*/
/*----------------------------------------------------------------------------*/
#ifndef _COMMON_
#define _COMMON_

#include "typedef.h"

/*
 * boot info, init in maskrom
 * */

// u8 osc_type;
#define MASK_ROM_BT_XOSC				0x01
#define MASK_ROM_RTC_XOSC				0x02
#define MASK_ROM_RC_HOSC				0x03

// u8 dev_type;
#define MASK_ROM_SDDEVICE_NOR_FLASH		0x00
#define MASK_ROM_SDDEVICE_SDCARD		0x01
#define MASK_ROM_SDDEVICE_NAND_FLASH	0x02

// u8 spi_port;
#define MASK_ROM_SPI0_PORTD_A			0x00
#define MASK_ROM_SPI0_PORTB_B			0x01

// u32 boot_method;
#define MASK_ROM_RUN_UBOOT_NORMAL		0x00
#define MASK_ROM_RUN_UBOOT_FROM_FIX		0x01

/*
 * delay api ******
 * */
void delay(u32  t);
void delay_2ms(u32 delay_time);
#define delay_n10ms(x)  delay_2ms(5*(x))


#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */


u16 CRC16(const void *ptr, u32 len);


/*不同功能的循环检测功能统一用LOOP_DETECT_REGISTER注册，可搜索关键字看例子*/
struct loop_detect_handler {
    int time;/*TIMER周期的倍数。timer周期默认一般是2ms */
    void (*fun)();
};
extern struct loop_detect_handler loop_detect_handler_begin[];
extern struct loop_detect_handler loop_detect_handler_end[];

#define LOOP_DETECT_REGISTER(handler) \
	const struct loop_detect_handler handler \
		SEC_USED(.loop_detect_region)

#define list_for_each_loop_detect(h) \
	for (h=loop_detect_handler_begin; h<loop_detect_handler_end; h++)

extern struct loop_detect_handler loop_ui_handler_begin[];
extern struct loop_detect_handler loop_ui_handler_end[];

#define LOOP_UI_REGISTER(handler) \
	const struct loop_detect_handler handler \
	    sec_used(.loop_ui_region)

#define list_for_each_loop_ui(h) \
	for (h=loop_ui_handler_begin; h<loop_ui_handler_end; h++)

extern struct loop_detect_handler usloop_detect_handler_begin[];
extern struct loop_detect_handler usloop_detect_handler_end[];

#define USLOOP_DETECT_REGISTER(handler) \
	struct loop_detect_handler handler \
		sec_used(.usloop_detect_region)

#define list_for_each_usloop_detect(h) \
	for (h=usloop_detect_handler_begin; h<usloop_detect_handler_end; h++)


extern struct loop_detect_handler sync_time_detect_handler_begin[];
extern struct loop_detect_handler sync_time_detect_handler_end[];

#define TWS_DETECT_REGISTER(handler) \
    const struct loop_detect_handler handler \
        SEC_USED(.sync_time_loop_detect_region)

#define list_for_each_tws_detect(h) \
     for (h=sync_time_detect_handler_begin; h<sync_time_detect_handler_end; h++)




/*
 * system enter critical and exit critical handle
 * */
struct sys_critical_handler {
    void (*enter)();
    void (*exit)();
};

#define SYS_CRITICAL_HANDLE_REG(enter, exit) \
	const struct sys_critical_handler enter_##exit \
		AT(.sys_critical_txt) = {enter, exit};

extern struct sys_critical_handler sys_critical_handler_begin[];
extern struct sys_critical_handler sys_critical_handler_end[];

#define list_for_each_loop_sys_critical(h) \
	for (h=sys_critical_handler_begin; h<sys_critical_handler_end; h++)

void system_enter_critical(void);
void system_exit_critical(void);


/*
 * 下面的结构体用于定义一些系统不同功能之间的变量判断，
 * 用于减少系统模块之间的耦合性
 * */
typedef struct sys_global {
    u8 fast_test_flag;		/*1A:enter fast test*/
    u8 going_to_pwr_off;    /*enter pwr_off flag,maybe play some tone */
    u8 mask_task_switch_flag; /*when a phone call ,it can't change mode */
    volatile u8 sys_halfsec;   //timer use
    volatile u16 t0_cnt1;		//timer0 use
    volatile u16 t2_cnt1;		//timer2 use
    volatile u16 t3_cnt1;		//timer3 use
} system_global_t;

extern system_global_t sys_global_value;

u8 get_going_to_pwr_off();
void set_going_to_pwr_off(u8 cnt);



/*
 * heap_mem_api
 * */
#ifdef HAVE_MALLOC
void *malloc(u32 size);
void free(void *mem);
u32 getfreeheapsize(void);
u32 getminimumeverfreeheapsize(void);
#define HEAP_MEM_STATUS()	\
							getfreeheapsize();	\
							getminimumeverfreeheapsize()
#else
#define HEAP_MEM_STATUS()	(...)
#endif


#endif
