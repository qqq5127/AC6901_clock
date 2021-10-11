
#ifndef   _ATT_SEND_H_
#define   _ATT_SEND_H_

#include "sdk_cfg.h"
#include <le_user.h>
#include "circular_buf.h"

#if (BLE_BREDR_MODE&BT_BLE_EN)

#define BLE_ATT_SEND_TEST         0
//------------
typedef struct {
    u16  send_handle;
    u16  send_len: 12;
    u16  send_type: 4;
} user_send_head_t;

void att_send_init(u16 conn_handle);
void att_send_set_mtu_size(u16 mtu_size);
u32 user_data_att_send(u16 handle, u8 *data, u16 len, u8 send_type);
u32 user_data_cbuf_is_write_able(u32 len);
u32 user_data_cbuf_is_null(void);

void att_ccc_config_init(void);
void att_set_ccc_config(uint16_t handle, uint16_t cfg);
uint16_t att_get_ccc_config(uint16_t handle);
void att_regist_wakeup_send(void *cbk);
void att_wakeup_send_process(void);
void att_set_request_thread_deal(void);
void att_regist_thread_deal_cbk(void *cbk);
void att_regist_client_callback(btstack_packet_handler_t  cbk);

//----------------------------------

typedef struct {
    u8 *buf;
    u8 buf_size;
    u8 count;
    u8 read_pt;
    u8 write_pt;
} mini_cbuf_t;

void mini_cbuf_init(mini_cbuf_t *mcbuf, u8 *buf, u8 buf_size);
tbool mini_cbuf_read(mini_cbuf_t *mcbuf, u8 *value_pt);
tbool mini_cbuf_write(mini_cbuf_t *mcbuf, u8 value);
tbool mini_cbuf_is_emtpy(mini_cbuf_t *mcbuf);


#endif

#endif
