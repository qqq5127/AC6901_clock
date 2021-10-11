/*********************************************************************************************
    *   Filename        : le_server_module.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  : 2017-01-17 11:14

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

// *****************************************************************************
/* EXAMPLE_START(le_counter): LE Peripheral - Heartbeat Counter over GATT
 *
 * @text All newer operating systems provide GATT Client functionality.
 * The LE Counter examples demonstrates how to specify a minimal GATT Database
 * with a custom GATT Service and a custom Characteristic that sends periodic
 * notifications.
 */
// *****************************************************************************

#include <sys_timer.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "uart.h"
#include "ble_api.h"
#include "circular_buf.h"
#include "semlock.h"
#include "power_manage_api.h"
#include "att_send.h"
#include "msg.h"

#if (BLE_BREDR_MODE&BT_BLE_EN)
#if (BLE_GAP_ROLE == 0)
#include "le_server_module.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_const")
#pragma code_seg(".ble_app_code")
#endif

#define DEBUG_ENABLE
#include "debug_log.h"



/* @section Main Application Setup
 *
 * @text Listing MainConfiguration shows main application code.
 * It initializes L2CAP, the Security Manager and configures the ATT Server with the pre-compiled
 * ATT Database generated from $le_counter.gatt$. Finally, it configures the advertisements
 * and the heartbeat handler and boots the Bluetooth stack.
 * In this example, the Advertisement contains the Flags attribute and the device name.
 * The flag 0x06 indicates: LE General Discoverable Mode and BR/EDR not supported.
 */
#define VENDOR_DISC_TIMEOUT_REASON     (0xa0)

enum {
    SER_NULL = 0,
    SER_DIS_CONNECT,//1
    SER_ADV_ENABLE,
    SER_ADV_DISABLE,
    SER_LATENCY_ENABLE,
    SER_LATENCY_DISABLE,//5
    SER_CHECK_CONNECTION_UPDATE,
    SER_REPORT_DISCONN_STATE,//7
    SER_SET_RSP_DATA,
    SER_DIS_CONNECT_TIMEOUT,//
};
#define ACT_SET_SER_CMD(a)       {mini_cbuf_write(&act_cmd_mctl,a); server_thread_resume();}

#define CMD_LIST_SIZE  16
static u8 act_cmd_list[CMD_LIST_SIZE];
static mini_cbuf_t act_cmd_mctl;


//缓存等待传输参数
/* static timer_source_t heartbeat; */
static hci_con_handle_t con_handle;


// Complete Local Name  默认的蓝牙名字

//加密设置
static const uint8_t sm_min_key_size = 7;
static const uint8_t sm_encryption_flag = 0; ///0--not encrypt, 1--encrypt

//连接参数设置
static const uint8_t connection_update_enable = 1; ///0--disable, 1--enable
static uint8_t connection_update_cnt = 0; ///0--disable, 1--enable
static const struct conn_update_param_t connection_param_table[] = {
    {16, 24, 0, 600},//11
    {12, 28, 0, 600},//3.7
    {8,  20, 0,  600},
    /* {12, 28, 4, 600},//3.7 */
    /* {12, 24, 30, 600},//3.05 */
};
#define CONN_PARAM_TABLE_CNT      (sizeof(connection_param_table)/sizeof(struct conn_update_param_t))

#define ADV_INTERVAL_MIN          160
#define ADV_INTERVAL_MAX          160


// 广播包内容
static u8 adv_data[ADV_RSP_PACKET_MAX];//max is 31
// scan_rsp 内容
static u8 scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31

static u8 test_read_write_buf[8] = "01234567";
static u8 ble_test_flag = 0;
static char gap_device_name[BT_NAME_LEN_MAX];
static u8 gap_device_name_len = 0;
static u8 adv_enable_state;
static u8 ble_work_state = 0;

static void (*app_recieve_callback)(void *priv, void *buf, u16 len) = NULL;
static void (*app_ble_state_callback)(void *priv, ble_state_e state) = NULL;
static u32 channel_priv;
static volatile u32 test_record_data_cnt;
static volatile u32 index_record = 0;
//广播参数设置
static void advertisements_setup_init();

//设置广播使能接口
/* void gap_advertisements_enable(int enabled); */

//建立连接后的连接参数设置
static void send_request_connect_parameter(u8 table_index);

//发送连接命令后的反馈事件
static void connection_update_complete_success(u8 *packet);

//断开连接
uint8_t gap_disconnect(hci_con_handle_t handle);

static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

static void app_write_revieve_data(u16 handle, u8 *data, u16 len);
static int app_send_user_data(u16 handle, u8 *data, u16 len);
static int app_send_user_data_do(void *priv, u8 *data, u16 len);
static int set_adv_enable(void *priv, u32 en);
static int set_latency_enable(void *priv, u32 en);
static void server_thread_resume(void);
//----------------------------------------------------------------------------------------------------------------------------------------
static void send_request_connect_parameter(u8 table_index)
{
    struct conn_update_param_t *param = (void *)&connection_param_table[table_index];

    log_info("update_request:-%d-%d-%d-%d-\n", param->interval_min, param->interval_max, param->latency, param->timeout);
    if (con_handle) {
        gap_request_connection_parameter_update(con_handle, param->interval_min, param->interval_max, param->latency, param->timeout);
    }
}

static void check_connetion_updata_deal(void)
{
    if (connection_update_enable) {
        if (connection_update_cnt < CONN_PARAM_TABLE_CNT) {
            send_request_connect_parameter(connection_update_cnt);
        }
    }
}

static void connection_update_complete_success(u8 *packet)
{
    int con_handle, conn_interval, conn_latency, conn_timeout;

    con_handle = hci_subevent_le_connection_update_complete_get_connection_handle(packet);
    conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
    conn_latency = hci_subevent_le_connection_update_complete_get_conn_latency(packet);
    conn_timeout = hci_subevent_le_connection_update_complete_get_supervision_timeout(packet);

    log_info("conn_interval = %d\n", conn_interval);
    log_info("conn_latency = %d\n", conn_latency);
    log_info("conn_timeout = %d\n", conn_timeout);
}


static void set_ble_work_state(ble_state_e state)
{
    if (state != ble_work_state) {
        log_info("ble_work_st:%x->%x\n", ble_work_state, state);
        ble_work_state = state;
        if (app_ble_state_callback) {
            app_ble_state_callback((void *)channel_priv, state);
        }
    }
}

static ble_state_e get_ble_work_state(void)
{
    return ble_work_state;
}


//return
//0--confirm and pair ok
//others--not confirm,pair fail
static int cbk_sm_just_packet_control(uint8_t packet_type, uint8_t *packet, uint16_t size)
{
    uint8_t msg;
    u32 tmp32;
    sm_just_event_t *event = (void *)packet;
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (packet[0]) {
        case SM_EVENT_JUST_WORKS_REQUEST: {
            log_info("remote addr_type %d,address:", event->addr_type);
            log_info_hexdump(event->address, 6);
            log_info("\n");
        }
        log_info("Just Works Confirmed.\n");
        break;

        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            log_info("Passkey display: %06u.\n", event->passkey);
            break;
        }
        break;
    }

    return 0;
}


/*
 * @section Heartbeat Handler
 *
 * @text The heartbeat handler updates the value of the single Characteristic provided in this example,
 * and request a ATT_EVENT_CAN_SEND_NOW to send a notification if enabled see Listing heartbeat.
 */


/* LISTING_END */

/*
 * @section Packet Handler
 *
 * @text The packet handler is used to:
 *        - stop the counter after a disconnect
 *        - send a notification when the requested ATT_EVENT_CAN_SEND_NOW is received
 */

/* LISTING_START(packetHandler): Packet Handler */
static void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    int mtu;
    u32 tmp;

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {

        /* case DAEMON_EVENT_HCI_PACKET_SENT: */
        /* putchar('E'); */
        /* break; */

        case ATT_EVENT_CAN_SEND_NOW:
            /* log_info("ATT_EVENT_CAN_SEND_NOW\n"); */
            /* putchar('N'); */
            att_wakeup_send_process();
            break;

        case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
            log_info("ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE\n");
            att_wakeup_send_process();
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: {
                con_handle = little_endian_read_16(packet, 4);
                log_info("HCI_SUBEVENT_LE_CONNECTION_COMPLETE : %0x\n", con_handle);
                connection_update_complete_success(packet + 8);
                att_send_init(con_handle);
                set_ble_work_state(BLE_ST_CONNECT);
                test_record_data_cnt = 0;
                index_record = 0;
            }
            break;

            case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                connection_update_complete_success(packet);
                break;
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            log_info("---ble disconnect!\n");
            ble_test_flag = 0;
            con_handle = 0;
            att_send_init(con_handle);
            set_ble_work_state(BLE_ST_IDLE);
            /* gap_advertisements_enable(1); */
            printf("HCI_EVENT_DISCONNECTION_COMPLETE : %0x\n", packet[5]);
            set_adv_enable(0, 1);
            connection_update_cnt = 0;
            break;


        case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
            mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
            log_info("ATT MTU = %u\n", mtu);
            att_send_set_mtu_size(mtu);
            break;

        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                break;
            }
            log_info("init complete\n");
            set_ble_work_state(BLE_ST_INIT_OK);
            set_adv_enable(0, 1);
            break;

        case HCI_EVENT_VENDOR_REMOTE_TEST:
            log_info("--- HCI_EVENT_VENDOR_REMOTE_TEST\n");
            ble_test_flag = packet[1];
            break;

        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            tmp = little_endian_read_16(packet, 4);
            log_info("-update_rsp: %02x\n", tmp);
            if (tmp) {
                connection_update_cnt++;
                log_info("remoter reject!!!\n");
                check_connetion_updata_deal();
            } else {
                connection_update_cnt = CONN_PARAM_TABLE_CNT;
            }
            break;
        }
        break;
    }
}


/* LISTING_END */

/*
 * @section ATT Read
 *
 * @text The ATT Server handles all reads to constant data. For dynamic data like the custom characteristic, the registered
 * att_read_callback is called. To handle long characteristics and long reads, the att_read_callback is first called
 * with buffer == NULL, to request the total value length. Then it will be called again requesting a chunk of the value.
 * See Listing attRead.
 */

/* LISTING_START(attRead): ATT Read */

// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param offset defines start of attribute value
static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{

    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;

    /* printf("READ Callback, handle %04x, offset %u, buffer size %u\n", handle, offset, buffer_size); */

    log_info("read_callback, handle= 0x%04x\n", handle);

    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        att_value_len = strlen(gap_device_name);

        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &gap_device_name[offset], buffer_size);
            att_value_len = buffer_size;
            log_info("\n------read gap_name:");
        }
        break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        att_value_len = sizeof(test_read_write_buf);
        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &test_read_write_buf[offset], buffer_size);
            att_value_len = buffer_size;
        }
        break;


    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
        buffer[0] = att_get_ccc_config(handle);
        buffer[1] = 0;
        att_value_len = 2;
        break;

    default:
        break;
    }

    return att_value_len;

}


/* LISTING_END */
/*
 * @section ATT Write
 *
 * @text The only valid ATT write in this example is to the Client Characteristic Configuration, which configures notification
 * and indication. If the ATT handle matches the client configuration handle, the new configuration value is stored and used
 * in the heartbeat handler to decide if a new value should be sent. See Listing attWrite.
 */

/* LISTING_START(attWrite): ATT Write */
static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int result = 0;
    u16 tmp16;

    u16 handle = att_handle;

    /* printf("WRITE Callback, handle %04x, mode %u, offset %u, data: ", handle, transaction_mode, offset); */

    /* log_info("write_callback, handle= 0x%04x\n", handle); */

    switch (handle) {

    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        tmp16 = BT_NAME_LEN_MAX;
        if ((offset >= tmp16) || (offset + buffer_size) > tmp16) {
            break;
        }

        if (offset == 0) {
            memset(gap_device_name, 0x00, BT_NAME_LEN_MAX);
        }
        memcpy(&gap_device_name[offset], buffer, buffer_size);
        log_info("\n------write gap_name:");
        break;

    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
        set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
        check_connetion_updata_deal();
        log_info("\n------write ccc:%04x, %02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);
        att_wakeup_send_process();
        break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        tmp16 = sizeof(test_read_write_buf);
        if ((offset >= tmp16) || (offset + buffer_size) > tmp16) {
            break;
        }
        memcpy(&test_read_write_buf[offset], buffer, buffer_size);
        break;

    case ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE:
    case ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE:
        app_write_revieve_data(handle, buffer, buffer_size);
        break;

    default:
        break;
    }

    return 0;
}

static int app_send_user_data(u16 handle, u8 *data, u16 len)
{
    u32 send_len = user_data_cbuf_is_write_able(len);
    u32 ret = APP_BLE_NO_ERROR;

    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (send_len) {
        //printf("v_l= %d\n",send_len);
        ret = user_data_att_send(handle, data, len, ATT_OP_AUTO_READ_CCC);
    } else {
        putchar('m');
        ret = APP_BLE_BUFF_FULL;
    }
    return ret;
}

static void app_write_revieve_data(u16 handle, u8 *data, u16 len)
{
    log_info("write_hdl %02x -recieve(%d):", handle, len);
    /* printf_buf(data,len); */

    if (handle == ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE) {
#if BLE_ATT_SEND_TEST
        test_record_data_cnt += len;
        u32 index_id = little_endian_read_32(data, 0);
        log_info("test_record_data_cnt= %u, id= %u\n ", test_record_data_cnt, index_id);
        if (index_record != index_id) {
            log_info(" ddd_data: %u--%u\n", index_record, index_id);
        }
        index_record = index_id + 1;

#else
        if (app_recieve_callback) {
            app_recieve_callback((void *)channel_priv, data, len);
        }
        /* app_send_user_data(ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, data, len);//for test */
#endif
    } else if (handle == ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE) {
        /* app_send_user_data(ATT_CHARACTERISTIC_ae04_01_VALUE_HANDLE, data, len);//for test */
        /* app_send_user_data(ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE, data, len);//for test */
    }
}
//------------------------------------------------------
static int make_set_adv_data(void)
{
    u8 offset = 0;
    u8 *buf = adv_data;

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 6, 1);
    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0x1812, 2);

    if (offset > sizeof(adv_data)) {
        puts("***adv_data overflow!!!!!!\n");
        return -1;
    }
    /* printf_buf(adv_data,offset); */
    gap_advertisements_set_data(offset, (uint8_t *)adv_data);
    return 0;
}

static const char user_tag_string[] = { EIR_TAG_STRING};

static int make_set_rsp_data(void)
{
    u8 offset = 0;
    u8 *buf = scan_rsp_data;

    u8  tag_len = sizeof(user_tag_string);
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA, (void *)user_tag_string, tag_len);

    u8 name_len = strlen(gap_device_name);
    u8 vaild_len = ADV_RSP_PACKET_MAX - (offset + 2);
    if (name_len > vaild_len) {
        name_len = vaild_len;
    }
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)gap_device_name, name_len);

    if (offset > sizeof(scan_rsp_data)) {
        puts("***rsp_data overflow!!!!!!\n");
        return -1;
    }
    /* printf_buf(scan_rsp_data,offset); */
    gap_scan_response_set_data(offset, (uint8_t *)scan_rsp_data);
    return 0;
}

//广播参数设置
static void advertisements_setup_init()
{
    uint16_t adv_int_min = ADV_INTERVAL_MIN;
    uint16_t adv_int_max = ADV_INTERVAL_MAX;

    uint8_t adv_type = 0;
    uint8_t null_addr[6] = {0, 0, 0, 0, 0, 0};
    int   ret = 0;
    gap_device_name_len = get_ble_local_name((u8 *)gap_device_name);

    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);

    ret |= make_set_adv_data();
    ret |= make_set_rsp_data();

    if (ret) {
        puts("advertisements_setup_init fail !!!!!!\n");
        return;
    }

}

//-----------------------------------------------
static void server_thread_resume(void)
{
    att_set_request_thread_deal();
}

static void server_thread_process(void)
{
    u8 act_cmd;

    if (FALSE == mini_cbuf_read(&act_cmd_mctl, &act_cmd)) {
        return;
    }

    log_info("ACT:%02x\n", act_cmd);

    switch (act_cmd) {
    case SER_DIS_CONNECT_TIMEOUT:
        if (con_handle) {
            gap_disconnect_ext(con_handle, VENDOR_DISC_TIMEOUT_REASON); //for timeout disconn,error_id
        }
        break;

    case SER_DIS_CONNECT:
        //disconnect
        if (con_handle) {
            if (1) { //(check_acl_pakcet_sent_num())
                gap_disconnect(con_handle);
            } else {
                //not clear flag,set timer do it
                //sys_timer_register(&mi_server_timer, 20, mi_server_timeout_handler, 0);
            }
        }
        break;

    case SER_ADV_ENABLE:
        /* adv_arguments_set_up(); */
        gap_advertisements_enable(1);
        break;

    case SER_ADV_DISABLE:
        gap_advertisements_enable(0);
        break;

    case SER_LATENCY_ENABLE:
        ble_set_latency_enable(con_handle, 1);
        break;

    case SER_LATENCY_DISABLE:
        ble_set_latency_enable(con_handle, 0);
        break;

    case SER_CHECK_CONNECTION_UPDATE:
        check_connetion_updata_deal();
        break;

    case SER_REPORT_DISCONN_STATE:
        if ((!con_handle) && (get_ble_work_state() != BLE_ST_ADV)) {
            set_ble_work_state(BLE_ST_IDLE);
        }
        break;

    case SER_SET_RSP_DATA:
        make_set_rsp_data();
        break;

    default:
        log_info("unknow server act_cmd!\n");
        break;
    }

    if (!mini_cbuf_is_emtpy(&act_cmd_mctl)) {
        server_thread_resume();
    }
}

extern void get_random_number(u8 *ptr, u8 len);
extern void reset_PK_cb_register(void (*reset_pk)(u32 *));

//重设passkey回调函数，在这里可以重新设置passkey
//passkey为6个数字组成，十万位、万位。。。。个位 各表示一个数字 高位不够为0
static void reset_passkey_cb(u32 *key)
{
#if 1
    u32 newkey = 0;
    get_random_number((u8 *)&newkey, 4); //获取随机数

    newkey &= 0xfffff;
    if (newkey > 999999) {
        newkey = newkey - 999999; //不能大于999999
    }

    /* printf("old key=%06u\n",*key); */
    *key = newkey; //小于或等于六位数
    printf("set new_key= %06u\n", *key);
#else
    *key = 123456; //for debug
#endif
}

//need must
void ble_stack_config_init(void)
{
    u8 io_capability_mode = IO_CAPABILITY_NO_INPUT_NO_OUTPUT;
    connection_update_cnt = 0;
    set_ble_work_state(BLE_ST_NULL);
    ble_sm_setup_init(io_capability_mode, SM_AUTHREQ_BONDING, sm_min_key_size, sm_encryption_flag);
    ble_att_server_setup_init(profile_data, att_read_callback, att_write_callback);

    if (io_capability_mode == IO_CAPABILITY_DISPLAY_ONLY) {
        //need input PIN CODE
        reset_PK_cb_register(reset_passkey_cb); //注册回调
    }

    advertisements_setup_init();
    ble_cbk_handler_register(cbk_packet_handler, cbk_sm_just_packet_control);
    mini_cbuf_init(&act_cmd_mctl, act_cmd_list, CMD_LIST_SIZE);
    att_regist_thread_deal_cbk(server_thread_process);
}

static int set_adv_enable(void *priv, u32 en)
{
    ble_state_e next_state, cur_state;

    if (con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (en) {
        next_state = BLE_ST_ADV;
    } else {
        next_state = BLE_ST_IDLE;
    }

    cur_state =  get_ble_work_state();
    switch (cur_state) {
    case BLE_ST_ADV:
    case BLE_ST_IDLE:
    case BLE_ST_INIT_OK:
        break;
    default:
        return APP_BLE_OPERATION_ERROR;
        break;
    }

    if (cur_state == next_state) {
        return APP_BLE_NO_ERROR;
    }
    log_info("adv_en:%d\n", en);
    set_ble_work_state(next_state);
    if (en) {
        ACT_SET_SER_CMD(SER_ADV_ENABLE);
    } else {
        ACT_SET_SER_CMD(SER_ADV_DISABLE);
    }
    return APP_BLE_NO_ERROR;
}

static int set_latency_enable(void *priv, u32 en)
{
    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }
    if (en) {
        ACT_SET_SER_CMD(SER_LATENCY_ENABLE);
    } else {
        ACT_SET_SER_CMD(SER_LATENCY_DISABLE);
    }
    return APP_BLE_NO_ERROR;
}

static int ble_disconnect(void *priv)
{
    if (con_handle) {
        set_ble_work_state(BLE_ST_SEND_DISCONN);
        ACT_SET_SER_CMD(SER_DIS_CONNECT);
        return APP_BLE_NO_ERROR;
    } else {
        return APP_BLE_OPERATION_ERROR;
    }
}

static int get_buffer_vaild_len(void *priv)
{
    int vaild_len = 0;
    if (!con_handle) {
        return 0;
    }

    vaild_len = (int)user_data_cbuf_is_write_able(0);
    /* printf("v_len= %d\n",vaild_len); */
    return vaild_len;
}

static int app_send_user_data_do(void *priv, u8 *data, u16 len)
{
    return app_send_user_data(ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, data, len);
}


static int regiest_wakeup_send(void *priv, void *cbk)
{
    att_regist_wakeup_send(cbk);
    return APP_BLE_NO_ERROR;
}

static int regiest_recieve_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_recieve_callback = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_state_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_ble_state_callback = cbk;
    return APP_BLE_NO_ERROR;
}




static const struct ble_server_operation_t mi_ble_operation = {
    .adv_enable = set_adv_enable,
    .disconnect = ble_disconnect,
    .get_buffer_vaild = get_buffer_vaild_len,
    .send_data = (void *)app_send_user_data_do,
    .regist_wakeup_send = regiest_wakeup_send,
    .regist_recieve_cbk = regiest_recieve_cbk,
    .regist_state_cbk = regiest_state_cbk,
    .latency_enable = set_latency_enable,
};


void ble_get_server_operation_table(struct ble_server_operation_t **interface_pt)
{
    *interface_pt = (void *)&mi_ble_operation;
}

void ble_server_send_test_key_num(u8 key_num)
{
    if (ble_test_flag) {
        if (!con_handle) {
            return;
        }

        extern uint8_t gap_user_send_key_num(hci_con_handle_t handle, u8 num);
        gap_user_send_key_num(con_handle, key_num);
    }
}

bool ble_msg_deal(u32 param)
{
    struct ble_server_operation_t *test_opt;
    ble_get_server_operation_table(&test_opt);
    static u8 latency_sw = 0;

#if 0//for test key
    switch (param) {
    case MSG_BT_PP:
        test_opt->disconnect(0);
        break;

    case MSG_BT_NEXT_FILE:
        latency_sw = !latency_sw;
        test_opt->latency_enable(0, latency_sw);
        printf("latency_sw= %d\n", latency_sw);
        break;

    case MSG_HALF_SECOND:
        /* putchar('H'); */
        break;

    default:
        break;
    }
#endif

#if BLE_ATT_SEND_TEST
    {
        static u8 test_client_step = 0;
        static u8 start_send_flag = 0;
        static u32 index_cnt = 0;

        if (param == MSG_BT_PREV_FILE) {
            start_send_flag = !start_send_flag;
            log_info("test_record_data_cnt= %u\n ", test_record_data_cnt);
            if (start_send_flag) {
                test_record_data_cnt = 0;
            }
            return FALSE;
        }

        if (BLE_ST_NOTIFY_IDICATE != get_ble_work_state()) {
            test_client_step = 1;
            test_record_data_cnt = 0;
            index_cnt = 0;
            return FALSE;
        }

        if (!start_send_flag) {
            return FALSE;
        }

        while (1) {
            u32 send_len = test_opt->get_buffer_vaild(0);
            if (send_len >= 4) {

                if (send_len > 248) {
                    send_len = 248;
                }
                if (APP_BLE_NO_ERROR == test_opt->send_data(0, (void *)&index_cnt, send_len)) {
                    index_cnt++;
                    test_record_data_cnt += send_len;
                } else {
                    log_info("send_data fail!!!\n");
                    break;
                }
            } else {
                break;
            }
        }

    }
#endif
    return FALSE;
}

#endif

#endif

