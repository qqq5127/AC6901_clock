#ifndef __UART_PARAM_H__
#define __UART_PARAM_H__


#define MAX_UART_NUM   3

enum {
    UART_NO_ERR               = 0,
    UART_DRV_USED_ERR         = -1000,
    UART_DRV_NAME_ERR,
    UART_DRV_PARA_ERR,
    UART_DRV_ISRFUN_RDEF_ERR,
    UART_DRV_IO_ERR,
    UART_DRV_WORKMODE_ERR,
};

enum {
    UART_ISR_TYPE_DATA_COME = 0,
    UART_ISR_TYPE_WRITE_OVER,
    UART_ISR_TYPE_TIMEOUT,
};

typedef struct uart_info {
    void (*callback_fun)(u8 value, void *p, u8 isr_flag);
} __uart_info;

extern u8 *rx_uart0_buf;
extern u8 *rx_uart1_buf;
extern u8 *rx_uart2_buf;
extern u32 ut_dma_wr_cnt[MAX_UART_NUM];


#endif
