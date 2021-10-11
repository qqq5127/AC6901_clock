#ifndef __FMTX_API_H__
#define __FMTX_API_H__

#include "typedef.h"

/*
*********************************************************************
                        FM_Transmitter
*********************************************************************
*/
#define FREQ_MAX  					1080
#define FREQ_MIN					875

#define CLK_BY_CPU_IO               1	//通过IO口输出时钟给发射芯片
#define CLK_BY_CPU_OSC              2	//发射芯片与CPU共用晶振时钟
#define CLK_BY_EXTERN_OSC			3	//外挂晶振

#define FMTX_CHIP_CLK_SOURCE	  CLK_BY_CPU_OSC	//发射芯片时钟来源选择

//时钟选择
#define OSC_12M						1
#define OSC_24M						2

#define FMTX_CHIP_OSC_SELECT	  OSC_24M

#define FMTX_CHIP_SELECT_BY_RES     0  //是否通过外部电阻选择发射芯片

#define PA_OFF_WHEN_NO_AUDIO		0  //一分钟关闭发射,目前适用8027

/*IO口输出时钟时，选择时钟输出引脚*/
#define FMTX_CLK_PORT					JL_PORTA
#define FMTX_CLK_BIT					BIT(15)

/*调频时屏幕闪烁的时间*/
#define UI_FREQ_RETURN				6

typedef enum {
    FREQ_NEXT = 0,     ///<下一频点
    FREQ_PREV,        ///<上一频点
    FREQ_SEL,         ///<指定频点
} FREQ_SEL_MODE;

typedef enum {
    FREQ_TRANSMIT = 0, 	///< 正常模式
    FREQ_SETTING,   	///< 调频模式
} FREQ_STATE;

typedef struct _FMTX_API {
    void (*init)(u16 freq);	//初始化函数
    void (*start)(void);	//开始发射
    void (*stop)(void);		//停止发射
    void (*set_freq)(u16 freq);	//设置发射频点
    void (*set_power)(u8 power, u16 freq);	//设置发射功率
} FMTX_API;

typedef struct _FMTX_VAR {
    u16 freq;
    u16 max_freq;
    u16 min_freq;
    FREQ_STATE state;
    u8 freq_set_total_time;	//进入调频模式的总时间
} FMTX_VAR;


void fmtx_init(void);
void fmtx_setfre(FREQ_SEL_MODE mode, u16 fre);
void fmtx_setpower(u8 power, u16 fre);
void fmtx_start(void);
void fmtx_stop(void);

u16 fmtx_get_freq(void);
FREQ_STATE fmtx_get_state(void);
void fmtx_set_state(FREQ_STATE state);
u8 fmtx_get_whether_to_flash_freq(void);


#endif

