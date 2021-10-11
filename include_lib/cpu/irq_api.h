/*******************************************************************************************
 *
 *   File Name: .h
 *
 *   Version: 1.00
 *
 *   Discription:
 *
 *   Author:Bingquan Cai
 *
 *   Email :bingquan_cai@zh-jieli.com
 *
 *   Date:
 *
 *   Copyright:(c)JIELI  2016  @ , All Rights Reserved.
 *
 *   *******************************************************************************************/

#ifndef __INTERRUPT_API_H__
#define __INTERRUPT_API_H__

#include "typedef.h"

#define IRQ_EXCEPTION_IDX  0		//0
#define IRQ_TICK_TMR_IDX   1		//0
#define IRQ_TIME0_IDX      2		//0
#define IRQ_TIME1_IDX      3		//0
#define IRQ_TIME2_IDX      4		//0
#define IRQ_TIME3_IDX      5		//0
#define IRQ_USB_SOF_IDX    6		//1
#define IRQ_USB_CTRL_IDX   7		//1
#define IRQ_RTC_IDX        8		//0
#define IRQ_ALINK_IDX      9		//1
#define IRQ_DAC_IDX        10		//1
#define IRQ_PORT_IDX       11		//0
#define IRQ_SPI0_IDX       12		//0
#define IRQ_SPI1_IDX       13		//0
#define IRQ_SD0_IDX        14		//0
#define IRQ_SD1_IDX        15		//0
#define IRQ_UART0_IDX      16		//0
#define IRQ_UART1_IDX      17		//0
#define IRQ_UART2_IDX      18		//0
#define IRQ_PAP_IDX        19		//0
#define IRQ_IIC_IDX        20		//0
#define IRQ_SARADC_IDX     21		//0
#define IRQ_PDM_LINK_IDX   22		//1
#define IRQ_LRC_IDX        24       //1
#define IRQ_BREDR_IDX      25		//2
#define IRQ_BT_CLKN_IDX    26		//2
#define IRQ_BT_DBG_IDX     27		//1
#define IRQ_BT_PCM_IDX     28		//2
#define IRQ_SRC_IDX        29		//1
#define IRQ_EQ_IDX         31		//1
#define IRQ_SPDIF_IDX      33		//1
#define IRQ_WD_IDX         34
#define IRQ_BLE_RX_IDX     36		//2
#define IRQ_BLE_EVENT_IDX  37		//1
#define IRQ_MCTMR_IDX      39       //3
#define IRQ_MCPWM_IDX      40       //0
#define IRQ_CTM_IDX		   43		//1
#define IRQ_FM_IDX         50		//1
#define IRQ_FM_LOFC_IDX    51		//1
#define IRQ_SOFT_REC_IDX   57		//0
#define IRQ_SOFT_LOOP_IDX  58
#define IRQ_UI_LCD_IDX     59		//0
#define IRQ_SOFT_ECHO_IDX  60		//0
#define IRQ_SOFT_USBC_IDX  61		//0
#define IRQ_SOFT0_IDX      62		//0
#define IRQ_SOFT_IDX       63		//0

#define IRQ_MEM_ADDR        0x00000

/********************************************************************************/
/*
 *      API
 */
void irq_init(void);

void irq_handler_register(u32 idx, void *hdl, u32 prio);

void irq_handler_unregister(u32 idx);

void irq_common_handler(u32 idx);

void irq_enable(u32 idx);

void irq_disable(u32 idx);

bool irq_read(u32 idx);

void irq_global_enable(void);

void irq_global_disable(void);

void irq_clear_all_ie(void);

void irq_sys_enable(void);

void irq_sys_disable(void);

void irq_set_pending(u32 irq_idx);

void irq_index_tab_reg(void *tab, u32 max_cnt); //max_cnt:byte

u8 irq_index_to_prio(u8 idx);
void irq_set_prio(u32 irq_idx, u32 irq_prio);
#define IRQ_REGISTER(idx, hdl) \
    SET_INTERRUPT void irq_##hdl() \
    {\
        hdl();\
        irq_common_handler(idx);\
    }

#define IRQ_REQUEST(idx, hdl) \
    irq_handler_register(idx, irq_##hdl, irq_index_to_prio(idx))

#define IRQ_RELEASE(idx) \
	irq_handler_unregister(idx)
#endif
