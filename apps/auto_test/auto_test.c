#include "sdk_cfg.h"
#include "common.h"
#include "auto_test.h"
#include "clock.h"
#include "irq_api.h"
#include "msg.h"
#include "board_init.h"

static AUTO_TEST_CMD rev_cmd;
static AUTO_TEST_CMD send_cmd;
static u8 cmd_uart_rx_buf[CMD_LEN];
static u8 cmd_uart_tx_buf[CMD_LEN];

#define BigLittleSwap16(A)        ((((u16)(A) & 0xff00) >> 8) | \
                                                       (((u16)(A) & 0x00ff) << 8))


#define BigLittleSwap32(A)        ((((u32)(A) & 0xff000000) >> 24) | \
                                                       (((u32)(A) & 0x00ff0000) >> 8) | \
                                                       (((u32)(A) & 0x0000ff00) << 8) | \
                                                       (((u32)(A) & 0x000000ff) << 24))

static TypeHEX2ASCII CharToAscII(unsigned char uch)
{
    TypeHEX2ASCII StructAsc;
    StructAsc.High = ((uch & 0xF0) >> 4);
    if (StructAsc.High > 9) {
        StructAsc.High = (StructAsc.High - 10 + 'A');
    } else {
        StructAsc.High = StructAsc.High + '0';
    }
    StructAsc.Low = uch & 0x0F;
    if (StructAsc.Low > 9) {
        StructAsc.Low = (StructAsc.Low - 10 + 'A');
    } else {
        StructAsc.Low = StructAsc.Low + '0';
    }
    return StructAsc;
}

static u32 CheckBCC(UART_CMD rxmsg)
{
    u32 i;
    u8 mBcc;
    TypeHEX2ASCII StructAsc;

    u8 *RxBuf = (u8 *)&rxmsg;

    mBcc = 0;
    for (i = 0; i < sizeof(UART_CMD) - 3; i++) {
        mBcc ^= *(RxBuf + i);
    }
    StructAsc = CharToAscII(mBcc);
    /* printf("%d%d\n",StructAsc.High,StructAsc.Low); */
    if (!(StructAsc.High == rxmsg.BCC.High && StructAsc.Low == rxmsg.BCC.Low)) {
        return CMD_BCC_ERR;
    }
    return CMD_SUCC;
}
static void cmd_init(AUTO_TEST_CMD *cmd)
{
    memset(cmd->msg, 0, sizeof(cmd->msg));
    cmd->read_ptr = 0;
    cmd->write_ptr = 0;
    cmd->data_len = 0;
    cmd->total_len = sizeof(cmd->msg) / sizeof(UART_CMD);
}


static u32 cmd_write(AUTO_TEST_CMD *cmd, void *buf)
{
    if (!cmd) {
        return 0;
    }

    if (cmd->total_len <= cmd->data_len) {
        return 0;
    }

    memcpy((cmd->msg) + cmd->write_ptr, buf, sizeof(UART_CMD));

    cmd->data_len ++;
    cmd->write_ptr ++;

    if ((u32)cmd->write_ptr >= (u32)cmd->total_len) {
        cmd->write_ptr = 0;
    }

    return 1;
}

static u32 cmd_read(AUTO_TEST_CMD *cmd, void *buf)
{
    if (!cmd) {
        return 0;
    }

    if (cmd->data_len == 0) {
        return 0;
    }

    memcpy(buf, (cmd->msg) + cmd->read_ptr, sizeof(UART_CMD));

    cmd->read_ptr ++;
    cmd->data_len --;

    if ((u32)cmd->read_ptr >= (u32)cmd->total_len) {
        cmd->read_ptr = 0;
    }

    return 1;
}

#if (TEST_UART==UART0)
#define IRQ_AUTO_TEST_IDX   IRQ_UART0_IDX
#define AUTO_TEST_UART      JL_UART0
#else
#define IRQ_AUTO_TEST_IDX   IRQ_UART1_IDX
#define AUTO_TEST_UART      JL_UART1
#endif
static void test_uart_cmd_isr()
{
    u32 ret = 0;
    if ((AUTO_TEST_UART->CON0 & BIT(3)) && (AUTO_TEST_UART->CON0 & BIT(14))) {
        AUTO_TEST_UART->CON0 |= BIT(12);
        test_buf(cmd_uart_rx_buf, sizeof(cmd_uart_rx_buf));
        ret = cmd_write(&rev_cmd, cmd_uart_rx_buf);
        /* test_printf("ret = %d\n",ret); */
        AUTO_TEST_UART->RXSADR = (u32)cmd_uart_rx_buf;
        /* AUTO_TEST_UART->RXEADR = (u32)(cmd_uart_rx_buf + sizeof(cmd_uart_rx_buf)); */
        AUTO_TEST_UART->RXCNT = sizeof(cmd_uart_rx_buf);
        AUTO_TEST_UART->OTCNT = 0xffff;
    }
    if ((AUTO_TEST_UART->CON0 & BIT(2)) \
        && (AUTO_TEST_UART->CON0 & BIT(15))) {     //TX PND
        AUTO_TEST_UART->CON0 |= BIT(13);     //清TX PND
    }

    if (AUTO_TEST_UART->CON0 & BIT(11)) {    //OTCNT PND
        AUTO_TEST_UART->CON0 |= BIT(10);    //清OTCNT PND
        AUTO_TEST_UART->CON0 |= BIT(7);     //RDC
        asm volatile("nop");   //写RDC立刻读JL_UART0->HRCNT会有问题
        AUTO_TEST_UART->CON0 |= BIT(12);    //清RX PND(这里的顺序不能改变，这里要清一次)
        AUTO_TEST_UART->RXSADR = (u32)cmd_uart_rx_buf;
        /* AUTO_TEST_UART->RXEADR = (u32)(cmd_uart_rx_buf + sizeof(cmd_uart_rx_buf)); */
        AUTO_TEST_UART->RXCNT = sizeof(cmd_uart_rx_buf);
        AUTO_TEST_UART->OTCNT = 0xffff;
    }
    /* test_buf(cmd_uart_rx_buf,sizeof(cmd_uart_rx_buf)); */

}
IRQ_REGISTER(IRQ_AUTO_TEST_IDX, test_uart_cmd_isr);

static void uart_tx_cmd()
{
    AUTO_TEST_UART->CON0 |= BIT(13);     //清TX PND
    AUTO_TEST_UART->TXADR = (u32)cmd_uart_tx_buf;
    AUTO_TEST_UART->TXCNT_HRXCNT = sizeof(cmd_uart_tx_buf);
}

static void auto_test_init(void)
{
    cmd_init(&rev_cmd);
    cmd_init(&send_cmd);

    //choose io
#if (TEST_UART==UART0)
#ifdef TEST_UART_TXPA5_RXPA6
    JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
    JL_PORTA->OUT |= BIT(5) ;
    JL_PORTA->DIR |= BIT(6) ;
    JL_PORTA->DIR &= ~BIT(5) ;
#endif

#ifdef TEST_UART_TXPB5_RXPA0
    JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
    JL_IOMAP->CON0 |= BIT(6);
    JL_PORTB->OUT |= BIT(5) ;
    JL_PORTA->DIR |= BIT(0) ;
    JL_PORTB->DIR &= ~BIT(5) ;
#endif

#ifdef TEST_UART_TXPC2_RXPC3
    JL_IOMAP->CON0 &= ~(BIT(7) | BIT(6));
    JL_IOMAP->CON0 |= BIT(7);
    JL_PORTC->OUT |= BIT(2) ;
    JL_PORTC->DIR |= BIT(3) ;
    JL_PORTC->DIR &= ~BIT(2) ;
#endif

#ifdef TEST_UART_TXPA7_RXPA8
    JL_IOMAP->CON0 |= BIT(7) | BIT(6);
    JL_PORTA->OUT |= BIT(7) ;
    JL_PORTA->DIR |= BIT(8) ;
    JL_PORTA->DIR &= ~BIT(7) ;
#endif

#ifdef TEST_UART0_OUTPUT_CHAL
    JL_IOMAP->CON3 &= ~BIT(3);
    JL_IOMAP->CON1 &= ~(BIT(8) | BIT(9) | BIT(10));

    /*串口映射到任意IO口*/
    JL_PORTA->DIR &= ~BIT(5);
    JL_PORTA->DIE &= ~BIT(5);
    JL_PORTA->PD |= BIT(5);
    JL_PORTA->PU |= BIT(5);
#endif

#elif (TEST_UART==UART1)
#ifdef TEST_UART_TXPB0_RXPB1
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_PORTB->OUT |= BIT(0) ;
    JL_PORTB->DIR |= BIT(1) ;
    JL_PORTB->DIR &= ~BIT(0) ;
#endif

#ifdef TEST_UART_TXPC0_RXPC1
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_IOMAP->CON1 |= BIT(2);
    JL_PORTC->OUT |= BIT(0) ;
    JL_PORTC->DIR |= BIT(1) ;
    JL_PORTC->DIR &= ~BIT(0) ;
#endif

#ifdef TEST_UART_TXPA1_RXPA2
    JL_IOMAP->CON1 &= ~(BIT(3) | BIT(2));
    JL_IOMAP->CON1 |= BIT(3);
    JL_PORTA->OUT |= BIT(1) ;
    JL_PORTA->DIR |= BIT(2) ;
    JL_PORTA->DIR &= ~BIT(1) ;
#endif
#ifdef TEST_UART_USBP_USBM
    JL_IOMAP->CON1 |= BIT(3) | BIT(2);

    JL_USB->CON0 = (BIT(0));//USB_PHY_ON
    JL_USB->IO_CON0 = (BIT(11) | BIT(10) | BIT(9)); //USB_IO_MODE	//DMDIE	//DPDIE

    JL_USB->IO_CON0 |= BIT(0); //TX DP
    JL_USB->IO_CON0 |= BIT(3);//RX DM
    JL_USB->IO_CON0 &= ~BIT(2);//tx dp

    JL_USB->IO_CON0 &= ~BIT(5);//DM下拉
    JL_USB->IO_CON0 &= ~BIT(7);//DM上拉
#endif

#ifdef TEST_UART1_OUTPUT_CHAL
    JL_IOMAP->CON3 &= ~BIT(3);
    JL_IOMAP->CON1 &= ~(BIT(8) | BIT(9) | BIT(10));

    /*串口映射到任意IO口*/
    JL_PORTB->DIR &= ~BIT(0);
    JL_PORTB->DIE &= ~BIT(0);
    JL_PORTB->PD |= BIT(0);
    JL_PORTB->PU |= BIT(0);
#endif
#endif
    AUTO_TEST_UART->CON0 = BIT(14) | BIT(12) | BIT(10) | BIT(7) | BIT(6) | BIT(5) | BIT(3) | BIT(0);
    AUTO_TEST_UART->BAUD = (UART_CLK / AUTO_TEST_BANDRATE) / 4 - 1;
    AUTO_TEST_UART->RXSADR = (u32)cmd_uart_rx_buf;
    /* AUTO_TEST_UART->RXEADR = (u32)(cmd_uart_rx_buf + sizeof(cmd_uart_rx_buf)); */
    AUTO_TEST_UART->RXCNT = sizeof(cmd_uart_rx_buf);
    AUTO_TEST_UART->OTCNT = 0xffff;


    IRQ_REQUEST(IRQ_AUTO_TEST_IDX, test_uart_cmd_isr);
    AUTO_TEST_UART->CON0 |= BIT(15);

    test_puts("\r\n\r\n/****************test_init_ok****************\r\n\r\n");
}


/****************************************************************************
 * 消息处理上，采用的是和消息表一样的消息定义
 * 带有DATA部分做case处理，其他的直接推进消息池
 ***************************************************************************/
static u32 msg_deal(UART_CMD rxmsg)
{
    u32 ret = CMD_SUCC;
    switch (rxmsg.cmd) {
    case MSG_BT_MUSIC_EQ:
    case MSG_BT_TWS_EQ:
    case MSG_MUSIC_RPT:
    case MSG_MUSIC_EQ:
    case MSG_MUSIC_SPC_FILE:
    case MSG_FM_SELECT_CHANNEL:
    case MSG_FM_DEL_CHANNEL:
    case MSG_FM_SAVE_CHANNEL:
    case MSG_FM_SELECT_FREQ:
    case MSG_RTC_SETTING:
    case MSG_ALM_SETTING:
    case MSG_INPUT_TIMEOUT:
        task_post_msg(NULL, 2, (u16)(rxmsg.cmd), rxmsg.data);
        break;

    default://只有cmd的命令，直接推进消息池
        test_printf("num = %x cmd = %x\n", rxmsg.num, rxmsg.cmd);
        if ((rxmsg.cmd < MSG_MAIN_MAX)               || \
            ((rxmsg.cmd >= 0xB0) && (rxmsg.cmd < MSG_EVENT_MAX))	    || \
            ((rxmsg.cmd >= 0x100) && (rxmsg.cmd < MSG_BT_MAX))	   	|| \
            ((rxmsg.cmd >= 0x200) && (rxmsg.cmd < MSG_MUSIC_MAX))	   	|| \
            ((rxmsg.cmd >= 0x300) && (rxmsg.cmd < MSG_FM_MAX))    	|| \
            ((rxmsg.cmd >= 0x400) && (rxmsg.cmd < MSG_AUX_MAX))	   	|| \
            ((rxmsg.cmd >= 0x500) && (rxmsg.cmd < MSG_PC_MAX))	   	|| \
            ((rxmsg.cmd >= 0x600) && (rxmsg.cmd < MSG_RTC_MAX))	   	|| \
            ((rxmsg.cmd >= 0x900) && (rxmsg.cmd < MSG_IDLE_MAX))	   	|| \
            ((rxmsg.cmd >= 0xA00) && (rxmsg.cmd < MSG_COMMON_MAX))   	|| \
            ((rxmsg.cmd == MSG_CLEAN_ALL_MSG))	            	|| \
            ((rxmsg.cmd == MSG_LIBS1))) {
            task_post_msg(NULL, 1, (u16)(rxmsg.cmd));
        } else {
            ret = CMD_MSG_ERR;
        }
        break;

    }
    return ret;
}

/****************************************************************************
 * 控制处理上，采用的是和事件的定义
 * 带有DATA部分做case处理，其他的直接推进消息池
 ***************************************************************************/
static u32 ctl_deal(UART_CMD rxmsg)
{
    u32 ret = CMD_SUCC;
    /* switch (rxmsg.cmd) {
    case MSG_CACHE_ONLINE:
        break;

    case MSG_CACHE_OFFLINE:
        break;

    default://只有cmd的命令，直接推进消息池
        break;

    } */
    ret = CMD_CTL_ERR;
    return ret;
}

/****************************************************************************
 * 应答处理，反应处理结果
 ***************************************************************************/
static u32 ans_deal(UART_CMD rxmsg)
{
    u32 ret = CMD_SUCC;

    ret = CMD_ANS_ERR;
    return ret;
}

/****************************************************************************
 * 应答处理，反应处理结果
 ***************************************************************************/
static u32 fbk_deal(UART_CMD rxmsg)
{
    u32 ret = CMD_SUCC;

    ret = CMD_FBK_ERR;
    return ret;
}


static u32 ans_cmd(u16 num, u32 ans, u32 data)
{
    u32 ret = CMD_SUCC;
    UART_CMD answer;
    u8 *TxBuf = NULL;
    u32 i;
    u8 mBcc = 0;

    TxBuf = (u8 *)(&answer);
    answer.SOH = '#';
    answer.type = '%';
    answer.num = num;
    answer.cmd = BigLittleSwap32(ans);
    answer.data = data;
    for (i = 0; i < sizeof(UART_CMD) - 3; i++) {
        mBcc ^= *(TxBuf + i);
    }
    answer.BCC = CharToAscII(mBcc);
    answer.EOT = '$';

    cmd_write(&send_cmd, &answer);

    test_buf((u8 *)&answer, sizeof(answer));
    return ret;
}

static u32 auto_test_deal(UART_CMD rxmsg)
{
    u32 ret = CMD_SUCC;
    rxmsg.cmd = BigLittleSwap32(rxmsg.cmd);
    rxmsg.data = BigLittleSwap32(rxmsg.data);
    ret = CheckBCC(rxmsg);
    if (CMD_SUCC == ret) {
        if ((rxmsg.SOH == '#') && (rxmsg.EOT == '$')) {
            switch (rxmsg.type) {
            case TYPE_MSG:
                ret = msg_deal(rxmsg);
                break;

            case TYPE_ANS:
                ret = ans_deal(rxmsg);
                break;

            case TYPE_CTL:
                ret = ctl_deal(rxmsg);
                break;

            case TYPE_FBK:
                ret = fbk_deal(rxmsg);
                break;

            default:
                ret = CMD_TYPE_ERR;
                break;
            }
        } else {
            ret = CMD_FORMAT_ERR;
        }
    } else {
        ret = CMD_BCC_ERR;
    }

    ans_cmd(rxmsg.num, ret, 0); //rxmsg.data
    return ret;
}


static void auto_test_scan()
{
    u32 ret = CMD_SUCC;
    UART_CMD rxmsg;

    if (cmd_read(&rev_cmd, &rxmsg) != 0) {
        ret = auto_test_deal(rxmsg);
        /* test_buf((u8*)&rxmsg,15); */
    }

    if ((AUTO_TEST_UART->CON0 & BIT(15))) {
        if (cmd_read(&send_cmd, cmd_uart_tx_buf) != 0) {
            uart_tx_cmd();
        }
    }
}
/*
no_sequence_initcall(auto_test_init);
LOOP_DETECT_REGISTER(auto_test_det) = {
    .time = 5,
    .fun  = auto_test_scan,
};
*/



