#ifndef __WPC_API_H_
#define __WPC_API_H_

#include "typedef.h"

typedef enum {
    UI_CMD_POWER_ON,//上电
    UI_CMD_STANDBY,//进入待机模式
    UI_CMD_CHARGEING,//充电中
    UI_CMD_CHARGE_FULL,//充满电
    UI_CMD_FOD,//检测到金属异物
    UI_CMD_EXCEPTION,//发生异常
    UI_CMD_OVER_VOLTAGE,//过电压
    UI_CMD_OVER_CURRENT,//过电流
    UI_CMD_OVER_TEMPERATURE,//过温度
} UI_CMD;

typedef enum {
    ADC_CHL_FOD_VOL,//采集FOD VOL电压AD
    ADC_CHL_VOLTAGE,//采集电源电压AD
    ADC_CHL_CURRENT,//采集电流AD
    ADC_CHL_TEMPERATURE,//采集温度AD
} ADC_CHL;

//选择对应的通道输出,根据硬件画板修改
typedef enum {
    USE_PWM0H,
    USE_PWM0L,
    USE_PWM1H,
    USE_PWM1L,
    USE_PWM2H,
    USE_PWM2L,
} PWMCH_SEL;

typedef struct _wpc_cfg {
    void (*ui_set)(UI_CMD cmd);//设置UI
    void (*qc_to_9v)(u8 step);//qc充电器进入9V模式
    u16(*get_ad)(ADC_CHL channel);  //获取对应通道的AD值
    u8(*get_lvl)(u8 channel);   //读取通信信号的电平
    void (*pwm_out)(u8 en);//PWM输出或禁止
//-----配置这四个值的时候请把debug打开--------------------
    u32  cur_5v: 16; //5V电压供电时,模拟ping的电流值
    u32  cur_9v: 16; //9V电压供电时,模拟ping的电流值
    u32  q_5v: 16; //5V电压供电时,Q值
    u32  q_9v: 16; //9V电压供电时,Q值
//--------------------------------------------------------
    u32  debug: 1; //调试,用于协助配置cur_5v/9v q_5v/9v
    u32  ad_priod: 8; //ad执行的周期 us

    u32  phase0_sel: 3; //PWMCH_SEL
    u32  phase0_io: 1; //是否使用output channle
    u32  phase1_sel: 3; //PWMCH_SEL
    u32  phase1_io: 1; //是否使用output channel
} WPC_CFG;

void wpc_api_ms_do(u8 ms);//无线充电主循环,必须1ms调用一次
void wpc_api_us_do(u8 us);//无线充电解码循环,必须20us调用一次
int wpc_api_init(WPC_CFG *cfg);//无线充电协议初始化函数

#endif
