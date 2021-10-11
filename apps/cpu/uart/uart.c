#include "sdk_cfg.h"
#include "uart_user.h"
#include "uart_param.h"
#include "common/msg.h"
#include "audio/eq.h"
#include "clock.h"
#include "fcc_test.h"
/* #include "uart_driver.h" */
#include "irq_api.h"
#include "updata.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif

extern void register_handle_printf_putchar(void (*handle)(char a));


#define UART_DEBUG_RAE	 UART_BAUD_RAE
#define UART_FCC_RAE     9600
#define UART_BQB_RAE	 57600
#define UART_EQ_BDG_RAE  9600

u8 *rx_uart0_buf = NULL;
u8 *rx_uart1_buf = NULL;
u8 *rx_uart2_buf = NULL;
u32 ut_dma_wr_cnt[MAX_UART_NUM];

static __uart_info uart_info[MAX_UART_NUM];

void putbyte(char a)
{
#ifdef __DEBUG
#if (DEBUG_UART == UART0)
    JL_UART0->BUF = a;
    __asm__ volatile("csync");

    while ((JL_UART0->CON0 & BIT(15)) == 0);    //TX IDLE
#elif (DEBUG_UART == UART1)
    JL_UART1->BUF = a;
    __asm__ volatile("csync");

    while ((JL_UART1->CON0 & BIT(15)) == 0);    //TX IDLE
#elif (DEBUG_UART == UART2)
    JL_UART2->BUF = a;
    __asm__ volatile("csync");

    while ((JL_UART2->CON & BIT(15)) == 0);    //TX IDLE
#endif
#endif
}

char getbyte()
{
#ifdef __DEBUG
    s32 i;
    char c = 0;
#if (DEBUG_UART == UART0)
    if ((JL_UART0->CON0 & BIT(14))) {
        c = JL_UART0->BUF;
        JL_UART0->CON0 |= BIT(12);
    }

#elif (DEBUG_UART == UART1)
    if ((JL_UART1->CON0 & BIT(14))) {
        c = JL_UART1->BUF;
        JL_UART1->CON0 |= BIT(12);
        JL_UART1->CON1 |= BIT(13);
    }

#elif (DEBUG_UART == UART2)
    if ((JL_UART2->CON & BIT(14))) {
        c = JL_UART2->BUF;
        JL_UART2->CON |= BIT(12);
    }
#endif

    return c;
#endif
    return 0;
}



void putchar(char a)
{
#ifdef __DEBUG
    if (a == '\n') {
        putbyte(0x0d);
        putbyte(0x0a);
    } else {
        putbyte(a);
    }
#endif
}

void put_u4hex(u8 dat)
{
#ifdef __DEBUG
    dat = 0xf & dat;

    if (dat > 9) {
        putbyte(dat - 10 + 'A');
    } else {
        putbyte(dat + '0');
    }
#endif
}
void put_u32hex(u32 dat)
{
#ifdef __DEBUG
    putbyte('0');
    putbyte('x');
    put_u4hex(dat >> 28);
    put_u4hex(dat >> 24);

    put_u4hex(dat >> 20);
    put_u4hex(dat >> 16);

    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar('\n');
#endif
}

void put_u32hex0(u32 dat)
{
#ifdef __DEBUG
    put_u4hex(dat >> 28);
    put_u4hex(dat >> 24);

    put_u4hex(dat >> 20);
    put_u4hex(dat >> 16);

    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
#endif
}

void put_u64hex(u64 dat)
{
#ifdef __DEBUG
    putbyte('0');
    putbyte('x');
    put_u32hex0(dat >> 32);
    put_u32hex0(dat);
    putchar('\n');
#endif
}
void put_u16hex(u16 dat)
{
#ifdef __DEBUG
    putbyte('0');
    putbyte('x');


    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
#endif
}

void put_u8hex(u8 dat)
{
#ifdef __DEBUG
    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
#endif
}

void printf_buf(u8 *buf, u32 len)
{
#ifdef __DEBUG
    put_u32hex((u32)buf);
    u32 i ;
    for (i = 0 ; i < len ; i++) {
        if (i && (i % 16) == 0) {
            putbyte('\r') ;
            putbyte('\n') ;
        }
        put_u8hex(buf[i]) ;
    }
    putbyte('\n') ;
#endif

}

int puts(const char *out)
{
#ifdef __DEBUG
    while (*out != '\0') {
        putchar(*out);
        out++;
    }
#endif
    return 0;
}

void put_buf(u8 *buf, u32 len)
{
#ifdef __DEBUG
    u32 i ;
    for (i = 0 ; i < len ; i++) {
        if ((i % 16) == 0) {
            putchar('\n') ;
        }
        put_u8hex(buf[i]) ;
    }
    putchar('\n') ;
#endif
}

void uart0_isr_fun()
{
    u8 uto_buf;
    if ((JL_UART0->CON0 & BIT(3)) && (JL_UART0->CON0 & BIT(14))) {
        uto_buf = JL_UART0->BUF;
        JL_UART0->CON0 |= BIT(12);
        if (uart_info[0].callback_fun) {
            uart_info[0].callback_fun(uto_buf, rx_uart0_buf, UART_ISR_TYPE_DATA_COME);
        }
    }
    if ((JL_UART0->CON0 & BIT(2)) && (JL_UART0->CON0 & BIT(15))) {  //TX PND
        JL_UART0->CON0 |= BIT(13);     //清TX PND
        if (uart_info[0].callback_fun) {
            uart_info[0].callback_fun(0, NULL, UART_ISR_TYPE_WRITE_OVER);
        }
    }

    if (JL_UART0->CON0 & BIT(11)) {    //OTCNT PND
        JL_UART0->CON0 |= BIT(10);    //清OTCNT PND
        JL_UART0->CON0 |= BIT(7);     //RDC
        asm volatile("nop");   //写RDC立刻读JL_UART1->HRCNT会有问题
        JL_UART0->CON0 |= BIT(12);    //清RX PND(这里的顺序不能改变，这里要清一次)
        if (uart_info[0].callback_fun) {
            uart_info[0].callback_fun(0, NULL, UART_ISR_TYPE_TIMEOUT);
        }
        if (rx_uart0_buf != NULL) {
            JL_UART0->RXSADR = (u32)rx_uart0_buf;
            JL_UART0->RXCNT = (u32)ut_dma_wr_cnt[0];;
        }
    }
}
IRQ_REGISTER(IRQ_UART0_IDX, uart0_isr_fun);

void uart1_isr_fun()
{
    u8 uto_buf;
    if ((JL_UART1->CON0 & BIT(3)) && (JL_UART1->CON0 & BIT(14))) {
        uto_buf = JL_UART1->BUF;
        JL_UART1->CON0 |= BIT(12);
        if (uart_info[1].callback_fun) {
            uart_info[1].callback_fun(uto_buf, rx_uart1_buf, UART_ISR_TYPE_DATA_COME);
        }
    }

    if ((JL_UART1->CON0 & BIT(2)) \
        && (JL_UART1->CON0 & BIT(15))) {     //TX PND
        JL_UART1->CON0 |= BIT(13);     //清TX PND
        if (uart_info[1].callback_fun) {
            uart_info[1].callback_fun(0, NULL, UART_ISR_TYPE_WRITE_OVER);
        }
    }

    if (JL_UART1->CON0 & BIT(11)) {    //OTCNT PND
        JL_UART1->CON0 |= BIT(10);    //清OTCNT PND
        JL_UART1->CON0 |= BIT(7);     //RDC
        asm volatile("nop");   //写RDC立刻读JL_UART1->HRCNT会有问题
        JL_UART1->CON0 |= BIT(12);    //清RX PND(这里的顺序不能改变，这里要清一次)
        if (uart_info[1].callback_fun) {
            uart_info[1].callback_fun(0, rx_uart1_buf, UART_ISR_TYPE_TIMEOUT);
        }
        if (rx_uart1_buf != NULL) {
            JL_UART1->RXSADR = (u32)rx_uart1_buf;
            JL_UART1->RXCNT = (u32)ut_dma_wr_cnt[1];;
        }
    }
}
IRQ_REGISTER(IRQ_UART1_IDX, uart1_isr_fun);

void uart2_isr_fun()
{
    u8 uto_buf;
    if ((JL_UART2->CON & BIT(14))) {
        uto_buf = JL_UART2->BUF;
        JL_UART2->CON |= BIT(12);
        if (uart_info[2].callback_fun) {
            uart_info[2].callback_fun(uto_buf, rx_uart2_buf, UART_ISR_TYPE_DATA_COME);
        }
    }
}
IRQ_REGISTER(IRQ_UART2_IDX, uart2_isr_fun);

static void uart_module_on()
{
    /* SFR(JL_CLOCK->CLK_CON1, 10, 2, 1); //use pll48m */
    SFR(JL_CLOCK->CLK_CON1, 10, 2, 0); //use osc
    /* SFR(JL_CLOCK->CLK_CON1, 10, 2, 2); //use lsb clk */

    uart_info[0].callback_fun = NULL;
    uart_info[1].callback_fun = NULL;
    uart_info[2].callback_fun = NULL;

    JL_UART0->CON0 = 0;
    JL_UART1->CON0 = 0;
    JL_UART2->CON = 0;
}

extern u8 ble_dut_uart_rx_isr(u8 rx_data);
extern void dut_uart_write_callback_register(void (*uart_write)(char a), u8 ble_bredr_mode);
#if (BT_MODE == TEST_FCC_MODE)
void fcc_uart_write(char a)
{
    JL_UART1->BUF = a;
    __asm__ volatile("csync");

    while ((JL_UART1->CON0 & BIT(15)) == 0);    //TX IDLE

}

void fcc_common_uart_isr(u8 uto_buf, void *p, u8 isr_flag)
{
    static u8 ble_dut_cmd_flag = 0;
    static u8 edr_fcc_cmd_flag = 0;

    if (UART_ISR_TYPE_DATA_COME == isr_flag) {
#if (BLE_BREDR_MODE&BT_BREDR_EN)
        if (0 == ble_dut_cmd_flag) {
            edr_fcc_cmd_flag = fcc_uart_isr_callback(uto_buf);
        }
#endif

#if (BLE_BREDR_MODE&BT_BLE_EN)
        if (0 == edr_fcc_cmd_flag) {
            ble_dut_cmd_flag = ble_dut_uart_rx_isr(uto_buf);
        }
#endif
    }
}
static s32 fcc_test_uart_init(u32 baud)
{
    u32 status = 0;
#if 0
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_PORTB->OUT |= BIT(0) ;
    JL_PORTB->DIR |= BIT(1) ;
    JL_PORTB->DIR &= ~BIT(0) ;
#else
    JL_IOMAP->CON1 |= BIT(3) | BIT(2);
    JL_USB->CON0 |= BIT(0);
    JL_USB->IO_CON0 = BIT(9) | BIT(10) | BIT(11);   //IO_MODE & default DP_DIE DM_DIE
    JL_USB->IO_CON0 |= BIT(0); //TX DP
    JL_USB->IO_CON0 |= BIT(3);//RX DM
    JL_USB->IO_CON0 &= ~BIT(2);//tx dp
    JL_USB->IO_CON0 &= ~BIT(5);//DM下拉
    JL_USB->IO_CON0 &= ~BIT(7);//DM上拉
#endif

    JL_UART1->BAUD = (UART_CLK / baud) / 4;;
    uart_info[1].callback_fun = fcc_common_uart_isr;//fcc_uart_isr_callback;

    IRQ_REQUEST(IRQ_UART1_IDX, uart1_isr_fun);
    JL_UART1->CON0 = BIT(14) | BIT(13) | BIT(12) | BIT(10) | BIT(3) | BIT(0);
#if (BLE_BREDR_MODE&BT_BLE_EN)
    dut_uart_write_callback_register(fcc_uart_write, BLE_BREDR_MODE);
#endif
    return 0;
}
#endif
#if (BT_MODE == TEST_BQB_MODE)
extern void dut_uart_write_callback_register(void (*uart_write)(char a), u8 ble_bredr_mode);
extern u8 ble_dut_uart_rx_isr(u8 rx_data);
void dut_uart_isr_callback(u8 uto_buf, void *p, u8 isr_flag)
{
    if (UART_ISR_TYPE_DATA_COME == isr_flag) {
        ble_dut_uart_rx_isr(uto_buf);
        //putchar('#');
        //put_u8hex(uto_buf);
    }
}

void dut_uart_write(char a)
{
    JL_UART1->BUF = a;
    __asm__ volatile("csync");

    while ((JL_UART1->CON0 & BIT(15)) == 0);    //TX IDLE

}

static s32 dut_test_uart_init(u32 baud)
{
    u32 status = 0;
#if 0
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_PORTB->OUT |= BIT(0) ;
    JL_PORTB->DIR |= BIT(1) ;
    JL_PORTB->DIR &= ~BIT(0) ;
#else
    JL_IOMAP->CON1 |= BIT(3) | BIT(2);
    JL_USB->CON0 |= BIT(0);
    JL_USB->IO_CON0 = BIT(9) | BIT(10) | BIT(11);   //IO_MODE & default DP_DIE DM_DIE
    JL_USB->IO_CON0 |= BIT(0); //TX DP
    JL_USB->IO_CON0 |= BIT(3);//RX DM
    JL_USB->IO_CON0 &= ~BIT(2);//tx dp
    JL_USB->IO_CON0 &= ~BIT(5);//DM下拉
    JL_USB->IO_CON0 &= ~BIT(7);//DM上拉
#endif
    JL_UART1->BAUD = (UART_CLK / baud) / 4;;
    uart_info[1].callback_fun = dut_uart_isr_callback;

    IRQ_REQUEST(IRQ_UART1_IDX, uart1_isr_fun);
    JL_UART1->CON0 = BIT(14) | BIT(13) | BIT(12) | BIT(10) | BIT(3) | BIT(0);
    dut_uart_write_callback_register(dut_uart_write, BLE_BREDR_MODE);
    return 0;
}
#endif

#if (EQ_UART_DEBUG)
u8 eq_uart_recv_buf[512] __attribute__((aligned(4)));
extern void eq_uart_debug_isr_callback(u8 uto_buf, void *p, u8 isr_flag);
void eq_uart_debug_write(char a)
{
    JL_UART1->BUF = a;
    __asm__ volatile("csync");
    while ((JL_UART1->CON0 & BIT(15)) == 0);    //TX IDLE
}


static s32 eq_uart_debug_init(u32 baud)
{
    u32 status = 0;
#if (EQ_DEBUG_UART_SEL == UART1_TXPB0_RXPB1)
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_PORTB->OUT |= BIT(0) ;
    JL_PORTB->DIR |= BIT(1) ;
    JL_PORTB->DIR &= ~BIT(0) ;
#elif (EQ_DEBUG_UART_SEL == UART1_USB_TXDP_RXDM)
    JL_IOMAP->CON1 |= BIT(3) | BIT(2);
    JL_USB->CON0 |= BIT(0);
    JL_USB->IO_CON0 = BIT(9) | BIT(10) | BIT(11);   //IO_MODE & default DP_DIE DM_DIE
    JL_USB->IO_CON0 |= BIT(0); //TX DP
    JL_USB->IO_CON0 |= BIT(3);//RX DM
    JL_USB->IO_CON0 &= ~BIT(2);//tx dp
    JL_USB->IO_CON0 &= ~BIT(5);//DM下拉
    JL_USB->IO_CON0 &= ~BIT(7);//DM上拉
#else
#error "EQ uart debug not config uart port IO"
#endif

    JL_UART1->BAUD = (UART_CLK / baud) / 4;;
    uart_info[1].callback_fun = eq_uart_debug_isr_callback;

    IRQ_REQUEST(IRQ_UART1_IDX, uart1_isr_fun);
    rx_uart1_buf = eq_uart_recv_buf;
    ut_dma_wr_cnt[1] = sizeof(eq_uart_recv_buf);

    JL_UART1->RXCNT = (u32)ut_dma_wr_cnt[1];
    JL_UART1->RXSADR = (u32)rx_uart1_buf;
    JL_UART1->RXEADR = (u32)(rx_uart1_buf + ut_dma_wr_cnt[1]);
    JL_UART1->OTCNT = 20000 * 1000;

    JL_UART1->CON0 =  BIT(13) | BIT(12) | BIT(10) | BIT(6) | BIT(5) | BIT(7) | BIT(3) | BIT(0);

    return 0;
}
#endif



s32 uart_debug_int(u32 baud_rate)
{
    s32 status = 0;
//UART0 CONFIG
#if (DEBUG_UART == UART0)
#if (DEBUG_UART_SEL == UART0_TXPA5_RXPA6)
    JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
    JL_PORTA->OUT |= BIT(5) ;
    JL_PORTA->DIR |= BIT(6) ;
    JL_PORTA->DIR &= ~BIT(5) ;

#elif (DEBUG_UART_SEL ==   UART0_TXPB5_RXPA0)
    JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
    JL_IOMAP->CON0 |= BIT(6);
    JL_PORTB->OUT |= BIT(5) ;
    JL_PORTA->DIR |= BIT(0) ;
    JL_PORTB->DIR &= ~BIT(5) ;
#elif (DEBUG_UART_SEL == UART0_TXPC2_RXPC3)
    JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
    JL_IOMAP->CON0 |= BIT(7);
    JL_PORTC->OUT |= BIT(2) ;
    JL_PORTC->DIR |= BIT(3) ;
    JL_PORTC->DIR &= ~BIT(2) ;
#elif (DEBUG_UART_SEL == UART0_TXPA7_RXPA8)
    JL_IOMAP->CON0 |= BIT(7) | BIT(6);
    JL_PORTA->OUT |= BIT(7) ;
    JL_PORTA->DIR |= BIT(8) ;
    JL_PORTA->DIR &= ~BIT(7) ;
#elif (DEBUG_UART_SEL ==  UART0_OUTPUT_CHAL)
    JL_IOMAP->CON3 &= ~BIT(3);
    JL_IOMAP->CON1 &= ~(BIT(8) | BIT(9) | BIT(10));
    //任意IO口输出,所选则的IO口需要设置为输出, 且开上下拉.  //这里以PA11为例
    JL_PORTA->DIR &= ~BIT(11);
    JL_PORTA->DIE &= ~BIT(11);
    JL_PORTA->PD |= BIT(11);
    JL_PORTA->PU |= BIT(11);
#endif  //end of UART0 IO SEL
    JL_UART0->BAUD = (UART_CLK / baud_rate) / 4 - 1;
    JL_UART0->CON0 = BIT(13) | BIT(12) | BIT(10) | BIT(0);
#endif   //end of UART0_CONFIG

//UART1 CONFIG
#if (DEBUG_UART==UART1)
#if (DEBUG_UART_SEL ==  UART1_TXPB0_RXPB1)
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_PORTB->OUT |= BIT(0) ;
    JL_PORTB->DIR |= BIT(1) ;
    JL_PORTB->DIR &= ~BIT(0) ;

#elif (DEBUG_UART_SEL == UART1_TXPC0_RXPC1)
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_IOMAP->CON1 |= BIT(2);
    JL_PORTC->OUT |= BIT(0) ;
    JL_PORTC->DIR |= BIT(1) ;
    JL_PORTC->DIR &= ~BIT(0) ;

#elif (DEBUG_UART_SEL ==  UART1_TXPA1_RXPA2)
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_IOMAP->CON1 |= BIT(3);
    JL_PORTA->OUT |= BIT(1) ;
    JL_PORTA->DIR |= BIT(2) ;
    JL_PORTA->DIR &= ~BIT(1) ;

#elif (DEBUG_UART_SEL ==  UART1_USB_TXDP_RXDM)
    JL_IOMAP->CON1 |= BIT(3) | BIT(2);

    JL_USB->CON0 = (BIT(0));//USB_PHY_ON
    JL_USB->IO_CON0 = (BIT(11) | BIT(10) | BIT(9)); //USB_IO_MODE	//DMDIE	//DPDIE

    JL_USB->IO_CON0 |= BIT(0); //TX DP
    JL_USB->IO_CON0 |= BIT(3);//RX DM
    JL_USB->IO_CON0 &= ~BIT(2);//tx dp

    JL_USB->IO_CON0 &= ~BIT(5);//DM下拉
    JL_USB->IO_CON0 &= ~BIT(7);//DM上拉

#elif (DEBUG_UART_SEL ==  UART1_OUTPUT_CHAL)
    JL_IOMAP->CON3 &= ~BIT(7);
    //JL_IOMAP->CON3 &= ~(BIT(22) | BIT(21) | BIT(20));
    JL_IOMAP->CON1 &= ~(BIT(8) | BIT(9) | BIT(10) | BIT(11));
    JL_IOMAP->CON1 |= BIT(8);

    JL_PORTA->DIR &= ~BIT(11);
    JL_PORTA->DIE &= ~BIT(11);
    JL_PORTA->PD |= BIT(11);
    JL_PORTA->PU |= BIT(11);
#endif  //end of UART1 IO SEL
    JL_UART1->BAUD = (UART_CLK / baud_rate) / 4 - 1;
    JL_UART1->CON0 = BIT(13) | BIT(12) | BIT(10) | BIT(0);
#endif  //end of UART1 CONFIG

//UART2 CONFIG
#if (DEBUG_UART == UART2)
#if (DEBUG_UART_SEL == UART2_TXPA3_RXPA4)
    JL_IOMAP->CON1 &= ~(BIT(15) | BIT(14));
    JL_PORTA->OUT |= BIT(3) ;
    JL_PORTA->DIR |= BIT(4) ;
    JL_PORTA->DIR &= ~BIT(3) ;

#elif(DEBUG_UART_SEL ==  UART2_TXPA9_RXPA10)
    JL_IOMAP->CON1 &= ~(BIT(15) | BIT(14));
    JL_IOMAP->CON1 |= BIT(14);
    JL_PORTA->OUT |= BIT(9) ;
    JL_PORTA->DIR |= BIT(10) ;
    JL_PORTA->DIR &= ~BIT(9) ;

#elif (DEBUG_UART_SEL ==  UART2_TXPB2_RXPB3)
    JL_IOMAP->CON1 &= ~(BIT(15) | BIT(14));
    JL_IOMAP->CON1 |= BIT(15);
    JL_PORTB->OUT |= BIT(2) ;
    JL_PORTB->DIR |= BIT(3) ;
    JL_PORTB->DIR &= ~BIT(2) ;

#elif (DEBUG_UART_SEL ==   UART2_TXPC4_RXPC5)
    JL_IOMAP->CON1 |= BIT(15) | BIT(14);
    JL_PORTC->OUT |= BIT(4) ;
    JL_PORTC->DIR |= BIT(5) ;
    JL_PORTC->DIR &= ~BIT(4) ;

#elif (DEBUG_UART_SEL ==   UART2_OUTPUT_CHAL)
    JL_IOMAP->CON3 &= ~BIT(11);
    JL_IOMAP->CON1 &= ~(BIT(8) | BIT(9) | BIT(10) | BIT(11));
    JL_IOMAP->CON1 |= 0x07 << 8;
    JL_PORTA->DIR &= ~BIT(11);
    JL_PORTA->DIE &= ~BIT(11);
    JL_PORTA->PD |= BIT(11);
    JL_PORTA->PU |= BIT(11);
#endif  //end of UART2 IO SEL
    JL_UART2->BAUD = (UART_CLK / baud_rate) / 4 - 1;
    JL_UART2->CON = BIT(13) | BIT(12) | BIT(10) | BIT(0);
#endif  //end of UART2 CONFIG

    register_handle_printf_putchar(putchar);
    return status;
}



void uart1_rtx_cts(void)
{
#if 0
    //PA5:CTS
    JL_PORTA->DIR |= BIT(5);
    JL_PORTA->PU  |= BIT(5);
    JL_PORTA->PD  &= ~BIT(5);

    //PA6:RTS
    JL_PORTA->DIR &= ~BIT(6);
    JL_PORTA->PU  &= ~BIT(6);
    JL_PORTA->PD  &= ~BIT(6);

    JL_PORTA->DIE &= ~(BIT(5) | BIT(6));

    //FLOW_CONTROL
    JL_UART1->CON1 &= ~(BIT(3) | BIT(2) | BIT(1) | BIT(0));
    /* JL_UART1->CON1 |= (BIT(1));//DMA_mode */
    /* JL_UART1->CON1 |= (BIT(0));//BUF_mode */
#endif

#if 0
    //BAUD_setting = 120000 baud
    JL_UART1->CON0 = BIT(13) | BIT(12) | BIT(10) | BIT(4) | BIT(0);
    JL_UART1->BAUD = 132;

    JL_UART1->CON1 &= ~(BIT(5) | BIT(4));
    JL_UART1->CON1 |= (BIT(4));
#endif

#if 0
    //BAUD_setting = 159000 baud
    JL_UART1->CON0 = BIT(13) | BIT(12) | BIT(10) | BIT(0);
    JL_UART1->BAUD = 74;

    JL_UART1->CON1 &= ~(BIT(5) | BIT(4));
    JL_UART1->CON1 |= (BIT(5));
#endif
}

#if UART_UPDATA_EN

UPDATA_UART updata_uart = {
    .control_io = UART0_TXPA5_RXPA6,//UART0_TXPC2_RXPC3,//
    .control_baud = 460800,
    .control_timeout = 300,
};

static u8 rx_uart_updata_buf[12];
const u8 uart_updata_flag[] = {"UART_UPDATE!"};
const u8 uart_updata_star[] = {"UPDATE_STAR!"};
const u8 uart_updata_receive_flag[] = {"RECEIVE_CMD!"};

void uart_updata_io_ctrl(void **parm)
{
    /* memcpy(parm,&updata_uart,sizeof(UPDATA_UART));	 */
    *parm = &updata_uart;
    /* printf("up_io = %x\nup_baud = %d\nup_timeout = %dms\n",((UPDATA_UART *)parm)->control_io,((UPDATA_UART *)parm)->control_baud,((UPDATA_UART *)parm)->control_timeout*10); */
}
void uart_updata_send_data(char *data, u32 len)
{
    u32 i;
    char *arr;
    arr = data;
    for (i = 0; i < len; i++) {
        JL_UART0->BUF = *(arr + i);
        __asm__ volatile("csync");

        while ((JL_UART0->CON0 & BIT(15)) == 0);    //TX IDLE
    }
}

void uart_updata_receive_cmd()
{
    uart_updata_send_data((char *)uart_updata_receive_flag, 12);
}

void uart_updata_isr_callback()
{
    u32 ret = 0;
    if ((JL_UART0->CON0 & BIT(3)) && (JL_UART0->CON0 & BIT(14))) {
        JL_UART0->CON0 |= BIT(12);
        ret = memcmp(uart_updata_star, rx_uart_updata_buf, 12);
        if (0 == ret) {
            uart_updata_receive_cmd();
            puts("----MSG_UPDATA_STAR----\n");
        } else {
            ret = memcmp(uart_updata_flag, rx_uart_updata_buf, 12);
            if (0 == ret) {
                puts("----MSG_UPDATA MSG_UPDATA----\n");
                task_post_msg(NULL, 1, MSG_UART_UPDATA);
            }
        }
        JL_UART0->RXSADR = (u32)rx_uart_updata_buf;
        /* JL_UART0->RXEADR = (u32)(rx_uart_updata_buf+ sizeof(rx_uart_updata_buf)); */
        JL_UART0->RXCNT = sizeof(rx_uart_updata_buf);
        JL_UART0->OTCNT = 0xffff;
    }
    if ((JL_UART0->CON0 & BIT(2)) && (JL_UART0->CON0 & BIT(15))) {     //TX PND
        JL_UART0->CON0 |= BIT(13);     //清TX PND
    }

    if (JL_UART0->CON0 & BIT(11)) {    //OTCNT PND
        JL_UART0->CON0 |= BIT(10);    //清OTCNT PND
        JL_UART0->CON0 |= BIT(7);     //RDC
        ret = memcmp(uart_updata_star, rx_uart_updata_buf, 12);
        if (0 == ret) {
            uart_updata_receive_cmd();
            puts("----MSG_UPDATA_STAR----\n");
        } else {
            ret = memcmp(uart_updata_flag, rx_uart_updata_buf, 12);
            if (0 == ret) {
                puts("----MSG_UPDATA MSG_UPDATA----\n");
                task_post_msg(NULL, 1, MSG_UART_UPDATA);
            }
        }
        asm volatile("nop");   //写RDC立刻读JL_UART0->HRCNT会有问题
        JL_UART0->CON0 |= BIT(12);    //清RX PND(这里的顺序不能改变，这里要清一次)
        JL_UART0->RXSADR = (u32)rx_uart_updata_buf;
        /* JL_UART0->RXEADR = (u32)(rx_uart_updata_buf+ sizeof(rx_uart_updata_buf)); */
        JL_UART0->RXCNT = sizeof(rx_uart_updata_buf);
        JL_UART0->OTCNT = 0xffff;
    }
    printf_buf(rx_uart_updata_buf, sizeof(rx_uart_updata_buf));
}
IRQ_REGISTER(IRQ_UART0_IDX, uart_updata_isr_callback);

void uart_update_init()
{
    printf("uart_update_init\n");
//UART0 CONFIG
    if (updata_uart.control_io == UART0_TXPA5_RXPA6) {
        JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
        JL_PORTA->OUT |= BIT(5) ;
        JL_PORTA->DIR |= BIT(6) ;
        JL_PORTA->DIR &= ~BIT(5) ;
    }

    else if (updata_uart.control_io ==   UART0_TXPB5_RXPA0) {
        JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
        JL_IOMAP->CON0 |= BIT(6);
        JL_PORTB->OUT |= BIT(5) ;
        JL_PORTA->DIR |= BIT(0) ;
        JL_PORTB->DIR &= ~BIT(5) ;
    } else if (updata_uart.control_io == UART0_TXPC2_RXPC3) {
        JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
        JL_IOMAP->CON0 |= BIT(7);
        JL_PORTC->OUT |= BIT(2) ;
        JL_PORTC->DIR |= BIT(3) ;
        JL_PORTC->DIR &= ~BIT(2) ;
    } else if (updata_uart.control_io == UART0_TXPA7_RXPA8) {
        JL_IOMAP->CON0 |= BIT(7) | BIT(6);
        JL_PORTA->OUT |= BIT(7) ;
        JL_PORTA->DIR |= BIT(8) ;
        JL_PORTA->DIR &= ~BIT(7) ;
    } else {
        printf("update_uart io is err!!!!!!!!");
    }

    JL_UART0->CON0 =  BIT(14) |  BIT(12) | BIT(10) | BIT(7) | BIT(6) | BIT(5) | BIT(3) | BIT(0);
    JL_UART0->BAUD = (UART_CLK / updata_uart.control_baud) / 4 - 1;
    JL_UART0->RXSADR = (u32)rx_uart_updata_buf;
    /* JL_UART0->RXEADR = (u32)(rx_uart_updata_buf+sizeof(rx_uart_updata_buf)); */
    JL_UART0->RXCNT = sizeof(rx_uart_updata_buf);
    JL_UART0->OTCNT = 0xffff;
    IRQ_REQUEST(IRQ_UART0_IDX, uart_updata_isr_callback);
}
#else
void uart_updata_io_ctrl(void **parm)
{

}

void uart_updata_receive_cmd()
{

}

#endif  //end of UART update

void uart_module_init()
{
    uart_module_on();
#ifdef __DEBUG
    uart_debug_int(DEBUG_UART_RATE);
#endif

#if (BT_MODE == TEST_FCC_MODE)
    fcc_test_uart_init(UART_FCC_RAE);
#endif

#if (BT_MODE == TEST_BQB_MODE) && (BLE_BREDR_MODE&BT_BLE_EN)
    dut_test_uart_init(UART_BQB_RAE);
#endif

#if EQ_UART_DEBUG
    eq_uart_debug_init(EQ_DEBUG_UART_RATE);
#endif

#if UART_UPDATA_EN
    uart_update_init();
#endif

    /* uart1_rtx_cts(); */
}

