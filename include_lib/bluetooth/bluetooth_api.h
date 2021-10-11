#ifndef _BLUETOOTH_API_H_
#define _BLUETOOTH_API_H_

#include "typedef.h"
#include "uart.h"
//#include "common/printf.h"

// #define UART_BT_LIB_DEG


#ifdef UART_BT_LIB_DEG
#define bt_puts                 puts
#define bt_lmp_cmd_puts          puts
#define bt_put_u32d             put_u32d
#define bt_put_u32hex0          put_u32hex0
#define bt_put_u32hex           put_u32hex
#define bt_printf               printf

#define bt_printf_without_lock  printf_without_lock
#define bt_puts_without_lock    puts_without_lock
#define bt_put_u16hex           put_u16hex
#define bt_put_u8hex0           put_u8hex0
#define bt_put_u8hex            put_u8hex
#define bt_put_buf              put_buf
#define bt_printf_buf           printf_buf
#define bt_putchar              putchar
#else
#define bt_puts(...)
#define bt_lmp_cmd_puts(...)
#define bt_put_u32d(...)
#define bt_put_u32hex0(...)
#define bt_put_u32hex(...)
#define bt_printf(...)

#define bt_printf_without_lock(...)
#define bt_puts_without_lock(...)
#define bt_put_u16hex(...)
#define bt_put_u8hex0(...)
#define bt_put_u8hex(...)
#define bt_put_buf(...)
#define bt_printf_buf(...)
#define bt_putchar(...)
#endif


#define ACL_PACKETS_TOTAL_NUM               4
#define ACL_DATA_PACKET_LENGTH              680

//配置测试盒测试功能
#define NON_TEST         0          ///<没频偏和距离测试
#define FRE_OFFSET_TEST  BIT(0)     ///<频偏测试
#define DISTANCE_TEST    BIT(1)     ///<距离测试


#define NORMAL_MODE         0
#define TEST_BQB_MODE       1       ///<测试bqb认证
#define TEST_FCC_MODE       2       ///<测试fcc认证
#define TEST_FRE_OFF_MODE   3       ///<测试频偏(使用频谱分析仪-手提测试仪-中心频率默认2422M)
#define TEST_PERFOR_MODE    4       ///<指标性能测试(使用MT8852A仪器测试,测试芯片性能的时候使用)
#define TEST_BOX_MODE       5       ///<测试盒测试

//配置Low power mode
#define SNIFF_EN                            BIT(0)  ///<SNIFF使能
#define SNIFF_TOW_CONN_ENTER_POWERDOWN_EN   BIT(3)  ///<SNIFF 等待两台都连接才进powerdown
#define SNIFF_CNT_TIME                      10      ///<空闲10S之后进入sniff模式

#define SNIFF_MAX_INTERVALSLOT        800
#define SNIFF_MIN_INTERVALSLOT        100
#define SNIFF_ATTEMPT_SLOT            12
#define SNIFF_TIMEOUT_SLOT            1
#define SNIFF_DATA_SLOT               0

#define SNIFF_WAKEUP_SLOT             4
// #define SNIFF_IN_SLOT                 80  //在可以进入power down的状态前延时80slot,预留时间给page_scan

//配置Low power mode
#define BT_POWER_OFF_EN                   BIT(1)  ///<SNIFF 进入poweroff
#define BT_POWER_DOWN_EN                  BIT(2)  ///<SNIFF 进入powerdown

#define S_DEVICE_ROLE_SLAVE           BIT(0)//手机连接设备，设备做从
#define S_DEVICE_ROLE_SLAVE_TO_MASTER BIT(1)//手机连接设备，设备从变主

#define M_DEVICE_ROLE_MASTER          BIT(2)//设备回连手机，设备做主
#define M_DEVICE_ROLE_MASTER_TO_SLAVE BIT(3) //设备回连手机，设备主变从


#define BRAOABCASET_MASTER_START            1////主开始广播
#define BRAOABCASET_MASTER_CANCLE_STOP      2////主取消广播
#define BRAOABCASET_MASTER_CON_STOP         3////广播主断开手机连接，同时stop广播
#define BRAOABCASET_SLAVE_SCAN_START        4////开始监听广播连接信息
#define BRAOABCASET_SLAVE_SCAN_STOP         5////停止监听广播连接信息
#define BRAOABCASET_SLAVE_CON_START         6/////监听广播连接信息成功，连接链路
#define BRAOABCASET_SLAVE_CON_STOP          7/////断开监听广播 连接链路

#define BT_BREDR_EN  BIT(0)
#define BT_BLE_EN    BIT(1)
// #define BT_TWS_MONITOR_LIB_EN    //去掉库里面的监听和一拖多的相关代码，省出8k 代码空间

#define BT_EMITTER_TRAIN    BIT(0)//发射器
#define BT_TWS_TRANSMIT     BIT(1)//tws类型为转发的方式，即一拖二的方式
#define BT_TWS_MONITOR      BIT(2)//tws类型为监听的方式，即一拖二的方式
#define BT_TWS_BROADCAST    BIT(3)//tws类型为广播的方式，即一拖多

#define   BT_TWS_CUR_STATE_TWS_MASTER_LINEIN                 1////主机没连接从机，并且插入linein
#define   BT_TWS_CUR_STATE_TWS_MASTER_SLAVE_LINEIN           2////主机连接了从机，并且插入linein
#define   BT_TWS_CUR_STATE_TWS_SLAVE_LINEIN                  3////从机没连接着主机，并且插入linein
#define   BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_LINEIN           4////从机连接着主机，并且插入linein
#define   BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_PLAY_LINEIN      5////从机连接着主机，并且插入linein,并且主机已播放
#define   BT_TWS_CUR_STATE_TWS_NOT_ROLE_PLAY_LINEIN          6////NOT role，并且插入linein,播放
extern void background_resume();
extern void no_background_suspend();
extern void background_suspend();
extern bool get_auto_suspend_flag();
extern void clean_auto_suspend_flag();
extern void register_edr_init_handle(u8 mode);

extern void ble_register_init_config(void (*handler)(void));
extern void register_stereo_init_handle();

/*******************************************************************/
/*
 *-------------------LE READ PARAMETER
 */
void ble_controller_suspend(void);

void ble_controller_resume(void);


extern void bt_stack_init(void (*resume_handle)(void));
extern void bt_mode_init();
extern void controller_mode_init();

extern void bt_close_eninv();
extern void bt_open_eninv();

extern void btrf_int(u8 ble_bredr_mode,  u8 lmp_work_mode);
extern void btrf_close(u8 ble_bredr_mode, u8 clk_flag);
extern void bt_osc_internal_cfg(u8 sel_l, u8 sel_r);

/**********************tws*******************/
extern void register_tws_lmp_init_handle(u8 enble);
extern void register_tws_hci_handle(u8 enble, void *a2dp_mem, u16 mem_len);
extern void user_tws_linein_int_handle(u8 tws_linein_en, u16 rate, void *buf, u32 len, int (*set_tws_linein_state)(u8 state), void (*tws_linein_aux_open_callback_t)(void));
extern void register_tws_linein_hci_handle(u8 tws_linein_en, u16 rate, void *buf, u32 len, int (*set_tws_linein_state)(u8 state), void (*tws_linein_aux_open_callback_t)(void));


#endif
