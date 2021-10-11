#include "bluetooth/avctp_user.h"
#include "task_bt.h"
#include "sdk_cfg.h"
#include "fcc_test.h"
#include "audio/ladc.h"
#include "audio/dac_api.h"
#include "audio/tone.h"
#include "aec_main.h"
#include "crc_api.h"
#include "power.h"
#include "aec_user.h"
#include "msg.h"
#include "audio/dac_api.h"
#include "audio/dac.h"
#include "audio/audio.h"
#include "a2dp_decode.h"
#include "clock_interface.h"
#include "flash_api.h"
#include "cpu/rand64.h"
#include "dev_manage.h"
#include "fs.h"
#include "bt_tws.h"
#include "irq_api.h"
#include "audio/syn_tws.h"
#include "common.h"
#include "resource_manage.h"

enum {
    BDHW_PAGE_STATE = 0,
    BDHW_INQUIRY_STATE,
    BDHW_PAGE_SCAN_STATE,
    BDHW_INQUIRY_SCAN_STATE,
    BDHW_CONNECTION_STATE,
};
extern u16 bt_debug_baseband_state;
static u8 tws_master_phone_state = 0;
static u8 tws_linein_state = 0;
extern u8 a2dp_get_status(void);
extern void ladc_to_dac(void *buf, u32 len);
extern void bt_work_state_control(u8 enble);
extern u8 get_tws_mem_role(void);
extern u8   get_aux_sta(void);
#if BT_TWS
static u8 bt_tws_rx_buf[8 * 1024] sec_used(.tws_sbc_rx)__attribute__((aligned(4)));//接收sbc cbuf缓存
static u32 inquiry_result_mem[150] sec(.bt_classic_data)__attribute__((aligned(4)));//搜索信息缓存
// *INDENT-OFF*
#define TWS_BUF_SIZE    (4*1024+200)//预解码编码work_buf[2500]+read_dec_buf[700]
// *INDENT-ON*
static u8 tws_buf[TWS_BUF_SIZE] sec(.bt_sbc_dec) __attribute__((aligned(4)));

/*设置对箱搜索标识，inquiry时候用,搜索到相应的标识才允许连接*/
char tws_device_indicate[2] = {'J', 'L'};
void tws_deal_cmd(int msg, u8 value)
{
    int tmp_msg = NO_MSG;
    log_printf("tws_rx_msg:0x%x,value:%d\n", msg, value);
    /*这里对一些主从都要实现的同步消息进行转换，防止同步消息循环发送*/
    switch (msg) {
    case MSG_VOL_UP:
    case MSG_VOL_DOWN:
    case MSG_VOL_UP_SHORT:
    case MSG_VOL_DOWN_SHORT:
        task_post_msg(NULL, 2, MSG_BT_TWS_VOL_KEY, value);
        return;
    case MSG_EQ_MODE:
        tmp_msg = MSG_EQ_MODE_TWS;
        break;
    case MSG_POWER_OFF_TWS:
        task_post_msg(NULL, 2, MSG_POWER_OFF_TWS, value);
        break;
    case MSG_BT_TWS_ALL_DELETE_ADDR:
        task_post_msg(NULL, 2, MSG_BT_TWS_DELETE_ADDR, value);
        break;
    case MSG_BT_TWS_STATE:
        tws_master_phone_state = value;
        return;
        break;
    case MSG_VOL_KEY_UP:
	case MSG_VOL_UP_HOLD_UP:
	case MSG_VOL_DOWN_HOLD_UP:
        tmp_msg = MSG_BT_TWS_VOL_KEY_UP;
        break;
    }
    if (tmp_msg != NO_MSG) {
        task_post_msg(NULL, 2, tmp_msg, value);
    } else {
        task_post_msg(NULL, 2, msg, value);
    }
}

/*
 *主机按键消息滤波器
 *return 0:表示主从机需要同步实现的按键消息
 *return 1:表示主从不需要同步实现的按键消息，过滤掉
 */
bool tws_master_cmd_filt(int msg)
{
    if ((msg == MSG_VOL_UP) 	||	\
        (msg == MSG_VOL_DOWN) ||	\
        (msg == MSG_VOL_UP_SHORT) 	||	\
        (msg == MSG_VOL_DOWN_SHORT) ||	\
        (msg == MSG_POWER_OFF_TWS) ||	\
        (msg == MSG_BT_TWS_ALL_DELETE_ADDR) ||	\
        (msg == MSG_BT_TWS_STATE) ||	\
        (msg == MSG_VOL_KEY_UP) ||		\
        (msg == MSG_VOL_UP_HOLD_UP) ||		\
        (msg == MSG_VOL_DOWN_HOLD_UP) ||		\
        (msg == MSG_EQ_MODE)) {
        return 0;
    } else {
        return 1;
    }
}
void bt_baseband_state_debug()
{
#if 0//debug BT_BDHW_STATE
    /* printf(" state=%x ",bt_debug_baseband_state); */
    if (bt_debug_baseband_state & BIT(BDHW_PAGE_STATE)) {
        putchar('q');
    }
    if (bt_debug_baseband_state & BIT(BDHW_INQUIRY_STATE)) {
        putchar('j');
    }
    if (bt_debug_baseband_state & BIT(BDHW_PAGE_SCAN_STATE)) {
        putchar('p');
    }
    if (bt_debug_baseband_state & BIT(BDHW_INQUIRY_SCAN_STATE)) {
        putchar('i');
    }
    if (bt_debug_baseband_state & BIT(BDHW_CONNECTION_STATE)) {
        putchar('c');
    }
#endif

}
bool tws_key_cmd_send(int msg, u8 value)
{
    u8 cmd_vlue[5];
    u8 tws_role;
    bt_baseband_state_debug();
    /*get_tws_device_role()返回0，表示没有对箱连接*/
    tws_role = get_tws_device_role();
    if ((tws_role == 0) || ((tws_role == TWS_ROLE_MASTER) && tws_master_cmd_filt(msg))) {
        /*
         *如果TWS没有连接，直接返回
         *主机有些按键消息,从机不需要同步实现
         */
        return FALSE;
    }
    switch (msg) {
    case MSG_VOL_DOWN:
    case MSG_VOL_UP:
	case MSG_VOL_DOWN_SHORT:
	case MSG_VOL_UP_SHORT:
        value = sound.vol.sys_vol_l;
    case MSG_EQ_MODE:
    case MSG_BT_NEXT_FILE:
    case MSG_BT_PREV_FILE:
    case MSG_BT_ANSWER_CALL:
    case MSG_BT_CALL_LAST_NO:
    case MSG_BT_CALL_REJECT:
    case MSG_BT_CALL_HANGUP:
    case MSG_BT_CALL_CONTROL:
    case MSG_BT_PP:
    case MSG_POWER_OFF_TWS:
    case MSG_BT_TWS_ALL_DELETE_ADDR:
    case MSG_BT_TWS_STATE:
    case MSG_BT_HID_TAKE_PIC:
    case MSG_VOL_KEY_UP:
    case MSG_VOL_DOWN_HOLD_UP:
    case MSG_VOL_UP_HOLD_UP:
        if ((msg == MSG_VOL_KEY_UP)
			||(msg == MSG_VOL_DOWN_HOLD_UP)
			||(msg == MSG_VOL_UP_HOLD_UP)){
            tws_cmd_send(msg, value);
        }
        log_printf("tws_tx_msg:0x%x", msg);
        tws_cmd_send(msg, value);
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

#define TWS_CMD_POOL_SIZE	20
typedef struct {
    int msg;
    u8 value;
} tws_cmd_t;
typedef struct {
    u8 idx;
    volatile u8 busy;
    volatile u32 mask;
} tws_cmd_var;
tws_cmd_var tws_cmd;
tws_cmd_t tws_cmd_pool[TWS_CMD_POOL_SIZE];
void tws_cmd_send(int msg, u8 value)
{
    //log_printf("tws_tx_cmd1:0x%x,value:%d\n", msg, value);
    if (tws_cmd.mask & BIT(tws_cmd.idx)) {
        puts("\n\n============tws_cmd_pool full\n");
        return;
    }
    if (tws_cmd.mask == 0) {
        //如果命令pool没有消息，从第一个开始存放新消息
        tws_cmd.idx = 0;
    }
    tws_cmd.busy = 1;
    tws_cmd_pool[tws_cmd.idx].msg = msg;
    tws_cmd_pool[tws_cmd.idx].value = value;
    tws_cmd.mask |= BIT(tws_cmd.idx);
    tws_cmd.idx++;
    if (tws_cmd.idx >= TWS_CMD_POOL_SIZE) {
        tws_cmd.idx = 0;
    }
    tws_cmd.busy = 0;
}

static void tws_cmd_scan(void)
{
    u32 res = 0;
    u8 cmd_vlue[5];
    u8 i;
    if (tws_cmd.busy || (tws_cmd.mask == 0)) {
        return;
    }
    for (i = 0; i < TWS_CMD_POOL_SIZE; i++) {
        if (tws_cmd.mask & BIT(i)) {
            memcpy(cmd_vlue, (u8 *)&tws_cmd_pool[i].msg, sizeof(int));
            cmd_vlue[4] = tws_cmd_pool[i].value;
            log_printf("tws_tx_cmd2:0x%x,value:%d\n", tws_cmd_pool[i].msg, tws_cmd_pool[i].value);
            res = user_send_cmd_prepare(USER_CTRL_TWS_COTROL_CDM, sizeof(cmd_vlue), cmd_vlue);
            if (res == 0) {
                /*send tws cmd ok*/
                tws_cmd.mask &= ~BIT(i);
            } else {
                /*send tws cmd failed*/
                printf(">>>tws_cmd res:%d\n", res);
                /*
                 * 以下情况清除tws消息池：
                 * (1)res = 5:edr task exit(connectless)
                 */
                if (res == 5) {
                    tws_cmd.mask = 0;
                }
            }
            break;
        }
    }
}
LOOP_DETECT_REGISTER(tws_cmd_loop) = {
    .time = 10,
    .fun  = tws_cmd_scan,
};

void user_tws_init()
{
    __set_tws_device_indicate(tws_device_indicate);
    inquiry_result_int(inquiry_result_mem, sizeof(inquiry_result_mem), INQUIRT_USER_PRIV_VERSION, tws_device_indicate);
    user_tws_int_handle(BT_TWS, tws_buf);

    user_set_tws_rx_buf_handle(bt_tws_rx_buf, sizeof(bt_tws_rx_buf), get_tws_sync_parm());
#if BT_TWS_LINEIN
    user_tws_linein_int_handle(BT_TWS_LINEIN, TWS_LINEIN_SR44100, NULL, 0, set_tws_linein_state, tws_linein_aux_open_callback);
#endif
    memset(&tws_cmd, 0, sizeof(tws_cmd));
    memset(tws_cmd_pool, 0, sizeof(tws_cmd_pool));
#if BT_TWS_SYNC_CON_STATE_ENBLE
    user_set_sync_conn_handle(tws_sync_fun);
#endif

}
void bt_tws_poweroff_togerther()
{
    if (is_tws_device_master() && (get_total_connect_dev() > 1)) {
        /*
         *(1)当前为TWS主机
         *(2)连接设备超过1个
         *综上，先断开和手机的连接，再一起关机,防止关机过程收到手机的数据
         */
        puts("disconn phone first\n");
        user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
    }
#if BT_TWS_POWEROFF_TOGETHER
    puts("bt_tws_poweroff_togerther\n");
    /* user_send_cmd_prepare(USER_CTRL_SNIFF_EXIT, 0, NULL); */
    /* delay_n10ms(2); */
    tws_key_cmd_send(MSG_POWER_OFF_TWS, 0xff);
    delay_n10ms(3);
#endif

}

#endif
bool is_baseband_stata_inquiry_ing()
{
    if (bt_debug_baseband_state & BIT(BDHW_INQUIRY_STATE)) {
        return TRUE;
    }
    return FALSE;
}
void bt_tws_delete_addr()
{
    puts("bt_tws_delete_addr");
#if BT_TWS
    user_send_cmd_prepare(USER_CTRL_TWS_CLEAR_INFO, 0, NULL);
    tws_key_cmd_send(MSG_BT_TWS_ALL_DELETE_ADDR, 0);
    delay_n10ms(3);
#endif

}
//主机发命令通知从机 主机与手机当前的状态
void set_tws_master_phone_state(u8 cmd, u8 state)
{
#if BT_TWS
    puts("set_tws_master_phone_state\n");
    if (cmd >= TWS_BT_STATUS_CLEAR_STATE) {
        puts("set_tws_master_phone_state clear state");
        tws_master_phone_state = 0;
    }
    if (is_tws_device_master()) {

        if (cmd < TWS_BT_STATUS_CLEAR_STATE) {
            if (state) {
                tws_master_phone_state |= BIT(cmd);
            } else {
                tws_master_phone_state &= ~BIT(cmd);
            }
        }
        tws_key_cmd_send(MSG_BT_TWS_STATE, tws_master_phone_state);
    }
#endif
}
//从机获取 主机与手机当前的状态
u8 get_tws_master_phone_state()
{
#if BT_TWS
    if (is_tws_device_slave()) {
        printf("get_tws_master_phone_state=%x\n", tws_master_phone_state);
        return tws_master_phone_state;
    }
#endif
    return 0;
}
void tws_change_eir_priv_version(u8 en)
{
#if BT_TWS
#if BT_TWS_SCAN_ENBLE
    u8 change_device_indicate[2];
    change_device_indicate[0] = tws_device_indicate[0] + 1;
    change_device_indicate[1] = tws_device_indicate[1] + 1;
    change_eir_priv_version(en, change_device_indicate);
#endif
#endif
}
/*******************linein tws********************/
struct tws_linein_parm_t tws_linein_parm;
int get_tws_linein_state()
{
#if BT_TWS_LINEIN
    return 	tws_linein_state;
#endif
    return 0;
}
int set_tws_linein_state(u8 state)
{
#if BT_TWS_LINEIN
    tws_linein_state = state;
    if (get_tws_linein_state() == BT_TWS_CUR_STATE_TWS_MASTER_SLAVE_LINEIN) {
        set_tws_master_phone_state(TWS_BT_STATUS_LINEIN_MUSIC, 1);
    }
    printf("set_tws_linein_state=0x%x\n", tws_linein_state);
#endif
    return 0;
}
static void bt_tws_linein_channel_close(void *parm)
{
#if BT_TWS_LINEIN
    puts("bt_tws_linein_channel_close\n");
    emitter_aux_close(NULL);
    mutex_resource_release("linein_tws");
    user_send_cmd_prepare(USER_CTRL_TWS_LINEIN_CLOSE, 0, NULL);
    set_tws_master_phone_state(TWS_BT_STATUS_LINEIN_MUSIC, 0);
    tws_linein_parm.adc_2_dac = 0;
    tws_linein_parm.rate = 0;
    tws_linein_state = 0;
#endif
}
void tws_linein_aux_open_callback()
{
#if BT_TWS_LINEIN
    tws_linein_parm.adc_2_dac = 0;
    tws_linein_parm.rate = SR44100;
    emitter_aux_open(&tws_linein_parm);
#endif
}
static int bt_tws_linein_channel_open(void *parm)
{
#if BT_TWS_LINEIN
    u8 role = 0;
    if (!get_aux_sta()) {
        return 0;
    }
    /*通话过程 主机插入linein 挂电话*/
    if ((BT_STATUS_CONNECTING == get_bt_connect_status()) ||
        (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) ||
        (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status())) {
        if (get_call_status() != BT_CALL_HANGUP) {
            /* puts("call hangup\n");           */
            /* return TWS_LINEIN_TASK_STATE; */
        } else if (get_tws_master_phone_state()&BIT(TWS_BT_STATUS_TAKEING_PHONE)) { //主机通话中
            /* puts("slave call hangup\n");     */
        }
    }

    if ((BT_STATUS_CONNECTING == get_tws_connect_status()) ||
        (BT_STATUS_TAKEING_PHONE == get_tws_connect_status()) ||
        (BT_STATUS_PLAYING_MUSIC == get_tws_connect_status())) { /*连接状态*/
        if (is_tws_device_master()) {//主机插入linein ,有手机连接先断开手机，然后开启tws_linein
            puts("bt_tws_linein_channel_open master\n");
            if (get_total_connect_dev() > 1) {
                user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);//断开手机连接
            }
            bt_work_state_control(0);
            tws_linein_aux_open_callback();
            user_send_cmd_prepare(USER_CTRL_TWS_LINEIN_START, 0, NULL);
            return TWS_LINEIN_TASK_BT;
        } else if (is_tws_device_slave()) {
            puts("bt_tws_linein_channel_open slave\n");
            if (get_tws_master_phone_state()&BIT(TWS_BT_STATUS_LINEIN_MUSIC)) {//从机插入linein,判断主机tws_lienin work,与主机连接，自己进行播放
                tws_linein_state = BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_PLAY_LINEIN;
            } else if (get_tws_master_phone_state()&BIT(TWS_BT_STATUS_CONNECTING)) { //从机插入linein,判断主机与手机连接
                puts("bt_tws_linein_channel_open return\n");
                return TWS_LINEIN_TASK_STATE;//从机插入linein,判断主机与手机连接或者主机tws_lienin work时，忽略此消息
            } else {
                /* setp 1 从机插入linein,先断开与主机的连接，然后从机进行连接主机，从机变回主，再开启tws_linein*/
                tws_linein_state = BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_LINEIN;
            }
            puts("BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_LINEIN setp1\n");
            task_post_msg(NULL, 1, MSG_BT_TWS_CONNECT_CTL);
            return TWS_LINEIN_TASK_BT;
        }
    } else {
        puts("bt_tws_linein_channel_open single\n");
        bt_tws_linein_channel_close(NULL);
        user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);//断开手机连接
        role = get_tws_mem_role();
        if (TWS_ROLE_MASTER == role) {
            tws_linein_state = BT_TWS_CUR_STATE_TWS_MASTER_LINEIN;
        } else if (TWS_ROLE_SLAVE == role) {
            tws_linein_state = BT_TWS_CUR_STATE_TWS_SLAVE_LINEIN;
        } else {
            tws_linein_state = BT_TWS_CUR_STATE_TWS_NOT_ROLE_PLAY_LINEIN;
        }
        tws_linein_parm.adc_2_dac = 1;
        tws_linein_parm.rate = SR44100;
        mutex_resource_apply("linein_tws", 4, emitter_aux_open, emitter_aux_close, &tws_linein_parm);
        bt_work_state_control(0);
        return TWS_LINEIN_TASK_BT;
    }
    return TWS_LINEIN_TASK_STATE;
#endif
    return TWS_LINEIN_TASK_STATE;
}
int bt_tws_linein_open()
{
#if BT_TWS_LINEIN
    puts("bt_tws_linein_open\n");
    return bt_tws_linein_channel_open(NULL);
#endif
    return 0;
}
int bt_tws_linein_close()
{
#if BT_TWS_LINEIN
    puts("bt_tws_linein_close\n");
    bt_tws_linein_channel_close(NULL);
    bt_work_state_control(1);
#endif
    return 0;

}
s32 tws_linein_media_to_sbc_encoder(void *pbuf, u32 len,  u8 ch)
{
#if BT_TWS_LINEIN
    if (get_tws_linein_state() == BT_TWS_CUR_STATE_TWS_MASTER_SLAVE_LINEIN) {
        return a2dp_tws_linein_media_to_sbc_encoder(pbuf, len, ch);
    } else if (get_tws_linein_state()) {
        ladc_to_dac(pbuf, len);
    }
#endif
    return 0;
}

