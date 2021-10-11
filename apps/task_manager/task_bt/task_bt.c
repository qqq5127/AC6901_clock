#include "task_bt.h"
#include "task_bt_key.h"
#include "msg.h"
#include "task_manager.h"
#include "task_common.h"
#include "bluetooth/avctp_user.h"
#include "aec_main.h"
#include "uart.h"
#include "audio/dac_api.h"
#include "audio/dac.h"
#include "audio/audio.h"
#include "string.h"
#include "sdk_cfg.h"
#include "common/sys_timer.h"
#include "warning_tone.h"
#include "led.h"
#include "charge.h"
#include "power/power.h"
#include "tone.h"
#include "common/res_file.h"
#include "resource_manage.h"
#include "music_player.h"
#include "bt_ui.h"
#include "updata.h"
#include "bt_tws.h"
#include "task_echo_key.h"
#include "flash_api.h"
#include "echo_api.h"
#include "fmtx_api.h"
#include "fat_io.h"
#include "rec_api.h"
#include "clock.h"
#include "rtc_setting.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".task_bt_bss")
#pragma data_seg(	".task_bt_data")
#pragma const_seg(	".task_bt_const")
#pragma code_seg(	".task_bt_code")
#endif



/* #define TASK_BT_DEBUG_ENABLE */
#ifdef TASK_BT_DEBUG_ENABLE
#define task_bt_printf log_printf
#define task_bt_puts puts
#else
#define task_bt_printf(...)
#define task_bt_puts(...)
#endif// TASK_BT_DEBUG_ENABLE

#define		BT_CUR_STATE_PHONE 				2	/*连接手机					*/
#define    	BT_CUR_STATE_TWS_MASTER		   	4	/*连接主机(本身是从机)		*/
#define    	BT_CUR_STATE_TWS_SLAVE			8	/*连接从机(本身是主机)		*/
#define    	BT_CUR_STATE_TWS_SLAVE_PHONE	10	/*连接从机和手机(本身是主机)*/

#define TWS_SYNC_PLAY_TONE_SLOT     100
#define TWS_SYNC_LED_SCAN_SLOT      50

enum {
    TWS_SYNC_PLAY_TONE = 1,
    TWS_SYNC_LED_SCAN = 2,
};
enum {
    UPDATA_OSC_TRIM = 1,
    UPDATA_BT_NAME,
};

enum {
    USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_DISBLE,
    USER_CTRL_WRITE_SCAN_EN_OR_CONN_EN,
    USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_EN,
    USER_CTRL_WRITE_SCAN_EN_OR_CONN_DISBLE,

};
struct user_ctrl {
    u8 phone_num_len;
    u8 income_phone_num[30];
    u8 auto_connection_counter;
    u8 connect_addr[6];
    u8 phone_connect_addr[6];
    u8 phone_ring_flag;
    u8 phone_num_flag;
    u8 phone_num_idx;
    u8 inband_ringtone;
    u16 auto_shutdown_cnt;
    u8 last_call_type;
    struct sys_timer bt_connect_timeout;
    struct sys_timer bt_prompt_timeout;
    struct sys_timer bt_linein_tws_play;
    u16 power_on_cnt;
    u8 update_name_end;
    u8 resume_flag;
    u8 auto_suspend_flag;
    u8 tws_role;
    u8 not_play_bt_mode_tone;
    u8 inquiry_flag;
    u8 tws_change_mode_state;
    u8 broadcast_state;
    u8 connected_bd;/*保存当前连接设备类型*/
    u8 conn_tws_after_conn_phone;/*回连完手机再回连对箱*/
    u8 conn_phone_after_conn_tws;/*回连完对箱再回连手机*/
};

static struct user_ctrl __user_val sec_used(.ram1_bss);
#define user_val (&__user_val)

BT_DIS_VAR bt_ui_var;
RECORD_OP_API *bt_rec_api = NULL;
extern u8 a2dp_get_status(void);
extern bool ble_msg_deal(u32 param);

void app_send_keypress(u8 key)
{
    user_send_cmd_prepare(USER_CTRL_KEYPRESS, 1, &key);
}

/*开关可发现可连接的函数接口*/
void bt_work_state_control(u8 enble)
{
    printf("bt_work_state_control=%d,1t2_connection=%d,is_tws_device_slave=%d\n", enble, is_1t2_connection(), is_tws_device_slave());
    u8 bt_state_control = 0;
    if (sys_global_value.fast_test_flag == 0x1A) {
        enble = 0;
    }
    if (enble == 0) {
        /*关闭蓝牙可发现可连接*/
        bt_state_control = USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_DISBLE;
    } else if (enble == 1) {
        bt_state_control = USER_CTRL_WRITE_SCAN_EN_OR_CONN_EN;
        if (is_tws_device_slave() || is_1t2_connection()) {
            /*
             *可发现(0)可连接(0):即以下情况蓝牙不可发现不可连接
             *(1)设备作为TWS从机连接成功
             *(2)连接设备数已经达到最大
             */
            bt_state_control = USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_DISBLE;
        } else if (get_total_connect_dev() == 1) {
            /*已经连接一台设备，可发现(0)可连接(1)，给已经配对过的手机连接*/
            bt_state_control = USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_EN;
#if BT_TWS
            if (is_tws_device_master()) {
                /*只连接对箱，可发现(1)可连接(1)，给手机连接*/
                bt_state_control = USER_CTRL_WRITE_SCAN_EN_OR_CONN_EN;
            } else {
                if (user_val->tws_role == TWS_ROLE_SLAVE) {
                    /*只连接手机，作为从机，不可发现不可连接*/
                    bt_state_control = USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_DISBLE;
                } else {
                    /*只连接手机，作为主机，不可发现可连接*/
                    bt_state_control = USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_EN;
                }
            }

#endif
        }
#if BT_TWS_LINEIN
        if (get_tws_linein_state()) {
            bt_state_control = USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_DISBLE;
        }
#endif
    }
    printf("<bt_state_crtl=0x%x>\n", bt_state_control);
    switch (bt_state_control) {
    case USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_DISBLE:
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
        break;
    case USER_CTRL_WRITE_SCAN_EN_OR_CONN_EN:
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
        break;
    case USER_CTRL_WRITE_SCAN_DISBLE_OR_CONN_EN:
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
        break;
    case USER_CTRL_WRITE_SCAN_EN_OR_CONN_DISBLE:
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
        break;
    }
}
u16 control_power_on_cnt(u8 mode, u16 poweron_cnt)
{
    if (POWER_ON_CNT_SET == mode) {
        user_val->power_on_cnt = poweron_cnt;
    } else if (POWER_ON_CNT_INC == mode) {
        if (user_val->power_on_cnt) {
            user_val->power_on_cnt--;
        }
    }
    return user_val->power_on_cnt;
}
static void __control_power_on_cnt()
{
    control_power_on_cnt(POWER_ON_CNT_INC, 0);
}
LOOP_DETECT_REGISTER(power_on_cnt_detect) = {
    .time = 250,
    .fun  = __control_power_on_cnt,
};

void update_bt_tws_info(u8 info)
{
    user_val->tws_role = info;
}

/*获取到来电电话号码，用于回拨功能*/
void hook_hfp_incoming_phone_number(char *number, u16 length)
{
    user_val->phone_num_len = length;
    //printf("%x",sizeof(user_val->income_phone_num));

    if (length <= sizeof(user_val->income_phone_num)) {
        memcpy(user_val->income_phone_num, number, length);
    }
    if (user_val->phone_num_len) {
        user_val->phone_num_flag = 1;
        puts("phone_num:");
        put_buf(user_val->income_phone_num, user_val->phone_num_len);
    }

}

void bt_update_name_end()
{
    if (user_val->update_name_end == 0xaa) {
        puts("bt_update_name_end\n");
        bt_work_state_control(0);
        set_sys_vol(SYS_DEFAULT_VOL, SYS_DEFAULT_VOL, FADE_OFF);
        sin_tone_play(500);
        led_bt_update_name_end();//led_fre_set(C_ALL_ON_MODE);

    }

}

void phone_ring_play_timer(struct sys_timer *ts)
{
    if (user_val->phone_ring_flag) {
        task_post_msg(NULL, 1, MSG_BT_TONE_RING);
        sys_timer_reset(ts, 4000);
    }
}

void phone_num_play()
{
    /*play 0~9*/
    if ((user_val->income_phone_num[user_val->phone_num_idx] >= 0x30) &&
        (user_val->income_phone_num[user_val->phone_num_idx] <= 0x39)) {
        if (user_val->phone_num_idx == (user_val->phone_num_len - 1)) {
            tone_play(TONE_NUM_0 + user_val->income_phone_num[user_val->phone_num_idx] - 0x30, 0);
        } else {
            tone_play(TONE_NUM_0 + user_val->income_phone_num[user_val->phone_num_idx] - 0x30, 1);
        }
    }
    user_val->phone_num_idx++;
    if (user_val->phone_num_idx >= user_val->phone_num_len) {
        user_val->phone_num_idx = 0;;
        sys_timer_remove(&user_val->bt_prompt_timeout);
        if (user_val->inband_ringtone == 0) {
            /*
             *报号完接着播来电提示音
             *这个定时如果过长，同时又支持inband_ringtone，就会先播一下inband_ringtone
             */
            sys_timer_register(&user_val->bt_prompt_timeout, 1500, phone_ring_play_timer, 1);
        }
    }
}
void phone_num_play_timer(struct sys_timer *ts)
{
    if (user_val->phone_num_flag) {
        if (user_val->phone_num_idx < user_val->phone_num_len) {
            task_post_msg(NULL, 1, MSG_BT_TONE_PHONE_NUM);
            sys_timer_reset(ts, 1200);
        }
    } else {
        /*电话号码还没有获取到，定时查询*/
        sys_timer_reset(ts, 200);
    }
}

void phone_num_play_start()
{
    user_val->phone_num_flag = 0;
    user_val->phone_num_idx = 0;
    sys_timer_register(&user_val->bt_prompt_timeout, 500, phone_num_play_timer, 1);
}

void phone_ring_play_start()
{
    /* check if support inband ringtone */
    if (user_val->inband_ringtone == 0) {
        sys_timer_register(&user_val->bt_prompt_timeout, 500, phone_ring_play_timer, 1);
    }
}
void bt_linein_tws_auto_play(struct sys_timer *ts)
{
#if BT_TWS_LINEIN
    puts("bt_linein_tws_auto_play\n");
    if (get_tws_linein_state() == BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_LINEIN) {
        /* step 3从机插入linein,先断开与主机的连接，然后从机进行连接主机，从机变回主，再开启tws_linein*/
        puts("BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_LINEIN setp 3\n");
        task_post_msg(NULL, 1, MSG_BT_TWS_LINEIN_START);
    } else if (get_tws_linein_state() == BT_TWS_CUR_STATE_TWS_MASTER_LINEIN) {
        puts("BT_TWS_CUR_STATE_TWS_MASTER_LINEIN setp 1\n");
        task_post_msg(NULL, 1, MSG_BT_TWS_LINEIN_STOP);
        task_post_msg(NULL, 1, MSG_BT_TWS_LINEIN_START);
    }
    sys_timer_remove(&user_val->bt_linein_tws_play);
#endif
}
/*后台管理的几个函数*/
#if BT_BACKGROUND_EN
void hook_btstack_suspend()
{
    /*蓝牙电话自动回蓝牙，挂断后自动回上一个模式 */
    user_val->auto_suspend_flag = 0;
//	if(!is_bt_in_background_mode())
    {
        task_post_msg(NULL, 1, MSG_LAST_WORKMOD);
    }
}
bool get_auto_suspend_flag()
{
    return user_val->auto_suspend_flag ? true : false;
}
bool bredr_auto_suspend()
{
    puts("auto suspend\n");
    if (get_auto_suspend_flag() && (!is_bt_stack_cannot_exist())) {
        hook_btstack_suspend();
        return TRUE;
    }
    return FALSE;
}
void hook_btstack_resume(u8 suspend_flag)
{
    if (get_is_in_background_flag() && user_val->resume_flag != 0xff) {
        user_val->auto_suspend_flag = suspend_flag;
        user_val->resume_flag = 0xff;
        task_post_msg(NULL, 1, MSG_BACK_TO_BT);
    }
}
bool get_resume_flag()
{
    return (user_val->resume_flag == 0xff) ? true : false;
}
void clean_auto_suspend_flag()
{
    user_val->auto_suspend_flag = 0;
}
u32 msg_mask_off_in_bt(u32 msg)
{
    if (is_bt_stack_cannot_exist()) {
        sys_global_value.mask_task_switch_flag = 1;
    } else {
        sys_global_value.mask_task_switch_flag = 0;
    }

    return msg;
}

#else

u32 msg_mask_off_in_bt(u32 msg)
{
    return msg;
}
void hook_btstack_resume(u8 suspend_flag)
{

}
#endif
/******后台管理函数end********/

/*协议栈状态变化用户处理接口*/
int btstack_status_update_deal(u8 *info, u16 len)
{
    u8 status;
    u8 newest_bd = 0;
    u8 zero_addr[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    // put_buf(info, len);
    status = info[3];
    if (info[0] == 4 && info[3] == BT_STATUS_PHONE_MANUFACTURER) {
        //put_buf(&info[4], 6);//address
        //puts((const char *)&info[4+6]);
    }
#if BT_BACKGROUND_EN
    if (info[0] == 5) {
        //background state
        if (info[3] == 0x01) {
            user_val->not_play_bt_mode_tone = 0xaa;
            hook_btstack_resume(info[4]);
        } else if (info[3] == 0x02) {
            bredr_auto_suspend();
        }
        return 0 ;
    }
#endif
    switch (status) {
    case BT_STATUS_INIT_OK:
        task_bt_puts("BT_STATUS_INIT_OK\n");

#if ((BT_MODE==TEST_FRE_OFF_MODE) || (BT_MODE==TEST_FCC_MODE))
        task_post_msg(NULL, 1, MSG_BT_FCC);
        break;
#endif

#if (BT_MODE != NORMAL_MODE)
        bt_work_state_control(1);
        break;
#endif

        led_bt_idle();//led_fre_set(C_RB_FAST_MODE);


#if BT_TWS
#if BT_TWS_SCAN_ENBLE
        tws_change_eir_priv_version(1);
#endif
        extern u8 get_tws_mem_role(void);
        user_val->tws_role = get_tws_mem_role();
        if (user_val->tws_role == TWS_ROLE_MASTER) {
            puts(">>>>TWS_MASTER\n");
            user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
        } else {
            puts(">>>>TWS_SLAVE\n");
#if BT_TWS_ROLE_SWITCH
            clear_current_search_index();
            bt_work_state_control(1);
#else
            user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
            break;
#endif
        }

#else
        user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
#endif

        //  u8 temp_addr[6] = {0x9c, 0xfb, 0xd5, 0xe0, 0x9b, 0xd6};
        /* user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, temp_addr); */

        break;
    case BT_STATUS_INQUIRY_TIMEOUT:
        printf("inquiry_timeout:%d\n", info[4]);
        user_val->inquiry_flag = 0;
        /*info[4]:inquiry_result
         *0:inquiry_timeout
         *1:inquiry_complete
         */
#if BT_TWS_SCAN_ENBLE
        tws_change_eir_priv_version(1);
#endif
        if (info[4] == 0) {
            bt_work_state_control(1);
        }
        break;
#if BT_TWS
#if (BT_TWS&BT_TWS_BROADCAST)
    case BT_STATUS_BROADCAST_STATE:
        user_val->broadcast_state = info[4];
        printf("<<<<<<<BT_STATUS_BROADCAST_STATE=%d>>>>>>\n", info[4]);
        if (user_val->broadcast_state == BRAOABCASET_MASTER_CON_STOP || user_val->broadcast_state == BRAOABCASET_MASTER_CANCLE_STOP || user_val->broadcast_state == BRAOABCASET_SLAVE_SCAN_STOP) {
            bt_work_state_control(1);
            if (user_val->broadcast_state == BRAOABCASET_MASTER_CANCLE_STOP || user_val->broadcast_state == BRAOABCASET_SLAVE_SCAN_STOP) {
                user_val->broadcast_state = 0;
            }
        } else if (user_val->broadcast_state == BRAOABCASET_SLAVE_CON_STOP) {
            user_val->broadcast_state = 0;
            task_post_msg(NULL, 1, MSG_BT_TRAIN_SCAN_DEVICE);
        } else {
            bt_work_state_control(0);
        }
        break;
#endif
    case BT_STATUS_MONITOR_WAITING_CONN:
        puts("BT_STATUS_MONITOR_WAITING_CONN\n");
#if (BT_TWS&BT_TWS_BROADCAST)
        if (user_val->broadcast_state == BRAOABCASET_MASTER_CON_STOP) {
            user_val->broadcast_state = 0;
            task_post_msg(NULL, 1, MSG_BT_TRAIN_DEVICE);
        }
#endif
#if (BT_TWS&BT_TWS_MONITOR)
        bt_work_state_control(0);
        user_send_cmd_prepare(USER_CTRL_MONITOR, 0, NULL);
#endif
        break;
    case BT_STATUS_BT_TWS_CONNECT_CMD:
        log_printf("BT_STATUS_BT_TWS_CONNECT_CMD:%d\n", info[4]);
        if (!info[4]) {
            bt_work_state_control(0);
            break;
        }
        bt_work_state_control(1);
#endif
    case BT_STATUS_FIRST_CONNECTED:
    case BT_STATUS_SECOND_CONNECTED:
        user_val->auto_suspend_flag = 0;   /*每次连接成功清除自动返回上一个模式标识*/
#if BT_TWS
        /*判断当前连接的设备类型:对箱或者手机*/
        newest_bd = get_cur_conn_bd(1);
        log_printf("BT_CONNTCTED:%d\n", newest_bd);
        user_val->connected_bd |= BIT(newest_bd);
        if (newest_bd == BT_CUR_CONN_PHONE) {
            puts("[connect_dev]phone\n");
            user_val->conn_phone_after_conn_tws = 0;
            memset(user_val->phone_connect_addr, 0, 6);
#if BT_TWS_LINEIN
            set_tws_linein_state(0);
#endif
        } else if (newest_bd == BT_CUR_CONN_TWS_MASTER) {
            user_val->conn_tws_after_conn_phone = 0;
            puts("[connect_dev]tws_master\n");
#if BT_TWS_SCAN_ENBLE
            user_send_cmd_prepare(USER_CTRL_INQUIRY_CANCEL, 0, NULL);
#endif
        } else if (newest_bd == BT_CUR_CONN_TWS_SLAVE) {
            user_val->conn_tws_after_conn_phone = 0;
            puts("[connect_dev]tws_slave\n");
        }
#endif


#if BT_TWS
        if (user_val->connected_bd == BT_CUR_STATE_TWS_SLAVE) {
            led_bt_idle();//led_fre_set(C_RB_FAST_MODE);
        } else {
            led_bt_connect();//led_fre_set(C_BLED_FAST_ONE_5S_MODE);
        }
        if (user_val->connected_bd == BT_CUR_STATE_TWS_SLAVE_PHONE) {
            set_tws_master_phone_state(TWS_BT_STATUS_CONNECTING, 1);
        }
        user_val->auto_shutdown_cnt = AUTO_SHUT_DOWN_TIME;
        bt_tws_sync_connect_deal(newest_bd);
#else
        task_post_msg(NULL, 1, MSG_BT_TONE_CONN);
        led_bt_connect();//led_fre_set(C_BLED_FAST_ONE_5S_MODE);
#endif
        break;
    case BT_STATUS_FIRST_DISCONNECT:
    case BT_STATUS_SECOND_DISCONNECT:
#if BT_TWS
        /*判断当前断开的设备类型：对箱或者手机*/
        newest_bd = get_cur_conn_bd(0);
        log_printf("BT_DISCONN:%d\n", newest_bd);
        user_val->connected_bd &= ~BIT(newest_bd);
        if (newest_bd == BT_CUR_CONN_PHONE) {
            puts("[disconn_dev]phone\n");
            set_tws_master_phone_state(TWS_BT_STATUS_CLEAR_STATE, 0);
        } else if (newest_bd == BT_CUR_CONN_TWS_MASTER) {
            puts("[disconn_dev]tws_master\n");
            set_tws_master_phone_state(TWS_BT_STATUS_CLEAR_STATE, 0);
        } else if (newest_bd == BT_CUR_CONN_TWS_SLAVE) {
            puts("[disconn_dev]tws_slave\n");
        }
        if (user_val->connected_bd == BT_CUR_STATE_PHONE) {
            led_bt_connect();//led_fre_set(C_BLED_FAST_ONE_5S_MODE);
        } else {
            led_bt_idle();//led_fre_set(C_RB_FAST_MODE);
        }
        if (newest_bd == BT_CUR_CONN_PHONE)
		{
        	task_post_msg(NULL, 1, MSG_BT_TONE_DISCONN);
        }
		else
		{
        	task_post_msg(NULL, 1, MSG_BT_TONE_DISCONN_TWS);
		}
#else
        task_post_msg(NULL, 1, MSG_BT_TONE_DISCONN);
#endif
        break;

    case BT_STATUS_PHONE_INCOME:
        user_val->phone_ring_flag = 1;
#if BT_PHONE_NUMBER
        phone_num_play_start();
#else
        phone_ring_play_start();
#endif
        user_send_cmd_prepare(USER_CTRL_HFP_CALL_CURRENT, 0, NULL); //发命令获取电话号码
        sound_automute_set(0, -1, -1, -1); // 关自动mute
        task_post_event(NULL, 1, EVENT_AUTOMUTE_OFF);
        set_tws_master_phone_state(TWS_BT_STATUS_TAKEING_PHONE, 1);
        task_post_msg(NULL, 1, MSG_BT_REC_EXIT);
        puts("phone_income\n");
        break;
    case BT_STATUS_PHONE_OUT:
        sound_automute_set(0, -1, -1, -1); // 关自动mute
        task_post_event(NULL, 1, EVENT_AUTOMUTE_OFF);
        user_send_cmd_prepare(USER_CTRL_HFP_CALL_CURRENT, 0, NULL); //发命令获取电话号码
        set_tws_master_phone_state(TWS_BT_STATUS_TAKEING_PHONE, 1);
        task_post_msg(NULL, 1, MSG_BT_REC_EXIT);
        puts("phone_out\n");
        break;

    case BT_STATUS_PHONE_ACTIVE:
        if (1 == user_val->phone_ring_flag) {
            user_val->phone_ring_flag = 0;
            puts("call active,release tone\n");
            mutex_resource_release("tone");
            sys_timer_remove(&user_val->bt_prompt_timeout);
        }
        puts("phone_active\n");
        break;
    case BT_STATUS_PHONE_HANGUP:
        if (1 == user_val->phone_ring_flag) {
            user_val->phone_ring_flag = 0;
            mutex_resource_release("tone");
            sys_timer_remove(&user_val->bt_prompt_timeout);
        }
        task_post_event(NULL, 1, EVENT_AUTOMUTE_ON);
#if BT_TWS
        if ((user_val->tws_role == TWS_ROLE_MASTER) && (get_tws_device_role() == 0)) {
            /*主机打完电话，如果对箱没有连接，继续回连对箱*/
            /*主机打完电话，如果对箱没有连接，继续回连对箱*/
            u8 tws_addr[6];
            if (get_tws_db_addr(tws_addr)) {
                task_post_msg(NULL, 1, MSG_BT_TWS_CONNECT_CTL);
            } else {
                bt_work_state_control(1);
            }
        } else {
            bt_work_state_control(1);
        }
        /* tws_cmd_send(MSG_BT_TWS_VOL_SYNC, sound.vol.sys_vol_l); */
#endif
        set_tws_master_phone_state(TWS_BT_STATUS_TAKEING_PHONE, 0);
        task_post_msg(NULL, 1, MSG_BT_REC_EXIT);
        puts("phone_hangup\n");
        break;
    case BT_STATUS_INBAND_RINGTONE:
        log_printf("inband ringtone:0x%x\n", info[4]);
#if BT_INBAND_RINGTONE
        user_val->inband_ringtone = info[4];
#else
        user_val->inband_ringtone = 0;
#endif
        break;
    case BT_STATUS_MUSIC_SOUND_COME:
        puts("SOUND COME\n");
        extern void tws_resource_a2dp_apply_control(u8 en, u32 limit_size);
        tws_resource_a2dp_apply_control(1, 2 * 1024);
        set_tws_master_phone_state(TWS_BT_STATUS_PLAYING_MUSIC, 1);
        break;
    case BT_STATUS_MUSIC_SOUND_GO:
        puts("SOUND go\n");
        set_tws_master_phone_state(TWS_BT_STATUS_PLAYING_MUSIC, 0);
        break;
    case BT_STATUS_CONN_A2DP_CH:
        log_printf("BT_STATUS_CONN_A2DP_CH:%d\n", is_1t2_connection(), user_val->conn_phone_after_conn_tws);
#if BT_TWS
        /*对箱从机，不需要继续连下一个设备*/
        if (is_tws_device_slave() || (user_val->tws_role == TWS_ROLE_SLAVE)) {
            puts("tws_slave,break\n");
            break;
        }
        if (user_val->conn_tws_after_conn_phone) {//回连成功phone继续回连tws
            puts("phone connected conn tws now\n");
            user_val->conn_tws_after_conn_phone = 0;
            sys_timer_remove(&user_val->bt_connect_timeout);
            user_send_cmd_prepare(USER_CTRL_TWS_START_CONNECTION, 0, NULL);
        }
        if (user_val->conn_phone_after_conn_tws && memcmp(user_val->phone_connect_addr, zero_addr, 6)) {
            //回连成功tws,继续回连phone
            user_val->conn_phone_after_conn_tws = 0;
            puts("phone connected conn now\n");
            user_val->auto_connection_counter = 10;
            sys_timer_remove(&user_val->bt_connect_timeout);
            user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, user_val->phone_connect_addr);
            memset(user_val->phone_connect_addr, 0, 6);

        }
#if BT_TWS_LINEIN
        sys_timer_register(&user_val->bt_linein_tws_play, 1000, bt_linein_tws_auto_play, 1);
#endif

#endif
        if (!is_1t2_connection() && get_current_search_index()) { //回连下一个device
            puts("conn_next_dev\n");
            user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
        }
        break;
    case BT_STATUS_RECONN_OR_CONN:
        log_printf("BT_STATUS_RECONN_OR_CONN:%d\n", info[4]);
        if (info[4]) {
            task_bt_puts("reconn ok\n");
        } else {
            task_bt_puts("phone conn ok\n");
        }
        if (get_remote_test_flag()) {
            bt_work_state_control(0);
        } else {
            bt_work_state_control(1);
        }
        break;
    case BT_STATUS_SNIFF_DAC_CTL:
        log_printf("BT_STATUS_SNIFF_dac:%d\n", info[4]);
        if (info[4]) {
            task_post_msg(NULL, 1, MSG_DAC_ON);
        } else {
            task_post_msg(NULL, 1, MSG_DAC_OFF);
        }
        break;
    case BT_STATUS_LAST_CALL_TYPE_CHANGE:
        log_printf("BT_STATUS_LAST_CALL_TYPE_CHANGE:%d\n", info[4]);
        user_val->last_call_type = info[4];
        break;
    case BT_STATUS_CALL_VOL_CHANGE:
#if BT_PHONE_VOL_SYNC
    {
        extern bool get_esco_busy_flag();
        log_printf("VOL:%d,phone_vol:%d\n", sound.vol.sys_vol_l, info[4]);
        sound.phone_vol = info[4];
        if (!get_esco_busy_flag()) {
            break;
        }
        sound.vol.sys_vol_l = (info[4] * aec_param.dac_analog_gain / 15) ;
        sound.vol.sys_vol_r = sound.vol.sys_vol_l;
        set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
    }
#endif
    break;
    case BT_STATUS_PHONE_NUMBER:
        log_printf("BT_STATUS_PHONE_NUMBER\n");
        hook_hfp_incoming_phone_number((char *)&info[6], info[5]);
        break;
    case BT_STATUS_PHONE_DATE_AND_TIME:
        log_printf("BT_STATUS_PHONE_DATE_AND_TIME\n");
        puts((const char *)&info[6]);
        break;
    case BT_STATUS_CONNECT_WITHOUT_LINKKEY:
        //log_printf("BT_STATUS_CONNECT_WITHOUT_LINKKEY\n");
        if (info[4] == 1 || info[4] == 2) {
            // user_val->not_play_bt_mode_tone = 0xaa;
            // hook_btstack_resume(0);
        }
        if (info[4] == 3) {
            user_val->not_play_bt_mode_tone = 0xaa;
            hook_btstack_resume(0);
        }
        clear_led_rb_flag();
        break;
    case BT_STATUS_A2DP_ABORT_STATE:
        puts("<<<<<<<<<<<<<<BT_STATUS_A2DP_ABORT_STATE\n");
        if (user_val->tws_role == TWS_ROLE_MASTER) {
            user_val->broadcast_state = BRAOABCASET_MASTER_CON_STOP;
        }
        break;

    default:
        break;
    }
    return 0;
}
//进入powerdonw校准自动关机时间
void low_pwr_deal_time(u32 time_ms)
{
    static u16 low_pwr_time_ms_cnt = 0;
    static u16 low_pwr_time_ms_cnt1 = 0;
    u8 halfsec = 0;
    low_pwr_time_ms_cnt += (time_ms + low_pwr_time_ms_cnt1);
    low_pwr_time_ms_cnt1 = 0;

    if (low_pwr_time_ms_cnt >= 500) {
        halfsec = low_pwr_time_ms_cnt / 500;
        low_pwr_time_ms_cnt1 = low_pwr_time_ms_cnt % 500;
        low_pwr_time_ms_cnt = 0;

        if (user_val->auto_shutdown_cnt != AUTO_SHUT_DOWN_TIME) {
            if (user_val->auto_shutdown_cnt >= halfsec) {
                user_val->auto_shutdown_cnt -= halfsec;
            }
            if (!user_val->auto_shutdown_cnt) {
                /* bt_msg_power_off(); */
            }

        }
    }
}
//主从、手机都连接上一起点灯
void tws_sync_led_connect()
{
#if BT_TWS_SYNC_CON_STATE_ENBLE
    if (user_val->connected_bd == BT_CUR_STATE_TWS_SLAVE_PHONE || user_val->connected_bd == BT_CUR_STATE_TWS_MASTER) {
        clear_led_cnt();
    }
#endif
}
//主从、连接上sync play tone ,sync_led_scan
void tws_sync_connect_deal()
{
#if BT_TWS_SYNC_CON_STATE_ENBLE
    puts("tws_sync_play_connect_tone\n");
    task_post_msg(NULL, 1, MSG_BT_TONE_CONN_RIGHT);//主从、连接上sync play tone
    tws_sync_led_connect();//主从、手机都连接上一起点灯
#endif
}
//主从、连接上sync play tone_left ,sync_led_scan
void tws_sync_connect_left_deal()
{
#if BT_TWS_SYNC_CON_STATE_ENBLE
    puts("tws_sync_play_connect_tone\n");
    task_post_msg(NULL, 1, MSG_BT_TONE_CONN_LEFT);//主从、连接上sync play tone
    tws_sync_led_connect();//主从、手机都连接上一起点灯
#endif
}
//从rec sync 连接成功，播连接成功提示音/led_scan
void tws_sync_fun(u8 set_cmd, u8 cmd)
{
#if BT_TWS
#if BT_TWS_SYNC_CON_STATE_ENBLE
    static u8 tws_sync_cmd = 0;
    /* puts("tws_sync_fun\n"); */
    if (set_cmd) {
        tws_sync_cmd = cmd;
        return ;
    }
    switch (tws_sync_cmd) {
    case TWS_SYNC_PLAY_TONE:
        puts("sync play tone\n");
        tws_sync_connect_deal();
        break;
    case TWS_SYNC_LED_SCAN:
        puts("sync led scan\n");
        tws_sync_led_connect();
        break;
    }
    tws_sync_cmd = 0;
#endif

#endif
}
//主连接成功，粗调法sync_send同时播连接成功提示音/led_scan
void bt_tws_sync_connect_deal(u8 conn_role)
{
#if BT_TWS
#if BT_TWS_SYNC_CON_STATE_ENBLE
    u8 wait_try_cnt = 3;
    while (wait_try_cnt) {
        wait_try_cnt--;
        if (conn_role == BT_CUR_CONN_TWS_SLAVE) { //主机send sync_pany_tone
            if (send_sync_tws_cmd_fun(TWS_SYNC_PLAY_TONE, TWS_SYNC_PLAY_TONE_SLOT, tws_sync_connect_left_deal)) {
                return;
            }
        } else if (conn_role == BT_CUR_CONN_TWS_MASTER) { //从机等待rec sync_pany_tone
            return;
        } else if (conn_role == BT_CUR_CONN_PHONE && user_val->connected_bd == BT_CUR_STATE_TWS_SLAVE_PHONE) { //当前连接是手机，并且主从已经连接上
            if (send_sync_tws_cmd_fun(TWS_SYNC_LED_SCAN, TWS_SYNC_LED_SCAN_SLOT, tws_sync_led_connect)) {
                break;
            }
        }
    }
#endif
    task_post_msg(NULL, 1, MSG_BT_TONE_CONN);
#endif

}

static void sys_time_auto_connection_deal(struct sys_timer *ts)
{
    if (user_val->auto_connection_counter && get_call_status() == BT_CALL_HANGUP) {
        bt_work_state_control(0);
        log_printf("auto_conn_cnt:%d\n", user_val->auto_connection_counter);
        user_val->auto_connection_counter--;
        clear_led_rb_flag();
        user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, user_val->connect_addr);
        if (get_total_connect_dev() == 0) {
            bt_work_state_control(1);
        }
    }
}
static void sys_timer_auto_connect(u8 *addr, u8 reason)
{
    if (reason != 0x0a) {
        memcpy(user_val->connect_addr, addr, 6);
    }
    sys_timer_register(&user_val->bt_connect_timeout, 5000, sys_time_auto_connection_deal, 1);
}

/*不属于用户接口，协议栈回调函数*/
void bt_discon_complete_handle(u8 *addr, int reason)
{
    u8 zero_addr[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    u8 cur_search_index = 0;
    cur_search_index = get_current_search_index();
    log_printf("bt_discon_complete:0x%x\n", reason);
    log_printf("cur_search_index=0x%x\n", cur_search_index);
#if (BT_TWS==0)
    if ((get_total_connect_dev() == 0) && ((cur_search_index == 0) || cur_search_index == 0xff)) {
        led_bt_idle();//led_fre_set(C_RB_FAST_MODE);
    }
#endif
    if (reason == 0) {
        //连接成功
        if (reason == 0) {
            if (get_remote_test_flag()) {
                bt_work_state_control(0);
                return;
            }
#if BT_TWS
            /*连接手机的tws作为主机*/
#if BT_TWS_ROLE_SWITCH
            if (get_tws_device_role() == 0 && !get_remote_test_flag()) {
                bt_tws_info_save(TWS_ROLE_MASTER);
            }
#endif
            /*
             *当前连接成功的设备和回连的设备一致，删除回连timer
             *否则，继续执行回连流程
             */
            if (memcmp(addr, user_val->connect_addr, 6) == 0) {
                puts("remove auto_conn timer\n");
                user_val->auto_connection_counter = 0;
                sys_timer_remove(&user_val->bt_connect_timeout);
            }
#else
            user_val->auto_connection_counter = 0;
            sys_timer_remove(&user_val->bt_connect_timeout);
#endif
        }
        return ;
    } else if (reason == 0xfc) {
        //新程序没有记忆地址是无法发起回连
        puts("no_db_addr\n");
        bt_work_state_control(1);
        return ;
    } else if ((reason == 0x10) || (reason == 0xf)) {
        task_bt_puts("conneciton accept timeout\n");
#if 1
        bt_work_state_control(1);
#else
        /*
         *连接接受超时
         *如果支持1t2，可以选择继续回连下一台，除非已经回连完毕
         */
        if (get_current_search_index() >= 1) {
            user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
        } else {
            bt_work_state_control(1);
        }
#endif
        return ;
    }
#if BT_TWS
    extern void clear_a2dp_source_addr(u8 * addr);
    clear_a2dp_source_addr(addr);
#endif

    if (reason == 0x16 || reason == 0x13 || (reason == 0xFE)) {
        /*
         *0xFE:TWS role conflict
         */
        task_bt_puts("Conn Terminated by Local Host\n");
#if BT_TWS
        if ((get_current_search_index() >= 1) && (user_val->tws_role == TWS_ROLE_MASTER)) {
            //继续连接下一个设备
            user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
        } else {
            if (user_val->broadcast_state == BRAOABCASET_MASTER_CON_STOP && get_tws_db_addr(addr)) {
                puts("abort_MASTER_CON_STOP\n");
                user_val->broadcast_state = 0;
                task_post_msg(NULL, 1, MSG_BT_TWS_CONNECT_CTL);
            }
            bt_work_state_control(1);
        }
#if BT_TWS_LINEIN
        if (get_tws_linein_state() == BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_LINEIN) {
            /* step 2从机插入linein,先断开与主机的连接，然后从机进行连接主机，从机变回主，再开启tws_linein*/
            puts("BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_LINEIN setp2\n");
            task_post_msg(NULL, 1, MSG_BT_TWS_CONNECT_CTL);
        } else if (get_tws_linein_state() == BT_TWS_CUR_STATE_TWS_MASTER_SLAVE_LINEIN) {
            task_post_msg(NULL, 1, MSG_BT_TWS_LINEIN_START);
            task_post_msg(NULL, 1, MSG_BT_TWS_CONNECT_CTL);
        } else if (get_tws_linein_state() == BT_TWS_CUR_STATE_TWS_SLAVE_MASTER_PLAY_LINEIN) {
            task_post_msg(NULL, 1, MSG_BT_TWS_LINEIN_START);
        }
#endif

#else
        bt_work_state_control(1);
#endif
    } else if (reason == 0x08) {
        task_bt_puts("\nconntime timeout\n");
        printf("tws_role:%x,test_flag:%x,call_status:%x\n", user_val->tws_role, get_remote_test_flag(), get_call_status());
        if ((check_dev_type_via_addr(addr) == 0) 	&&
            (get_tws_device_role() == 0) 			&&
            (user_val->tws_role == TWS_ROLE_MASTER) &&
            (!get_remote_test_flag())) {
            /*(1)手机连接超时
             *(2)对箱没有连接
             *(3)当前角色是主机
             *(4)非测试模式
             *综上
             */
            puts("<<<<<<<<<conntime timeout after con_tws>>>>>>>>>>\n");
            sys_timer_remove(&user_val->bt_connect_timeout);
            user_val->conn_tws_after_conn_phone = 1;
            user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
        }
        if ((check_dev_type_via_addr(addr) == 1) 	&&
            (get_tws_device_role() == 0) 			&&
            (user_val->tws_role == TWS_ROLE_MASTER) &&
            (user_val->auto_connection_counter) && memcmp(user_val->phone_connect_addr, zero_addr, 6) &&
            (!get_remote_test_flag())) {
            /*(1)tws连接超时
             *(2)对箱没有连接
             *(3)当前角色是主机
             *(4)还在超时回连着手机
             *(5)非测试模式
             *综上
             */
            puts("<<<<<<<<<tws_conntime timeout after con_phone>>>>>>>>>>\n");
            sys_timer_remove(&user_val->bt_connect_timeout);
            user_val->conn_phone_after_conn_tws = 1;
            user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
        }
        if (!get_remote_test_flag()) {
            if ((user_val->tws_role != TWS_ROLE_SLAVE) ||
                ((user_val->tws_role == TWS_ROLE_SLAVE) && !check_dev_type_via_addr(addr))) {
                /*(1)主机设备，啥都回连
                 *(2)从机设备，刚断开手机，回连手机
                 */
                puts("\n start connect disconecting device \n");
                if (get_call_status() == BT_CALL_HANGUP) {
                    user_val->auto_connection_counter = 10;
                    puts("\nsuper timeout,auto_conn\n");
                    sys_timer_remove(&user_val->bt_connect_timeout);
                    clear_led_rb_flag();
                    user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, addr);
                    if (check_dev_type_via_addr(addr) == 0) {
                        memcpy(user_val->phone_connect_addr, addr, 6);
                    }
                }
            } else {
                /* 从机设备，断开的是主机地址，不回连了  */
                puts("slave device not start connect master\n");
                user_val->auto_connection_counter = 0;
                bt_work_state_control(1);
            }
        } else {
            user_val->auto_connection_counter = 0;
            bt_work_state_control(1);
        }
    } else if ((reason == 0x04) || (reason == 0xFD)) {
        /*0xFD:vendor cancel page*/
        if (reason == 0xFD) {
#if BT_TWS
            if (user_val->conn_tws_after_conn_phone) {
                /*该page是手动取消的，不需要即刻回连，做完其他事情再连接该设备*/
                puts("conn_tws_after_conn_phone\n");
                return;
            }
            if ((get_tws_device_role() == 0) && (check_dev_type_via_addr(addr))) {
                puts("continue connect tws\n");
                /*对箱还没有连接，同时被取消配对的是对箱设备*/
                user_val->auto_connection_counter = 10;
                sys_timer_remove(&user_val->bt_connect_timeout);
                sys_timer_auto_connect(addr, reason);
                return;
            } else {
                bt_work_state_control(1);
                return;
            }
#else
            //bt_work_state_control(1);
            return;
#endif
        }

        printf("--page_timeout,conn_cnt:%d\n", user_val->auto_connection_counter);
        if (!user_val->auto_connection_counter) {
#if BT_TWS
            /*
             *通过地址判断当前page_timeout的设备类型
             *如果是对箱，可以选择一直配对，或者自定义配对次数
             *inquiry的时候不page
             */
            if (check_dev_type_via_addr(addr) && (!user_val->inquiry_flag)) {
                puts("tws page timeout,page again\n");
                if (get_current_search_index() >= 1) {
                    //开机先回连从机，回连超时继续回连手机
                    puts("power_on page_tws timeout,page phone\n");
                    user_val->conn_tws_after_conn_phone = 1;
                    user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
                    return;
                }
                if (user_val->conn_phone_after_conn_tws && memcmp(user_val->phone_connect_addr, zero_addr, 6)) {
                    //回连over tws,继续回连phone
                    user_val->conn_phone_after_conn_tws = 0;
                    puts("phone connected conn now2\n");
                    user_val->auto_connection_counter = 6;
                    sys_timer_remove(&user_val->bt_connect_timeout);
                    user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, user_val->phone_connect_addr);
                    memset(user_val->phone_connect_addr, 0, 6);
                    user_val->conn_tws_after_conn_phone = 1;
                    return;

                }
                bt_work_state_control(0);
                user_val->auto_connection_counter = 10;
                sys_timer_remove(&user_val->bt_connect_timeout);
                clear_led_rb_flag();
                user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, addr);
                return;
            } else {
                puts("clear_phone_connect_addr\n");
                memset(user_val->phone_connect_addr, 0, 6);
            }
#endif

            if (get_current_search_index() >= 1) {
                //继续连接下一个设备
                user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
            } else {
                /*手机回连超时，继续回连对箱*/
                if (user_val->conn_tws_after_conn_phone) {
                    puts("phone_page_timeout,conn tws now\n");
                    user_val->conn_tws_after_conn_phone = 0;
                    sys_timer_remove(&user_val->bt_connect_timeout);
                    clear_led_rb_flag();
                    user_send_cmd_prepare(USER_CTRL_TWS_START_CONNECTION, 0, NULL);
                } else  {
                    bt_work_state_control(1);
                }
            }
        } else {
            /*
             *inquiry的时候不page
             */
            if (!user_val->inquiry_flag) {
                user_val->auto_connection_counter--;
                if (user_val->auto_connection_counter % 2) {
                    bt_work_state_control(1);
                    sys_timer_auto_connect(addr, reason);
                } else {
                    bt_work_state_control(0);
                    clear_led_rb_flag();
                    user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, user_val->connect_addr);
                }
            }
            return;
        }
    } else if (reason == 0x0b) {
        puts("Connection Exist\n");
        //user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, addr);
        sys_timer_remove(&user_val->bt_connect_timeout);
        memcpy(user_val->connect_addr, addr, 6);
        user_val->auto_connection_counter = 1;
        sys_timer_register(&user_val->bt_connect_timeout, 2000, sys_time_auto_connection_deal, 1);
    } else if (reason == 0x06) {
        puts("PIN or Key Missing\n");
        /*
         *回连设备linkkey missing，可以选择以下操作：
         *1、回连下一个设备(如果有的话)
         *2、继续回连当前设备
         */
#if BT_TWS
        /*主机继续连接，从机等待连接*/
        if (user_val->tws_role == TWS_ROLE_MASTER) {
            if (get_current_search_index() >= 1) {
                puts("connect next bd\n");
                user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
            } else {
                puts("connect current bd\n");
                bt_work_state_control(1);
                /* user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, addr); */
            }
        } else {
            bt_work_state_control(1);
        }
#else
        puts("connect current bd 1\n");
        //user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, addr);
#endif
    } else if ((reason == 0x0d) || (reason == 0x05) || (reason == 0x0A)) {
        /*
         *reason:
         *0x05:Authentication Fdilure
         *0x0A:Limit to a Device Exceeded
         *0x0D:Limit Resources
         */
        puts("connection rejected due to limited resources\n");
        bt_work_state_control(1);
    } else if (reason == 0x15) {
        printf("Due To Power Off:%x\n", user_val->tws_role);
        if ((user_val->tws_role == TWS_ROLE_MASTER) && (check_dev_type_via_addr(addr))) {
            /*
             *（1）当前设备是主机
             *（2）断开的设备时切模式或者关机的对箱设备
             *（1）（2）同时满足，主机发起回连
             */
            clear_led_rb_flag();
            user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, addr);
        } else {
            /*
             *认证测试会走这个分支
             */
            bt_work_state_control(1);
        }
    }
}


void bt_test_fun()
{
    user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY, 0, NULL);
}

void bt_rec_exit()
{
#if BT_REC_EN
    rec_exit(&bt_rec_api);
#endif
}

static BT_TASK_STATUS bt_task_sta = BT_WAITINT_INIT;

static void *task_bt_init(void *priv)
{
    printf("task_bt_init !!\n");

#if BT_REC_EN
    fat_init();
#endif

    __bt_set_hid_independent_flag(0);
    if (user_val->not_play_bt_mode_tone != 0xaa) {
        user_val->connected_bd = 0;
        if (bt_task_sta == BT_SUSPEND) {
            background_resume();
            bt_task_sta = BT_INIT_OK;
            user_val->resume_flag = 0;
            if (get_total_connect_dev() == 0) {
                user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY, 0, NULL);
            }
        }
        tone_play(TONE_BT_MODE, 0);
    } else {
        background_resume();
        bt_task_sta = BT_INIT_OK;
        user_val->resume_flag = 0;
        task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
    }
    user_val->not_play_bt_mode_tone = 0;
    user_val->last_call_type = BT_CALL_OUTGOING;
    user_val->inquiry_flag = 0;
    user_val->conn_tws_after_conn_phone = 0;
    dac_toggle(1);
//  controller_mode_init();
    led_bt_idle();//led_fre_set(C_RB_FAST_MODE);
    ui_open_bt(&bt_ui_var, sizeof(bt_ui_var));
	mode_type = TASK_ID_BT;


    return NULL;
}
static void *task_bt_hid_init(void *priv)
{
    task_bt_printf("\n------------task_bt_hid_init !!\n");
    __bt_set_hid_independent_flag(1);
    tone_play(TONE_POWER_ON, 0);//hid_independent tone
    user_val->last_call_type = BT_CALL_OUTGOING;
    dac_toggle(1);
    led_bt_idle();//led_fre_set(C_RB_FAST_MODE);
    return NULL;
}
static void task_bt_exit(void **hdl)
{
    if (bt_task_sta  != BT_WAITINT_INIT) {
        printf("task_bt_exit !!\n");
        ui_close_bt();

        if (is_tws_device_slave()) {
            user_val->tws_change_mode_state = 1;
        }
#if BT_TWS_LINEIN
        bt_tws_linein_close();
#endif
#if BT_BACKGROUND_EN
        background_suspend();
        bt_task_sta = BT_SUSPEND;
#else
        no_background_suspend();
        bt_task_sta = BT_WAITINT_INIT;
#endif
        user_val->auto_suspend_flag = 0;
    }
    user_val->not_play_bt_mode_tone = 0;
    task_clear_all_message();

#if BT_REC_EN
    printf("bt rec exit\n");
    rec_exit(&bt_rec_api);
    fat_del();
    set_sys_freq(SYS_Hz);
#endif
    /**为了解决在powerdown模式的时候检测跳转没声音**/
    dac_toggle(1);
	mode_type = 0;
    printf("task_bt_exit !!\n");
}

void stereo_led_deal()
{
	//static u8 led_status=0xFF;
	if ((BT_STATUS_TAKEING_PHONE == get_bt_connect_status())||
       (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status()))
	{
		//if (led_status2 != LED_BT_PLAY)
		{
			//led_status = 1;
			led_bt_play();//led_fre_set(100,0);
		}
	}
	else if (BT_STATUS_CONNECTING == get_bt_connect_status())
	{
		//if (led_status2 != LED_BT_CONNECT)
		{
			//led_status = 2;
			led_bt_connect();//led_fre_set(100,0);
		}
	}
	else
	{
		//if (led_status2 != LED_BT_IDLE)
		{
			//led_status = 3;
            led_bt_idle();//led_fre_set(7,1);
		}
	}
#if 0//BT_TWS 
   if(BT_CURRENT_CONN_PHONE==get_bt_current_conn_type())
   {

   }
   else if(BT_CURRENT_CONN_STEREO_MASTER==get_bt_current_conn_type())
   {

   }
   else if(BT_CURRENT_CONN_STEREO_SALVE==get_bt_current_conn_type())
   {

   }
   else if(BT_CURRENT_CONN_STEREO_MASTER_PHONE==get_bt_current_conn_type())
   {

   }
   else if(BT_CURRENT_CONN_STEREO_PHONE_MASTER==get_bt_current_conn_type())
   {

   }
   else 
   {
   }
#endif
}

void task_bt_volume_tune(u32 msg)
{
    u8 test_box_vol_up = 0x41;
    u8 test_box_vol_down = 0x42;
	switch(msg)
	{
	    case MSG_VOL_UP_TWS:
	    case MSG_VOL_UP:
	    case MSG_VOL_UP_SHORT:
			if (get_tone_status())
				break;
#if 0//FMTX_EN
	        if (fmtx_get_state() == FREQ_SETTING) {
	            fmtx_setfre(FREQ_NEXT, 0);
	            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
	            break;
	        }
#endif
        #if WARNING_VOL_MAX
            if (volume_temp >= MAX_SYS_VOL_TEMP)//(sound.vol.sys_vol_l >= get_max_sys_vol(0))
            {
                if (msg == MSG_VOL_UP_SHORT)
        		    task_post_msg(NULL, 1, MSG_PROMPT_VOL_MAXMIN_SHORT);
                else
        		    task_post_msg(NULL, 1, MSG_PROMPT_VOL_MAXMIN);
    			goto vol_up_end;
			}
		#endif
		#if USE_16_LEVEL_VOLUME
			if (volume_temp < MAX_SYS_VOL_TEMP)
				volume_temp++;
			sound.vol.sys_vol_l = volume_table[volume_temp];
		#else
        	sound.vol.sys_vol_l++;
		#endif
#if 0
	        if (sound.vol.sys_vol_l > get_max_sys_vol(0)) {
	            sound.vol.sys_vol_l = get_max_sys_vol(0);
	            /*
	            if (get_tone_status() == 0) {
	                tone_play(TONE_WARNING, 1);
	            }
	            */
	            /* if (get_tone_status() == 0) {
	                tone_play(TONE_NWT_WARNING, 0);
	            } */
	        #if WARNING_VOL_MAX
	    		task_post_msg(NULL, 1, MSG_PROMPT_VOL_MAXMIN);
				goto vol_up_end;
			#else
	            //sin_tone_play(250);
	            //AMP_UNMUTE();
				//goto vol_up_end;
			#endif
	        }
#endif
	        log_printf("VOL+:%d\n", sound.vol.sys_vol_l);
	        sound.vol.sys_vol_r = sound.vol.sys_vol_l;
			if (get_tone_status() == 0)
			{
	        #if 0//WARNING_VOL_MAX
				if (vol_up_flag)
				{
					vol_up_flag = 0;
	        		set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
				}
				else
				{
	        		set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
				}
			#else
	        	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
			#endif
			}
	        if (get_call_status() != BT_CALL_HANGUP) {
	            user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_UP, 0, NULL);
	        } else {
	            user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);
	        }
            user_send_cmd_prepare(USER_CTRL_TEST_KEY, 1, &test_box_vol_up); //音量加
#if (SYS_DEFAULT_VOL == 0)
	        vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
vol_up_end:
#if (BT_TWS_LINEIN==0)
	#if WARNING_VOL_MAX
		//if (sound.vol.sys_vol_l == get_max_sys_vol(0))
	#endif
		{
	        if (is_dac_mute()) {                //dac mute时调节音量解mute
	            dac_mute(0, 1);
	            //if (task_get_cur() == TASK_ID_LINEIN) {
	            //    linein_mute(0);
	            //}
	            //mute_flag = 0;
	        }
	        mute_flag = 0;
		}
#endif
	        volume_display();//UI_menu(MENU_MAIN_VOL, 0, 3);
	        break;

	    case MSG_VOL_DN_TWS:
	    case MSG_VOL_DOWN:
	    case MSG_VOL_DOWN_SHORT:
			if (get_tone_status())
				break;
#if 0//FMTX_EN
	        if (fmtx_get_state() == FREQ_SETTING) {
	            fmtx_setfre(FREQ_PREV, 0);
	            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
	            break;
	        }
#endif
	   #if WARNING_VOL_MIN
			if (sound.vol.sys_vol_l == 0)
			{
                if (msg == MSG_VOL_DOWN_SHORT)
        		    task_post_msg(NULL, 1, MSG_PROMPT_VOL_MAXMIN_SHORT);
                else
	    		    task_post_msg(NULL, 1, MSG_PROMPT_VOL_MAXMIN);
				goto vol_down_end;
			}
		#endif
		#if USE_16_LEVEL_VOLUME
			if (volume_temp)
				volume_temp--;
			sound.vol.sys_vol_l = volume_table[volume_temp];
		#else
	        if (sound.vol.sys_vol_l) {
	            sound.vol.sys_vol_l--;
	        }
		#endif
	        log_printf("VOL-:%d\n", sound.vol.sys_vol_l);
	        sound.vol.sys_vol_r = sound.vol.sys_vol_l;
			if (get_tone_status() == 0)
			{
	        	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
			}
	        if (get_call_status() != BT_CALL_HANGUP) {
	            user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_DOWN, 0, NULL);
	        } else {
	            user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);
	        }
            user_send_cmd_prepare(USER_CTRL_TEST_KEY, 1, &test_box_vol_down); //音量减
#if (SYS_DEFAULT_VOL == 0)
	        vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
vol_down_end:
#if (BT_TWS_LINEIN==0)
	        if (is_dac_mute()) {
				if (sound.vol.sys_vol_l)
				{
	                dac_mute(0, 1);
				}
	            //if (task_get_cur() == TASK_ID_LINEIN) {
	            //    linein_mute(0);
	            //}
	            //mute_flag = 0;
	        }
	        mute_flag = 0;
#endif
	        volume_display();//UI_menu(MENU_MAIN_VOL, 0, 3);
	        break;
        //case MSG_BT_TWS_VOL_KEY_UP:
        //case MSG_VOL_KEY_UP:
        //    puts("MSG_VOL_KEY_UP\n");
	    //#if (WARNING_VOL_MAX || WARNING_VOL_MIN)
        //    music_tone_stop();
		//#endif
        //    break;
	}
}

#include "echo_api.h"
void task_bt_deal(void *hdl)
/* void task_bt_deal(void) */
{
    int msg[2];
    int error = MSG_NO_ERROR;
	u8 i;
    /* control_power_on_cnt(POWER_ON_CNT_SET, 6); */
    task_bt_printf("task_bt_deal !!\n");

#if (BT_MODE != NORMAL_MODE)
    task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
#endif
#if (TASK_MANGER_ENABLE == 0)
    task_bt_init(NULL);
#endif

    u32 loop_cnt = 0;
    user_val->auto_shutdown_cnt = AUTO_SHUT_DOWN_TIME;
    while (1) {
        loop_cnt++;
        if (loop_cnt > 300000) {
            /* putchar('<'); */
        }
        aec_task_main();
        if (loop_cnt > 300000) {
            /* putchar('M'); */
        }
        if (loop_cnt > 300000) {
            loop_cnt = 0;
            /* putchar('>'); */
        }
        error = task_get_msg(0, 2, msg);
        msg[0] = msg_mask_off_in_bt(msg[0]);//when there is a phone call,it can't exit bluetooth
        if (task_common_msg_deal(NULL, msg[0]) == false) {
            music_tone_stop();
            mutex_resource_release("record_play");
            task_common_msg_deal(NULL, NO_MSG);
            /* break; */
            return ;
        }
		task_bt_volume_tune(msg[0]);

#if BT_REC_EN
        rec_msg_deal_api(&bt_rec_api, msg[0]); //record 流程
#endif

        if (NO_MSG == msg[0]) {
            continue;
        }


#if BT_TWS
        if (tws_key_cmd_send(msg[0], 0)) {
            continue;
        }
#endif

#if (BLE_BREDR_MODE&BT_BLE_EN)
        if (ble_msg_deal(msg[0])) {
            continue;
        }
#endif


        switch (msg[0]) {
        case SYS_EVENT_PLAY_SEL_END:

		#if USE_MUTE_PALYTONE_ENABLE
			if (mute_flag)
			{
	            dac_mute(1, 1);
			}
		#endif
            if (bt_task_sta == BT_WAITINT_INIT) {
                bt_task_sta = BT_INIT_OK;
                sys_timer_var_init();
#if (SNIFF_MODE_CONF&SNIFF_EN)
                led_bt_sniff_init();
#endif
                bt_mode_init();
            }
            //ui_open_bt(&bt_ui_var, sizeof(bt_ui_var));

            /* #if ECHO_EN //BT&ECHO TEST */
            /*             msg[0] = MSG_ECHO_START; */
            /*             echo_msg_deal(hdl, msg[0]); */
            /* #endif */

            break;
#if BT_TWS
        case MSG_BT_SEARCH_DEVICE:
            puts("MSG_BT_INQIRY_DEVICE\n");

#if 0
            if ((BT_STATUS_CONNECTING == get_tws_connect_status()) ||
                (BT_STATUS_TAKEING_PHONE == get_tws_connect_status()) ||
                (BT_STATUS_PLAYING_MUSIC == get_tws_connect_status())) { /*连接状态*/
                /*对耳有连接手机没连接上，按配对按键断开连接取消配对，删除地址*/
                bt_tws_delete_addr();
                task_post_msg(NULL, 1, MSG_BT_TWS_CONNECT_CTL);
                break;
            }
#endif

            if (get_tws_device_role() == 0) {
                if (get_call_status() != BT_CALL_HANGUP) {
                    puts("bt_call_ing not do\n");
                    break;
                }
                user_val->inquiry_flag = 1;
                bt_work_state_control(0);
#if BT_TWS_SCAN_ENBLE
                tws_change_eir_priv_version(0);
                bt_work_state_control(1);
#endif
                user_send_cmd_prepare(USER_CTRL_SEARCH_DEVICE, 0, NULL);
            } else {
                puts("TWS_connected,ignore INQUIRY\n");
            }
            break;
#if BT_TWS&BT_TWS_BROADCAST
        case MSG_BT_TRAIN_DEVICE:
            puts("MSG_BT_TRAIN_DEVICE\n");
            if ((BT_STATUS_CONNECTING == get_bt_connect_status())   ||
                (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) ||
                (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status())) { /*连接状态*/

                /* task_bt_printf("S:%d  ", a2dp_get_status()); */
                bt_work_state_control(0);
                user_send_cmd_prepare(USER_CTRL_SYNC_TRAIN, 0, NULL);

            }
            break;
        case MSG_BT_TRAIN_SCAN_DEVICE:
            puts("MSG_BT_TRAIN_SCAN_DEVICE\n");
            if ((BT_STATUS_CONNECTING == get_bt_connect_status())   ||
                (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) ||
                (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status())) { /*连接状态*/
                break;
            }
            bt_work_state_control(0);
            user_send_cmd_prepare(USER_CTRL_SYNC_TRAIN_SCAN, 0, NULL);
            break;
#endif
        /*对箱连接控制*/
        case MSG_BT_TWS_TEST:
#if (BT_TWS&BT_TWS_BROADCAST)
            //user_send_cmd_prepare(USER_CTRL_SYNC_TRAIN_CANCEL, 0, NULL);// 停止广播
            //user_send_cmd_prepare(USER_CTRL_SYNC_TRAIN_SCAN_CANCEL, 0, NULL);//停止监听广播
            // break;
#endif
        case MSG_BT_TWS_CONNECT_CTL:
            if ((get_tws_connect_status() == BT_STATUS_CONNECTING)   ||
                (get_tws_connect_status() == BT_STATUS_TAKEING_PHONE) ||
                (get_tws_connect_status() == BT_STATUS_PLAYING_MUSIC)) {
                puts("disconn_tws\n");
                user_send_cmd_prepare(USER_CTRL_TWS_DISCONNECTION_HCI, 0, NULL);
            } else {
                if (get_call_status() != BT_CALL_HANGUP) {
                    puts("bt_call_ing not do\n");
                    break;
                }
                puts("conn_tws\n");
                user_send_cmd_prepare(USER_CTRL_TWS_START_CONNECTION, 0, NULL);
            }
            break;
        /*删除记忆的对箱地址*/
        case MSG_BT_TWS_DELETE_ADDR:
            user_send_cmd_prepare(USER_CTRL_TWS_CLEAR_INFO, 0, NULL);
            break;
        /*TWS按键音量同步*/
        case MSG_BT_TWS_VOL_KEY:
		#if 0
            if (get_bt_connect_status() == BT_STATUS_TAKEING_PHONE) {
                //puts("filt vol_sync when phone_talking\n");
                break;
                //通话的时候，可以选择要不要同步音量,要就打开下面的代码
                /* if(msg[1] > get_max_sys_vol(0)) {
                	msg[1] = get_max_sys_vol(0);
                } */
            }
		#else
            if (msg[1] > get_max_sys_vol(0))
            {
                msg[1] = get_max_sys_vol(0);
            }
		#endif
            /*本来音量已经最大，再收到一个同样的音量值时，播最大音量提示*/
            if ((sound.vol.sys_vol_l == get_max_sys_vol(0)) && (sound.vol.sys_vol_l == msg[1])) {
                /* if (get_tone_status() == 0) {
                   dac_channel_off(LINEIN_CHANNEL, FADE_OFF);
                   tone_play(TONE_WARNING, 1);
                   } */
                /* if (get_tone_status() == 0) {
                   tone_play(TONE_NWT_WARNING, 0);
                   } */
                //sin_tone_play(250);
            }
            log_printf("MSG_BT_TWS_VOL_KEY:%d\n", msg[1]);
            sound.vol.sys_vol_l = msg[1];
            sound.vol.sys_vol_r = msg[1];
			if (get_tone_status() == 0)
			{
            	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
			}
			else
			{
			    sound.tmp_sys_vol_l = sound.vol.sys_vol_l;
			    sound.tmp_sys_vol_r = sound.vol.sys_vol_r;
			}
            if (is_tws_device_master()) {
                user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);
            }
#if (SYS_DEFAULT_VOL == 0)
            vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
		#if USE_16_LEVEL_VOLUME
			for (i=0; i<=MAX_SYS_VOL_TEMP; i++)
			{
				if (sound.vol.sys_vol_l <= volume_table[i])
				{
					volume_temp = i;
					break;
				}
			}
		#endif
			UI_menu(MENU_MAIN_VOL, 0, 3);
            break;
        case MSG_BT_TWS_VOL_SYNC:
		#if 0
            if (get_bt_connect_status() == BT_STATUS_TAKEING_PHONE) {
                //puts("filt vol_sync when phone_talking\n");
                break;
                //通话的时候，可以选择要不要同步音量,要就打开下面的代码
                /* if(msg[1] > get_max_sys_vol(0)) {
                	msg[1] = get_max_sys_vol(0);
                } */
            }
		#else
            if (msg[1] > get_max_sys_vol(0))
            {
                msg[1] = get_max_sys_vol(0);
            }
		#endif
            log_printf("MSG_BT_TWS_VOL_SYNC:%d\n", msg[1]);
            sound.vol.sys_vol_l = msg[1];
            sound.vol.sys_vol_r = msg[1];
			if (get_tone_status() == 0)
			{
            	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
			}
			else
			{
			    sound.tmp_sys_vol_l = sound.vol.sys_vol_l;
			    sound.tmp_sys_vol_r = sound.vol.sys_vol_r;
			}
#if (SYS_DEFAULT_VOL == 0)
            vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
		#if USE_16_LEVEL_VOLUME
			for (i=0; i<=MAX_SYS_VOL_TEMP; i++)
			{
				if (sound.vol.sys_vol_l <= volume_table[i])
				{
					volume_temp = i;
					break;
				}
			}
		#endif
			UI_menu(MENU_MAIN_VOL, 0, 3);
            break;
        case MSG_BT_TWS_EQ_SYNC:
            log_printf("MSG_BT_TWS_EQ_SYNC:%d\n", msg[1]);
            sound.eq_mode = msg[1];
            eq_mode_switch_bt_sync(sound.eq_mode);//eq_mode_switch(sound.eq_mode);
            break;

#endif

        case MSG_BT_REC_EXIT:
            bt_rec_exit();
            break;
        case MSG_BT_PAGE_SCAN:
            puts("MSG_BT_PAGE_SCAN\n");
            bt_work_state_control(1);
            break;

        case MSG_BT_CONNECT_CTL:
            puts("MSG_BT_CONNECT_CTL\n");
            if ((BT_STATUS_CONNECTING == get_bt_connect_status())   ||
                (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) ||
                (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status())) { /*连接状态*/
                puts("bt_disconnect\n");/*手动断开连接*/
                user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
            } else {
                puts("bt_connect\n");/*手动连接*/
                user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY, 0, NULL);

            }
            break;

        case MSG_BT_CLR:
            if ((BT_STATUS_CONNECTING == get_bt_connect_status())   ||
                (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) ||
                (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status())) { /*连接状态*/
                puts("bt_disconnect\n");/*手动断开连接*/
                user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
			    UI_menu(MENU_FM_CLR, 0, 10);
            } else {
                puts("bt_connect\n");/*手动连接*/
                user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY, 0, NULL);
            }
            break;
        case MSG_BT_PP:
            puts("MSG_BT_PP\n");
#if BT_TWS_LINEIN
            if (get_tws_linein_state()) {
                task_post_msg(NULL, 1, MSG_BT_TWS_LINEIN_STOP);
            } else {
                task_post_msg(NULL, 1, MSG_BT_TWS_LINEIN_START);
            }
#endif
            if (get_bt_connect_status() < BT_STATUS_CONNECTING)
            {
                user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY,0,NULL);
				break;
            }
            task_bt_printf("call_status:%d\n", get_call_status());
            if ((get_call_status() == BT_CALL_OUTGOING) ||
                (get_call_status() == BT_CALL_ALERT)) {
                //user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
            } else if (get_call_status() == BT_CALL_INCOMING) {
                //user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
            } else if (get_call_status() == BT_CALL_ACTIVE) {
                //user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP,0,NULL);//user_send_cmd_prepare(USER_CTRL_SCO_LINK, 0, NULL);
            } else {
                user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
            }
            break;

		case MSG_BT_PP_LONG:
			if (get_call_status() != BT_CALL_HANGUP)
			{
				if (is_tws_device_slave())
					;//tws_cmd_send(MSG_BT_CALL_REJECT,0);
				else
					user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP,0,NULL);
				break;
			}
		#if KL_PLAY_DISCONNECT
			#if 1
			if (BT_STATUS_CONNECTING <= get_bt_connect_status())
			{
				if (is_tws_device_slave())
					;
				else
					user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI,0,NULL);
			}
			#else
			else if ((BT_STATUS_TAKEING_PHONE == get_bt_connect_status())||
				(BT_STATUS_TAKEING_PHONE == get_stereo_bt_connect_status()))
			{
				break;
			}
			else if (BT_STATUS_CONNECTING == get_bt_connect_status())
			{
				if (is_tws_device_slave())
					;
				else
					user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI,0,NULL);
				break;
			}
			else if (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status())
			{
				if (is_tws_device_slave())
					;
				else
					user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI,0,NULL);
				break;
			}
			#endif
		#endif
			break;

        case MSG_BT_CALL_HANGUP:
            puts("MSG_BT_CALL_HANGUP\n");
            if ((get_call_status() == BT_CALL_ACTIVE) ||
                (get_call_status() == BT_CALL_OUTGOING) ||
                (get_call_status() == BT_CALL_ALERT) ||
                (get_call_status() == BT_CALL_INCOMING)) {
                user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
            }
            break;

        case MSG_BT_CALL_REJECT:
            puts("MSG_BT_CALL_REJECT\n");
            if (get_call_status() == BT_CALL_INCOMING) {
                user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
            }

            break;
        case MSG_BT_NEXT_FILE:
            puts("MSG_BT_NEXT_FILE\n");

#if FMTX_EN
            if (fmtx_get_state() == FREQ_SETTING) {
                fmtx_setfre(FREQ_NEXT, 0);
                UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
                break;
            }
#endif
            if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status())
            {
                user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
            }
			else
			{
                if (get_call_status() == BT_CALL_ACTIVE) {
                    user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_UP, 0, NULL);
                } else {
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_NEXT, 0, NULL);
                }
			}
			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
		    UI_DIS_MAIN();
            break;

        case MSG_BT_PREV_FILE:
            puts("MSG_BT_PREV_FILE\n");
#if FMTX_EN
            if (fmtx_get_state() == FREQ_SETTING) {
                fmtx_setfre(FREQ_PREV, 0);
                UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
                break;
            }
#endif
            if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status())
            {
                user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
            }
			else
			{
                if (get_call_status() == BT_CALL_ACTIVE) {
                    user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_DOWN, 0, NULL);
                } else {
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PREV, 0, NULL);
                }
			}
			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
		    UI_DIS_MAIN();
            break;
        case MSG_BT_HID_TAKE_PIC:
            if (get_curr_channel_state()&HID_CH) {
                puts("---USER_CTRL_HID_IOS\n");
                user_send_cmd_prepare(USER_CTRL_HID_IOS, 0, NULL);
            } else {
                printf("cur_ch_state:%x\n", get_curr_channel_state());
            }
            break;
        case MSG_BT_CALL_LAST_NO:
            task_bt_puts("MSG_BT_CALL_LAST_NO\n");
            if (get_call_status() != BT_CALL_HANGUP) {
                break;
            }

            if (user_val->last_call_type == BT_CALL_OUTGOING) {
                user_send_cmd_prepare(USER_CTRL_HFP_CALL_LAST_NO, 0, NULL);
            } else if (user_val->last_call_type == BT_CALL_INCOMING) {
                user_send_cmd_prepare(USER_CTRL_DIAL_NUMBER, user_val->phone_num_len,
                                      user_val->income_phone_num);
            }
            break;

        case MSG_DAC_ON:
            puts("MSG_DAC_ON\n");
            dac_toggle(1);
            break;
        case MSG_DAC_OFF:
            puts("MSG_DAC_OFF\n");
            dac_toggle(0);
            break;
        case MSG_AUTOMUTE_ON:
            puts("MSG_AUTOMUTE_ON\n");
            sound_automute_set(AUTO_MUTE_CFG, -1, -1, -1); // 开自动mute
            break;
        case MSG_AUTOMUTE_OFF:
            puts("MSG_AUTOMUTE_OFF\n");
            dac_mute(0, 0);
            break;
        case MSG_BT_TONE_CONN:
            puts("MSG_BT_TONE_CONN\n");
#if (BT_HID_INDEPENDENT_MODE==1) && (USER_SUPPORT_PROFILE_HID==1)
            if (__get_hid_independent_flag()) {
            #if WARNING_BT_CONNECT
                tone_play(TONE_BT_CONN, 0);
			#endif
            } else
#endif
            {
            #if WARNING_BT_CONNECT
                tone_play(TONE_BT_CONN, 0);
			#endif
            }
            break;
        case MSG_BT_TONE_CONN_LEFT:
            puts("MSG_BT_TONE_CONN_LEFT\n");
            {
            #if WARNING_BT_CONNECT
                tone_play(TONE_BT_CONN_LEFT, 0);
			#endif
            }
            break;
        case MSG_BT_TONE_CONN_RIGHT:
            puts("MSG_BT_TONE_CONN_RIGHT\n");
            {
            #if WARNING_BT_CONNECT
                tone_play(TONE_BT_CONN_RIGHT, 0);
			#endif
            }
            break;
        case MSG_BT_TONE_DISCONN:
#if (BT_HID_INDEPENDENT_MODE==1) && (USER_SUPPORT_PROFILE_HID==1)
            if (__get_hid_independent_flag()) {
                puts("MSG_BT_hid_DISCONN\n");
            #if WARNING_BT_DISCONNECT
                tone_play(TONE_BT_DISCON, 0);
			#endif
            } else
#endif
            {
                puts("MSG_BT_TONE_DISCONN\n");
            #if WARNING_BT_DISCONNECT
                tone_play(TONE_BT_DISCON, 0);
			#endif
            }
            break;
        case MSG_BT_TONE_DISCONN_TWS:
            {
                puts("MSG_BT_TONE_DISCONN_TWS\n");
            #if WARNING_BT_DISCONNECT
                tone_play(TONE_BT_DISCON_TWS, 0);
			#endif
            }
            break;

#if BT_TWS_LINEIN
        case MSG_BT_TONE_LINEIN:
            puts("MSG_BT_TONE_LINEIN\n");
            tone_play(TONE_LINEIN_MODE, 0);
            break;
        case MSG_BT_TWS_LINEIN_START:
            puts("MSG_BT_TWS_LINEIN_START\n");
            bt_tws_linein_open();
            break;
        case MSG_BT_TWS_LINEIN_STOP:
            puts("MSG_BT_TWS_LINEIN_STOP\n");
            bt_tws_linein_close();
            break;
#endif
        case MSG_BT_TONE_RING:
            tone_play(TONE_RING, 1);
            break;
        case MSG_BT_TONE_PHONE_NUM:
            phone_num_play();
            break;
        case MSG_BT_UPDATA_START:
            puts("MSG_BT_UPDATA_START\n");
            updata_mode_api(BT_UPDATA);
            /* bt_updata_test_user(); */
            break;

        case MSG_BT_UPDATA_END:
            puts("MSG_BT_UPDATA_END\n");
            user_val->update_name_end = 0xaa;
            break;

        case MSG_HALF_SECOND:
		#if DAC_AUTO_MUTE_EN
			if ((sound.vol.sys_vol_r == 0)||(is_dac_mute())||(is_auto_mute())) 
			{
				//mute_cnt++;
				//if (mute_cnt >= 4)
				{
				//	mute_cnt = 4;
					if ((!get_tone_status())&&(!is_sin_tone_busy()))
						AMP_MUTE();
				}
			}
			else
			{
				AMP_UNMUTE();
	            mute_flag = 0;
			}
		#endif
			stereo_led_deal();
            bt_update_name_end();
            //printf("<%d>",user_val->connected_bd);
            /* extern void cache_test(void); */
            /* cache_test(); */
            /* task_bt_printf(" BT_H %d \n", get_bt_connect_status()); */
#if (SNIFF_MODE_CONF&SNIFF_EN)
            extern bool is_baseband_stata_inquiry_ing();
            if (user_val->connected_bd != BT_CUR_STATE_TWS_MASTER &&
                (!is_baseband_stata_inquiry_ing()) &&
                user_sniff_check_req(SNIFF_CNT_TIME)) {
                ///<空闲10S之后进入sniff模式
                printf("check_sniff_req\n");
                user_send_cmd_prepare(USER_CTRL_SNIFF_IN, 0, NULL);
            }
#endif

#if BT_TWS
            /*
                        if (is_tws_device_slave()) {
                            //对箱从机的状态判断例子,通过下面的函数&BIT()去获取需要的状态
                            if (get_tws_master_phone_state()&BIT(TWS_BT_STATUS_PLAYING_MUSIC)) {
                                puts("music");
                            }else{
                                puts("idle");
                            }
                            break;
                        }
            */
#endif
		#if 0
            if ((BT_STATUS_CONNECTING == get_bt_connect_status())   ||
                (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) ||
                (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status())) { /*连接状态*/

                /* task_bt_printf("S:%d  ", a2dp_get_status()); */

                if (BT_MUSIC_STATUS_STARTING == a2dp_get_status()) {    /*播歌状态*/
                    //    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_GET_PLAY_TIME, 0, NULL);
                    /* task_bt_puts("bt_music\n"); */
                    /* led_fre_set(C_BLED_SLOW_MODE); */
                } else if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) {
                    task_bt_puts("bt_phone\n");
                } else {
                    /* led_fre_set(C_BLED_ON_MODE); */
                }

                user_val->auto_shutdown_cnt = AUTO_SHUT_DOWN_TIME;

            } else  if (BT_STATUS_WAITINT_CONN == get_bt_connect_status() && user_val->auto_shutdown_cnt) {
                //task_bt_puts("BT_STATUS_WAITINT_CONN\n");
                user_val->auto_shutdown_cnt--;
                /* log_printf("power cnt:%d\n",user_val->auto_shutdown_cnt); */
                if (user_val->auto_shutdown_cnt == 0) {
                    //软关机
                    task_bt_puts("*****shut down*****\n");
                    task_post_msg(NULL, 1, MSG_POWER_OFF_AUTO);
                }
                /* led_fre_set(C_RB_FAST_MODE); */
            }
		#else
			#if (BT_AUTO_STANDBY_EN&&AUTO_SHUT_DOWN_TIME)
				if (get_bt_connect_status() == BT_STATUS_TAKEING_PHONE)
				{
					user_val->auto_shutdown_cnt = AUTO_SHUT_DOWN_TIME;
				}
				else if (get_bt_connect_status() == BT_STATUS_PLAYING_MUSIC)/*连接状态*/
				{
				#if 0//BT_NOPLAY_STANDBY_EN
					if (sound.vol.sys_vol_r == 0)
					{
						if (user_val->auto_shutdown_cnt)
                			user_val->auto_shutdown_cnt--;
					}
					else
				#endif
					{
						user_val->auto_shutdown_cnt = AUTO_SHUT_DOWN_TIME;
					}
	            }
				else if (get_bt_connect_status() == BT_STATUS_CONNECTING)/*连接状态*/
				{
				#if BT_NOPLAY_STANDBY_EN
					if (user_val->auto_shutdown_cnt)
                		user_val->auto_shutdown_cnt--;
				#else
					user_val->auto_shutdown_cnt = AUTO_SHUT_DOWN_TIME;
				#endif
	            }
				else if (get_bt_connect_status() < BT_STATUS_CONNECTING)
				{
					if (user_val->auto_shutdown_cnt)
            			user_val->auto_shutdown_cnt--;
				}
				if (user_val->auto_shutdown_cnt == 0)
				{
					//软关机
					puts("*****shut down*****\n");
				    rtc_tone_enable_flag = 0;
					task_post_msg(NULL, 1, MSG_RTC_MODE);
				}
			#endif
		#endif
            if (coordinate_back_cnt)
            {
                coordinate_back_cnt--;
				if (coordinate_back_cnt == 0)
				{
			        rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
       			    rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
				}
            }
            ui_bt_update_var(&bt_ui_var);
            /* UI_REFRESH(MENU_REFRESH); */
            break;

        case MSG_INPUT_NUMBER_END:
        case MSG_INPUT_TIMEOUT:
            get_input_number(NULL);
            break;

        case MSG_BT_FCC:
#if ((BT_MODE==TEST_FRE_OFF_MODE) || (BT_MODE==TEST_FCC_MODE))
        {
            extern void bt_fcc_test_init(u8 bt_mode);
            close_wdt();
            bt_fcc_test_init(BT_MODE);

        }
#endif
        break;

        case MSG_PROMPT_PLAY:
            tone_play(TONE_LOW_POWER,0);
            break;
		case MSG_PROMPT_VOL_MAXMIN:
		case MSG_PROMPT_VOL_MAXMIN_SHORT:
			if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status())
			{
            	if ((vol_maxmin_play_flag == MSG_VOL_UP_SHORT)||(vol_maxmin_play_flag == MSG_VOL_DOWN_SHORT))
            		vol_maxmin_play_flag = 0;
				break;
			}
			if (get_tone_status())
				break;
			//if (vol_maxmin_play_flag == 0)
			//	break;
			if (msg[0] == MSG_PROMPT_VOL_MAXMIN_SHORT)
			{
			    tone_play(TONE_VOLMAXMIN, 0);
			}
			else
			{
			    tone_play(TONE_VOLMAXMIN, 1);
			}
			break;
    	case MSG_BT_CHANGE_WORKMODE:
			if (dev_get_phydev_total(MUSIC_DEV_TYPE, DEV_ONLINE) > 0)
			{
				task_post_msg(NULL,1,MSG_CHANGE_WORKMODE);
			}
			break;

	    case MSG_MUTE:
	        puts("MSG_MUTE\n");
	        if (mute_flag) {//(is_dac_mute()) {
	            dac_mute(0, 1);
				//AMP_UNMUTE();
				mute_flag = 0;
	        } else {
				//AMP_MUTE();
	            dac_mute(1, 1);
				mute_flag = 1;
	        }
	        break;
        case MSG_DIMMER:
            #if 0
        	//if (rtc_set.rtc_set_mode == RTC_DISPLAY)
        	{
        	    led7_display_level++;
				if (led7_display_level > LED7_LEVEL_OFF)
					led7_display_level = LED7_LEVEL_MAX;
                set1628Display();
        	}
			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
			#else
            if ((get_call_status() == BT_CALL_OUTGOING) ||
                (get_call_status() == BT_CALL_ALERT)) {
                user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
		        UI_DIS_MAIN();
            } else if (get_call_status() == BT_CALL_INCOMING) {
                user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
		        UI_DIS_MAIN();
            } else if (get_call_status() == BT_CALL_ACTIVE) {
                user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP,0,NULL);//user_send_cmd_prepare(USER_CTRL_SCO_LINK, 0, NULL);
		        UI_DIS_MAIN();
            } else {
                //user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
			    task_post_msg(NULL, 1, MSG_SLEEP_SET);
            }
			#endif
			break;
        case MSG_RTC_SET:
        #if 0
            rtc_set.calendar_set.coordinate++;
			if (rtc_set.calendar_set.coordinate == RTC_DAT_SETTING)
                rtc_set.calendar_set.coordinate++;
			if (rtc_set.calendar_set.coordinate >= COORDINATE_MAX)
			{
    			rtc_set.calendar_set.coordinate = COORDINATE_MIN;
			}
			rtc_display_cnt = 0;
			coordinate_back_cnt = RTC_COORDINATE_BACK_CNT;
		    UI_DIS_MAIN();
		#else
			if (time_format_flag == FORMAT_24)
				time_format_flag = FORMAT_12;
			else
				time_format_flag = FORMAT_24;
            UI_menu(MENU_TIME_FORMAT, 0, 5);
		#endif
            break;
        //case MSG_ALARM1_SET:
        //    UI_menu(MENU_ALM_DISPLAY, 0, 2);
        //    break;
        //case MSG_ALARM2_SET:
        //    UI_menu(MENU_ALM2_DISPLAY, 0, 2);
        //    break;
        case MSG_ALARM_STOP:
			alarm_time_reset();
			alarm2_time_reset();
            break;
		case MSG_ALARM1_SET:
    		//alarm_time_reset();
		    break;
		case MSG_ALARM2_SET:
    		//alarm2_time_reset();
		    break;

        default:
            /*if (msg[0] == MSG_CHANGE_WORKMODE) {
                tone_play(TONE_POWER_OFF, 0);
            }*/
            break;
        }
    }
}

void bt_info_clear(void)
{
    u8 temp_buf[28],i;
    memset(temp_buf,0xff,sizeof(temp_buf));
    for (i=0;i<20;i++)
    {
        vm_write(VM_REMOTE_DB+i,temp_buf,28);
    }
}

#if TASK_MANGER_ENABLE
const TASK_APP task_bt_info = {
    .skip_check = NULL,
    .init 		= task_bt_init,
    .exit 		= task_bt_exit,
    .task 		= task_bt_deal,
    .key 		= &task_bt_key,
};
#endif
#if TASK_MANGER_ENABLE
const TASK_APP task_bt_hid_info = {
    .skip_check = NULL,
    .init 		= task_bt_hid_init,
    .exit 		= task_bt_exit,
    .task 		= task_bt_deal,
    .key 		= &task_bt_key,
};
#endif
