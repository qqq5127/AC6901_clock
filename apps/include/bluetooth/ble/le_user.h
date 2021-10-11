
#ifndef _LE_USER_H_
#define _LE_USER_H_

#if defined __cplusplus
extern "C" {
#endif

#include <sdk_cfg.h>
#include <ble_api.h>


    /* 18  APPENDIX C (NORMATIVE): EIR AND AD FORMATS, data format used in the EIR data
      field and in the AD format */
    typedef enum {
        HCI_EIR_DATATYPE_FLAGS =                                                 0x01,
        HCI_EIR_DATATYPE_MORE_16BIT_SERVICE_UUIDS =                              0x02,
        HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS =                          0x03,
        HCI_EIR_DATATYPE_MORE_32BIT_SERVICE_UUIDS =                              0x04,
        HCI_EIR_DATATYPE_COMPLETE_32BIT_SERVICE_UUIDS =                          0x05,
        HCI_EIR_DATATYPE_MORE_128BIT_SERVICE_UUIDS =                             0x06,
        HCI_EIR_DATATYPE_COMPLETE_128BIT_SERVICE_UUIDS =                         0x07,
        HCI_EIR_DATATYPE_SHORTENED_LOCAL_NAME =                                  0x08,
        HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME =                                   0x09,
        HCI_EIR_DATATYPE_TX_POWER_LEVEL =                                        0x0A,
        HCI_EIR_DATATYPE_CLASS_OF_DEVICE =                                       0x0D,
        HCI_EIR_DATATYPE_SIMPLE_PAIRING_HASH_C =                                 0x0E,
        HCI_EIR_DATATYPE_SIMPLE_PAIRING_RANDOMIZER_R =                           0x0F,
        HCI_EIR_DATATYPE_SECURITY_MANAGER_TK_VALUE =                             0x10,
        HCI_EIR_DATATYPE_SECURITY_MANAGER_OOB_FLAGS =                            0x11,
        HCI_EIR_DATATYPE_SLAVE_CONNECTION_INTERVAL_RANGE =                       0x12,
        HCI_EIR_DATATYPE_16BIT_SERVICE_SOLICITATION_UUIDS =                      0x14,
        HCI_EIR_DATATYPE_128BIT_SERVICE_SOLICITATION_UUIDS =                     0x15,
        HCI_EIR_DATATYPE_SERVICE_DATA =                                          0x16,
        HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA =                            0xFF
    } HCI_EIR_datatype_t;

    typedef enum {
        HCI_STATE_OFF = 0,
        HCI_STATE_INITIALIZING,
        HCI_STATE_WORKING,
        HCI_STATE_HALTING,
        HCI_STATE_SLEEPING,
        HCI_STATE_FALLING_ASLEEP
    } HCI_STATE;


//--------------------
#define HCI_COMMAND_DATA_PACKET       0x01
#define HCI_ACL_DATA_PACKET           0x02
#define HCI_SCO_DATA_PACKET           0x03
#define HCI_EVENT_PACKET              0x04
#define HCI_MONITOR_ACL_DATA_PACKET   0x05
// Security Manager protocol data
#define SM_DATA_PACKET                0x09

//------------------------
#define HCI_EVENT_LE_META                                    0x3E
#define HCI_SUBEVENT_LE_CONNECTION_COMPLETE                  0x01
#define HCI_SUBEVENT_LE_ADVERTISING_REPORT                   0x02
#define HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE           0x03
#define HCI_SUBEVENT_LE_READ_REMOTE_USED_FEATURES_COMPLETE   0x04
#define HCI_SUBEVENT_LE_LONG_TERM_KEY_REQUEST                0x05
#define HCI_SUBEVENT_LE_REMOTE_CONNECTION_PARAMETER_REQUEST  0x06
#define HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE                   0x07
#define HCI_EVENT_ENCRYPTION_CHANGE                          0x08

//-----------
#define HCI_EVENT_DISCONNECTION_COMPLETE                   0x05
#define BTSTACK_ACL_BUFFERS_FULL                           0x57
#define BTSTACK_EVENT_STATE                                0x60

#define ATT_HANDLE_VALUE_INDICATION_IN_PORGRESS            0x90

#define GATT_CLIENT_BUSY                                   0x94
#define GATT_CLIENT_IN_WRONG_STATE                         0x95


#define ATT_EVENT_MTU_EXCHANGE_COMPLETE                    0xB5
#define ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE         0xB6
#define ATT_EVENT_CAN_SEND_NOW                             0xB7

#define SM_EVENT_JUST_WORKS_REQUEST                        0xD0
#define SM_EVENT_PASSKEY_DISPLAY_NUMBER                    0xD2

#define SM_EVENT_PAIR_RECONNECT_PROCESS                          0xDE
#define SM_EVENT_PAIR_SUB_COMPLETED                              0x01
#define SM_EVENT_PAIR_SUB_USER_CONFIRM                           0x02
#define SM_EVENT_PAIR_SUB_REJECT                                 0x03
#define SM_EVENT_PAIR_SUB_PAIR_FAIL                              0x04
#define SM_EVENT_PAIR_SUB_PAIR_TIMEOUT                           0x05

#define HCI_EVENT_VENDOR_REMOTE_TEST                       0xFE

// data: event(8)
#define DAEMON_EVENT_HCI_PACKET_SENT                       0x6C

// data: event(8), len(8), handle(16), result (16) (0 == ok, 1 == fail)
#define L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE   0x77
//--------------------------------------------
// IO Capability Values
    typedef enum {
        IO_CAPABILITY_DISPLAY_ONLY = 0,
        IO_CAPABILITY_DISPLAY_YES_NO,
        IO_CAPABILITY_KEYBOARD_ONLY,
        IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
        IO_CAPABILITY_KEYBOARD_DISPLAY, // not used by secure simple pairing
    } io_capability_t;


// Authentication requirement flags
#define SM_AUTHREQ_NO_BONDING           0x00
#define SM_AUTHREQ_BONDING              0x01
#define SM_AUTHREQ_MITM_PROTECTION      0x04
#define SM_AUTHREQ_SECURE_CONNECTION    0x08
#define SM_AUTHREQ_KEYPRESS             0x10


#define BLE_ADDR_TYPE_PUBLIC       0
#define BLE_ADDR_TYPE_RANDOM       1

    /**
     * @brief Security Manager event
     */
    typedef struct {
        //base info
        uint8_t   type;                 ///< See <btstack/hci_cmds.h> SM_...
        uint8_t   addr_type;
        uint8_t   size;
        uint8_t   address[6];
        hci_con_handle_t con_handle;
        //extend info
        union {
            uint32_t  passkey;              ///< only used for SM_EVENT_PASSKEY_DISPLAY_NUMBER
            uint16_t  le_device_db_index;   ///< only used for SM_IDENTITY_RESOLVING_..
            uint8_t   authorization_result; ///< only use for SM_EVENT_AUTHORIZATION_RESULT
        };
    } sm_just_event_t;


//--------------------------------------------
#define ADV_NORMAL_MS       (100)
#define ADV_LOWPOWER_MS     (500)


    typedef enum {
        BLE_ST_NULL = 0,
        BLE_ST_INIT_OK, //init ok
        BLE_ST_IDLE,
        BLE_ST_ADV,
        BLE_ST_CONNECT,
        BLE_ST_SEND_DISCONN,
        BLE_ST_NOTIFY_IDICATE,//server ok
        BLE_ST_SCAN,
        BLE_ST_SEARCH_COMPLETE,//client ok
    } ble_state_e;


    struct ble_server_operation_t {
        int(*adv_enable)(void *priv, u32 enable);
        int(*disconnect)(void *priv);
        int(*get_buffer_vaild)(void *priv);
        int(*send_data)(void *priv, void *buf, u16 len);
        int(*regist_wakeup_send)(void *priv, void *cbk);
        int(*regist_recieve_cbk)(void *priv, void *cbk);
        int(*regist_state_cbk)(void *priv, void *cbk);
        int(*latency_enable)(void *priv, u32 enable);
    };
    void ble_get_server_operation_table(struct ble_server_operation_t **interface_pt);


    struct ble_client_operation_t {
        int(*scan_enable)(void *priv, u32 enable);
        int(*disconnect)(void *priv);
        int(*get_buffer_vaild)(void *priv);
        int(*write_data)(void *priv, void *buf, u16 len);
        int(*read_do)(void *priv);
        int(*regist_wakeup_send)(void *priv, void *cbk);
        int(*regist_recieve_cbk)(void *priv, void *cbk);
        int(*regist_state_cbk)(void *priv, void *cbk);
    };
    void ble_get_client_operation_table(struct ble_client_operation_t **interface_pt);


//--------------------------------------------
    struct conn_update_param_t {
        u16 interval_min;
        u16 interval_max;
        u16 latency;
        u16 timeout;
    };


    struct conn_param_t {
        u16 interval;
        u16 latency;
        u16 timeout;
    };

//--------------------------------------------
    enum {
        ATT_OP_AUTO_READ_CCC = 0,
        ATT_OP_NOTIFY = 1,
        ATT_OP_INDICATE = 2,
        ATT_OP_READ,
        ATT_OP_WRITE,
        ATT_OP_WRITE_WITHOUT_RESPOND,
        //add here

        ATT_OP_CMD_MAX = 15,
    };

#define NOTIFY_TYPE           1
#define INDICATION_TYPE       2
#define BT_NAME_LEN_MAX		  30
// Minimum/default MTU
#define ATT_DEFAULT_MTU       23
#define ADV_RSP_PACKET_MAX    31


    enum {
        APP_BLE_NO_ERROR = 0,
        APP_BLE_BUFF_ERROR = 1,
        APP_BLE_BUFF_FULL = 2,
        APP_BLE_OPERATION_ERROR = 3,
        APP_BLE_IS_DISCONN =  4,
    };

    static inline u8 make_eir_packet_data(u8 *buf, u16 offset, u8 data_type, u8 *data, u8 data_len)
    {
        if (ADV_RSP_PACKET_MAX - offset < data_len + 2) {
            return 0;
        }

        buf[0] = data_len + 1;
        buf[1] = data_type;
        memcpy(buf + 2, data, data_len);
        return data_len + 2;
    }

    static inline u8 make_eir_packet_val(u8 *buf, u16 offset, u8 data_type, u32 val, u8 val_size)
    {
        if (ADV_RSP_PACKET_MAX - offset < val_size + 2) {
            return 0;
        }

        buf[0] = val_size + 1;
        buf[1] = data_type;
        memcpy(buf + 2, &val, val_size);
        return val_size + 2;
    }



#define EIR_TAG_STRING   'Z', 'H', 'J', 'I', 'E', 'L', 'I'

    enum {
        TEST_BLE_MSG_ENTRY = 0,
        TEST_BLE_MSG_HALFSECOND,
        TEST_BLE_MSG_DISCONN,
        TEST_BLE_MSG_CREAT_CONN,
        TEST_BLE_MSG_SENDDATA,
    };

//---------------------------------------------------------------------------------------------------


// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param con_handle of hci le connection
// @param attribute_handle to be read
// @param offset defines start of attribute value
// @param buffer
// @param buffer_size
    typedef uint16_t (*att_read_callback_t)(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

// ATT Client Write Callback for Dynamic Data
// @param con_handle of hci le connection
// @param attribute_handle to be written
// @param transaction - ATT_TRANSACTION_MODE_NONE for regular writes, ATT_TRANSACTION_MODE_ACTIVE for prepared writes and ATT_TRANSACTION_MODE_EXECUTE
// @param offset into the value - used for queued writes and long attributes
// @param buffer
// @param buffer_size
// @param signature used for signed write commmands
// @returns 0 if write was ok, ATT_ERROR_PREPARE_QUEUE_FULL if no space in queue, ATT_ERROR_INVALID_OFFSET if offset is larger than max buffer
    typedef int (*att_write_callback_t)(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

//----------------------------------------
    extern uint16_t little_endian_read_16(const uint8_t *buffer, int pos);
    extern uint32_t little_endian_read_24(const uint8_t *buffer, int pos);
    extern uint32_t little_endian_read_32(const uint8_t *buffer, int pos);

//----------------------------------------

    extern void ble_sm_setup_init(io_capability_t io_type, u8 auth_req, uint8_t min_key_size, u8 security_en);

    extern void ble_att_server_setup_init(const u8 *profile_db, att_read_callback_t read_cbk, att_write_callback_t write_cbk);

    extern void att_server_request_can_send_now_event(hci_con_handle_t con_handle);
    extern int att_server_notify(hci_con_handle_t con_handle, uint16_t attribute_handle, uint8_t *value, uint16_t value_len);

    extern uint8_t gatt_client_read_long_value_of_characteristic_using_value_handle_with_offset(btstack_packet_handler_t callback, hci_con_handle_t con_handle, uint16_t characteristic_value_handle, uint16_t offset);
    extern uint8_t gatt_client_read_value_of_characteristic_using_value_handle(btstack_packet_handler_t callback, hci_con_handle_t con_handle, uint16_t value_handle);
    extern uint8_t gatt_client_write_value_of_characteristic_without_response(hci_con_handle_t con_handle, uint16_t value_handle, uint16_t value_length, uint8_t *value);
    extern uint8_t gatt_client_write_value_of_characteristic(btstack_packet_handler_t callback, hci_con_handle_t con_handle, uint16_t value_handle, uint16_t value_length, uint8_t *data);

    extern int get_ble_btstack_state(void);
    extern int get_indicate_state(void);
    extern int att_server_indicate(hci_con_handle_t con_handle, uint16_t attribute_handle, uint8_t *value, uint16_t value_len);

    extern void ble_cbk_handler_register(btstack_packet_handler_t packet_cbk, sm_stack_packet_handler_t sm_cbk);

    extern int gap_request_connection_parameter_update(hci_con_handle_t con_handle, uint16_t conn_interval_min,
            uint16_t conn_interval_max, uint16_t conn_latency, uint16_t supervision_timeout);


    extern void gap_advertisements_enable(int enabled);
    extern void gap_advertisements_set_data(uint8_t advertising_data_length, uint8_t *advertising_data);
    extern void gap_scan_response_set_data(uint8_t scan_response_data_length, uint8_t *scan_response_data);
    extern void gap_advertisements_set_params(uint16_t adv_int_min, uint16_t adv_int_max, uint8_t adv_type,
            uint8_t direct_address_typ, uint8_t *direct_address, uint8_t channel_map, uint8_t filter_policy);

    extern uint8_t gap_disconnect_ext(hci_con_handle_t handle, uint8_t reason);

    extern u8 get_ble_local_name(u8 *name_buf);
    extern u8 get_ble_local_name_len();


    extern void gatt_client_request_can_send_now_event(hci_con_handle_t con_handle);

//--------------------------------------
    static inline hci_con_handle_t hci_subevent_le_connection_update_complete_get_connection_handle(const uint8_t *event)
    {
        return little_endian_read_16(event, 4);
    }

    static inline uint16_t hci_subevent_le_connection_update_complete_get_conn_interval(const uint8_t *event)
    {
        return little_endian_read_16(event, 6);
    }
    static inline uint16_t hci_subevent_le_connection_update_complete_get_conn_latency(const uint8_t *event)
    {
        return little_endian_read_16(event, 8);
    }
    static inline uint16_t hci_subevent_le_connection_update_complete_get_supervision_timeout(const uint8_t *event)
    {
        return little_endian_read_16(event, 10);
    }

    static inline uint8_t hci_event_packet_get_type(const uint8_t *event)
    {
        return event[0];
    }

    static inline uint8_t hci_event_le_meta_get_subevent_code(const uint8_t *event)
    {
        return event[2];
    }
    static inline uint16_t att_event_mtu_exchange_complete_get_MTU(const uint8_t *event)
    {
        return little_endian_read_16(event, 4);
    }

    static inline uint8_t btstack_event_state_get_state(const uint8_t *event)
    {
        return event[2];
    }


#endif

