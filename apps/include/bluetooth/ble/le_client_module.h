// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)
#ifndef _LE_CLIENT_MODULE_H
#define _LE_CLIENT_MODULE_H

#include <stdint.h>
#include <le_user.h>


#define GATT_EVENT_NOTIFICATION                                  0xA7
#define GATT_EVENT_INDICATION                                    0xA8
#define GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT             0xA5
#define GATT_EVENT_LONG_CHARACTERISTIC_VALUE_QUERY_RESULT        0xA6
#define GATT_EVENT_SERVICE_QUERY_RESULT                    0xA1
#define GATT_EVENT_CHARACTERISTIC_QUERY_RESULT             0xA2
#define GATT_EVENT_QUERY_COMPLETE                          0xA0
#define GAP_EVENT_ADVERTISING_REPORT                       0xE2


#define GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NONE          0
#define GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION  1
#define GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION    2

#define ATT_PROPERTY_READ                0x02
#define ATT_PROPERTY_WRITE_WITHOUT_RESPONSE 0x04
#define ATT_PROPERTY_WRITE               0x08
#define ATT_PROPERTY_NOTIFY              0x10
#define ATT_PROPERTY_INDICATE            0x20


typedef enum {
    PFL_SERVER_UUID16,
    PFL_SERVER_UUID128,
    PFL_SERVER_ALL,
} search_profile_type_e;

typedef struct {
    uint16_t start_group_handle;
    uint16_t end_group_handle;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} le_service_t, gatt_client_service_t ;

typedef struct le_service_event {
    uint8_t  type;
    uint16_t handle;
    le_service_t service;
} le_service_event_t;

typedef struct le_characteristic {
    uint16_t start_handle;
    uint16_t value_handle;
    uint16_t end_handle;
    uint16_t properties;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} le_characteristic_t, gatt_client_characteristic_t ;

typedef struct gatt_client_notification {
    linked_item_t    item;
    btstack_packet_handler_t callback;
    hci_con_handle_t con_handle;
    uint16_t attribute_handle;
} gatt_client_notification_t;

typedef struct {
    u8   event_type;
    u8   address_type;
    u8   address[6];
    u8   reserve;
    u8   length;
    u8   data[1];
} adv_report_t;

typedef struct {
    u16  value_handle;
    u16  value_offset;
    u16  blob_length;
    u8  *blob;
} att_data_report_t;

typedef struct le_connection_parameter_range {
    uint16_t le_conn_interval_min;
    uint16_t le_conn_interval_max;
    uint16_t le_conn_latency_min;
    uint16_t le_conn_latency_max;
    uint16_t le_supervision_timeout_min;
    uint16_t le_supervision_timeout_max;
} le_connection_parameter_range_t;

#define BD_ADDR_LEN 6
typedef uint8_t bd_addr_t[BD_ADDR_LEN];
typedef enum {
    BD_ADDR_TYPE_LE_PUBLIC = 0,
    BD_ADDR_TYPE_LE_RANDOM = 1,
    BD_ADDR_TYPE_SCO       = 0xfe,
    BD_ADDR_TYPE_CLASSIC   = 0xff,
    BD_ADDR_TYPE_UNKNOWN   = 0xfe
} bd_addr_type_t;

typedef struct {
    uint16_t service_uuid16;
    uint16_t read_uuid16;
    uint16_t write_uuid16;
    uint16_t notify_uuid16;
    uint16_t indicate_uuid16;
} target_service_t;

typedef struct {
    uint16_t read_handle;
    uint16_t write_handle;
    uint16_t notify_handle;
    uint16_t indicate_handle;
} target_handle_t;



void gap_le_set_connection_parameter_range(le_connection_parameter_range_t *range);
void gap_stop_scan();
u8 gap_connect(bd_addr_t  addr, bd_addr_type_t addr_type);
u8 gap_connect_cancel();
uint8_t gap_disconnect(hci_con_handle_t handle);
void gap_set_scan_parameters(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window);
void gap_start_scan();
uint8_t gatt_client_discover_primary_services_by_uuid16(btstack_packet_handler_t callback, hci_con_handle_t con_handle, uint16_t uuid16);
uint8_t gatt_client_discover_primary_services_by_uuid128(btstack_packet_handler_t callback, hci_con_handle_t con_handle, const uint8_t *uuid128);
uint8_t gatt_client_discover_primary_services(btstack_packet_handler_t callback, hci_con_handle_t con_handle);
uint8_t gatt_client_get_mtu(hci_con_handle_t con_handle, uint16_t *mtu);
uint8_t gatt_client_discover_characteristics_for_service(btstack_packet_handler_t callback, hci_con_handle_t con_handle, gatt_client_service_t *service);
void gatt_client_deserialize_service(const uint8_t *packet, int offset, gatt_client_service_t *service);
void gatt_client_deserialize_characteristic(const uint8_t *packet, int offset, gatt_client_characteristic_t *characteristic);
uint8_t gatt_client_write_client_characteristic_configuration(btstack_packet_handler_t callback, hci_con_handle_t con_handle, gatt_client_characteristic_t *characteristic, uint16_t configuration);
void gatt_client_listen_for_characteristic_value_updates(gatt_client_notification_t *notification, btstack_packet_handler_t packet_handler, hci_con_handle_t con_handle, gatt_client_characteristic_t *characteristic);
void gatt_client_init(void);


static inline void gatt_event_service_query_result_get_service(const uint8_t *event, le_service_t *service)
{
    gatt_client_deserialize_service(event, 4, service);
}
static inline void gatt_event_characteristic_query_result_get_characteristic(const uint8_t *event, le_characteristic_t *characteristic)
{
    gatt_client_deserialize_characteristic(event, 4, characteristic);
}


#endif
