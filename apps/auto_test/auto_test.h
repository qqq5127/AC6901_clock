#ifndef __AUTO_TEST_H__
#define __AUTO_TEST_H__

#include "typedef.h"
#include "sdk_cfg.h"
#define TEST_UART_DEBUG

#ifdef TEST_UART_DEBUG
#define test_puts           puts
#define test_printf         printf
#define test_buf            put_buf
#else
#define test_puts(...)
#define test_printf(...)
#define test_buf(...)
#endif/*KEY_UART_DEBUG*/

#define CMD_LEN  15
#define BUF_LEN  5

#define TEST_UART 				UART1 /*select uart port*/
#define AUTO_TEST_BANDRATE  	115200
#if (TEST_UART == UART0)
// #define TEST_UART_TXPA5_RXPA6
#define TEST_UART_TXPB5_RXPA0
//#define TEST_UART_TXPC2_RXPC3
//#define TEST_UART_TXPA7_RXPA8
//#define TEST_UART0_OUTPUT_CHAL
#endif

#if (TEST_UART == UART1)
#define TEST_UART_TXPB0_RXPB1
//#define TEST_UART_TXPC0_RXPC1
//#define TEST_UART_TXPA1_RXPA2
//#define TEST_UART_USBP_USBM
//#define TEST_UART1_OUTPUT_CHAL
#endif



enum {
    CMD_SUCC = 0x0,
    CMD_BCC_ERR,      //BCC检查错误，优先检查BCC
    CMD_FORMAT_ERR,   //格式错误，头信息或者尾部错误
    CMD_TYPE_ERR,     //命令格式错误
    CMD_MSG_ERR,      //消息指令错误
    CMD_ANS_ERR,      //应答指令错误
    CMD_CTL_ERR,      //控制指令错误
    CMD_FBK_ERR,      //反馈指令错误
};

enum {
    TYPE_MSG = '!',
    TYPE_ANS = '%',
    TYPE_CTL = '&',
    TYPE_FBK = '?',
};

typedef struct {
    unsigned char High;
    unsigned char Low;
} __attribute__((packed)) TypeHEX2ASCII;

typedef struct __uart_cmd {
    //HEAD
    u8 SOH;
    u8 type;
    u16 num;
    //CMD
    u32 cmd;
    //DATA
    u32 data;
    //END
    TypeHEX2ASCII BCC;
    u8 EOT;
} __attribute__((packed)) UART_CMD, *UART_CMD_P;

typedef struct __auto_test_cmd {
    // struct list_head entry;		     		///<命令管理链表
    // uart_cmd msg;                    ///<消息内容
    UART_CMD msg[BUF_LEN];
    u32 read_ptr;
    u32 write_ptr;
    u32 data_len;
    u32 total_len;
} __attribute__((packed)) AUTO_TEST_CMD;




#endif//__AUTO_TEST_H__


