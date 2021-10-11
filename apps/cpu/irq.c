#include "irq_api.h"
#include "irq.h"
#include "uart.h"
#include "sdk_cfg.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".irq_api_bss")
#pragma data_seg(	".irq_api_data")
#pragma const_seg(	".irq_api_const")
#pragma code_seg(	".irq_api_code")
#endif

static const u8 irq_index_2_prio_tab[][2] = {
    {IRQ_EXCEPTION_IDX, 	0},
    {IRQ_TICK_TMR_IDX, 		0},
    {IRQ_TIME0_IDX, 		1},
    {IRQ_TIME1_IDX, 		3},
    {IRQ_TIME2_IDX, 		0},
    {IRQ_TIME3_IDX, 		3},
    {IRQ_USB_SOF_IDX, 		2},
    {IRQ_USB_CTRL_IDX, 		2},
    {IRQ_RTC_IDX, 			0},
    {IRQ_ALINK_IDX, 		2},
    {IRQ_DAC_IDX, 			3},
    {IRQ_PORT_IDX, 			0},
    {IRQ_SPI0_IDX, 			0},
    {IRQ_SPI1_IDX, 			0},
    {IRQ_SD0_IDX, 			2},
    {IRQ_SD1_IDX, 			2},
    {IRQ_UART0_IDX, 		0},
    {IRQ_UART1_IDX, 		3},
    {IRQ_UART2_IDX, 		0},
    {IRQ_PAP_IDX, 			0},
    {IRQ_IIC_IDX, 			0},
    {IRQ_SARADC_IDX, 		0},
    {IRQ_PDM_LINK_IDX, 		1},
    {IRQ_BREDR_IDX, 		2},
    {IRQ_BT_CLKN_IDX, 		2},
    {IRQ_BT_DBG_IDX, 		1},
    {IRQ_BT_PCM_IDX, 		2},
    {IRQ_SRC_IDX, 			1},
    {IRQ_EQ_IDX, 			2},
    {IRQ_SPDIF_IDX, 		3},
    {IRQ_FM_IDX, 			2},
    {IRQ_FM_LOFC_IDX,		1},
    {IRQ_BLE_RX_IDX, 		2},
    {IRQ_BLE_EVENT_IDX,		2},
    {IRQ_SOFT_REC_IDX,  	0},
    {IRQ_SOFT0_IDX, 		2},
    {IRQ_SOFT_IDX, 			0},
    {IRQ_SOFT_ECHO_IDX, 	0},
    {IRQ_UI_LCD_IDX, 	    0},
    {IRQ_SOFT_LOOP_IDX,     0},
    {IRQ_SOFT_USBC_IDX,     0},

};

/*----------------------------------------------------------------------------*/
/**@brief  异常错误中断服务程序
   @param
   @return
   @note
 */
/*----------------------------------------------------------------------------*/
extern void the_exception_isr(void);
void exception_isr_handler(u32 *sp, u32 *usp, u32 *ssp)
{
#ifndef __DEBUG
    JL_POWER->CON |= BIT(4);
#endif

    u32 rets = sp[15];
    u32 reti = sp[16];

    log_printf("DEBUG_MSG = 0x%x\n", JL_DEBUG->MSG);
    log_printf("PRP MMU_MSG = 0x%x\n", JL_DEBUG->PRP_MMU_MSG);
    log_printf("LSB MMU MSG = 0x%x\n", JL_DEBUG->LSB_MMU_MSG_CH);
    log_printf("PRP WR LIMIT MSG = 0x%x\n", JL_DEBUG->PRP_WR_LIMIT_MSG);
    log_printf("LSB WR LIMIT MSG = 0x%x\n", JL_DEBUG->LSB_WR_LIMIT_CH);

    log_printf("\nRETS=0x%x\n", rets);
    log_printf("\nRETI=0x%x\n", reti);

    log_printf("\nSP  =0x%x\n", sp);
    printf_buf((void *)sp, (6 * 1024));	//printf 6K

    log_printf("\nUSP =0x%x\n", usp);
    printf_buf((void *)usp, 0x80);

    log_printf("\nSSP =0x%x\n", ssp);
    printf_buf((void *)ssp, 0x20);

    log_printf("\nRETS : (-1K ~ +1k) (0x%x ~ 0x%x)\n", rets - 1024, rets + 1024);
    printf_buf((void *)(rets - 1024), 1024 * 2);

    log_printf("\nRETI : (-1K ~ +1k) (0x%x ~ 0x%x)\n", reti - 1024, reti + 1024);
    printf_buf((void *)(reti - 1024), 1024 * 2);

    extern u32 STACK_START[];
    extern u32 STACK_END[];

    log_printf("\nUSP_END\n");
    printf_buf((void *)((u32)STACK_START), 0x80);

    log_printf("\nSSP_END\n");
    printf_buf((void *)((u32)STACK_END - 0x20), 0x20);

    log_printf("\nIRQ\n");
    printf_buf((void *)(0x00), 0x100);
}



/*----------------------------------------------------------------------------*/
/**@brief  中断服务程序初始化
   @param
   @return
   @note
 */
/*----------------------------------------------------------------------------*/
void app_irq_init()
{
    irq_init();
    irq_index_tab_reg((void *)irq_index_2_prio_tab, sizeof(irq_index_2_prio_tab));

    irq_handler_register(IRQ_EXCEPTION_IDX, the_exception_isr, 0);
}


