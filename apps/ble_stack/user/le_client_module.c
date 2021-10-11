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
#if (BLE_GAP_ROLE == 1)

#include "le_client_module.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_const")
#pragma code_seg(".ble_app_code")
#endif

#define DEBUG_ENABLE
#include "debug_log.h"


#if 0
#define client_puts       puts
#define client_printf     printf
#define client_put_buf    put_buf
#else
#define client_puts(...)
#define client_printf(...)
#define client_put_buf(...)

#endif


typedef enum {
    TC_IDLE,
    TC_W4_SCAN_RESULT,
    TC_W4_CONNECT,
    TC_W4_ENCRYPTED_CONNECTION,

    TC_W4_SERVICE_RESULT,
    TC_W4_SERVICE_WITH_UUID_RESULT,

    TC_W4_CHARACTERISTIC_RESULT,
    TC_W4_CHARACTERISTIC_WITH_UUID_RESULT,
    TC_W4_CHARACTERISTIC_DESCRIPTOR_RESULT,

    TC_W4_INCLUDED_SERVICE_RESULT,

    TC_W4_READ_RESULT,
    TC_W4_READ_LONG_RESULT,

    TC_W2_WRITE_WITHOUT_RESPONSE,
    TC_W4_WRITE_WITHOUT_RESPONSE_SENT,

    TC_W4_WRITE_RESULT,
    TC_W4_LONG_WRITE_RESULT,
    TC_W4_RELIABLE_WRITE_RESULT,

    TC_W4_ACC_ENABLE,
    TC_W4_ACC_CLIENT_CONFIG_CHARACTERISTIC_RESULT,
    TC_W4_ACC_SUBSCRIBE,
    TC_W4_ACC_DATA,

    TC_W4_DISCONNECT,
    TC_DISCONNECTED,
    TC_CONNECTED,

} tc_state_t;


enum {
    CLI_SEARCH_PROFILE_START = 0,
    CLI_DIS_CONNECT,
    CLI_WRITE_CCC,
    CLI_SCAN_ENABLE,
    CLI_SCAN_DISABLE,

};

enum {
    CLI_CREAT_BY_ADDRESS = 0,
    CLI_CREAT_BY_NAME,
    CLI_CREAT_BY_TAG,
    CLI_CREAT_BY_LAST_SCAN,
};


#define SET_SCAN_WINDOW     16
#define SET_SCAN_INTERVAL   48


#define SET_CONN_INTERVAL_MIN   0x10
#define SET_CONN_INTERVAL_MAX   0x20
#define SET_CONN_TIMEOUT        0x180

#define CMD_LIST_SIZE  8
static u8 act_cmd_list[CMD_LIST_SIZE];
static mini_cbuf_t act_cmd_mctl;

#define ACT_SET_FLAG(a)       {mini_cbuf_write(&act_cmd_mctl,a); client_thread_resume();}

/* static u32 act_operate_control = 0; */
/* #define ACT_SET_FLAG(a)       {act_operate_control |= BIT(a); client_thread_resume();} */
/* #define ACT_CLR_FLAG(a)       act_operate_control &= (~BIT(a)) */
/* #define ACT_CHECK_FALG(a)     act_operate_control & BIT(a) */
/* #define ACT_FLAG_IS_NULL()    (!act_operate_control) */
/* #define ACT_RESET_FLAG(a)      act_operate_control = a */

static u8 ble_work_state = 0;
static void (*app_recieve_callback)(void *priv, void *buf, u16 len) = NULL;
static void (*app_ble_state_callback)(void *priv, ble_state_e state) = NULL;
static u32 channel_priv;

static volatile u32 test_record_data_cnt;
static volatile u32 index_record = 0;
//----------------------------------------------------------------------------
static hci_con_handle_t gc_handle;
//加密设置
static const uint8_t sm_min_key_size = 7;
static const uint8_t sm_encryption_flag = 0; ///0--not encrypt, 1--encrypt
//----------------------------------------------------------------------------
static void client_thread_resume(void);
static struct sys_timer client_timer;
//--------------------------------------------------------------------------------------------------------------


static uint16_t gc_handle;
static tc_state_t tc_state;

static le_service_t current_service;
static le_characteristic_t current_characteristic;
static le_characteristic_t *deal_characteristic;

static u16 client_reading_handle = 0;
static u16 service_count = 0;
static u16 service_index = 0;
static u16 characteristic_count = 0;
static u16 characteristic_index = 0;


static u16 target_services_uuid16 = 0xae00;
static u16 target_characteristic_uuid16 = 0;


#define SUPPORT_CCC_MAXNUM           3
#define SUPPORT_SERVICES_MAXNUM      1
static  u16 record_ccc_count = 0;
static  le_characteristic_t *enable_ccc_group = 0;

static le_service_t remote_services[SUPPORT_SERVICES_MAXNUM];
static le_characteristic_t remote_ccc_characteristic[SUPPORT_CCC_MAXNUM];
static gatt_client_notification_t notification_registration[SUPPORT_CCC_MAXNUM];

static void s_client_gatt_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void client_creat_connection(u8 *conn_addr, u8 addr_type);

/* static const u8 create_conn_mode = BIT(CLI_CREAT_BY_LAST_SCAN) | BIT(CLI_CREAT_BY_ADDRESS) | BIT(CLI_CREAT_BY_NAME) */
static const u8 create_conn_mode = BIT(CLI_CREAT_BY_TAG);// BIT(CLI_CREAT_BY_ADDRESS) | BIT(CLI_CREAT_BY_NAME)
static const u8 create_conn_remoter[6] = {0x11, 0x22, 0x33, 0x88, 0x88, 0x88};
static const u8 create_remoter_name[32] = "AC692x_testAa18";
static u8 adv_last_remoter_name[32];
static u8 conn_last_remoter_flag = 0;
static u8 create_conn_flag = 0;

#define CREAT_CONN_CHECK_FLAG(a)  create_conn_mode & BIT(a)

//-------------------------------------------------------------------------------
static u8 target_services_index = 0;
static const target_service_t target_services = {
    .service_uuid16 = 0xae00,
    .read_uuid16 = 0xae10,
    .write_uuid16 = 0xae01,
    .notify_uuid16 = 0xae02,
    .indicate_uuid16 = 0,
};

static target_handle_t target_handle;

//
enum {
    RECORD_NULL = 0,
    RECORD_SERVICES,
    RECORD_READ,
    RECORD_WRITE,
    RECORD_CCC,
};

#define IS_RECORD_TYPE(a,x)    (a & BIT(x))
#define SET_RECORD_TYPE(a,x)    a |= BIT(x)

#define JUST_IS_INVAIL(a)  if(a == 0)return APP_BLE_OPERATION_ERROR;
//-------------------------------------------------------------------------------
static u32 check_target_uuid_match(le_service_t *service_p, le_characteristic_t *characteristic_p)
{
    u32 record_type = RECORD_NULL;
    if (service_p->uuid16 == target_services.service_uuid16) {
        SET_RECORD_TYPE(record_type, RECORD_SERVICES);
    }

    if (characteristic_p == NULL) {
        return record_type;
    }

    if (characteristic_p->uuid16 == target_services.read_uuid16) {
        target_handle.read_handle = characteristic_p->value_handle;
        SET_RECORD_TYPE(record_type, RECORD_READ);
    }

    if (characteristic_p->uuid16 == target_services.write_uuid16) {
        target_handle.write_handle = characteristic_p->value_handle;
        SET_RECORD_TYPE(record_type, RECORD_WRITE);
    }

    if (characteristic_p->uuid16 == target_services.notify_uuid16) {
        target_handle.notify_handle = characteristic_p->value_handle;
        SET_RECORD_TYPE(record_type, RECORD_CCC);
    }

    if (characteristic_p->uuid16 == target_services.indicate_uuid16) {
        target_handle.indicate_handle = characteristic_p->value_handle;
        SET_RECORD_TYPE(record_type, RECORD_CCC);
    }

    return record_type;
}


// TEST CODE
static void printUUID(uint8_t *uuid128, uint16_t uuid16)
{
    client_printf(", uuid ");
    if (uuid16) {
        client_printf(" 0x%02x", uuid16);
    } else {
        client_put_buf(uuid128, 8);
    }
    client_printf("\n");
}

static void dump_characteristic(le_characteristic_t *characteristic)
{
    client_printf("    *** characteristic *** properties %x, start handle 0x%02x, value handle 0x%02x, end handle 0x%02x",
                  characteristic->properties, characteristic->start_handle, characteristic->value_handle, characteristic->end_handle);
    printUUID(characteristic->uuid128, characteristic->uuid16);
}

static void dump_service(le_service_t *service)
{
    client_printf("    *** service *** start group handle %02x, end group handle %02x", service->start_group_handle, service->end_group_handle);
    printUUID(service->uuid128, service->uuid16);
}

// general swap/endianess utils
static void swapXX(const uint8_t *src, uint8_t *dst, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        dst[len - 1 - i] = src[i];
    }
}

//------------------------------------------------------------
static void client_timer_handle(struct sys_timer *ts)
{
    sys_timer_register(&client_timer, 20, client_timer_handle, 1);
}

//------------------------------------------------------------
static void set_ble_work_state(ble_state_e state)
{
    if (state != ble_work_state) {
        log_info("ble_client_work_st:%x->%x\n", ble_work_state, state);
        ble_work_state = state;
        if (app_ble_state_callback) {
            app_ble_state_callback((void *)channel_priv, state);
        }
    }
}
//------------------------------------------------------------
static ble_state_e get_ble_work_state(void)
{
    return ble_work_state;
}

//------------------------------------------------------------
static const char user_tag_string[] = { EIR_TAG_STRING};
static void client_report_adv_data(adv_report_t *report_pt, u16 len)
{
    u8 i, lenght, ad_type;
    u8 *adv_data_pt;
    u8 find_remoter = 0;
    u8 tmp_addr[6];
    u32 tmp32;
    /* printf("event_type,addr_type: %x,%x; ",report_pt->event_type,report_pt->address_type); */
    /* put_buf(report_pt->address,6); */
    /* puts("\n"); */

    /* puts("adv_data_display:"); */
    /* put_buf(adv_pt->data,adv_pt->length); */

    if (CREAT_CONN_CHECK_FLAG(CLI_CREAT_BY_ADDRESS)) {
        swapXX(create_conn_remoter, tmp_addr, 6);
        if (0 == memcmp(tmp_addr, report_pt->address, 6)) {
            find_remoter = 1;
        }
    }

    adv_data_pt = report_pt->data;
    for (i = 0; i < report_pt->length;) {
        if (*adv_data_pt == 0) {
            /* puts("analyze end\n"); */
            break;
        }

        lenght = *adv_data_pt++;
        ad_type = *adv_data_pt++;

        i += (lenght + 1);

        switch (ad_type) {
        case 1:
            /* puts("flags:"); */
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            /* puts("service uuid:"); */
            /* client_put_buf(adv_data_pt, lenght - 1); */
            /* client_puts("\n"); */
            break;
        case 8:
        case 9:

            tmp32 = adv_data_pt[lenght - 1];
            adv_data_pt[lenght - 1] = 0;;
            log_info("local name: %s\n", adv_data_pt);

            if (CREAT_CONN_CHECK_FLAG(CLI_CREAT_BY_LAST_SCAN)) {
                if (!conn_last_remoter_flag) {
                    memcpy(adv_last_remoter_name, adv_data_pt, lenght);
                } else {
                    if (0 == memcmp(adv_last_remoter_name, adv_data_pt, strlen((void *)adv_last_remoter_name))) {
                        find_remoter = 1;
                        conn_last_remoter_flag = 0;
                    }
                }
            }
            adv_data_pt[lenght - 1] = tmp32;

            if (CREAT_CONN_CHECK_FLAG(CLI_CREAT_BY_NAME)) {
                if (0 == memcmp(create_remoter_name, adv_data_pt, strlen((void *)create_remoter_name))) {
                    find_remoter = 1;
                }

            }
            break;

        case 0xff:
            if (CREAT_CONN_CHECK_FLAG(CLI_CREAT_BY_TAG)) {
                if (memcmp(adv_data_pt, (void *)user_tag_string, sizeof(user_tag_string)) == 0) {
                    log_info("get_tag_string!\n");
                    find_remoter = 1;
                }
            }
            break;

        default:
            /* puts("unknow ad_type:"); */
            break;
        }

        if (find_remoter) {
            client_put_buf(adv_data_pt, lenght - 1);
        }
        adv_data_pt += (lenght - 1);
    }


    if (find_remoter) {
        if (create_conn_flag) {
            puts("\n*********creat_connection***********\n");
            printf("***remote type %d,addr:", report_pt->address_type);
            put_buf(report_pt->address, 6);
            puts("\n");
            create_conn_flag = 0;
            client_creat_connection(report_pt->address, report_pt->address_type);
            ACT_SET_FLAG(CLI_SCAN_DISABLE);
        }
    }
}


static void client_set_creat_conn_param(u16 conn_interval_min, u16 conn_interval_max, u16 supervision_timeout)
{
    le_connection_parameter_range_t gc_conn_param;
    gc_conn_param.le_conn_interval_min = conn_interval_min;
    gc_conn_param.le_conn_interval_max = conn_interval_max;
    gc_conn_param.le_conn_latency_min = 0;
    gc_conn_param.le_conn_latency_max = 1;
    gc_conn_param.le_supervision_timeout_min = supervision_timeout;
    gc_conn_param.le_supervision_timeout_max = supervision_timeout;
    gap_le_set_connection_parameter_range(&gc_conn_param);
}

static void client_creat_connection(u8 *conn_addr, u8 addr_type)
{
    u8 tmp_addr[6];

    swapXX(conn_addr, tmp_addr, 6);

    if (tc_state == TC_W4_CONNECT) {
        client_puts("already creat conn!!!\n");
        return;
    }
    client_set_creat_conn_param(SET_CONN_INTERVAL_MIN, SET_CONN_INTERVAL_MAX, SET_CONN_INTERVAL_MAX);
    gap_stop_scan();
    tc_state = TC_W4_CONNECT;
    gap_connect(tmp_addr, addr_type);
}

static void client_creat_connection_cannel(void)
{
    if (tc_state == TC_W4_CONNECT) {
        if (gc_handle == 0) {
            gap_connect_cancel();
        }

    }
}

static void client_disconnect(u16 handle)
{
    if ((gc_handle != 0) && (handle == gc_handle)) {
        tc_state = TC_W4_DISCONNECT;
        client_puts("gap_disconnect\n");
        gap_disconnect(gc_handle);
        /* le_hci_send_cmd(&hci_disconnect, handle, 0x13); */
    }
}

static void client_scan_set_param(u8 scan_type, u16 scan_interval, u16 scan_window)
{
    gap_set_scan_parameters(scan_type, scan_interval, scan_window);
}


static void client_scan_enable(int enable)
{

    if (tc_state ==  TC_W4_CONNECT) {
        return;
    }

    if (enable) {
        tc_state = TC_W4_SCAN_RESULT;
        create_conn_flag = 1;
        gap_start_scan();
        set_ble_work_state(BLE_ST_SCAN);
    } else {
        if (tc_state == TC_W4_SCAN_RESULT) {
            tc_state = TC_IDLE;
        }
        gap_stop_scan();
        set_ble_work_state(BLE_ST_IDLE);
    }

}

static void client_search_service_init(void)
{
    tc_state = TC_W4_SERVICE_RESULT;
    service_count = 0;
    service_index = 0;
    characteristic_count = 0;
    characteristic_index = 0;
    record_ccc_count = 0;
    enable_ccc_group = remote_ccc_characteristic;
}

static void client_search_characteristic_init(void)
{
    tc_state = TC_W4_CHARACTERISTIC_RESULT;
    characteristic_count = 0;
    characteristic_index = 0;
    record_ccc_count = 0;
}

static u16  client_search_profile_start(u8 search_pfl_type, u32 search_data)
{
    u16 res = 0;

    P_FUNC_LINE();

    client_search_service_init();
    switch (search_pfl_type) {
    case PFL_SERVER_UUID16:
        gatt_client_discover_primary_services_by_uuid16(s_client_gatt_event, gc_handle, (u16)search_data);
        break;

    case PFL_SERVER_UUID128:
        gatt_client_discover_primary_services_by_uuid128(s_client_gatt_event, gc_handle, (void *)search_data);
        break;

    case PFL_SERVER_ALL:
        gatt_client_discover_primary_services(s_client_gatt_event, gc_handle);
        break;

    default:
        res = -1;
        break;
    }

    return res;
}


static void client_search_profile_complete(void)
{
    u16 mtu;

    if (service_count == 0) {
        log_info("find no services!!!\n");
    }

    gatt_client_get_mtu(gc_handle, &mtu);
    log_info("ATT MTU = %u\n", mtu);
    att_send_set_mtu_size(mtu);

    client_puts("\n-target_handle:");
    client_put_buf((void *)&target_handle, sizeof(target_handle_t));
    set_ble_work_state(BLE_ST_SEARCH_COMPLETE);
    log_info("client_search_profile_complete\n");
}

static void  client_search_report_characteristtic(le_characteristic_t *charact)
{
    characteristic_count++;
    dump_characteristic(charact);
}

static void  client_record_notify_indicate_characteristtic(le_characteristic_t *charact)
{
    client_printf("uuid = %04x\n,charact->properties = %2x\n", charact->uuid16, charact->properties);
    if (charact->properties & (ATT_PROPERTY_NOTIFY | ATT_PROPERTY_INDICATE)) {
        if (record_ccc_count < SUPPORT_CCC_MAXNUM) {
            memcpy(&enable_ccc_group[record_ccc_count], charact, sizeof(le_characteristic_t));
            record_ccc_count++;
        }
    }

}

static int client_discover_next_service(void)
{
    if (service_index < service_count) {
        client_printf("\n----------------discover service_index: %d --------------\n", service_index);
        current_service = remote_services[service_index++];
        dump_service(&current_service);
        gatt_client_discover_characteristics_for_service(s_client_gatt_event, gc_handle, &current_service);
        return 1;
    } else {
        log_info("discover_service end!!!\n");
        return 0;
    }
}

static le_characteristic_t *client_get_next_record_ccc(void)
{
    le_characteristic_t *charact;
    if (characteristic_index < record_ccc_count) {
        /* client_puts("get one ccc\n"); */
        charact = &enable_ccc_group[characteristic_index];
        characteristic_index++;
        return charact;
    }
    return NULL;
}

static int client_operation_send(u16 handle, u8 *data, u16 len, u8 att_op_type)
{
    u32 ret = APP_BLE_NO_ERROR;
    u32 send_len = user_data_cbuf_is_write_able(len);

    JUST_IS_INVAIL(gc_handle);
    JUST_IS_INVAIL(handle);

    if (send_len) {
        //printf("v_l= %d\n",send_len);
        ret = user_data_att_send(handle, data, len, att_op_type);
    } else {
        putchar('m');
        ret = APP_BLE_BUFF_FULL;
    }
    return ret;
}


#define CHARACTERISTIC_VALUE_EVENT_HEADER_SIZE      8
void client_report_acc_data(uint8_t *packet, uint16_t size)
{
    att_data_report_t report_data;
    u8 get_data_flag = 1;
    u32 evt_packet_type = hci_event_packet_get_type(packet);

    report_data.value_handle = packet[4] + (packet[5] << 8);
    report_data.blob = packet + CHARACTERISTIC_VALUE_EVENT_HEADER_SIZE;
    report_data.blob_length = size - CHARACTERISTIC_VALUE_EVENT_HEADER_SIZE;
    report_data.value_offset = 0;

    switch (evt_packet_type) {
    case GATT_EVENT_NOTIFICATION:
        log_info("\n-ntf %04x (%d):", report_data.value_handle, report_data.blob_length);

#if BLE_ATT_SEND_TEST
        test_record_data_cnt += report_data.blob_length;
        u32 index_id = little_endian_read_32(report_data.blob, 0);
        log_info("test_record_data_cnt= %u , id= %u\n", test_record_data_cnt, index_id);

        if (index_record != index_id) {
            log_info(" ddd_data: %u--%u\n", index_record, index_id);
        }
        index_record = index_id + 1;

#endif
        break;

    case GATT_EVENT_INDICATION:
        log_info("\n-ind %04x (%d):", report_data.value_handle, report_data.blob_length);

#if BLE_ATT_SEND_TEST
        test_record_data_cnt += report_data.blob_length;
        u32 index_id1 = little_endian_read_32(report_data.blob, 0);
        log_info("test_record_data_cnt= %u , id= %u\n", test_record_data_cnt, index_id1);
#endif
        break;

    case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT:
        log_info("\n-read %04x (%d):", report_data.value_handle, report_data.blob_length);
        att_wakeup_send_process();
        break;

    case GATT_EVENT_QUERY_COMPLETE                          :
        get_data_flag = 0;
        att_wakeup_send_process();
        break;

    default:
        client_printf("\n-utype= %02x\n", hci_event_packet_get_type(packet));
        get_data_flag = 0;
        att_wakeup_send_process();
        break;
    }

    if (get_data_flag) {
        if (app_recieve_callback) {
            app_recieve_callback((void *)evt_packet_type, report_data.blob, report_data.blob_length);
        }
        /* client_put_buf(report_data.blob, report_data.blob_length); */
    }
}


static void s_client_gatt_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    u32 tmp32, ret_type;
    /* printf("client cbk gatt event %x \n",tc_state);	 */
    switch (tc_state) {

    case TC_W4_ACC_DATA:
        client_report_acc_data(packet, size);
        break;

    case TC_W4_SERVICE_RESULT:
        switch (hci_event_packet_get_type(packet)) {
        case GATT_EVENT_SERVICE_QUERY_RESULT:
            gatt_event_service_query_result_get_service(packet, &current_service);
            ret_type = check_target_uuid_match(&current_service, NULL);
            if (IS_RECORD_TYPE(ret_type, RECORD_SERVICES)) {
                if (service_index < SUPPORT_SERVICES_MAXNUM) {
                    client_printf("record server %d \n", service_index);
                    remote_services[service_index++] = current_service;
                    service_count++;
                } else {
                    client_puts("services is overflow!!!!!!\n");
                }
            }
            break;

        case GATT_EVENT_QUERY_COMPLETE:
            if (packet[4] != 0) {
                client_printf("SERVICE_QUERY_RESULT - Error status %x.\n", packet[4]);
                /* gap_disconnect(gc_handle); */
                /* client_disconnect(gc_handle); */
                break;
            }

            if (service_count == 0) {
                client_search_profile_complete();
            } else {
                log_info("search_service finish (%d)!!!\n", service_count);
                /* dump_service(&current_service); */
                service_index = 0;
                client_discover_next_service();
                client_search_characteristic_init();
                tc_state = TC_W4_CHARACTERISTIC_RESULT;
                gatt_client_discover_characteristics_for_service(s_client_gatt_event, gc_handle, &current_service);
            }
            break;
        default:
            break;
        }
        break;

    case TC_W4_CHARACTERISTIC_RESULT:
        switch (hci_event_packet_get_type(packet)) {
        case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
            gatt_event_characteristic_query_result_get_characteristic(packet, &current_characteristic);
            client_search_report_characteristtic(&current_characteristic);
            ret_type = check_target_uuid_match(&current_service, &current_characteristic);
            if (IS_RECORD_TYPE(ret_type, RECORD_CCC)) {
                client_record_notify_indicate_characteristtic(&current_characteristic);
            }
            break;

        case GATT_EVENT_QUERY_COMPLETE:
            if (packet[4] != 0) {
                client_printf("CHARACTERISTIC_QUERY_RESULT - Error status %x.\n", packet[4]);
                /* gap_disconnect(gc_handle); */
                /* client_disconnect(gc_handle); */
                break;
            }

            client_printf("current_service characteristics = %d\n", characteristic_count);

            if (client_discover_next_service()) {
                client_search_characteristic_init();
                gatt_client_discover_characteristics_for_service(s_client_gatt_event, gc_handle, &current_service);
                break;
            }

            client_puts("\n\n------------search_characteristics finish!!!----------\n\n");


            /* client_search_profile_complete(); */
            /* tc_state = TC_W4_ACC_DATA; */
            /* break; */

            if (record_ccc_count == 0) {
                client_search_profile_complete();
                break;
            }

            client_puts("start enable ccc\n");
            tc_state = TC_W4_ACC_CLIENT_CONFIG_CHARACTERISTIC_RESULT;
            characteristic_index = 0;
            log_info("record ccc cnt:%x\n", record_ccc_count);
            deal_characteristic = client_get_next_record_ccc();
            if (deal_characteristic->properties & ATT_PROPERTY_INDICATE) {
                gatt_client_write_client_characteristic_configuration(s_client_gatt_event, gc_handle, deal_characteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION);
            } else {
                gatt_client_write_client_characteristic_configuration(s_client_gatt_event, gc_handle, deal_characteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
            }
            break;
        default:
            break;
        }
        break;


    case TC_W4_ACC_CLIENT_CONFIG_CHARACTERISTIC_RESULT:
        switch (hci_event_packet_get_type(packet)) {
        case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
            break;
        case GATT_EVENT_QUERY_COMPLETE:
            client_printf("set ccc sucess,%d\n", characteristic_index);
            gatt_client_listen_for_characteristic_value_updates(&notification_registration[characteristic_index - 1], s_client_gatt_event, gc_handle, deal_characteristic);
            deal_characteristic = client_get_next_record_ccc();
            if (deal_characteristic == NULL) {
                client_search_profile_complete();
                tc_state = TC_W4_ACC_DATA;
                client_puts("set ccc complete\n");
                break;
            }

            client_puts("set next ccc\n");
            if (deal_characteristic->properties & ATT_PROPERTY_INDICATE) {
                gatt_client_write_client_characteristic_configuration(s_client_gatt_event, gc_handle, deal_characteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION);
            } else {
                gatt_client_write_client_characteristic_configuration(s_client_gatt_event, gc_handle, deal_characteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
            }

            break;
        default:
            break;
        }
        break;

    default:
        client_printf("Client, unhandled state %d\n", tc_state);
        break;
    }

}

//----------------------------------------------------------------------------------------------------------------------------------

//return
//0--confirm and pair ok
//others--not confirm,pair fail
static int cbk_sm_just_packet_control(uint8_t packet_type, uint8_t *packet, uint16_t size)
{
    uint8_t msg;
    u32 tmp32;
    sm_just_event_t  *event = (sm_just_event_t *)packet;

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
 * @section Packet Handler
 *
 * @text The packet handler is used to:
 *        - stop the counter after a disconnect
 *        - send a notification when the requested ATT_EVENT_CAN_SEND_NOW is received
 */

/* LISTING_START(packetHandler): Packet Handler */
static void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    u16 mtu;

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        /* printf("client cbk packet event %x \n",hci_event_packet_get_type(packet));	 */
        switch (hci_event_packet_get_type(packet)) {
        case ATT_EVENT_CAN_SEND_NOW:
            /* putchar('N'); */
            att_wakeup_send_process();
            break;

        case HCI_EVENT_LE_META:
            /* printf("meta=%x \n",hci_event_le_meta_get_subevent_code(packet));	 */
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: {
                if (tc_state != TC_W4_CONNECT) {
                    client_printf("CONN ACTIVE err,%d\n", tc_state);
                    break;
                }

                if (packet[3] != 0) {
                    client_puts("client creat_conn fail!\n");
                    client_put_buf(packet, size);
                    if (packet[3] != 0x0c) {
                        tc_state = TC_IDLE;//;
                        ACT_SET_FLAG(CLI_SCAN_ENABLE);
                    }
                    break;
                }
                client_puts("client conn sucess\n");
                /* client_put_buf(packet, size); */
                gc_handle = little_endian_read_16(packet, 4);
                log_info("HCI_SUBEVENT_LE_CONNECTION_COMPLETE : %0x\n", gc_handle);
                tc_state = TC_CONNECTED;//TC_W4_ENCRYPTED_CONNECTION;
                set_ble_work_state(BLE_ST_CONNECT);
                ACT_SET_FLAG(CLI_SEARCH_PROFILE_START);//debug
                att_send_init(gc_handle);
                att_regist_client_callback(s_client_gatt_event);
                test_record_data_cnt = 0;
                index_record = 0;
                break;

                case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                    log_info("CONNECTION_UPDATE_COMPLETE\n\n", gc_handle);
                    break;
                }
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            tc_state = TC_IDLE;
            gc_handle = 0;
            set_ble_work_state(BLE_ST_IDLE);
            client_puts("client disconn\n");
            ACT_SET_FLAG(CLI_SCAN_ENABLE);
            att_send_init(gc_handle);
            memset(&target_handle, 0, sizeof(target_handle_t));
            break;

        /* case ATT_EVENT_MTU_EXCHANGE_COMPLETE: */
        /* break; */

        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                break;
            }
            log_info("init complete\n");
            set_ble_work_state(BLE_ST_IDLE);
            client_scan_set_param(1, SET_SCAN_INTERVAL, SET_SCAN_WINDOW);
            ACT_SET_FLAG(CLI_SCAN_ENABLE);
            break;

        case HCI_EVENT_VENDOR_REMOTE_TEST:
            log_info("--- HCI_EVENT_VENDOR_REMOTE_TEST\n");
            break;

        case GAP_EVENT_ADVERTISING_REPORT:
            /* client_puts("GAP_LE_ADVERTISING_REPORT\n"); */
            /* putchar('V'); */
            if (tc_state != TC_W4_SCAN_RESULT) {
                client_printf("SCAN ACTIVE err,%d\n", tc_state);
                break;
            }
            client_report_adv_data((void *)&packet[2], packet[1]);
            break;

        case HCI_EVENT_ENCRYPTION_CHANGE:
            log_info("--- HCI_EVENT_ENCRYPTION_CHANGE\n");
            break;

        }
        break;
    }
    /* client_puts("cbk_packet_handler end\n"); */
}

//-----------------------------------------------
static void client_thread_process(void)
{
    u8 act_cmd;

    if (FALSE == mini_cbuf_read(&act_cmd_mctl, &act_cmd)) {
        return;
    }

    log_info("ACT:%02x\n", act_cmd);

    switch (act_cmd) {
    case CLI_DIS_CONNECT:
        client_disconnect(gc_handle);
        break;

    case CLI_SEARCH_PROFILE_START:
        client_search_profile_start(PFL_SERVER_ALL, 0);
        break;

    case CLI_SCAN_ENABLE:
        client_scan_enable(1);
        break;

    case CLI_SCAN_DISABLE:
        client_scan_enable(0);
        break;

    default:
        log_info("unknow client act_cmd!\n");
        break;
    }

    if (!mini_cbuf_is_emtpy(&act_cmd_mctl)) {
        client_thread_resume();
    }

}


static void client_thread_resume(void)
{
    att_set_request_thread_deal();
}

void ble_stack_config_init(void)
{
    ble_sm_setup_init(IO_CAPABILITY_NO_INPUT_NO_OUTPUT, SM_AUTHREQ_BONDING, sm_min_key_size, sm_encryption_flag);
    ble_cbk_handler_register(cbk_packet_handler, cbk_sm_just_packet_control);
    gatt_client_init();
    mini_cbuf_init(&act_cmd_mctl, act_cmd_list, CMD_LIST_SIZE);
    att_regist_thread_deal_cbk(client_thread_process);
    /* sys_timer_register(&client_timer, 20, client_timer_handle, 1); */
    memset(&target_handle, 0, sizeof(target_handle_t));
    set_ble_work_state(BLE_ST_NULL);
}


static int app_client_write_send(void *priv, u8 *data, u16 len)
{
    return client_operation_send(target_handle.write_handle, data, len, ATT_OP_WRITE);
}

static int app_client_write_without_respond_send(void *priv, u8 *data, u16 len)
{
    return client_operation_send(target_handle.write_handle, data, len, ATT_OP_WRITE_WITHOUT_RESPOND);
}

static int app_client_read_send(void *priv)
{
    u16 tmp = 0x55aa;//need data
    return client_operation_send(target_handle.read_handle, (u8 *)&tmp, 2, ATT_OP_READ);
}

static int app_client_disconn(void *priv)
{
    log_info("send app_client_disconn\n");
    JUST_IS_INVAIL(gc_handle);

    ACT_SET_FLAG(CLI_DIS_CONNECT);
    set_ble_work_state(BLE_ST_SEND_DISCONN);
    return APP_BLE_NO_ERROR;
}


static int app_client_scan_enable(void *priv, u32 enable)
{
    JUST_IS_INVAIL(gc_handle);

    if (enable) {
        ACT_SET_FLAG(CLI_SCAN_ENABLE);
    } else {
        ACT_SET_FLAG(CLI_SCAN_DISABLE);
    }
    return APP_BLE_OPERATION_ERROR;
}

static int app_client_get_buffer_vaild_len(void *priv)
{
    int vaild_len = 0;

    if (!gc_handle) {
        return 0;
    }

    vaild_len = (int)user_data_cbuf_is_write_able(0);
    /* printf("v_len= %d\n",vaild_len); */
    return vaild_len;
}

static int app_client_regiest_wakeup_send(void *priv, void *cbk)
{
    att_regist_wakeup_send(cbk);
    return APP_BLE_NO_ERROR;
}

static int app_client_regiest_recieve_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_recieve_callback = cbk;
    return APP_BLE_NO_ERROR;
}

static int app_client_regiest_state_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_ble_state_callback = cbk;
    return APP_BLE_NO_ERROR;
}



static const struct ble_client_operation_t client_operation = {
    .scan_enable = app_client_scan_enable,
    .disconnect = app_client_disconn,
    .get_buffer_vaild = app_client_get_buffer_vaild_len,
    .write_data = (void *)app_client_write_without_respond_send,
    .read_do = (void *)app_client_read_send,
    .regist_wakeup_send = app_client_regiest_wakeup_send,
    .regist_recieve_cbk = app_client_regiest_recieve_cbk,
    .regist_state_cbk = app_client_regiest_state_cbk,
};


void ble_get_client_operation_table(struct ble_client_operation_t **interface_pt)
{
    *interface_pt = (void *)&client_operation;
}


bool ble_msg_deal(u32 param)
{
    struct ble_client_operation_t *test_opt;
    ble_get_client_operation_table(&test_opt);

#if 0//for test key
    switch (param) {
    case MSG_BT_PP:
        test_opt->disconnect(0);
        break;

    case MSG_BT_NEXT_FILE:
        conn_last_remoter_flag = 1;
        if (conn_last_remoter_flag) {
            log_info("\n------ creat connect device (%s) ------\n", adv_last_remoter_name);
        }
        break;

    case MSG_BT_PREV_FILE:
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
        static u8 test_client_step = 1;
        static u8 start_send_flag = 0;
        static u32 index_cnt = 0;

        if (BLE_ST_SEARCH_COMPLETE != get_ble_work_state()) {
            test_client_step = 1;
            test_record_data_cnt = 0;
            index_cnt = 0;
            return FALSE;
        }


        if (param == MSG_BT_PREV_FILE) {
            start_send_flag = !start_send_flag;
            log_info("test_record_data_cnt= %u\n", test_record_data_cnt);
            if (start_send_flag) {
                test_record_data_cnt = 0;
            }
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
                int ret_val = test_opt->write_data(0, (void *)&index_cnt, send_len);
                if (APP_BLE_NO_ERROR == ret_val) {
                    index_cnt++;
                    test_record_data_cnt += send_len;
                } else {
                    log_info("test send fail!!! %d\n", ret_val);
                }
            } else {
                break;
            }

        }
    }
#endif
    return FALSE;
}


//----------------------------------------------------------------------------------



#endif

#endif

