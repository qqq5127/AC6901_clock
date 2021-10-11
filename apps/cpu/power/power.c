#include "power.h"
#include "rtc/rtc_api.h"
#include "charge.h"
#include "audio/dac.h"
#include "power_manage_api.h"
#include "clock.h"
#include "charge.h"
#include "adc_api.h"
/* #include "dac.h" */
#include "audio/dac_api.h"
#include "msg.h"
#include "key_drv/key.h"
#include "led.h"
#include "bluetooth/avctp_user.h"
#include "crc_api.h"
#include "board_init.h"
#include "dev_mg_api.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".power_driver_bss")
#pragma data_seg(	".power_driver_data")
#pragma const_seg(	".power_driver_const")
#pragma code_seg(	".power_driver_code")
#endif
#define DEBUG_RUN_FLASH_POARTB  1 //run flash 大封装片80P的  flash: PB0 PB1 PB2 PB6
#define DEBUG_RUN_FLASH_POARTD  2 //run flash 小封装片24P、20P的  flash: PD0 PD1 PD2 PD3

#define DEBUG_RUN_FLASH_POART  DEBUG_RUN_FLASH_POARTD //选择 对应使用的FLASH POART口
u16 LDO_BT_value = 1200;//1.20V放大1000倍

void in_low_pwr_port_deal(u8 mode)
{
    u32 porta_value = 0xffff;
    u32 portb_value = 0xffff;
    u32 portc_value = 0xffff;
    u32 portd_value = 0xffff;

    /* if (get_work_mode() == RUN_FLASH_MODE) { //跑flash ，进入powerdown不能关闭对应的flash IO口 */
    /* #if (DEBUG_RUN_FLASH_POART==DEBUG_RUN_FLASH_POARTD) */
    portd_value &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(3));
    /* #elif(DEBUG_RUN_FLASH_POART==DEBUG_RUN_FLASH_POARTB) */
    portb_value &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(6));
    /* #endif */
    /* } */

    /*
    put_u32hex(porta_value);
    put_u32hex(portb_value);
    put_u32hex(portc_value);
    put_u32hex(portd_value);
    */

    JL_PORTA->DIR |=  porta_value;
    JL_PORTA->PU &= ~porta_value;
    JL_PORTA->PD &= ~porta_value;
    JL_PORTA->DIE &= ~porta_value;


    JL_PORTB->DIR |=  portb_value;
    JL_PORTB->PU &= ~portb_value;
    JL_PORTB->PD &= ~portb_value;
    JL_PORTB->DIE &= ~portb_value;


    JL_PORTC->DIR |=  portc_value;
    JL_PORTC->PU &= ~portc_value;
    JL_PORTC->PD &= ~portc_value;
    JL_PORTC->DIE &= ~portc_value;

    /*注意spi flash的几个脚不能关*/
    JL_PORTD->DIR |= portd_value;
    JL_PORTD->PU  &= ~portd_value;
    JL_PORTD->PD  &= ~portd_value;
    JL_PORTD->DIE &= ~portd_value;

    /////////////////usb///////////////////
    JL_USB->IO_CON0 = BIT(11) | BIT(2) | BIT(3); //io_mode,DP设置为输入//DP设置为输入

    /* JL_USB->IO_CON0 |= BIT(2) | BIT(3); //DP设置为输入//DP设置为输入 */
    /* JL_USB->IO_CON0 &= ~(BIT(7) | BIT(6) | BIT(5) | BIT(4));//close usb io PU PD */

    JL_USB->CON0 = 0;
}

static void out_low_pwr_deal(u32 time_ms)
{
    /* key_init(); */
#if KEY_AD_RTCVDD_EN
    extern void ad_rtcvdd_key_filter(u8 cnt);
    ad_rtcvdd_key_filter(0);
#endif
    extern void low_pwr_deal_time(u32 time_ms);
    low_pwr_deal_time(time_ms);
}

static void low_pwr_deal(u8 mode, u32 timer_ms)
{
    if (mode) {
        /* putchar('i'); */
        in_low_pwr_port_deal(mode);
    } else {
        /* putchar('o'); */
        //log_printf("timer_ms=%d\n", timer_ms);
        out_low_pwr_deal(timer_ms);
        get_jiffies(1, timer_ms);
    }
}

enum {
    WAKEUP_0 = 0,
    WAKEUP_1,
    WAKEUP_2,
    WAKEUP_3,
    WAKEUP_4,
    WAKEUP_5,
    WAKEUP_6,
    WAKEUP_7,
    WAKEUP_8,
    WAKEUP_9,
    WAKEUP_10,
    WAKEUP_11,
    WAKEUP_12,
    WAKEUP_13,
    WAKEUP_14,
    WAKEUP_15,
};

#define WAKEUP_UP_EDGE     0x0f
#define WAKEUP_DOWN_EDGE   0xf0
static void set_poweroff_wakeup_io()
{
    /* log_printf("wk io\n"); */

    u8 wakeup_io_en = 0;
    u8 wakeup_edge = 0;

    //BIT(0)  PR0 : 0 disable  1 enable
    //BIT(1)  PR1 : 0 disable  1 enable
    //BIT(2)  PR2 : 0 disable  1 enable
    //BIT(3)  PR3 : 0 disable  1 enable
    /* wakeup_io_en |= WAKE_UP_PR0 | WAKE_UP_PR1 | WAKE_UP_PR2 | WAKE_UP_PR3; */
    wakeup_io_en |=  WAKE_UP_PR1|WAKE_UP_PR2;

    //BIT(4)  PR0 : 0 rising dege  1 failling edge
    //BIT(5)  PR1 : 0 rising dege  1 failling edge
    //BIT(6)  PR2 : 0 rising dege  1 failling edge
    //BIT(7)  PR3 : 0 rising dege  1 failling edge
    /* wakeup_edge |= EDGE_PR0 | EDGE_PR1 | EDGE_PR2 | EDGE_PR3;     //failling edge */
    /* wakeup_edge &= ~(EDGE_PR0 | EDGE_PR1 | EDGE_PR2 | EDGE_PR3);  //rising dege */
    wakeup_edge |= EDGE_PR2;     //failling edge

    soft_poweroff_wakeup_io(wakeup_io_en, wakeup_edge);

    extern void wakeup_pending_deal(void);
    wakeup_pending_deal();
}

/*enter sleep mode wakeup IO setting*/
static void enter_sleep_mode_set(u16 wakeup_cfg, u8 wakeup_edge)
{
#if 1
    close_wdt();

    dac_toggle(0); //close dac mudule

    while (!(BIT(7) & JL_ADC->CON));
    JL_ADC->CON = 0;

    JL_AUDIO->LADC_CON = 0;
    JL_AUDIO->ADA_CON0 = 0;
    JL_AUDIO->ADA_CON1 = 0;

    JL_AUDIO->DAA_CON0 = 0;
    JL_AUDIO->DAA_CON1 = 0;
    JL_AUDIO->DAA_CON2 = 0;
    JL_AUDIO->DAA_CON3 = 0;

    /* JL_SYSTEM->LVD_CON = 0; */
    JL_SYSTEM->LDO_CON1 = 0;

    JL_WAKEUP->CON0 = 0;      //wakeup enbale
    JL_WAKEUP->CON1 = 0;      //wakeup edge
    JL_WAKEUP->CON2 = 0xffff; //wakeup pending(clear)

    switch (wakeup_cfg) {

    case WAKEUP_0:
        /*
         * WAKEUP_0可以设置64个IO作为唤醒源（不能同时使能，只能使能其中一个）
         * WAKEU0_0可以设置任意一个IO作为唤醒IO。具体的设置方法参考JL_WAKEUP寄存器和JL_IOMAP->CON2寄存器的低5位。
         * JL_IOMAP->CON2的低5位的数值范围是:0~63，对应WAKEUP_0的64个IO唤醒源。0：PA0，1：PA1，... 63：1'd0
         * 下面以PA2为例：（PA2不属于特定的唤醒IO，所以用任意IO映射）
         */

        log_printf("WAKEUP_0\n");

        JL_WAKEUP->CON2 |= BIT(0);
        JL_PORTA->DIR |= BIT(2);
        JL_PORTA->DIE |= BIT(2);

        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTA->PU |= BIT(2);
            JL_PORTA->PD &= ~BIT(2);
            JL_IOMAP->CON2 &= ~0x1F;
            JL_IOMAP->CON2 |= BIT(1);   //映射PA2口为唤醒IO
            JL_WAKEUP->CON1 |= BIT(0);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTA->PU &= ~BIT(2);
            JL_PORTA->PD |= BIT(2);
            JL_IOMAP->CON2 &= ~0x1F;
            JL_IOMAP->CON2 |= BIT(1);
            JL_WAKEUP->CON1 &= ~BIT(0);
        }
        JL_WAKEUP->CON0 |= BIT(0);

        break;


    case WAKEUP_1:

        break;



    case WAKEUP_2:

        log_printf("WAKEUP_2\n");

        JL_WAKEUP->CON2 |= BIT(2);
        JL_PORTA->DIR |= BIT(8);
        JL_PORTA->DIE |= BIT(8);

        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTA->PU |= BIT(8);
            JL_PORTA->PD &= ~BIT(8);
            JL_WAKEUP->CON1 |= BIT(2);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTA->PU &= ~BIT(8);
            JL_PORTA->PD |= BIT(8);
            JL_WAKEUP->CON1 &= ~BIT(2);
        }
        JL_WAKEUP->CON0 |= BIT(2);

        break;


    case WAKEUP_3:

        log_printf("WAKEUP_3\n");

        JL_WAKEUP->CON2 |= BIT(3);
        JL_PORTA->DIR |= BIT(10);
        JL_PORTA->DIE |= BIT(10);

        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTA->PU |= BIT(10);
            JL_PORTA->PD &= ~BIT(10);
            JL_WAKEUP->CON1 |= BIT(3);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTA->PU &= ~BIT(10);
            JL_PORTA->PD |= BIT(10);
            JL_WAKEUP->CON1 &= ~BIT(3);
        }
        JL_WAKEUP->CON0 |= BIT(3);

        break;

    case WAKEUP_4:
        log_printf("WAKEUP_4\n");
        JL_WAKEUP->CON2 |= BIT(4);
        JL_PORTA->DIR |= BIT(12);
        JL_PORTA->DIE |= BIT(12);
        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTA->PU |= BIT(12);
            JL_PORTA->PD &= ~BIT(12);
            JL_WAKEUP->CON1 |= BIT(4);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTA->PU &= ~BIT(12);
            JL_PORTA->PD |= BIT(12);
            JL_WAKEUP->CON1 &= ~BIT(4);
        }
        JL_WAKEUP->CON0 |= BIT(4);

        break;
    case WAKEUP_5:
        break;
    case WAKEUP_6:
        break;
    case WAKEUP_7:
        break;


    case WAKEUP_8:

        log_printf("WAKEUP_8\n");

        JL_WAKEUP->CON2 |= BIT(8);
        JL_PORTA->DIR |= BIT(3);
        JL_PORTA->DIE |= BIT(3);

        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTA->PU |= BIT(3);
            JL_PORTA->PD &= ~BIT(3);
            JL_WAKEUP->CON1 |= BIT(8);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTA->PU &= ~BIT(3);
            JL_PORTA->PD |= BIT(3);
            JL_WAKEUP->CON1 &= ~BIT(8);
        }
        JL_WAKEUP->CON0 |= BIT(8);

        break;

    case WAKEUP_9:

        log_printf("WAKEUP_9\n");

        JL_WAKEUP->CON2 |= BIT(9);
        JL_PORTA->DIR |= BIT(5);
        JL_PORTA->DIE |= BIT(5);

        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTA->PU |= BIT(5);
            JL_PORTA->PD &= ~BIT(5);
            JL_WAKEUP->CON1 |= BIT(9);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTA->PU &= ~BIT(5);
            JL_PORTA->PD |= BIT(5);
            JL_WAKEUP->CON1 &= ~BIT(9);
        }
        JL_WAKEUP->CON0 |= BIT(9);

        break;

    case WAKEUP_10:

        log_printf("WAKEUP_10\n");

        JL_WAKEUP->CON2 |= BIT(10);
        JL_PORTB->DIR |= BIT(0);
        JL_PORTB->DIE |= BIT(0);

        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTB->PU |= BIT(0);
            JL_PORTB->PD &= ~BIT(0);
            JL_WAKEUP->CON1 |= BIT(10);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTB->PU &= ~BIT(0);
            JL_PORTB->PD |= BIT(0);
            JL_WAKEUP->CON1 &= ~BIT(10);
        }
        JL_WAKEUP->CON0 |= BIT(10);

        break;

    case WAKEUP_11:

        log_printf("WAKEUP_11\n");

        JL_WAKEUP->CON2 |= BIT(11);
        JL_PORTB->DIR |= BIT(2);
        JL_PORTB->DIE |= BIT(2);

        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTB->PU |= BIT(2);
            JL_PORTB->PD &= ~BIT(2);
            JL_WAKEUP->CON1 |= BIT(11);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTB->PU &= ~BIT(2);
            JL_PORTB->PD |= BIT(2);
            JL_WAKEUP->CON1 &= ~BIT(11);
        }
        JL_WAKEUP->CON0 |= BIT(11);
        break;
    case WAKEUP_12:
        log_printf("WAKEUP_12\n");
        JL_WAKEUP->CON2 |= BIT(12);
        JL_PORTB->DIR |= BIT(4);
        JL_PORTB->DIE |= BIT(4);
        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTB->PU |= BIT(4);
            JL_PORTB->PD &= ~BIT(4);
            JL_WAKEUP->CON1 |= BIT(12);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTB->PU &= ~BIT(4);
            JL_PORTB->PD |= BIT(4);
            JL_WAKEUP->CON1 &= ~BIT(12);
        }
        JL_WAKEUP->CON0 |= BIT(12);
        break;
    case WAKEUP_13:
        log_printf("WAKEUP_13\n");
        JL_WAKEUP->CON2 |= BIT(13);
        JL_PORTA->DIR |= BIT(1);
        JL_PORTA->DIE |= BIT(1);
        if (wakeup_edge == WAKEUP_DOWN_EDGE) {
            log_printf("WAKEUP_DOWN_EDGE\n");
            JL_PORTA->PU |= BIT(1);
            JL_PORTA->PD &= ~BIT(1);
            JL_WAKEUP->CON1 |= BIT(13);
        } else {
            log_printf("WAKEUP_UP_EDGE\n");
            JL_PORTA->PU &= ~BIT(1);
            JL_PORTA->PD |= BIT(1);
            JL_WAKEUP->CON1 &= ~BIT(13);
        }
        JL_WAKEUP->CON0 |= BIT(13);

        break;
    case WAKEUP_14:
        break;
    case WAKEUP_15:

        break;

    default :
        return;
    }
#endif
}

/*sleep mode wakeup io setting fuction*/
static void set_sleep_mode_wakeup_io()
{
    enter_sleep_mode_set(WAKEUP_3, WAKEUP_DOWN_EDGE);
}

/*check wheather can enter lowpower*/
/*return 0:can not enter lowpower  1:can enter lowpower*/
static u32 wheather_can_enter_lowpower(void)
{
    if (get_dev_mg_stats()) {		//device is busy
        return 0;
    }

    if (get_key_invalid_flag()) {
        putchar('k');
        return 0;
    }
    if (sys_global_value.fast_test_flag == 0x1A) {			//fast test
        return 0;
    }
    /* if (!is_auto_mute()) { */
    /* putchar('a'); */
    /* return 0; */
    /* } */
    if (get_edr_status()) {
        return 0;
    }

    if (dac.toggle) {
        putchar('d');
        task_post_msg(NULL, 1, MSG_DAC_OFF);
        return 0;
    }

    return 1;
}

/*bt noconn enter powerdown callback*/
u32 bt_noconn_pwr_down_in(void)
{
    puts("pwr_d_in\n");

    if (sys_global_value.fast_test_flag == 0x1A) {			//fast test
        return 0;
    }

    //close dac module
    task_post_msg(NULL, 1, MSG_DAC_OFF);
    return 0;
}

/*bt noconn exit powerdown callback*/
u32 bt_noconn_pwr_down_out(void)
{
    puts("pwr_d_out\n");
    //open dac module
    task_post_msg(NULL, 1, MSG_DAC_ON);
    return 0;
}

extern u8 lmp_get_sniff_flag(void);
void get_powerdown_osc_info(u32 *osc, u32 *oshz)
{
    u8 flag = lmp_get_sniff_flag();

    if (flag) {
        *osc = LRC_32K;
        *oshz = 32000L;
    } else {
        *osc = BT_OSC;
        *oshz = 24000000L;
    }
}
/*
***********************************************************************************
*					PWR SETUP INIT
*
*Description: This function is called to init pwr_cfg when enter power_down
*
*Argument(s): none

*Returns	: none
*
*Note(s)	: 1)
***********************************************************************************
*/
struct _PWR_SETUP {
    u16 crc;
    /* 0(3.4v)  1(3.2v) 2(3.0v) 3(2.8v) 4(2.6v) 5(2.4v) 6(2.2v) 7(2.0v) */
    u8	vddio: 4;
    u8	rtcvdd: 4;
    /* 0(0.728v)  1(0.696v) 2(0.648v) 3(0.605v) 4(0.571v) 5(0.537v) 6(0.501v) 7(0.467v) */
    u8	wdvdd: 4;
    /* 0:152nA 1:308nA 2:660nA 3:660nA */
    u8	wvbg_cur: 4;
} _GNU_PACKED_;
typedef struct _PWR_SETUP PWR_SETUP;
PWR_SETUP *pwr_setup;
/* extern u32 pwr_info_base; */
extern u32 low_pwr_base;
extern u32 args[3];
void pwr_setup_init(void)
{
#if 0
    u8 idx = 0;
    u16 crc = 0;
    u32 setup_base = args[1];
    pwr_setup = (PWR_SETUP *)((u32)&pwr_info_base - setup_base);
    //printf("PWR_BASE:0x%x\n",&pwr_info_base);
    //printf("PWR_SETUP:0x%x\n",pwr_setup);
    put_buf((u8 *)pwr_setup, sizeof(PWR_SETUP) * 4);
    while (idx < 4) {
        crc = crc16((void *)((u8 *)&pwr_setup[idx].crc + 2), 2);
        if (pwr_setup[idx].crc == crc) {
            log_printf("idx:%d\n", idx);
            log_printf("vddio:%d\n", pwr_setup[idx].vddio);
            log_printf("rtcvdd:%d\n", pwr_setup[idx].rtcvdd);
            log_printf("wdvdd:%d\n", pwr_setup[idx].wdvdd);
            log_printf("wvbg_cur:%d\n", pwr_setup[idx].wvbg_cur);
            set_lowpower_pd_ldo_level(pwr_setup[idx].vddio, pwr_setup[idx].rtcvdd, pwr_setup[idx].wdvdd, pwr_setup[idx].wvbg_cur);
            break;
        }
        idx++;
    }
#endif
}
static void set_33v_clk_div(void)
{
    extern void P33_CLK_INIT(u8 div);
    extern void RTC_CLK_INIT(u8 div);
    extern void PMU_CLK_INIT(u8 div);

    u32 lsb_clk =  clock_get_lsb_freq();
    u8 div;

    div = lsb_clk / 24000000L;

    log_printf("lsb_clk:%d div:%d\n", lsb_clk, div);

    P33_CLK_INIT(div);
    RTC_CLK_INIT(div);
    PMU_CLK_INIT(div);

}
/*power init fuction*/
QLZ(.qlz_init)
extern void VCM_DET_EN(u8 en);
void power_init_app(u8 mode, u8 chargeV)
{
    set_33v_clk_div();

    set_lvd_mode(1, 4);

    VCM_DET_EN(1);

    extern u8 READ_PMU_RESET_SOURCE(void);
    log_printf("pmu reset : %x\n", READ_PMU_RESET_SOURCE());
    //set enter lowpower mode  0:not enter lowpower 1:powerdown 2:poweroff
    set_lowpower_mode_config(BT_LOW_POWER_MODE);
    //set wheather can enter lowpower callback
    set_wheather_can_enter_lowpower_callback(wheather_can_enter_lowpower);

    //set lowepower osc info
    set_lowpower_osc(LOWPOWER_OSC_TYPE, LOWPOWER_OSC_HZ);
    //set lowepower delay arg
    set_lowpower_delay_arg(SYS_Hz / 1000000L);
    //set wheather keep 32k osc when enter softpoweroff
#if (LOWPOWER_OSC_TYPE == RTC_OSCL)
    set_lowpower_keep_32K_osc_flag(1);
#else
    set_lowpower_keep_32K_osc_flag(0);
#endif
    //设置进入powerdown的时候vddio和rtcvdd降到多少伏
    //0(3.4v)  1(3.2v) 2(3.0v) 3(2.8v) 4(2.6v) 5(2.4v) 6(2.2v) 7(2.0v)
    //wdvdd lev
    //0(0.728v)  1(0.696v) 2(0.648v) 3(0.605v) 4(0.571v) 5(0.537v) 6(0.501v) 7(0.467v)
    //wvbg cur
    //0 - 3 0:min ... 3:max
    set_lowpower_pd_ldo_level(0x08, 0x08, 0x07, 0x01);
    pwr_setup_init();
    //set wheather disable btosc when enter lowpower
    set_lowpower_btosc_dis(0);
    //set io status when enter lowpower
    set_lowpower_io_status_set_callback(low_pwr_deal);
    //set lowpower wakeup io callback
    set_lowpower_wakeup_io_callback(set_poweroff_wakeup_io, set_sleep_mode_wakeup_io);

#if (LOWPOWER_OSC_TYPE == LRC_32K)
    set_get_osc_callback(get_powerdown_osc_info);
#else
    set_get_osc_callback(NULL);
#endif

    //init power
    set_sys_pwrmd(mode);

#if ((SDMMC0_EN == 1) || (SDMMC1_EN == 1) || (USB_DISK_EN == 1))
    set_main_ldo_en(1);
#endif

    /*
     * set sys ldo
     * vddio:0(3.4v),1(3.2v),2(3.0v),3(2.8v),4(2.6v),5(2.4v),6(2.2v),7(2.0v)
    */
    set_sys_ldo_level(SYS_VDDIO_LEVEL, SYS_RTCVDD_LEVEL);
    bt_power_reset_dual_mode(rf_reset_dual_mode);
#if BT_LOW_POWER_MODE

#if(BLE_BREDR_MODE&BT_BREDR_EN)
    edr_power_manage_struct_init(get_power_manage_op_str());
#endif

#if(BLE_BREDR_MODE&BT_BLE_EN)
    ble_power_manage_struct_init(get_power_manage_op_str());
#endif

#endif
    LDO_BT_value = get_ldo_bt();
    log_printf("**************ldo_ref = %d**********\n", LDO_BT_value);
}


/*----------------------------------------------------------------------------*/
/**@brief   获取电池电量
   @param   void
   @param   void
   @return  电池电量值
   @note    tu8 get_battery_level(void)
*/
/*----------------------------------------------------------------------------*/
u16 LDO_bt = 0x174;

u16 get_battery_level(void)
{
    u16 battery_value, LDOIN_12;

    LDOIN_12 = adc_value[R_AD_CH_VBAT];
    LDO_bt  = adc_value[R_AD_CH_BT];//0x181,1.2v
    ////注意：vddio和rtc_vdd的配置影响电压检测
    battery_value = (((u16)LDOIN_12 * LDO_BT_value * 4)) / ((u16)LDO_bt) / 10;  //针对AC69
    /* log_printf("battery_value:%d   AD_bat = %x  AD_bt:%x\n", battery_value,LDOIN_12, LDO_bt); */
    return battery_value;
}

#if SYS_LVD_EN

#define LOW_POWEROFF_VALUE   330  //低电关机电压
#define POWER_CHECK_CNT      100  //POWER_CHECK_CNT*10ms
u16 unit_cnt = 0;                 //计数单位时间POWER_CHECK_CNT*10ms
u16 low_warn_cnt = 0;             //单位时间内监测到报警电压次数
u16 low_off_cnt = 0;              //单位时间内监测到关机电压次数
u16 low_pwr_cnt = 0;
u16 normal_pwr_cnt = 0;
static u8 low_voice_cnt = 0;
static u8 low_power_cnt = 0;
volatile u8 low_power_flag = 0;
u32 power_on_cnt = 500;
u16 low_power_off_value = LOW_POWEROFF_VALUE;
u16 low_warn_value  = (LOW_POWEROFF_VALUE + 10);


/*----------------------------------------------------------------------------*/
/**@brief  pwr config
   @param  default_level=1:reset to default pwr output
   		   default_level=0:reduce pwr output
   @return void
   @note
*/
/*----------------------------------------------------------------------------*/
/*3.53v-3.34v-3.18v-3.04v-2.87v-2.73v-2.62v-2.52v*/
//extern void set_sys_ldo_level(u8 level);

volatile u8 low_power_set = 0;
u8 get_pwr_config_flag()
{
    return low_power_set;
}

#define AUDLDO_VSEL(x)              SFR(JL_AUDIO->ADA_CON0, 23, 3, x)
void pwr_level_config(u8 default_level)
{
#if LOW_POWER_NOISE_DEAL
    //default config
    if (default_level && low_power_set) {
        puts(">>>>>>>>>Normal_power\n");
        low_power_set = 0;
        AUDLDO_VSEL(6);
        set_sys_ldo_level(SYS_VDDIO_LEVEL, SYS_RTCVDD_LEVEL);	//level:0~7
    } else if (!low_power_set && !default_level) {
        puts(">>>>>>>>>Lower_power\n");
        low_power_set = 1;
        AUDLDO_VSEL(5);
        set_sys_ldo_level(SYS_LDO_REDUCE_LEVEL, SYS_LDO_REDUCE_LEVEL);	//level:0~7
    }
#endif
}

struct _LOW_PWR_SETUP {
    u16 crc;
    u8 low_power_off_value;			/*pwroff voltage*/
    u8 low_warn_value;			/*warn voltage	*/
} _GNU_PACKED_;
typedef struct _LOW_PWR_SETUP LOW_PWR_SETUP;
LOW_PWR_SETUP *low_pwr_setup;
/**
  *	Read low_pwr voltage value from CFG Zone
  */
void lowpwr_setup_init(void)
{
    u8 idx = 0;
    u16 crc = 0;
    u32 setup_base = args[1];
    low_pwr_setup = (LOW_PWR_SETUP *)((u32)&low_pwr_base - setup_base);
    low_power_off_value = LOW_POWEROFF_VALUE;
    low_warn_value  = (LOW_POWEROFF_VALUE + 10);
    put_buf((u8 *)low_pwr_setup, sizeof(PWR_SETUP) * 4);
    while (idx < 4) {
        crc = crc16((void *)((u8 *)&low_pwr_setup[idx].crc + 2), 2);
        if (low_pwr_setup[idx].crc == crc) {
            log_printf("idx:%d\n", idx);
            log_printf("low_power_off_value:%d\n", low_pwr_setup[idx].low_power_off_value);
            log_printf("low_warn_value:%d\n", low_pwr_setup[idx].low_warn_value);
            if (low_pwr_setup[idx].low_warn_value >= 30 && low_pwr_setup[idx].low_warn_value <= 40) {
                low_warn_value = (low_pwr_setup[idx].low_warn_value * 10);
            }
            if (low_pwr_setup[idx].low_power_off_value >= 30 && low_pwr_setup[idx].low_power_off_value <= 40) {
                low_power_off_value = (low_pwr_setup[idx].low_power_off_value * 10);
            }
            break;
        }
        idx++;
    }
}

volatile u8 low_poweroff_flag = 0;	//低电关机标志
u8 lowpower_warning_flag = 0;
u16 lowpower_warning_cycle = 0;
u16 lowpower_warn_cnt = 500;
u16 lowpower_cnt = 500;
u16 normol_power_cnt = 0;
#define LOWPOWER_WARNING_CYCLE		1000	//1000*10ms=10s
#define NORMOL_POWER_TIME			50		//50*10ms=0.5s
void battery_check(void)
{
    u16 val;
    u8 cnt;
#if USE_AD_CHECK_VOLTAGE
	val = voltage_value;
#else
    val = 400;//get_battery_level();
#endif

    if (power_on_cnt > 0) {
        power_on_cnt--;
        return;
    }
#if 1
#if WARNING_LOWPOWER
	if(2 == lowpower_warning_flag)
	{
		lowpower_warning_cycle++;
		if(lowpower_warning_cycle > LOWPOWER_WARNING_CYCLE)
		{   //1000*10ms = 10s提示一次低电
			lowpower_warning_cycle = 0;
			lowpower_warning_flag = 2;
            task_post_msg(NULL,1,MSG_PROMPT_PLAY);
		}
		if(val > LOWPOWER_WARNING_VOLTAGE)
		{
			normol_power_cnt++;
			if (normol_power_cnt > NORMOL_POWER_TIME)
			{
				lowpower_warning_flag = 0;
				low_power_flag = 0;
				low_poweroff_flag = 0;
				lowpower_warn_cnt = 500;
				lowpower_cnt = 500;
				
				normol_power_cnt = 0;
			}
			//else
			{
				//normol_power_cnt = 0;
			}
		}
	}
	//低电预报判断
	if(!lowpower_warning_flag)
	{
		if(val <= LOWPOWER_WARNING_VOLTAGE)
		{
			if(lowpower_warn_cnt > 0)
			{
				lowpower_warn_cnt--;
			}
			else
			{
				low_power_flag = 1;
				lowpower_warning_flag = 2;
            	task_post_msg(NULL,1,MSG_PROMPT_PLAY);
			}
		}
		else
		{
			low_power_flag = 0;
			low_poweroff_flag = 0;
			lowpower_warn_cnt = 500;
			lowpower_cnt = 500;          //lowpower_cnt*10ms
		}
	}
	else
#endif
	{
		if(val <= LOWPOWER_VOLTAGE)
		{
			if(lowpower_cnt > 0)
			{
				lowpower_cnt--;
			}
			else
			{
				low_power_flag = 1;
				low_poweroff_flag = 1;
				task_post_msg(NULL,1,MSG_LOW_POWER);
			}
		}
		else
		{
			//low_poweroff_flag = 0;
			lowpower_warn_cnt = 500;
			lowpower_cnt = 500;          //500*10ms
		}
	}
#else
    unit_cnt++;

    if (val < low_power_off_value) {
        low_off_cnt++;
    }
    if (val <= low_warn_value) {
        low_warn_cnt++;
    }

    if (val <= 340) {
        low_pwr_cnt++;
    }

    if (unit_cnt >= POWER_CHECK_CNT) {
        cnt = 10;
        /* if (is_sniff_mode()) { */
        /* cnt = 4; */
        /* } */
        /* [> else if(()) <] */
        /* [> cnt = 4; <] */
        /* else { */
        /* cnt = 10; */
        /* } */

        if (low_off_cnt > POWER_CHECK_CNT / 2) { //低电关机
            low_power_flag = 1;
            low_power_cnt++;
            if (low_power_cnt > 6) {
                puts("\n*******Low Power********\n");
                low_power_cnt = 0;
                task_post_msg(NULL, 1, MSG_LOW_POWER);
            }
        } else if (low_warn_cnt > POWER_CHECK_CNT / 2) { //低电提醒
            low_power_flag = 1;
            low_voice_cnt ++;
            if (low_voice_cnt > cnt) {
                puts("\n**Low Power,Please Charge Soon!!!**\n");
                low_voice_cnt = 0;
                task_post_msg(NULL, 1, MSG_LOW_POWER_VOICE);
            }
        } else {
            if (low_power_flag) {
                lower_power_led_flash(1, 0);
                /* R_LED_OFF(); */
                /* B_LED_OFF(); */
            }
            low_power_flag = 0;
        }

#if	LOW_POWER_NOISE_DEAL
        if (low_pwr_cnt > POWER_CHECK_CNT / 2) {
            pwr_level_config(0);
            normal_pwr_cnt = 0;
        } else {
            if (val > 340) {
                if (normal_pwr_cnt++ > 5) { //make sure battery full enough
                    normal_pwr_cnt = 0;
                    pwr_level_config(1);
                }
            } else {
                normal_pwr_cnt = 0;
            }
        }
#endif

        unit_cnt = 0;
        low_off_cnt = 0;
        low_warn_cnt = 0;
    }
#endif
}

LOOP_DETECT_REGISTER(battery_detect) = {
    .time = 5,
    .fun  = battery_check,
};

#endif

#if POWER_EXTERN_DETECT_EN

volatile u8 low_power_external_flag = 0;
u16 power_external_value = 0;

void power_external_detect_io_init(void)
{
#if 0
    AD_POWER_EXTERN_IO_PORT->PU  &= ~AD_POWER_EXTERN_IO_BIT;
    AD_POWER_EXTERN_IO_PORT->PD  &= ~AD_POWER_EXTERN_IO_BIT;
    AD_POWER_EXTERN_IO_PORT->DIR |=  AD_POWER_EXTERN_IO_BIT;
    AD_POWER_EXTERN_IO_PORT->DIE &= ~AD_POWER_EXTERN_IO_BIT;
#else
    PORTR_PU(AD_POWER_EXTERN_IO_PORT, 0);
    PORTR_PD(AD_POWER_EXTERN_IO_PORT, 0);
    PORTR_DIR(AD_POWER_EXTERN_IO_PORT, 1);
    PORTR_DIE(AD_POWER_EXTERN_IO_PORT, 1);
    PORTR2_ADCEN_CTL(1);
#endif
}

void power_external_value_check(void)
{
    u16 tmp = 0;
    u16 LDOIN_12 = 0, LDO_ref = 0;
    static u8 low_power_cnt = 0;
    static u8 normal_power_cnt = 0;
    static u32 sum = 0;
    static u8 cnt = 0;

    LDOIN_12 = adc_value[R_AD_CH_EXTERN_POWER];
    LDO_bt  = adc_value[R_AD_CH_BT];

    tmp = (LDOIN_12 * LDO_BT_value * 10) / ((u16)LDO_bt) / 10;	//这里需要根据实际硬件电路的分压电阻进行修改，现在默认是十分之一分压
//	printf("\ntmp=%d\n",tmp);
    cnt++;
    sum += tmp;
    if (cnt == 5) {
        power_external_value = sum / 5;
//        printf("\nvoltage_12v_value=%d\n",power_external_value);
        sum = 0;
        cnt = 0;
        if (power_external_value % 10 > 5) {
            power_external_value = power_external_value / 10 + 1;
        } else {
            power_external_value = power_external_value / 10;
        }
//         printf("power_extern_value: %d\n",power_external_value);

    }
//    printf("power_extern_value: 0x%x    0x%x   %d\n",LDOIN_12,LDO_ref,power_external_value);

    if (power_external_value < 120) { //低于12V
        normal_power_cnt = 0;
        low_power_cnt++;
        if (low_power_cnt >= 50) {
            low_power_cnt = 0;
            low_power_external_flag = 1;
        }
    } else {
        low_power_cnt = 0;
        normal_power_cnt++;
        if (normal_power_cnt >= 50) {
            normal_power_cnt = 0;
            low_power_external_flag = 0;
        }

    }

}

u16 get_power_external_value(void)
{
    return power_external_value;
}

u8 get_low_power_external_flag(void)
{
    return low_power_external_flag;
}

no_sequence_initcall(power_external_detect_io_init);

LOOP_DETECT_REGISTER(power_external_detect_loop) = {
    .time = 5,
    .fun  = power_external_value_check,
};

#else
u16 get_power_external_value(void)
{
    return 0;
}

u8 get_low_power_external_flag(void)
{
    return 0;
}

#endif

#if USE_AD_CHECK_VOLTAGE
void ad_check_voltage_io_init(void)
{
#if 1
    AD_CHECK_VOLTAGE_IO_PORT->PU  &= ~AD_CHECK_VOLTAGE_IO_BIT;
    AD_CHECK_VOLTAGE_IO_PORT->PD  &= ~AD_CHECK_VOLTAGE_IO_BIT;
    AD_CHECK_VOLTAGE_IO_PORT->DIR |=  AD_CHECK_VOLTAGE_IO_BIT;
    AD_CHECK_VOLTAGE_IO_PORT->DIE &= ~AD_CHECK_VOLTAGE_IO_BIT;
#else
    PORTR_PU(AD_CHECK_VOLTAGE_IO_PORT, 0);
    PORTR_PD(AD_CHECK_VOLTAGE_IO_PORT, 0);
    PORTR_DIR(AD_CHECK_VOLTAGE_IO_PORT, 1);
    PORTR_DIE(AD_CHECK_VOLTAGE_IO_PORT, 1);
    PORTR2_ADCEN_CTL(1);
#endif
}

u16 voltage_value=0;
void ad_check_voltage(void)
{
    u16 tmp = 0;
    u16 LDOIN_12 = 0;
    static u32 sum = 0;
    static u8 cnt = 0;

    LDOIN_12 = adc_value[R_AD_CH_CHECK_VOLTAGE];
    LDO_bt  = adc_value[R_AD_CH_BT];

    tmp = (LDOIN_12 * LDO_BT_value * 10) / ((u16)LDO_bt) / 10;	//这里需要根据实际硬件电路的分压电阻进行修改，现在默认是十分之一分压
    cnt++;
    sum += tmp;
    if (cnt == 5)
	{
        voltage_value = sum / 5;
        sum = 0;
        cnt = 0;
        if (voltage_value % 10 > 5)
		{
            voltage_value = voltage_value / 10 + 1;
        }
		else
		{
            voltage_value = voltage_value / 10;
        }
    }
}

no_sequence_initcall(ad_check_voltage_io_init);

LOOP_DETECT_REGISTER(ad_check_voltage_detect) = {
    .time = 5,
    .fun  = ad_check_voltage,
};
#endif

#if USE_SHOW_BAT//DCIN_DECT_IO && USE_SHOW_BAT
#define BAT_LEVEL_3				205//0xEF		//根据实际分压值填写
#define BAT_LEVEL_2				170//0xEF		//根据实际分压值填写
#define BAT_LEVEL_1				160//0xEF		//根据实际分压值填写

volatile u8 bat_level=0xFF;
volatile u8 bat_cnt=60;
void display_battery_value(void)
{
	u8 bat_val_temp;
	u16 voltage_value_temp;
#if USE_AD_CHECK_VOLTAGE
	voltage_value_temp = voltage_value;
#else
	voltage_value_temp = 400;//get_battery_level();
#endif

	if (voltage_value_temp >= BAT_LEVEL_3)
		bat_val_temp = battery_full;
	else if (voltage_value_temp >= BAT_LEVEL_2)
		bat_val_temp = battery_1;
	else
		bat_val_temp = battery_low;

	if (bat_level == bat_val_temp)
		bat_cnt = 50;
	else
	{
		if (bat_cnt > 0)
		{
			bat_cnt--;
		}
		else
		{
			bat_cnt = 50;
			bat_level = bat_val_temp;
		}
	}
}

LOOP_DETECT_REGISTER(display_battery_value_detect) = {
    .time = 50,
    .fun  = display_battery_value,
};
#endif

#if USE_AD_TUNER_VOLUME
void ad_tuner_volume_io_init(void)
{
#if 1
    AD_TUNER_VOL_IO_PORT->PU  &= ~AD_TUNER_VOL_IO_BIT;
    AD_TUNER_VOL_IO_PORT->PD  &= ~AD_TUNER_VOL_IO_BIT;
    AD_TUNER_VOL_IO_PORT->DIR |=  AD_TUNER_VOL_IO_BIT;
    AD_TUNER_VOL_IO_PORT->DIE &= ~AD_TUNER_VOL_IO_BIT;
#else
    PORTR_PU(AD_TUNER_VOL_IO_PORT, 0);
    PORTR_PD(AD_TUNER_VOL_IO_PORT, 0);
    PORTR_DIR(AD_TUNER_VOL_IO_PORT, 1);
    PORTR_DIE(AD_TUNER_VOL_IO_PORT, 1);
    PORTR2_ADCEN_CTL(1);
#endif
}

const u16 volume_value_ad_table[MAX_SYS_VOL_L+1] = {
	10,20,30,40,50,60,70,80,90,100,
	110,120,130,140,150,160,170,180,190,200,
	210,220,230,240,250,260,270,280,290,300,
	350,
};
u8 volume_chang_enable=0;
u8 volume_value=0;
u8 flag_100ms = 0;
void ad_tuner_volume(void)
{
    u16 tmp = 0;
    u16 LDOIN_12 = 0;
	u8 i,volume_value_temp=0,tuner_change_flag=0;
	static u8 cnt2=0;
    static u16 tuner_voltage_value_temp=0,last_voltage_value_temp=0;

    LDOIN_12 = adc_value[R_AD_CH_TUNER_VOLUME];
    LDO_bt  = adc_value[R_AD_CH_BT];

    tmp = (LDOIN_12 * LDO_BT_value * 10) / ((u16)LDO_bt) / 10;	//这里需要根据实际硬件电路的分压电阻进行修改，现在默认是十分之一分压

    if (tmp % 10 > 5)
	{
        tmp = tmp / 10 + 1;
    }
	else
	{
        tmp = tmp / 10;
    }

	if (tmp >= (tuner_voltage_value_temp + 6))
	{
		for (i=0;i<(MAX_SYS_VOL_L+1);i++)
		{
			if (tmp < volume_value_ad_table[i])
			{
				volume_value_temp = i;
				tuner_change_flag = true;
				break;
			}
		}
	}
	else if ((tmp + 6) <= tuner_voltage_value_temp)
	{
		for (i=0;i<(MAX_SYS_VOL_L+1);i++)
		{
			if (tmp < volume_value_ad_table[i])
			{
				volume_value_temp = i;
				tuner_change_flag = true;
				break;
			}
		}
	}

	if (tuner_change_flag == 0)
	{
    	cnt2 = 0;
	}
	else
    {
    	if (volume_value == volume_value_temp)
    	{
    		cnt2 = 0;
    	}
    	else
    	{
    		cnt2++;
    		if (cnt2 >= 10)
    		{
    			cnt2 = 0;
    			volume_chang_enable = 1;
    			volume_value = volume_value_temp;
    			tuner_voltage_value_temp = tmp;
				printf("~~~tuner_voltage_value_temp = %d\n",tuner_voltage_value_temp);
    		}
    	}
    }
}

no_sequence_initcall(ad_tuner_volume_io_init);

LOOP_DETECT_REGISTER(ad_tuner_volume_detect) = {
    .time = 5,
    .fun  = ad_tuner_volume,
};
#endif

extern void ad_key_init(void);
extern void io_key_init(void);

#define ADC10_17   (0x3ffL*240 /(240  + 220))     //24K
#define ADC10_13   (0x3ffL*150 /(150  + 220))     //15K
#define ADC10_10   (0x3ffL*91  /(91   + 220))     //9.1K


#define C_DCIN_CHECK_CNT			50
static void check_power_on_voltage(void)
{
    u16 val = 0;
    u8 tmp;

    u8 low_power_cnt = 0;
    u32 normal_power_cnt = 0, dcin_power_cnt = 0, display_test_cnt = 0;
    u32 delay_2ms_cnt = 0;

#if 1//(SOFT_POWER_ON_OFF_INSIDE || SOFT_POWER_ON_OFF)
    //LED_INIT_EN();
#if 1//DCIN_DECT_IO
	clr_PINR1_ctl();
	DCIN_INIT();
#endif
    ad_key_init();
    io_key_init();

    while (1) {
        clear_wdt();
        delay(10000);
        delay_2ms_cnt++;
        adc_scan(NULL);
        if ((delay_2ms_cnt % 5) == 0) {
            delay_2ms_cnt = 0;
	        POWER_KEY_INIT();
            /*battery check*/
#if 0//SYS_LVD_EN
        #if USE_AD_CHECK_VOLTAGE
            ad_check_voltage();
            val = voltage_value;
        #else
            val = get_battery_level();
        #endif
            if (val < low_power_off_value) {
                low_power_cnt++;
                if (low_power_cnt > 50) {
                    B_LED_OFF();
                    R_LED_ON();
                    delay(20000);
					set_lowpower_keep_32K_osc_flag(1);
				#if SOFT_POWER_ON_OFF
					while(1)
					{
			            SOFT_POWER_CTL_OFF();
						clear_wdt();
					}
				#endif
                    enter_sys_soft_poweroff();
                }
            } else
#endif
        #if 1
            {
                val = adc_value[R_AD_CH_KEY];
    		    if ((((ADC10_10 + ADC10_13)/2) < val)&&(val < ((ADC10_13 + ADC10_17)/2)))
    		    {
    		        display_test_cnt++;
    				if (display_test_cnt > 20)
    				{
    				    display_test_flag = 1;
    					break;
    				}
    		    }
    			else
    			{
    			#if 1//DCIN_DECT_IO
    				if (IS_POWER_KEY_DOWN())//(IS_DCIN())
    				{
    					puts("~~~~~\n");
    					dcin_power_cnt++;
    					if (dcin_power_cnt > 400)//(dcin_power_cnt > C_DCIN_CHECK_CNT)
    					{
    						//dcin_status = true;
    						//is_dcin_poweron = 1;
    						break;
    					}
    				}
    				else
    			//#endif
    			    {
        			    normal_power_cnt++;
        				if (normal_power_cnt > 10)
        				{
                            B_LED_OFF();
                            R_LED_ON();
                            delay(20000);
        					set_lowpower_keep_32K_osc_flag(1);
        				#if SOFT_POWER_ON_OFF
        					while(1)
        					{
        			            SOFT_POWER_CTL_OFF();
        						clear_wdt();
        					}
        				#endif
                            enter_sys_soft_poweroff();
        					break;
        				}
        			}
        		#else
    			    {
        			    normal_power_cnt++;
        				if (normal_power_cnt > 50)
        				{
        					break;
        				}
        			}
    			#endif
			    }
		    }
		#else
            {
			#if DCIN_DECT_IO
				if (IS_DCIN())
				{
					puts("~~~~~\n");
					dcin_power_cnt++;
					if (dcin_power_cnt > C_DCIN_CHECK_CNT)
					{
						dcin_status = true;
						is_dcin_poweron = 1;
						break;
					}
				}
				else
			#endif
	            {
	                low_power_cnt++;
	                normal_power_cnt++;
	                if (normal_power_cnt > 50) { //normal power
	                    delay_2ms_cnt = 0;
	                    POWER_KEY_INIT();
	                    while (1) {
	                        clear_wdt();
	                        delay(30000);

	                        if (IS_POWER_KEY_DOWN()) {
	                            putchar('+');
	                            delay_2ms_cnt++;
	                            if (delay_2ms_cnt > 400) {
	                                R_LED_OFF();
	                                B_LED_ON();
								#if SOFT_POWER_ON_OFF
									SOFT_POWER_CTL_ON();
								#endif
	                                return;
	                            }
	                        } else {
	                            putchar('-');
	                            delay_2ms_cnt = 0;
								set_lowpower_keep_32K_osc_flag(1);
							#if SOFT_POWER_ON_OFF
								while(1)
								{
						            SOFT_POWER_CTL_OFF();
									clear_wdt();
								}
							#endif
	                            enter_sys_soft_poweroff();
	                        }
	                    }
	                }
				}
            }
        #endif
        }
    }
#endif
}


extern u8 is_rtc_wakeup;
void power_on_detect_deal(void)
{
#if RTC_CLK_EN
	if (is_rtc_wakeup == RTC_WAKEUP_VALUE)
	{
		//led_mode_on();
	}
	else
#endif
	{
    	check_power_on_voltage();
	}
}

void wakeup_pending_deal(void)
{
    u8 reg = 0;
    u8 almen = 0;

    reg = get_wake_up_type();

    if (reg & BIT(0)) {
        printf("--PCNT_OVF--\n");
        RTC_SFR_SET(RTC_CON_ADDR11, 6, 1, 1);
    }
    if (reg & BIT(1)) {
        printf("--EDGE_WKUP--\n");
        if (check_io_wakeup_pend()) {               //PR PORT
            printf("--PR PORT--\n");
        }
    }
    if (reg & BIT(2)) {
        printf("--ALMOUT--\n");
        almen = get_rtc_alarm_en();
        alarm_sw(0);
        alarm_sw(almen);
    }
    if (reg & BIT(3)) {
        printf("--LDO5v_WKUP--\n");
        RTC_SFR_SET(RTC_CON_ADDR07, 6, 1, 1);
    }
    if (reg & BIT(4)) {
        printf("--VDD50_LVD_WKUP--\n");
        void CLR_LVD_PND(void);
        CLR_LVD_PND();
    }
    if (reg & BIT(5)) {
        printf("--RTCVDD_LVD_WKUP--\n");
    }
}

//
extern void resave_almtime(void);
void soft_power_ctl(u8 ctl)
{
    if(PWR_ON == ctl)
    {
    }
    else
    {
		printf("soft off\n");
		AMP_MUTE();
		delay_n10ms(2);
	#if RTC_CLK_EN
		resave_almtime();
        set_lowpower_keep_32K_osc_flag(1);
	#endif
	#if SOFT_POWER_ON_OFF
		while(1)
		{
            SOFT_POWER_CTL_OFF();
			clear_wdt();
		}
	#endif
        enter_sys_soft_poweroff();
    }
}
