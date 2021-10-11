
#ifndef _BLE_API_H_
#define _BLE_API_H_

#if defined __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <linked_list.h>

#ifndef hci_con_handle_t
    typedef uint16_t hci_con_handle_t;
#endif

// packet handler
#ifndef btstack_packet_handler_t
    typedef void (*btstack_packet_handler_t)(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
#endif

    typedef int (*sm_stack_packet_handler_t)(uint8_t packet_type, uint8_t *packet, uint16_t size);

    typedef void (*ble_cbk_handler_t)(void);

#if 0
#define P_FUNC_LINE() printf("---------%s---(%d)-------------\n",__FUNCTION__,__LINE__)
#define P_DATA_VALUE(a,b) printf("--%x--%x--\n",a,b)
#else
#define P_FUNC_LINE()
#define P_DATA_VALUE(a,b)
#endif

    extern void ble_btstack_run(void);
    extern void ble_btstack_resume(void);
    extern void ble_btstack_exit(void);
    extern void ble_btstack_init_cbk_handler_register(ble_cbk_handler_t before_init_cbk, ble_cbk_handler_t profile_config_cbk);
    extern void register_ble_init_handle(u8 mode);
    extern void ble_set_mac_addr(u8 addr_type, u8 *address);
    extern int ble_btstack_set_ram(u32 ram_addr, u32 size);
    extern void ble_set_gap_role(u8 role);
    extern void ble_set_latency_enable(u16 conn_handle, u8 enable);

    enum {
        MSG_BLE_RESUME_THREAD = 1,
    };

    u32 ble_user_ioctrl(int ctrl, ...);


#endif

