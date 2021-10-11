#ifndef __MCPWM_API_H__
#define __MCPWM_API_H__

#include "typedef.h"

#define MCPWM_USE_DENOMINATOR 100

enum {
    MCPWM_KEPP_TMRCNT_MODE = 0, //a保持TMR0CNT
    MCPWM_INCREASE_MODE,//递增模式
    MCPWM_INCREASE_REDUCE_MODE,//递增-递减循环模式
    MCPWM_EXTERNAL_CONTROL_MODE,//由外部引脚控制递增或递减
};

enum {
    //aim at sys clk
    MCPWM_CLK_DIV1 = 0,
    MCPWM_CLK_DIV2,
    MCPWM_CLK_DIV4,
    MCPWM_CLK_DIV8,
    MCPWM_CLK_DIV16,
    MCPWM_CLK_DIV32,
    MCPWM_CLK_DIV64,
    MCPWM_CLK_DIV128,
};

#define MCPWMCH0_H       BIT(0)
#define MCPWMCH0_L       BIT(1)
#define MCPWMCH1_H       BIT(2)
#define MCPWMCH1_L       BIT(3)
#define MCPWMCH2_H       BIT(4)
#define MCPWMCH2_L       BIT(5)
#define MCPWMCH3_H       BIT(6)
#define MCPWMCH3_L       BIT(7)
#define MCPWMCH4_H       BIT(8)
#define MCPWMCH4_L       BIT(9)
#define MCPWMCH5_H       BIT(10)
#define MCPWMCH5_L       BIT(11)
#define MCPWMCH_ALL      0xffff

#define MCPWMCH0_H_OPPOSITE       BIT(0)
#define MCPWMCH0_L_OPPOSITE       BIT(1)
#define MCPWMCH1_H_OPPOSITE       BIT(2)
#define MCPWMCH1_L_OPPOSITE       BIT(3)
#define MCPWMCH2_H_OPPOSITE       BIT(4)
#define MCPWMCH2_L_OPPOSITE       BIT(5)
#define MCPWMCH3_H_OPPOSITE       BIT(6)
#define MCPWMCH3_L_OPPOSITE       BIT(7)
#define MCPWMCH4_H_OPPOSITE       BIT(8)
#define MCPWMCH4_L_OPPOSITE       BIT(9)
#define MCPWMCH5_H_OPPOSITE       BIT(10)
#define MCPWMCH5_L_OPPOSITE       BIT(11)


/*----------------------------------------------------------------------------*/
/**@brief open mcpwm module
   @param pwm_md : 产生pwm方式  1:递增模式   2:递增-递减循环模式
		  sys_clk : 系统时钟
		  div : 系统时钟分频,pwm timer的clk为sys_clk/(2^n)
		  fre : 频率
		  deno : 占空比的分母
   @return NULL
   @note must init before use mcpwm module
 */
/*----------------------------------------------------------------------------*/
void mcpwm_module_on(u8 pwm_md, u32 sys_clk, u8 div, u32 fre, u16 deno);

/*----------------------------------------------------------------------------*/
/**@brief close mcpwm module
   @param ch : BIT 0~1 对应 MCPWMCH0_H MCPWMCH0_L
			   BIT 2~3 对应 MCPWMCH1_H MCPWMCH1_L
			   BIT 4~5 对应 MCPWMCH2_H MCPWMCH2_L
			   0xffff : close all mcpwm
   @return NULL
   @note
 */
/*----------------------------------------------------------------------------*/
void mcpwm_module_off(u16 ch);

/*----------------------------------------------------------------------------*/
/**@brief mcpwm0 init
   @param opposite_ctl : BIT 0~1 对应 MCPWMCH0_H MCPWMCH0_L反向控制 1:enable  0:disable
		  ch : BIT 0~1 对应 MCPWMCH0_H MCPWMCH0_L  1:enable  0:disable
   @return NULL
   @note must init before use mcpwm0
 */
/*----------------------------------------------------------------------------*/
void mcpwm0_init(u16 opposite_ctl, u16 ch);

/*----------------------------------------------------------------------------*/
/**@brief mcpwm0 set
   @param duty : 占空比设置,实际占空比为duty/deno
   @return NULL
   @note
 */
/*----------------------------------------------------------------------------*/
void set_mcpwm0(u32 duty);

/*----------------------------------------------------------------------------*/
/**@brief mcpwm1 init
   @param opposite_ctl : BIT 2~3 对应 MCPWMCH1_H MCPWMCH1_L反向控制 1:enable  0:disable
		  ch : BIT 2~3 对应 MCPWMCH1_H MCPWMCH1_L  1:enable  0:disable
   @return NULL
   @note must init before use mcpwm1
 */
/*----------------------------------------------------------------------------*/
void mcpwm1_init(u16 opposite_ctl, u16 ch);

/*----------------------------------------------------------------------------*/
/**@brief mcpwm1 set
   @param duty : 占空比设置,实际占空比为duty/deno
   @return NULL
   @note
 */
/*----------------------------------------------------------------------------*/
void set_mcpwm1(u32 duty);

/*----------------------------------------------------------------------------*/
/**@brief mcpwm2 init
   @param opposite_ctl : BIT 4~5 对应 MCPWMCH2_H MCPWMCH2_L反向控制 1:enable  0:disable
		  ch : BIT 4~5 对应 MCPWMCH2_H MCPWMCH2_L  1:enable  0:disable
   @return NULL
   @note must init before use mcpwm2
 */
/*----------------------------------------------------------------------------*/
void mcpwm2_init(u16 opposite_ctl, u16 ch);

/*----------------------------------------------------------------------------*/
/**@brief mcpwm2 set
   @param duty : 占空比设置,实际占空比为duty/deno
   @return NULL
   @note
 */
/*----------------------------------------------------------------------------*/
void set_mcpwm2(u32 duty);

void mcpwm_demo();

/**************************************************************************
 * For example:
    mcpwm_module_on(MCPWM_INCREASE_REDUCE_MODE, 48000000L, MCPWM_CLK_DIV8, 10000L, MCPWM_USE_DENOMINATOR);

    mcpwm0_init(MCPWMCH0_H_OPPOSITE, MCPWMCH0_H | MCPWMCH0_L);
    set_mcpwm0(25);

    mcpwm1_init(MCPWMCH1_L_OPPOSITE, MCPWMCH1_H | MCPWMCH1_L);
    set_mcpwm1(22);

    mcpwm2_init(MCPWMCH2_L_OPPOSITE, MCPWMCH2_H | MCPWMCH2_L);
    set_mcpwm2(33);
 ************************************************************************/
#endif   //__MCPWM_API_H__
