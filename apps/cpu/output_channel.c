#include "output_channel.h"
#include "uart.h"

enum {
    OUTPUT_CH0S	= 0u,/*DOUT:DIE = [0:0]*/
    OUTPUT_CH1S		,/*DOUT:DIE = [0:1]*/
    OUTPUT_CH2S		,/*DOUT:DIE = [1:0]*/
    OUTPUT_CH3S		,/*DOUT:DIE = [1:1]*/
};
/*OUTPUT_CH0S && OUTPUT_CH2S*/
enum {
    OC0_UT0_TX	= 0u,
    OC0_UT1_TX		,
    OC0_TMR0_PWM	,
    OC0_TMR1_PWM	,
    OC0_RTOSH_CLK	,
    OC0_BTOSC_CLK	,
    OC0_PLL_12M		,
    OC0_UT2_TX		,
    OC0_CH0_PWM_H	,
    OC0_CH0_PWM_L	,
    OC0_CH1_PWM_H	,
    OC0_CH1_PWM_L	,
    OC0_CH2_PWM_H	,
    OC0_CH2_PWM_L	,
    OC0_TMR2_PWM	,
    OC0_TMR3_PWM	,
};

/*OUTPUT_CH1S && OUTPUT_CH3S*/
enum {
    OC1_UT0_TX	= 0u,
    OC1_UT1_TX		,
    OC1_TMR0_PWM	,
    OC1_TMR1_PWM	,
    OC1_RTOSL_CLK	,
    OC1_BTOSC_CLK	,
    OC1_PLL_24M		,
    OC1_UT2_TX		,
    OC1_CH0_PWM_H	,
    OC1_CH0_PWM_L	,
    OC1_CH1_PWM_H	,
    OC1_CH1_PWM_L	,
    OC1_CH2_PWM_H	,
    OC1_CH2_PWM_L	,
    OC1_TMR2_PWM	,
    OC1_TMR3_PWM	,
};

void output_channel(u8 ch, u8 ch_out)
{
    log_printf("output_channel:%d\t%d\n", ch, ch_out);
    log_printf("output_channel:0x%x\t0x%x\n", JL_IOMAP->CON1, JL_IOMAP->CON3);
    switch (ch) {
    case OUTPUT_CH0S:
        JL_IOMAP->CON1 &=  ~(0xF << 8);
        JL_IOMAP->CON1 |= ch_out << 8;	/*IOMC1:[11:8] */
        break;
    case OUTPUT_CH1S:
        JL_IOMAP->CON3 &=  ~(0xF << 20);
        JL_IOMAP->CON3 |= ch_out << 20;	/*IOMC3:[23:20] */
        break;
    case OUTPUT_CH2S:
        JL_IOMAP->CON3 &=  ~(0xF << 24);
        JL_IOMAP->CON3 |= ch_out << 24;	/*IOMC3:[27:24] */
        break;
    case OUTPUT_CH3S:
        JL_IOMAP->CON3 &=  ~(0xF << 28);
        JL_IOMAP->CON3 |= ch_out << 28;	/*IOMC3:[31:28] */
        break;
    }
}

void output_channel_io(u8 ch)
{
    switch (ch) {
    case OUTPUT_CH0S:
        JL_PORTA->OUT &= ~BIT(8);
        JL_PORTA->DIE &= ~BIT(8);
        break;
    case OUTPUT_CH1S:
        JL_PORTA->OUT &= ~BIT(8);
        JL_PORTA->DIE |=  BIT(8);
        break;
    case OUTPUT_CH2S:
        JL_PORTA->OUT |=  BIT(8);
        JL_PORTA->DIE &= ~BIT(8);
        break;
    case OUTPUT_CH3S:
        JL_PORTA->OUT |=  BIT(8);
        JL_PORTA->DIE |=  BIT(8);
        break;
    }

    //output_ch_mode
    JL_PORTA->DIR &= ~BIT(8);
    JL_PORTA->PU  |=  BIT(8);
    JL_PORTA->PD  |=  BIT(8);

}

void output_channel_demo()
{
    u8 output_ch = OUTPUT_CH0S;
    output_channel(output_ch, OC0_BTOSC_CLK);
    output_channel_io(output_ch);
}
