#include "sdk_cfg.h"
#include "wpc_api.h"
#include "common/common.h"
#include "wpc_ui.h"
#include "adc_api.h"
#include "board_init.h"
#include "rtc_api.h"
#include "clock_api.h"
#include "irq_api.h"
//说明：
//1、无线充电主循环,必须是1ms调用一次
//2、无线充电解码循环必须是20us调用一次
//3、无线充电的每个通道的AD值采集,必须是3ms内采集>=5次,如果不满足请修改宏 ADC_DETECT_PERIOD
//4、无线充电库内部代码20us执行一次的都定义到内部RAM了, 其他代码基本定义在 wpc_code 段
//5、请将20us的中断里面的函数都定义到RAM,加快执行速度
//6、每做一款机器,需要修改几个宏定义, VUSB_5V_CUR/VUSB_5V_Q/VUSB_9V_CUR/VUSB_9V_Q,方法如下:
//   (1)、打开无线充电库的debug功能,
//   (2)、5V上电打印log, 例如:fod ping: 110, 132, 将 VUSB_5V_CUR = 110, VUSB_5V_Q = 132.
//   (3)、9V上电打印log, 例如:fod ping: 158, 142, 将 VUSB_9V_CUR = 158, VUSB_9V_Q = 142
//   (4)、完成一款无线充电的模拟ping校准,有助于检测手机靠近还是金属异物靠近!

#if WIRELESS_POWER_EN

//-------------改线圈是修改该值--------------
#define VUSB_5V_CUR     110//80
#define VUSB_9V_CUR     158//168
#define VUSB_5V_Q       132
#define VUSB_9V_Q       142
//-------------------------------------------

#define DECODE_ONE_PORT JL_PORTA
#define DECODE_ONE_BIT  12

#define DECODE_TWO_PORT JL_PORTC
#define DECODE_TWO_BIT  2

#define DM_CTRL_PORT    JL_PORTA
#define DM_CTRL_BIT     10
#define DP_CTRL_PORT    JL_PORTA
#define DP_CTRL_BIT     11

#define PHASE0_OUTCHL   0
#define PHASE0_PORT     JL_PORTA
#define PHASE0_BIT      3
#define PHASE1_OUTCHL   1
#define PHASE1_PORT     JL_PORTA
#define PHASE1_BIT      4

//获取对应通道的AD值
AT_RAM
static u16 wpc_get_ad(ADC_CHL chl)
{
    u16 ad;
    switch (chl) {
    case ADC_CHL_FOD_VOL:
        ad = adc_value[R_AD_CH_FOD];
        adc_value[R_AD_CH_FOD] |= BIT(15);
        break;
    case ADC_CHL_VOLTAGE:
        ad = adc_value[R_AD_CH_VOL];
        adc_value[R_AD_CH_VOL] |= BIT(15);
        break;
    case ADC_CHL_CURRENT:
        ad = adc_value[R_AD_CH_CUR];
        adc_value[R_AD_CH_CUR] |= BIT(15);
        break;
    case ADC_CHL_TEMPERATURE:
        ad = adc_value[R_AD_CH_TMP];
        adc_value[R_AD_CH_TMP] |= BIT(15);
        break;
    default:
        ad = 0;
        break;
    }
    return ad;
}

//获取解码通道的电平
AT_RAM
static u8 wpc_get_lvl(u8 io)
{
    u8 status;
    if (io == 0) {
        status = (DECODE_ONE_PORT->IN & BIT(DECODE_ONE_BIT)) >> DECODE_ONE_BIT;
    } else {
        status = (DECODE_TWO_PORT->IN & BIT(DECODE_TWO_BIT)) >> DECODE_TWO_BIT;
    }
    return status;
}

//QC充电器切到9V工作
AT_WPC
static void wpc_qc_to_9v(u8 step)
{
    switch (step) {
    case 0://初始化io为输入状态
        DP_CTRL_PORT->DIR |=  BIT(DP_CTRL_BIT);//dp
        DP_CTRL_PORT->PD  &= ~BIT(DP_CTRL_BIT);
        DP_CTRL_PORT->PU  &= ~BIT(DP_CTRL_BIT);
        DM_CTRL_PORT->DIR |=  BIT(DM_CTRL_BIT);//dm
        DM_CTRL_PORT->PD  &= ~BIT(DM_CTRL_BIT);
        DM_CTRL_PORT->PU  &= ~BIT(DM_CTRL_BIT);
        break;
    case 1://10ms后设置dp output 1
        DP_CTRL_PORT->OUT |=  BIT(DP_CTRL_BIT);
        DP_CTRL_PORT->DIR &= ~BIT(DP_CTRL_BIT);
        break;
    case 2://延时1.25s以上设置dm output 1
        DM_CTRL_PORT->OUT |=  BIT(DM_CTRL_BIT);
        DM_CTRL_PORT->DIR &= ~BIT(DM_CTRL_BIT);
        break;
    default:
        break;
    }
}

//PWM使能输出或者禁能输出时IO设置函数
AT_WPC
static void wpc_pwm_out(u8 en)
{
    if (en) {
        PHASE1_PORT->DIR &= ~BIT(PHASE1_BIT);
        PHASE1_PORT->PU  |=  BIT(PHASE1_BIT);
        PHASE1_PORT->PD  |=  BIT(PHASE1_BIT);
        PHASE1_PORT->OUT &= ~BIT(PHASE1_BIT);
        PHASE1_PORT->DIE &= ~BIT(PHASE1_BIT);//输出output channel 0
    } else {
        PHASE1_PORT->DIR &= ~BIT(PHASE1_BIT);
        PHASE1_PORT->PU  &= ~BIT(PHASE1_BIT);
        PHASE1_PORT->PD  &= ~BIT(PHASE1_BIT);
        PHASE1_PORT->DIE &= ~BIT(PHASE1_BIT);
        PHASE1_PORT->OUT &= ~BIT(PHASE1_BIT);
    }
}

//20us调用一次无线充电解码循环
AT_RAM
static void wpc_us_run(void)
{
    wpc_api_us_do(20);
}

USLOOP_DETECT_REGISTER(us_run) = {
    .time = 20,//20us
    .fun  = wpc_us_run,
};

//1ms调用一次无线充电主循环
AT_WPC
static void wpc_ms_run(void)
{
    wpc_api_ms_do(1);
}

LOOP_DETECT_REGISTER(ms_run) = {
    .time = 1,
    .fun  = wpc_ms_run,
};
/**spdif 也会使用这个tick模块。注意无线充不能跟spdif同时使用***/
static u32 t0_us_cnt1 = 0;
AT_RAM
static void wpc_tick_timer_isr_fun()
{
    struct loop_detect_handler *detect;
    JL_TICK->CON |= BIT(6);
    t0_us_cnt1 += 20;
    if (t0_us_cnt1 % 20) {
        t0_us_cnt1 = 0;
    }
    list_for_each_usloop_detect(detect) {
        if ((t0_us_cnt1 % detect->time) == 0) {
            if (detect->fun) {
                detect->fun();
            }
        }
    }
}
IRQ_REGISTER(IRQ_TICK_TMR_IDX, wpc_tick_timer_isr_fun);
static void wpc_tick_timer_init()
{
    /**公式： 1s/clock_get_sys_freq()*count = 20Us  **/
    IRQ_REQUEST(IRQ_TICK_TMR_IDX, wpc_tick_timer_isr_fun);
    JL_TICK->CNT   = 0;
    JL_TICK->PRD   = (clock_get_sys_freq() / 500000)  ;
    JL_TICK->CON  = BIT(0);
    puts("---tick_timer_init\n");
}

AT_WPC
void wpc_init(void)
{
    WPC_CFG cfg;
//V_COMM
    DECODE_ONE_PORT->DIR |=  BIT(DECODE_ONE_BIT);
    DECODE_ONE_PORT->PU  &= ~BIT(DECODE_ONE_BIT);
    DECODE_ONE_PORT->PD  &= ~BIT(DECODE_ONE_BIT);
    DECODE_ONE_PORT->DIE |=  BIT(DECODE_ONE_BIT);
//I_COMM
    DECODE_TWO_PORT->DIR |=  BIT(DECODE_TWO_BIT);
    DECODE_TWO_PORT->PU  &= ~BIT(DECODE_TWO_BIT);
    DECODE_TWO_PORT->PD  &= ~BIT(DECODE_TWO_BIT);
    DECODE_TWO_PORT->DIE |=  BIT(DECODE_TWO_BIT);

//PHASE0 -- 该相位是靠近电容边上的
    PHASE0_PORT->DIR &= ~BIT(PHASE0_BIT);
    PHASE0_PORT->PU  &= ~BIT(PHASE0_BIT);
    PHASE0_PORT->PD  &= ~BIT(PHASE0_BIT);
    PHASE0_PORT->DIE &= ~BIT(PHASE0_BIT);
    PHASE0_PORT->OUT &= ~BIT(PHASE0_BIT);
//PHASE1 -- 该相位是靠近线圈端的
    PHASE1_PORT->DIR &= ~BIT(PHASE1_BIT);
    PHASE1_PORT->PU  &= ~BIT(PHASE1_BIT);
    PHASE1_PORT->PD  &= ~BIT(PHASE1_BIT);
    PHASE1_PORT->DIE &= ~BIT(PHASE1_BIT);
    PHASE1_PORT->OUT &= ~BIT(PHASE1_BIT);
    SFR(JL_IOMAP->CON1, 8, 4, 0x0B);//select ch1_pwm_l

//PWM_EN 输出高电平使 LP1111工作
    PORTR_DIR(1, 0);
    PORTR_OUT(1, 1);

    cfg.ad_priod = ADC_DETECT_PERIOD;
    cfg.get_ad = wpc_get_ad;
    cfg.qc_to_9v = wpc_qc_to_9v;
    cfg.ui_set = ui_set;
    cfg.get_lvl = wpc_get_lvl;
    cfg.cur_5v = VUSB_5V_CUR;
    cfg.cur_9v = VUSB_9V_CUR;
    cfg.q_5v = VUSB_5V_Q;
    cfg.q_9v = VUSB_9V_Q;
    cfg.debug = 1;
    cfg.phase0_sel = USE_PWM0L;
    cfg.phase1_sel = USE_PWM1L;
    cfg.phase0_io  = PHASE0_OUTCHL;
    cfg.phase1_io  = PHASE1_OUTCHL;;
    cfg.pwm_out = wpc_pwm_out;
    wpc_led_struct_init();
    wpc_api_init(&cfg);
    wpc_tick_timer_init();
}
no_sequence_initcall(wpc_init);

//机器进入休眠时调用
AT_WPC
void wpc_uninit(void)
{
//PWM_EN 输出低电平使 LP1111不工作
    PORTR_DIR(1, 0);
    PORTR_OUT(1, 0);
}
#endif

