#ifndef _BT_TWS_
#define _BT_TWS_

enum {
    TWS_BT_STATUS_CONNECTING = 0,    /*当前主机与手机已连接成功*/
    TWS_BT_STATUS_TAKEING_PHONE = 1,  /*当前主机与手机正在通话*/
    TWS_BT_STATUS_PLAYING_MUSIC = 2,  /*当前主机与手机正在播放歌曲*/
    TWS_BT_STATUS_LINEIN_MUSIC = 3,  /*当前主机与手机正在播放linen歌曲*/
    TWS_BT_STATUS_CLEAR_STATE = 8,

};
enum {
    TWS_LINEIN_SR1600 = 0,
    TWS_LINEIN_SR32000,
    TWS_LINEIN_SR44100,
    TWS_LINEIN_SR48000,
};
enum {
    TWS_LINEIN_TASK_STATE = 0, //插入linein ，忽略消息
    TWS_LINEIN_TASK_BT = 1, //插入linein ，在蓝牙模式播linein
    TWS_LINEIN_TASK_AUX = 2, //插入linein ，切换到linein 模式
};

extern void tws_deal_cmd(int msg, u8 value);
extern bool tws_key_cmd_send(int msg, u8 value);
extern void tws_cmd_send(int msg, u8 value);
extern void user_tws_init();
extern void tws_inc_bt_irq_prio(u8 en);
extern void bt_tws_info_save(u8 info);
extern bool is_baseband_stata_inquiry_ing();
extern void bt_tws_delete_addr();
extern void set_tws_master_phone_state(u8 cmd, u8 state);
extern u8 get_tws_master_phone_state();
extern void tws_change_eir_priv_version(u8 en);
void tws_sync_fun(u8 set_cmd, u8 cmd);
void tws_sync_connect_deal();
void tws_sync_connect_left_deal();
void bt_tws_sync_connect_deal(u8 conn_role);
int bt_tws_linein_open();
int bt_tws_linein_close();
int set_tws_linein_state(u8 state);
int get_tws_linein_state();
void tws_linein_aux_open_callback();
s32 a2dp_tws_linein_media_to_sbc_encoder(void *pbuf, u32 len,  u8 ch);
void tws_change_ch_type(u8 type_ch);////此接口需要主机调用才能生效。从机切换的话通过消息告诉主机调用 0:master left  1:master right
#endif
