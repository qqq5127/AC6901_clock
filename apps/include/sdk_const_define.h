/*********************************************************************************************
    *   Filename        : sdk_const_define.h
    *   Description     : system const define,sdk_cfg.h use
    *   Author          : Junqian
    *   Last modifiled  : 2018-4-07
    *   Note            : system const define, user should not modifiled this file!!!!
*********************************************************************************************/

#ifndef _SDK_CONST_CONFI_
#define _SDK_CONST_CONFI_

/********************************************************************************/
//------------------------------电源类配置常量
/********************************************************************************/
///   0:  no change
#define    PWR_NO_CHANGE        0
///   1:  LDOIN 5v -> LDO   1.5v -> DVDD 1.2v, support bluetooth
#define    PWR_LDO15            1
///   2:  LDOIN 5v -> DCDC  1.5v -> DVDD 1.2v, support bluetooth
#define    PWR_DCDC15           2

/********************************************************************************/
//------------------------------EQ相关配置常量
/********************************************************************************/
#define    EQ_RUN_NULL          0    //硬件EQ
#define    EQ_RUN_HW            1    //硬件EQ
#define    EQ_RUN_SW            2    //软件EQ

/********************************************************************************/
//------------------------------LOWPOWER_OSC_TYPE  const define
/********************************************************************************/
#define BT_OSC                  0
#define RTC_OSCH                1
#define RTC_OSCL                2
#define LRC_32K                 3

/********************************************************************************/
//------------------------------DEBUG  UART CONST DEFINE
/********************************************************************************/
#define UART0   0x00
#define UART1   0x10
#define UART2   0x20
//UART0
#define UART0_TXPA5_RXPA6       0x01
#define UART0_TXPB5_RXPA0       0x02
#define UART0_TXPC2_RXPC3       0x03
#define UART0_TXPA7_RXPA8       0x04
#define UART0_OUTPUT_CHAL       0x05
//UART1
#define UART1_TXPB0_RXPB1       0x11
#define UART1_TXPC0_RXPC1       0x12
#define UART1_TXPA1_RXPA2       0x13
#define UART1_USB_TXDP_RXDM     0x14
#define UART1_OUTPUT_CHAL       0x15
//UART2
#define UART2_TXPA3_RXPA4       0x21
#define UART2_TXPA9_RXPA10      0x22
#define UART2_TXPB2_RXPB3       0x23
#define UART2_TXPC4_RXPC5       0x24
#define UART2_OUTPUT_CHAL       0x25

#endif  //end of _SDK_CONST_CONFI_


