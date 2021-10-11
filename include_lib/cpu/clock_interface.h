#ifndef _CLOCK_INTERFACE_H_
#define _CLOCK_INTERFACE_H_

#include "typedef.h"
//#include "sdk_cfg.h"
//此头文件为apps 依赖接口文件，不能依赖driver.h

#define RTC_OSC_Hz      32768L
#define RC_Hz           250000L
#define SPI_MAX_CLK	    100000000L	//<-SPI最大支持频率



typedef enum {
    ///原生时钟源作系统时钟源
    SYS_CLOCK_INPUT_RC,
    SYS_CLOCK_INPUT_BT_OSC,          //BTOSC 双脚(12-26M)
    // SYS_CLOCK_INPUT_RTOSCH,
    SYS_CLOCK_INPUT_RTOSCL,
    SYS_CLOCK_INPUT_PAT,

    ///衍生时钟源作系统时钟源
    SYS_CLOCK_INPUT_PLL_BT_OSC,
    // SYS_CLOCK_INPUT_PLL_RTOSCH,
    SYS_CLOCK_INPUT_PLL_PAT,
} SYS_CLOCK_INPUT;

typedef enum {
    SYS_ICLOCK_INPUT_BTOSC,          //BTOSC 双脚(12-26M)
    SYS_ICLOCK_INPUT_RTOSCL,
    SYS_ICLOCK_INPUT_PAT,
} SYS_ICLOCK_INPUT;

typedef enum {
    PB0_CLOCK_OUTPUT = 0,
    PB0_CLOCK_OUT_BT_OSC,
    // PB0_CLOCK_OUT_RTOSCH,
    PB0_CLOCK_OUT_RTOSCL,

    PB0_CLOCK_OUT_LSB = 4,
    PB0_CLOCK_OUT_HSB,
    PB0_CLOCK_OUT_SFC,
    PB0_CLOCK_OUT_PLL,
} PB0_CLK_OUT;

typedef enum {
    PA2_CLOCK_OUTPUT = 0,
    PA2_CLOCK_OUT_RC,
    PA2_CLOCK_OUT_LRC,
    PA2_CLOCK_OUT_RCCL,

    PA2_CLOCK_OUT_BT_LO_D32 = 4,
    PA2_CLOCK_OUT_APC,
    PA2_CLOCK_OUT_PLL320,
    PA2_CLOCK_OUT_PLL107,
} PA2_CLK_OUT;

///< 时钟配置初始化
///<APC CLK
#define NORMAL_APC_Hz	48000000L	//normal state
#define CALL_APC_Hz		96000000L	//call state
///<CLK_SRC
#define SYS_CLOCK_IN    SYS_CLOCK_INPUT_PLL_BT_OSC

#endif
