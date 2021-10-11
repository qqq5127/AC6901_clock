#include <sys_timer.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "uart.h"
#include "ble_api.h"
#include "circular_buf.h"
#include "semlock.h"
#include "att_send.h"
#include "power_manage_api.h"
#include "msg.h"

#define DEBUG_ENABLE
#include "debug_log.h"

#if (BLE_BREDR_MODE&BT_BLE_EN)

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_const")
#pragma code_seg(".ble_app_code")
#endif

#define  EMPTY_SIZE_CALLBACK_SEND   (210)

//send lock
static semlock_t send_lock;
static struct sys_timer att_send_timer;

#if BLE_ATT_SEND_TEST
static struct sys_timer att_test_timer;
static volatile att_send_record_cnt;
#endif

static u16 att_conn_handle;
static u16 att_payload_size;

//缓存等待传输参数
static u8 att_ready_send_type;
static u16 att_ready_send_len;
static u16 att_ready_send_handle;
static u8 *att_ready_send_buf;

static u8   att_send_data[256] __attribute__((aligned(4)));//need >= 20
static u8   att_wakeup_send_flag;

void (*att_wakeup_send_callback)(void) = NULL;
static btstack_packet_handler_t  client_event_callback = NULL;

#define ATT_THREAD_DEAL_MAX   4
u32 att_thread_deal_callback[ATT_THREAD_DEAL_MAX] = {0};

#define USER_BUF_LEN          (512*4)      //发送buf大小，可根据需求修改
cbuffer_t user_send_cbuf;
static u8 user_data_buf[USER_BUF_LEN] __attribute__((aligned(4)));

#define  SERVER_CCC_CFG_MAX  6
typedef struct {
    uint16_t handle;
    uint16_t cfg;
} server_ccc_cfg_t;

static server_ccc_cfg_t client_configuration[SERVER_CCC_CFG_MAX];

//--------------------------------------------------------------------------------------------------------
static void att_send_schedule(void);
static int att_operation_cmd_send(u8 send_type_flag, u16 send_handle, u8 *buf, u16 len);
extern void putchar(char a);



//-----------------------------------------------------------------------------------------

//---------------------
void mini_cbuf_init(mini_cbuf_t *mcbuf, u8 *buf, u8 buf_size)
{
    CPU_INT_DIS();
    memset(mcbuf, 0, sizeof(mini_cbuf_t));
    mcbuf->buf = buf;
    mcbuf->buf_size = buf_size;
    CPU_INT_EN();
}

tbool mini_cbuf_read(mini_cbuf_t *mcbuf, u8 *value_pt)
{
    tbool ret = TRUE;

    if (!mcbuf->buf) {
        return FALSE;
    }

    CPU_INT_DIS();
    if (mcbuf->count) {
        *value_pt = mcbuf->buf[mcbuf->read_pt++];
        if (mcbuf->read_pt >= mcbuf->buf_size) {
            mcbuf->read_pt = 0;
        }
        mcbuf->count--;
    } else {
        ret =  FALSE;
    }
    CPU_INT_EN();
    return ret;
}

tbool mini_cbuf_write(mini_cbuf_t *mcbuf, u8 value)
{
    tbool ret = TRUE;

    if (!mcbuf->buf) {
        printf("m_buf not init!!!\n");
        return FALSE;
    }

    CPU_INT_DIS();
    if (mcbuf->count < mcbuf->buf_size) {
        mcbuf->count++;
        mcbuf->buf[mcbuf->write_pt++] = value;
        if (mcbuf->write_pt >= mcbuf->buf_size) {
            mcbuf->write_pt = 0;
        }
    } else {
        printf("\n-m_cbuf is full!!!\n");
        ret = FALSE;
    }
    CPU_INT_EN();
    return ret;
}

tbool mini_cbuf_is_emtpy(mini_cbuf_t *mcbuf)
{
    if (!mcbuf->buf) {
        return TRUE;
    }
    return (!mcbuf->count);
}
//---------------------


//--------------------------------------------------------------------------------------------------------
static void att_test_timeout_handler(struct sys_timer *ts)
{
#if BLE_ATT_SEND_TEST
    static volatile u32 calc_speed_val = 0;

    if (att_conn_handle) {
        sys_timer_register(&att_test_timer, 1000, att_test_timeout_handler, 0);
    }

    calc_speed_val = att_send_record_cnt - calc_speed_val;
    if (calc_speed_val) {
        log_info("\n-att_speed= %u Bs\n", calc_speed_val);
    }
    calc_speed_val = att_send_record_cnt;
#endif
}

//--------------------------------------------------------------------------------------------------------
void att_ccc_config_init(void)
{
    memset(client_configuration, 0, sizeof(client_configuration));
}

//---------------------
void att_set_ccc_config(uint16_t handle, uint16_t cfg)
{
    int cnt;
    uint16_t find_handle;
    server_ccc_cfg_t *pt = client_configuration;

    if (cfg != 0) {
        find_handle = 0;
    } else {
        find_handle = handle;
        handle = 0;
    }

    for (cnt = 0; cnt < SERVER_CCC_CFG_MAX; cnt++) {
        if (find_handle == pt->handle) {
            pt->handle = handle;
            pt->cfg = cfg;
            break;
        }
        pt++;
    }
}

//---------------------
uint16_t att_get_ccc_config(uint16_t handle)
{
    int cnt;
    server_ccc_cfg_t *pt = client_configuration;
    for (cnt = 0; cnt < SERVER_CCC_CFG_MAX; cnt++) {
        if (handle == pt->handle) {
            return pt->cfg;
        }
        pt++;
    }
    return 0;
}

//---------------------
void att_request_can_send_now_event(void)
{
#if BLE_GAP_ROLE
    gatt_client_request_can_send_now_event(att_conn_handle);
#else
    att_server_request_can_send_now_event(att_conn_handle);
#endif
}

//---------------------
void att_regist_wakeup_send(void *cbk)
{
    att_wakeup_send_callback = cbk;
}
//---------------------
static void att_request_send_handle(void)
{
    /* putchar('Q'); */
    att_wakeup_send_flag = 0;
    att_send_schedule();
}

static void att_thread_deal(void)
{
    u32 i;
    void (*thread_deal_callback)(void);

    for (i = 0; i < ATT_THREAD_DEAL_MAX; i++) {
        thread_deal_callback = (void *)att_thread_deal_callback[i];
        if (thread_deal_callback) {
            thread_deal_callback();
        } else {
            break;
        }
    }

    if (att_wakeup_send_flag) {
        att_wakeup_send_flag = 0;
        att_request_send_handle();
    }
}

void att_set_request_thread_deal(void)
{
    ble_user_ioctrl(MSG_BLE_RESUME_THREAD, (int)att_thread_deal);
}

void att_regist_thread_deal_cbk(void *cbk)
{
    u32 i;

    CPU_INT_DIS();
    for (i = 0; i < ATT_THREAD_DEAL_MAX; i++) {
        if (!att_thread_deal_callback[i]) {
            break;
        }
        if (att_thread_deal_callback[i] == (u32)cbk) {
            break;
        }
    }
    if (i >= ATT_THREAD_DEAL_MAX) {
        printf("att cbk regist full!!!\n");
    } else {
        att_thread_deal_callback[i] = (u32)cbk;
    }
    CPU_INT_EN();
}

static void att_set_request_send_start(void)
{
    CPU_INT_DIS();
    if (!att_wakeup_send_flag) {
        /* putchar('K'); */
        att_wakeup_send_flag = 1;
        //用stack线程启动发送,隔离上下层的发送
        att_set_request_thread_deal();
    }
    CPU_INT_EN();
}

//---------------------
static void att_send_timeout_handler(struct sys_timer *ts)
{
    /* putchar('%'); */
    att_send_schedule();
}

//---------------------
u32 user_data_cbuf_is_write_able(u32 len)
{
    u32 buf_space, w_len;
    u16 head_size = sizeof(user_send_head_t);
    u16 pack_size = sizeof(user_send_head_t) + att_payload_size;

    if (!att_conn_handle) {
        return 0;
    }

    CPU_INT_DIS();
    buf_space = cbuf_get_space(&user_send_cbuf) - cbuf_get_data_size(&user_send_cbuf);
    if (buf_space <= head_size) {
        w_len = 0;
    } else if (buf_space < pack_size) {
        w_len = buf_space - head_size;
    } else {
        w_len = (buf_space / pack_size) * att_payload_size;
        buf_space = buf_space % pack_size;
        if (buf_space > head_size) {
            w_len += (buf_space - head_size);
        }
    }
    if (len > w_len) {
        w_len = 0;
    }
    /* log_info("ret_wlen= %d\n", w_len); */
    CPU_INT_EN();

    if (!w_len) {
        //full,resume send
        att_request_can_send_now_event();
    }
    return w_len;
}

u32 user_data_cbuf_is_null(void)
{
    return !cbuf_get_data_size(&user_send_cbuf);
}
//---------------------
static inline u32 user_data_att_send_sub(user_send_head_t *head, u8 *data)
{
    u16 wlen;
    u16 head_size = sizeof(user_send_head_t);

    if (user_data_cbuf_is_write_able(head->send_len) != 0) {
        wlen = cbuf_write(&user_send_cbuf, head, head_size);
        wlen += cbuf_write(&user_send_cbuf, data, head->send_len);

        if (wlen != head->send_len + head_size) {
            return APP_BLE_BUFF_ERROR;
        } else {
            return APP_BLE_NO_ERROR;
        }
    }
    return APP_BLE_BUFF_FULL;
}

//---------------------
u32 user_data_att_send(u16 handle, u8 *data, u16 len, u8 send_type)
{
    u16 wlen;
    u32 ret_val = APP_BLE_NO_ERROR;
    user_send_head_t send_head;
    u16 packet_cnt = 0;
    u16 request_flag = 0;

    send_head.send_handle = handle;
    //send_head.send_type = send_type;

    if (!att_conn_handle) {
        /* putchar('h'); */
        return APP_BLE_OPERATION_ERROR;
    }

    if (!len) {
        return APP_BLE_NO_ERROR;
    }

    if (send_type == ATT_OP_AUTO_READ_CCC) {
        send_head.send_type = att_get_ccc_config(handle + 1);
        if (!send_head.send_type) {
            /* putchar('2'); */
            return APP_BLE_OPERATION_ERROR;
        }
    } else {
        send_head.send_type = send_type;
    }

    /* set_power_off_lock_ext(); */

    log_info("s_len= %d\n", len);

    CPU_INT_DIS();
    if (user_data_cbuf_is_write_able(len)) {
        request_flag = 1;
        while (len > 0) {
            if (len > att_payload_size) {
                wlen = att_payload_size;
            } else {
                wlen = len;
            }
            packet_cnt++;
            send_head.send_len = wlen;
            ret_val = user_data_att_send_sub(&send_head, data);
            if (ret_val) {
                printf("err_s:wlen= %d,err= %d,p1= %d\n", wlen, ret_val, packet_cnt);
                return ret_val;
            }
            len -= wlen;
            data += wlen;
        }
    } else {
        ret_val = APP_BLE_BUFF_FULL;
    }
    /* log_info("p2= %d\n", packet_cnt); */
    if (request_flag) {
        att_set_request_send_start();
    }
    CPU_INT_EN();
    return ret_val;
}

extern u8 hci_get_acl_packet_num(u16 conn_handle);
u8 check_acl_pakcet_sent_num(void)
{
    if (0 == hci_get_acl_packet_num(att_conn_handle)) {
        return 1;
    } else {
        return 0;
    }
}

//---------------------
//return TRUE OR FALSE
static int user_att_send_streamer(void)
{
    int ret_val = 0;
    int n_packet = 0;
    user_send_head_t user_head;
    // check if we can send
    while (1) {

        if (semlock_read(&send_lock) == TRUE) {
            ret_val = 1;
            break;
        }

        CPU_INT_DIS();
        if (0 == cbuf_get_data_size(&user_send_cbuf)) {
            CPU_INT_EN();
            /* set_power_off_ulock_ext(); */
            break;
        }

        cbuf_read(&user_send_cbuf, &user_head, sizeof(user_send_head_t));
        if (user_head.send_len) {
            cbuf_read(&user_send_cbuf, att_send_data, user_head.send_len);
        }
        CPU_INT_EN();

        // send
        ret_val = att_operation_cmd_send(user_head.send_type, user_head.send_handle, att_send_data, user_head.send_len);

        if (ret_val == BTSTACK_ACL_BUFFERS_FULL) {
            /* puts("send fail\n"); */
            ret_val = 2;
            break;
        }
        n_packet++;
    }

    /* if(n_packet) */
    /* { */
    /* printf("-s_n= %d\n",n_packet); */
    /* } */

user_send_end:
    return ret_val;
}

//---------------------
static int att_operation_cmd_send(u8 send_type_flag, u16 send_handle, u8 *buf, u16 len)
{
    int ret_val;
    u32 param1;

    /* log_info("att_op_type: %d\n",send_type_flag); */

    switch (send_type_flag) {
    case ATT_OP_NOTIFY:
        ret_val = att_server_notify(att_conn_handle, send_handle, buf, len);
        break;

    case ATT_OP_INDICATE:
        ret_val = att_server_indicate(att_conn_handle, send_handle, buf, len);
        break;

    case ATT_OP_WRITE_WITHOUT_RESPOND:
        ret_val = gatt_client_write_value_of_characteristic_without_response(att_conn_handle, send_handle, len, buf);
        break;

    case ATT_OP_WRITE:
        ret_val = gatt_client_write_value_of_characteristic(client_event_callback, att_conn_handle, send_handle, len, buf);
        break;

    case ATT_OP_READ:
        ret_val = gatt_client_read_value_of_characteristic_using_value_handle(client_event_callback, att_conn_handle, send_handle);
        break;

    default:
        return 0;
        break;
    }

    /* if (ret_val) { */
    /* log_info("-ret_val: %2x\n", ret_val); */
    /* } */

    switch (ret_val) {

    case 0:
        /* putchar('&'); */
        semlock_set(&send_lock, FALSE);
        ret_val = 0;

#if BLE_ATT_SEND_TEST
        att_send_record_cnt += len;
        u32 index_id = little_endian_read_32(buf, 0);
        log_info("\n-att_s_cnt= %u, id= %u\n ", att_send_record_cnt, index_id);
#endif
        break;

    case BTSTACK_ACL_BUFFERS_FULL:
    case ATT_HANDLE_VALUE_INDICATION_IN_PORGRESS:
    case GATT_CLIENT_IN_WRONG_STATE:
    case GATT_CLIENT_BUSY:
        /* putchar('@'); */
        semlock_set(&send_lock, TRUE);
        att_ready_send_type = send_type_flag;
        att_ready_send_buf = buf;
        att_ready_send_len  = len;
        att_ready_send_handle  = send_handle;
        ret_val = BTSTACK_ACL_BUFFERS_FULL;

        sys_timer_register(&att_send_timer, 200, att_send_timeout_handler, 0);
        break;

    default:
        log_info("-unknow_ret: %2x\n", ret_val);
        semlock_set(&send_lock, FALSE);
        ret_val = 0;
        break;
    }

    return ret_val;
}


//---------------------
//check ready data wait send
static int check_att_server_ready_send(void)
{
    if (semlock_read(&send_lock) == TRUE) {
        if (att_operation_cmd_send(att_ready_send_type, att_ready_send_handle, att_ready_send_buf, att_ready_send_len)) {
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

//---------------------
static void att_send_schedule(void)
{
    static volatile u8 sending = 0;

    CPU_INT_DIS();
    if (sending) {
        //防重入
        /* putchar('B'); */
        CPU_INT_EN();
        return;
    }

    sending = 1;
    CPU_INT_EN();

    if (FALSE == check_att_server_ready_send()) {
        user_att_send_streamer();
        if (att_wakeup_send_callback && (user_data_cbuf_is_write_able(0) >= EMPTY_SIZE_CALLBACK_SEND)) {
            att_wakeup_send_callback();
        }
#if BLE_ATT_SEND_TEST
        task_post_msg(NULL, 1, MSG_BT_BLE_TEST);
#endif
    }


    att_request_can_send_now_event();


schedule_end:
    sending = 0;
}

//---------------------
void att_wakeup_send_process(void)
{
    att_send_schedule();
}
//---------------------
void att_send_set_mtu_size(u16 mtu_size)
{
    if (mtu_size > sizeof(att_send_data)) {
        att_payload_size = sizeof(att_send_data);
    } else {
        att_payload_size = mtu_size;
    }
    /* att_payload_size = ATT_DEFAULT_MTU - 3; */
    log_info("att_payload_size= %d \n", att_payload_size);
}

//---------------------
void att_send_init(u16 conn_handle)
{
    CPU_INT_DIS();
    att_conn_handle = conn_handle;
    att_ready_send_len = 0;
    att_payload_size = ATT_DEFAULT_MTU - 3;
    att_wakeup_send_flag = 0;

    att_ccc_config_init();
    semlock_set(&send_lock, FALSE);
    cbuf_init(&user_send_cbuf, user_data_buf, USER_BUF_LEN);

#if BLE_ATT_SEND_TEST
    att_send_record_cnt = 0;
    sys_timer_register(&att_test_timer, 1000, att_test_timeout_handler, 0);
#endif
    CPU_INT_EN();
}

void att_regist_client_callback(btstack_packet_handler_t  cbk)
{
    client_event_callback = cbk;
}

#endif

