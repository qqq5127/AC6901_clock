#include "board_init.h"
#include "audio/dac.h"
#include "audio/dac_api.h"
#include "audio/audio.h"
#include "common/common.h"
#include "common/resource_manage.h"
#include "timer.h"
#include "key_drv/key.h"
#include "led.h"
#include "rotate_dec.h"
#include "sdk_cfg.h"
#include "adc_api.h"
#include "power.h"
#include "uart_user.h"
#include "audio/alink.h"
#include "dev_manage.h"
#include "fs.h"
#include "rtc_setting.h"
#include "bluetooth/avctp_user.h"
#include "updata.h"
#include "echo_api.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif
//Sequence independent call
#define do_initcall(a) \
	do{ \
		extern initcall_t * a##_begin[];  \
		extern initcall_t * a##_end[];  \
		__do_initcall(a##_begin, a##_end); \
	}while(0)


static void __do_initcall(initcall_t *begin[], initcall_t *end[])
{
    initcall_t *func;

    for (func = (initcall_t *)begin; func < (initcall_t *)end; func++) {
        /* printf("func : %x\n", func); */
        (*func)();
    }
}

static void usb_2_io(void)
{
    JL_USB->CON0 = (BIT(0));//USB_PHY_ON
    JL_USB->IO_CON0 |= (BIT(11) | BIT(10) | BIT(9)); //USB_IO_MODE	//DMDIE	//DPDIE
}
///根据不同封装设置一些双绑脚IO,和mic绑在一起，设为高阻
static void set_port_init()
{

#ifndef UART_TXPA3_RXPA4
    JL_PORTA->DIR |=  BIT(3);///
    JL_PORTA->PD  &= ~BIT(3);
    JL_PORTA->PU  &= ~BIT(3);
#endif

    JL_PORTA->DIR |=  BIT(0);//MIC IO
    JL_PORTA->PD  &= ~BIT(0);
    JL_PORTA->PU  &= ~BIT(0);
#ifndef UART_USBP_USBM
    usb_2_io();//关闭usb的功能，作为普通IO口
    USB_DP_PU(0);
    USB_DP_PD(0);
    USB_DP_DIR(1);//6919 MIC 和DP绑在一起,DP设为高阻
#endif

}

/**
  * Read flash file should wait cache online
  *
  */
static void flash_file_read(void)
{
    eq_cfg_read();
}


extern void ui_init_api(void);
extern void ui_select(void);
extern void soft_power_check(void);
extern u32 g_updata_flag;
extern void aux_detect_init(void);
void board_init()
{
    // set_port_init();
    //B_LED_ON();//开机蓝先亮
#if UI_ENABLE
    ui_select();
#endif
    adc_init();
	if ((g_updata_flag & 0xffff) == UPDATA_NON)
	{
	#if SOFT_POWER_ON_OFF
		SOFT_POWER_CTL_INIT();
		SOFT_POWER_CHK_INIT();
	#endif
    	power_on_detect_deal();
	}
#if AUX_EN
	aux_detect_init();
#endif
    timer_init();
#if UI_ENABLE
	HT1621LCDInit();
	back_light_set_flag = 1;
	set_timer1_pwm(500,back_light_table[back_light_mode]);//BACKLIGHT_ENABLE();
	backlight_cnt = 11;
	back_light_set_flag = 0;
    ui_init_api();
#endif
	//BACKLIGHT_ENABLE();
	//backlight_cnt = 11;
    do_initcall(_sys_initcall);
    update_result_deal();
    dev_manage_init();
    flash_file_read();
	//soft_power_check();

#if ECHO_EN
    echo_start();
#endif

    resource_manage_init_app();
}
