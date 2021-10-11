#ifndef __AVCTP_USER_H__
#define __AVCTP_USER_H__


#include "typedef.h"


///***注意：该文件的枚举与库编译密切相关，主要是给用户提供调用所用。用户不能自己在中间添加值。*/
////----user (command) codes----////
typedef enum {
    /*
    使用user_send_cmd_prepare(USER_CMD_TYPE cmd,u16 param_len,u8 *param)发送命令
    //返回0表支持参数个数正确，返回1表不支持，2是参数错误
    要三个参数，没参数说明的命令参数param_len传0，param传NULL
    例子A、USER_CTRL_HFP_CALL_SET_VOLUME命令需要1个参数的使用例子：
    u8 vol = 8;
    user_send_cmd_prepare(USER_CTRL_HFP_CALL_SET_VOLUME,1, &vol);

    例子B、USER_CTRL_DIAL_NUMBER 参数要用数组先存起来，param_len是号码长度，param可传参数数组指针，
    user_val->income_phone_num已经存好号码
    user_send_cmd_prepare(USER_CTRL_DIAL_NUMBER,user_val->phone_num_len,user_val->income_phone_num);

    */

    //链路操作部分
    //回连,使用的是VM的地址，一般按键操作不使用该接口
    USER_CTRL_START_CONNECTION,
    //通过地址去连接，如果知道地址想去连接使用该接口
    USER_CTRL_START_CONNEC_VIA_ADDR,
    //通过指定地址手动回连，该地址是最后一个断开设备的地址
    USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY,

    //断开连接，断开当前所有蓝牙连接
    USER_CTRL_DISCONNECTION_HCI,

    //读取远端名字
    USER_CTRL_READ_REMOTE_NAME,
    //连接或断开SCO或esco,选择这个命令会自动判断要断开还是连接sco
    USER_CTRL_SCO_LINK,
    //连接SCO或esco
    USER_CTRL_CONN_SCO,
    //断开sco或esco
    USER_CTRL_DISCONN_SCO,
    //断开SDP，一般按键操作不使用该接口
    USER_CTRL_DISCONN_SDP_MASTER,

    //关闭蓝牙可发现
    USER_CTRL_WRITE_SCAN_DISABLE,
    //打开蓝牙可发现
    USER_CTRL_WRITE_SCAN_ENABLE,
//   USER_CTRL_WRITE_SCAN_ENABLE_KEY   ,
    //关闭蓝牙可连接
    USER_CTRL_WRITE_CONN_DISABLE,
    //打开蓝牙可连接
    USER_CTRL_WRITE_CONN_ENABLE,
    //  USER_CTRL_WRITE_CONN_ENABLE_KEY     ,
    //控制蓝牙搜索，需要搜索附件设备做功能的连续说明情况在补充完善功能
    USER_CTRL_SEARCH_DEVICE,
    //取消搜索
    USER_CTRL_INQUIRY_CANCEL,
    //取消配对
    USER_CTRL_PAGE_CANCEL,
    ///进入sniff模式，一般按键操作不使用该接口
    USER_CTRL_SNIFF_IN,
    USER_CTRL_SNIFF_EXIT,

    //hfp链路部分
    //控制打电话音量，注意可能有些手机进度条有变化音量大小没变化，同步要设置样机DAC音量
    /*跟电话音量操作有关的操作最终都执行回调函数call_vol_change*/
    USER_CTRL_HFP_CMD_BEGIN,
    USER_CTRL_HFP_CALL_VOLUME_UP,       /*音量加1，手机可以同步显示*/
    USER_CTRL_HFP_CALL_VOLUME_DOWN,      /*音量减1，手机可以同步显示*/
    USER_CTRL_HFP_CALL_SET_VOLUME,   /*设置固定值，手机可以同步显示，需要传1个音量值*/
    USER_CTRL_HFP_CALL_GET_VOLUME,  /*获取音量，默认从call_vol_change返回*/

    //来电接听电话
    USER_CTRL_HFP_CALL_ANSWER,
    //挂断电话
    USER_CTRL_HFP_CALL_HANGUP,
    //回拨上一个打出电话
    USER_CTRL_HFP_CALL_LAST_NO,
    //获取当前通话电话号码
    USER_CTRL_HFP_CALL_CURRENT,
    //通话过程中根据提示输入控制
    /*例子
    char num = '1';
    user_send_cmd_prepare(USER_CTRL_HFP_DTMF_TONES,1,(u8 *)&num);
    */
    //发送打电话时的信号选择DTMF tones ,有一个参数，参数支持{0-9, *, #, A, B, C, D}
    USER_CTRL_HFP_DTMF_TONES,
    //根据电话号码拨号
    /**USER_CTRL_DIAL_NUMBER命令有参数，参数要用数组先存起来，
    param_len是号码长度，param可传参数数组指针*/
    USER_CTRL_DIAL_NUMBER,
    //发送电量  /**要连接上HFP才有用*/
    USER_CTRL_SEND_BATTERY,
    //*控制siri状态*//*可以注册回调函数获取返回值*/
    USER_CTRL_HFP_GET_SIRI_STATUS,
    //*开启siri*/
    USER_CTRL_HFP_GET_SIRI_OPEN,
    //*关闭siri,一般说完话好像自动关闭了,如果要提前终止可调用*/
    USER_CTRL_HFP_GET_SIRI_CLOSE,
    /*获取手机的日期和时间，苹果可以，一般安卓机好像都不行*/
    USER_CTRL_HFP_GET_PHONE_DATE_TIME,
    USER_CTRL_HFP_CMD_SEND_BIA,
    /*获取手机厂商的命令 */
    USER_CTRL_HFP_CMD_GET_MANUFACTURER,
    //三方通话操作
    //应答
    USER_CTRL_HFP_THREE_WAY_ANSWER1,     //挂断当前去听另一个（未接听或者在保留状态都可以）
    USER_CTRL_HFP_THREE_WAY_ANSWER2,     //保留当前去接听, 或者用于两个通话的切换
    USER_CTRL_HFP_THREE_WAY_ANSWER1X,
    USER_CTRL_HFP_THREE_WAY_ANSWER2X,
    //拒听
    USER_CTRL_HFP_THREE_WAY_REJECT,           //拒绝后台来电
    USER_CTRL_HFP_DISCONNECT,                   //断开HFP连接
    USER_CTRL_HFP_REFLASH_CONN,                   //断开HFP之后重新连接
    USER_CTRL_HFP_CMD_END,

    //音乐控制部分
    USER_CTRL_AVCTP_CMD_BEGIN,
    //音乐播放
    USER_CTRL_AVCTP_OPID_PLAY,
    //音乐暂停
    USER_CTRL_AVCTP_OPID_PAUSE,
    //音乐停止
    USER_CTRL_AVCTP_OPID_STOP,
    //音乐下一首
    USER_CTRL_AVCTP_OPID_NEXT,
    //音乐上一首
    USER_CTRL_AVCTP_OPID_PREV,
    //音乐快进
    USER_CTRL_AVCTP_OPID_FORWARD,
    //音乐快退
    USER_CTRL_AVCTP_OPID_REWIND,
    //音乐循环模式
    USER_CTRL_AVCTP_OPID_REPEAT_MODE,
    USER_CTRL_AVCTP_OPID_SHUFFLE_MODE,
    //获取播放歌曲总时间和当前时间接口
    USER_CTRL_AVCTP_OPID_GET_PLAY_TIME,
    //同步音量接口
    USER_CTRL_AVCTP_OPID_SEND_VOL,
//    //AVCTP断开，是音乐控制链路，一般不使用
    USER_CTRL_AVCTP_DISCONNECT,
//    //AVCTP连接，是音乐控制链路，一般不使用
    USER_CTRL_AVCTP_CONN,

    USER_CTRL_AVCTP_CMD_END,

    //高级音频部分
    USER_CTRL_A2DP_CMD_BEGIN,
    //有判断条件的，回连过程连接高级音频，避免手机连也自动发起连接，一般按键操作不使用该接口
    USER_CTRL_AUTO_CONN_A2DP,
    //连接高级音频，回来最后一个断开设备的地址
    USER_CTRL_CONN_A2DP,
    //断开高级音频，只断开高级音频链路，如果有电话还会保留
    USER_CTRL_DISCONN_A2DP,
    //maybe BQB test will use
    USER_CTRL_A2DP_CMD_START					,
    USER_CTRL_A2DP_CMD_CLOSE				,
    USER_CTRL_A2DP_CMD_SUSPEND					,
    USER_CTRL_A2DP_CMD_GET_CONFIGURATION		,
    USER_CTRL_A2DP_CMD_ABORT					,
    USER_CTRL_A2DP_CMD_END,
    //蓝牙关闭
    USER_CTRL_POWER_OFF,
    //蓝牙开启
    USER_CTRL_POWER_ON,
    ///*hid操作定义*/
    USER_CTRL_HID_CMD_BEGIN,
    //按键连接
    USER_CTRL_HID_CONN,
//    //只发一个按键，安卓手机使用
    USER_CTRL_HID_ANDROID,
    //只发一个按键，苹果和部分安卓手机适用
    USER_CTRL_HID_IOS,
//    //发两个拍照按键
    USER_CTRL_HID_BOTH,
    //HID断开
    USER_CTRL_HID_DISCONNECT,
    //Home Key,apply to IOS and Android
    USER_CTRL_HID_HOME				,
    //Return Key,only support Android
    USER_CTRL_HID_RETURN			,
    //LeftArrow Key
    USER_CTRL_HID_LEFTARROW			,
    //RightArrow Key
    USER_CTRL_HID_RIGHTARROW		,

    USER_CTRL_HID_CMD_END,

    /*对箱操作命令*/
    USER_CTRL_SYNC_TRAIN,
    USER_CTRL_SYNC_TRAIN_SCAN,
    USER_CTRL_MONITOR,
    USER_CTRL_TWS_CONNEC_VIA_ADDR,
    USER_CTRL_TWS_COTROL_CDM,
    //清除对箱连接信息
    USER_CTRL_TWS_CLEAR_INFO,
    //断开对箱连接
    USER_CTRL_TWS_DISCONNECTION_HCI,
    //发起对箱连接
    USER_CTRL_TWS_START_CONNECTION,



    ///蓝牙串口发送命令
    USER_CTRL_SPP_CMD_BEGIN,
    /**USER_CTRL_SPP_SEND_DATA命令有参数，参数会先存起来，
    param_len是数据长度，param发送数据指针
    返回0,表示准备成功，会PENDing发完才返回
    3表示上一包数据没发完，*/
    USER_CTRL_SPP_SEND_DATA,
    USER_CTRL_SPP_UPDATA_DATA,
    //serial port profile disconnect command
    USER_CTRL_SPP_DISCONNECT,
    USER_CTRL_SPP_CMD_END,

    ///蓝牙电话本功能发送命令
    USER_CTRL_PBAP_CMD_BEGIN,
    //电话本功能读取通话记录的前n条
    USER_CTRL_PBAP_READ_PART,
    //电话本功能读全部记录
    USER_CTRL_PBAP_READ_ALL,
    //电话本功能中断读取记录
    USER_CTRL_PBAP_STOP_READING,

    USER_CTRL_PBAP_CMD_END,

    //蓝牙其他操作
//    //删除最新的一个设备记忆
//    USER_CTRL_DEL_LAST_REMOTE_INFO   ,
//    //删除所有设备记忆
    USER_CTRL_DEL_ALL_REMOTE_INFO,
    USER_CTRL_TEST_KEY,
    USER_CTRL_TWS_SYNC_CDM,
    USER_CTRL_TWS_SYNC_SBC_CDM,
    USER_CTRL_TWS_RESTART_SBC_CDM,
    USER_CTRL_SYNC_TRAIN_CANCEL,
    USER_CTRL_SYNC_TRAIN_SCAN_CANCEL,
    USER_CTRL_TWS_SYNC_CDM_FUN,
    USER_CTRL_TWS_LINEIN_START,
    USER_CTRL_TWS_LINEIN_CLOSE,

    USER_CTRL_CMD_SYNC_VOL_INC,
    USER_CTRL_CMD_SYNC_VOL_DEC,
    USER_CTRL_CMD_RESERVE_INDEX3,
    USER_CTRL_CMD_RESERVE_INDEX4,
    USER_CTRL_CMD_RESERVE_INDEX5,
    USER_CTRL_LAST,

    USER_CTRL_KEYPRESS,

} USER_CMD_TYPE;


////----反馈给客户使用的状态----////
typedef enum {
    /*下面是一些即时反馈的状态，无法重复获取的状态*/
    BT_STATUS_POWER_ON   = 1,   /*上电*/
    BT_STATUS_POWER_OFF  = 2,
    BT_STATUS_INIT_OK,          /*初始化完成*/
    BT_STATUS_FIRST_CONNECTED,        /*连接成功*/
    BT_STATUS_SECOND_CONNECTED,        /*连接成功*/
    BT_STATUS_FIRST_DISCONNECT,       /*断开连接*/
    BT_STATUS_SECOND_DISCONNECT,        /*断开连接*/
    BT_STATUS_PHONE_INCOME,     /*来电*/
    BT_STATUS_PHONE_NUMBER,     /*来电话号码*/
    BT_STATUS_PHONE_MANUFACTURER,     /*获取手机的厂商*/
    BT_STATUS_PHONE_OUT,        /*打出电话*/
    BT_STATUS_PHONE_ACTIVE,     /*接通电话*/
    BT_STATUS_PHONE_HANGUP,     /*挂断电话*/
    BT_STATUS_BEGIN_AUTO_CON,   /*发起回连*/
    BT_STATUS_MUSIC_SOUND_COME, /*库中加入auto mute判断音乐播放开始*/
    BT_STATUS_MUSIC_SOUND_GO,   /*库中加入auto mute判断音乐播放暂停*/
    BT_STATUS_RESUME,           /*后台有效，手动切回蓝牙*/
    BT_STATUS_RESUME_BTSTACK,   /*后台有效，后台时来电切回蓝牙*/
    BT_STATUS_SUSPEND,          /*蓝牙挂起，退出蓝牙*/
    BT_STATUS_LAST_CALL_TYPE_CHANGE,    /*最后拨打电话的类型，只区分打入和打出两种状态*/
    BT_STATUS_CALL_VOL_CHANGE,     /*通话过程中设置音量会产生这个状态变化*/
    BT_STATUS_SCO_STATUS_CHANGE,    /*当esco/sco连接或者断开时会产生这个状态变化*/
    BT_STATUS_CONNECT_WITHOUT_LINKKEY,   /*判断是首次连接还是配对后的连接，主要依据要不要简易配对或者pin code*/
    BT_STATUS_PHONE_BATTERY_CHANGE,     /*电话电量变化，该状态仅6个等级，0-5*/
    BT_STATUS_RECONNECT_LINKKEY_LOST,     /*回连时发现linkkey丢失了，即手机取消配对了*/
    BT_STATUS_RECONN_OR_CONN,       /*回连成功还是被连接*/
    BT_STATUS_BT_TEST_BOX_CMD,              /*蓝牙收到测试盒消息。1-升级，2-fast test*/
    BT_STATUS_BT_TWS_CONNECT_CMD,
    BT_STATUS_SNIFF_DAC_CTL,              /*SNIFF操作dac*/
    BT_STATUS_TONE_BY_FILE_NAME, /*直接使用文件名播放提示音*/
    BT_STATUS_PHONE_DATE_AND_TIME,   /*获取到手机的时间和日期，注意会有兼容性问题*/
    BT_STATUS_INBAND_RINGTONE,
    BT_STATUS_AVRCP_INCOME_OPID,     /*收到远端设备发过来的AVRCP命令*/

    BT_STATUS_CONN_A2DP_CH,
    BT_STATUS_INQUIRY_TIMEOUT,
    /*下面是1个持续的状态，是get_stereo_bt_connect_status获取*/
    BT_STATUS_MONITOR_WAITING_CONN,/*monitor中，还没连接上*/

    /*下面是6个持续的状态，是get_bt_connect_status()获取*/
    BT_STATUS_INITING,          /*正在初始化*/
    BT_STATUS_WAITINT_CONN,     /*等待连接*/
    BT_STATUS_AUTO_CONNECTINT,  /*正在回连*/
    BT_STATUS_CONNECTING,       /*已连接，没有电话和音乐在活动*/
    BT_STATUS_TAKEING_PHONE,    /*正在电话*/
    BT_STATUS_PLAYING_MUSIC,    /*正在音乐*/


    BT_STATUS_BROADCAST_STATE,/*braoadcaset中*/
    BT_STATUS_A2DP_ABORT_STATE,
} STATUS_FOR_USER;

typedef enum {
    BT_CALL_BATTERY_CHG = 0, //电池电量改变
    BT_CALL_SIGNAL_CHG,      //网络信号改变
    BT_CALL_INCOMING,   //电话打入
    BT_CALL_OUTGOING,   //电话打出
    BT_CALL_ACTIVE,     //接通电话
    BT_CALL_HANGUP,      //电话挂断
    BT_CALL_ALERT,       //远端reach
    BT_CALL_VOL_CHANGED,
} BT_CALL_IND_STA;

typedef enum {
    BT_MUSIC_STATUS_IDLE = 0,
    BT_MUSIC_STATUS_STARTING,
    BT_MUSIC_STATUS_SUSPENDING,
} BT_MUSIC_STATE;  //音乐状态

/*蓝牙当前的连接状态*/
typedef enum {
    BT_CUR_CONN_PHONE 		= 1	,/*连接手机							*/
    BT_CUR_CONN_TWS_MASTER		,/*连接对箱主机(本身是从机)			*/
    BT_CUR_CONN_TWS_SLAVE		,/*连接对箱从机(本身是主机)			*/
} BT_CUR_CONN_TYPE;



/*蓝牙当前的连接状态*/
typedef enum {
    BT_MONITOR_CONN 	= 1	,
    BT_MONITOR_DETCH,
    BT_MONITOR_START,
    BT_MONITOR_PAUSE,
} BT_MONITOR_TYPE;



#define BT_PROMPT_EN     //任意时间按照文件号播文件

#define    SPP_CH       0x01
#define    HFP_CH       0x02
#define    A2DP_CH      0x04    //media
#define    AVCTP_CH     0x08
#define    HID_CH       0x10
#define    AVDTP_CH     0x20
#define    PBAP_CH      0x40
/*
对象角色定义
TWS_ROLE_MASTER:主机
TWS_ROLE_SLAVE:从机
*/
#define TWS_ROLE_MASTER	0xAA  //master device
#define TWS_ROLE_SLAVE	0x55  //slave  device

extern void set_supprot_hfp();
extern void set_supprot_avctp();
extern void set_supprot_a2dp();
extern void set_supprot_spp();
extern void set_supprot_hid();
extern void set_supprot_a2dp_source();
extern void set_supprot_pbap();


extern u32 user_send_cmd_prepare(USER_CMD_TYPE cmd, u16 param_len, u8 *param);

/*
u16 get_curr_channel_state();  与  channel  判断区分
主动获取当前链路的连接状态，可以用来判断有哪些链路连接上了
*/
extern u16 get_curr_channel_state();
/*
u8 get_call_status(); 与BT_CALL_IND_STA 枚举的值判断
用于获取当前蓝牙电话的状态
*/
extern u8 get_call_status();
extern void user_cmd_ctrl_init(void *var);


/*个性化参数设置*/
/*后台的时候要设置1，预处理音频数据做跳转*/
extern void set_sbc_detect_module(u8 flag);
//sound_come计数是认为收到多少包非静音数据后返回蓝牙，
//如果void set_back_to_last_mode_flag设置了1后， sound_go计数是认为收到多少包静音数据后退出蓝牙
extern void __set_a2dp_sound_detect_counter(u16 sound_come, u16 sound_go);
/*音乐暂停或者有静音时会切出蓝牙。要注意换上下曲时的静音也会跳转*/
extern void __set_back_to_last_mode_flag(u8 flag);
/*用户调试设置地址，6个byte*/
extern void __set_bt_mac_addr(u8 *addr);

/*用户调试设置name,最长32个字符*/
extern void __set_host_name(const char *name, u8 len);
/*用户调试设置pin code*/
extern void __set_pin_code(const char *code);
/*该接口用于设置上电回连需要依次搜索设备的个数。*/
extern void __set_auto_conn_device_num(u8 num);

/*后台时手机开始播放音乐可以跳回蓝牙 */
extern void __bt_set_music_back_play_flag(u8 flag);
/*设置从后台跳回后，暂停是否要跳回上一个模式*/
extern void __set_back_to_last_mode_flag(u8 flag);
/*设置设备做主还是做从*/
void __set_device_role(u8 role) ;

/*//回连的超时设置。ms单位。但是对手机发起的连接是没作用的*/
extern void __set_super_timeout_value(u32 time);
/*外部设置支持什么协议*/
extern void bt_cfg_default_init(u8 support);

/*提供动态设置音乐自动播放的接口。
  该接口使用的时间点有要求，要在A2DP连接之前设置
   */
extern void __bt_set_a2dp_auto_play_flag(u8 flag);


/*提供接口外部设置配对方式*/
extern void __set_simple_pair_flag(u8 flag);

extern bool get_remote_test_flag();
/*//回连page的超时设置。ms单位*/
extern void __set_page_timeout_value(u16 time);
/*//设置开启蓝牙可发现可连接的时间，为了省电，设置一个自动关闭可连接的时间。ms单位。
    为 0 时不使用自动关闭*/
extern void __set_connect_scan_timeout_value(u32 time);
/*设置电量显示发送更新的周期时间，为0表示关闭电量显示功能*/
void __bt_set_update_battery_time(u32 time);
/*给用户设置蓝牙支持连接的个数，主要用于控制控制可发现可连接和回连流程*/
extern void __set_user_ctrl_conn_num(u8 num);
/*提供接口外部设置要保留hfp不要蓝牙通话*/
extern void __set_disable_sco_flag(u8 flag);
/*高级音频抢断开关*/
extern void __set_music_break_in_flag(u8 flag);
extern void __set_auto_pause_flag(u8 flag);
/*通话抢断开关，0：不抢断，1：抢断*/
extern void __set_hfp_switch(u8 switch_en);
/*通话恢复开关，0：不恢复，1：恢复*/
extern void __set_hfp_restore(u8 switch_en);
extern void __set_esco_packet_type(u8 type);  /*esco packet  */
/*设置蓝牙搜索的时间。ms单位。蓝牙inquiry搜索用*/
extern void __set_search_timeout_value(u16 time);
extern void __set_sbc_cap_bitpool(u16 sbc_cap_bitpoola);

/*有些自选接口用来实现个性化功能流程，回调函数注册，记得常来看看哟*/
extern void get_battery_value_register(int (*handle)(void));    /*电量发送时获取电量等级的接口注册*/
extern void music_vol_change_handle_register(void (*handle)(int vol), int (*handle2)(void)); /*手机更样机音乐模式的音量同步*/
extern void read_remote_name_handle_register(void (*handle)(u8 *name));   /*获取到名字后的回调函数接口注册函数*/
extern void spp_data_deal_handle_register(void (*handler)(u8 packet_type, u16 channel, u8 *packet, u16 size)); /*支持串口功能的数据处理接口*/
extern void discon_complete_handle_register(void (*handle)(u8 *addr, int reason)); /*断开或者连接上会调用的函数，给客户反馈信息*/

extern void bt_fast_test_handle_register(void (*handle)(void));
extern void bt_updata_handle_register(void (*start_handle)(u32), void (*end_handle)(u32));
extern void set_osc_internal_info_register(void (*set_osc_internal_info_t)(u32));
extern s32 bt_updata_test_user(void);
extern u32 get_updata_end_flag(void);

extern void update_bt_connecting_mac_addr(u8 *addr);
extern void update_bt_current_status(u8 *addr, u8 new_status, u8 conn_status);
extern void update_bt_tws_flag(u8 *addr, u8 flag);
extern u8 get_bt_connect_status(void);
extern u8 get_tws_connect_status(void);
extern u8 a2dp_get_status(void);


extern void update_profile_function_status(u8 *addr, u32 sta, int param);
extern void update_profile_function_status_monitor_slave(u8 *addr, u32 sta, int param);
extern void updata_profile_channels_status(u8 *addr, int state, int channel);
extern void bt_discon_complete_handle(u8 *addr, int reason);

extern void cfg_test_box_and_work_mode(u8 test_box, u8 work_mode);
extern void cfg_bt_pll_para(u32 osc, u32 sys, u8 low_power, u8 xosc);
extern void __bt_set_sniff(u8 sniff_mode_config);
extern tbool is_1t2_connection(void);
extern u8 get_total_connect_dev(void);
extern u8 get_current_search_index();
extern void clear_current_search_index();

extern void __set_ble_bredr_mode(u8 flag);
extern void fcc_test_handle_register(void (*handle)(void), void (*handle2)());
extern u8 is_bt_conn_hfp_hangup(u8 *addr);

extern void infor_2_user_handle_register(int (*handle)(u8 *info, u16 len), u8 *buffer_ptr);
extern void bt_music_info_handle_register(void (*handler)(u8 type, u32 time, u8 *info, u16 len));
extern void set_bt_vm_interface(u32 vm_index, void *interface);
extern int btstack_status_update_deal(u8 *ptr, u16 len);
extern u32 controller_need_buf(void);
extern void controller_buf_init(void *buf, void *tx_mem_buf, u16 tx_mem_len);
extern int classic_controller_init(u32 mode);
extern void bredr_stack_mem_init(void);
extern void bt_config_default_init();
extern int controller_mode_send_packet(u8 packet_type, u8 *packet, int len);
extern void controller_mode_packet_handler(void (*handler)(u8 type, u8 *packet, u16 size));
void __set_tws_device_indicate(const char *indicate);
int is_bt_stack_cannot_exist();
bool get_is_in_background_flag();
void __bt_set_hid_independent_flag(u8 flag);
u8 __get_hid_independent_flag();
void set_hid_independent_info();
void __set_hid_name(const char *name, u8 len);
void __set_tws_start_conn(u8 en);
/*连接对箱之前过滤手机的连接*/
void __set_phone_conn_filt(u8 en);
void __set_simple_pair_param(u8 io_cap, u8 oob_data, u8 mitm);

#define BD_CLASS_WEARABLE_HEADSET	0x240404/*ios10.2 display headset icon*/
#define BD_CLASS_HANDS_FREE			0x240408/*ios10.2 display bluetooth icon*/
#define BD_CLASS_MICROPHONE			0x240410
#define BD_CLASS_LOUDSPEAKER		0x240414
#define BD_CLASS_HEADPHONES			0x240418
#define BD_CLASS_CAR_AUDIO			0x240420
#define BD_CLASS_HIFI_AUDIO			0x240428
#define BD_CLASS_PHONEBOOK			0x340404
extern void __change_hci_class_type(u32 class);
void bt_phone_audio_handle_register(s32(*sco_conn_cb)(void *priv),
                                    s32(*sco_disconn_cb)(void *priv),
                                    void (*sco_rx_cb)(s16 *data, u16 point, u8 sco_flags),
                                    void (*sco_tx_cb)(s16 *data, u16 point));

extern void bt_a2dp_audio_handle_register(void *(*start_handle)(void *priv, const char *format,
        u16(*read_handle)(void *priv, u8 *buf, u16 len),
        bool (*seek_handle)(void *priv, u8 type, u32 offsiz),
        void *aac_setting,
        void *inbuf),
        void (*stop_handle)(void **hdl),
        u32 start_dec_len);
s32 bt_noconn_pwr_handle_register(u32(*in_fun)(void), u32(*out_fun)(void));

/**************************tws*****************/
enum {
    INQUIRT_USER_NULL = 0,
    INQUIRT_USER_NANME,
    INQUIRT_USER_PRIV_VERSION,

};
/*********************************************tws******************************/
void inquiry_result_int(void *mem, u32 mem_len, u8 search_device_type, char *search_ptr);
void user_set_tws_rx_buf_handle(void *tws_rx_buf, u16 size, void *parm);
void user_tws_int_handle(u8 tws_en, void *tws_buf);

void bt_tws_register(void (*tws_save_info_handle)(u8), void (*tws_sync_info_handle)(void), void (*tws_deal_cmd_handle)(int msg, u8 value), void (*tws_siri_handle)(u8));
void bt_tws_resgister_set_irq_prio(void (*tws_inc_bt_irq_prio)(u8 en));
void update_cur_conn_bd(u8 conn, u8 *addr);
u32 tws_need_buf(void);
void __set_phone_vol_sync_en(u8 en);		/*通话时音量同步*/
void get_siri_status_handle_register(void (*handle)(int flag));
bool check_connect_enter_sniff_status(void);
void resgister_check_connect_enter_sniff_status(bool (*check_connect_enter_sniff_status_t)(void));


/***********************TWS常用api********************************/
/*
 *是否对箱主机
 */
bool is_tws_device_master();

/*
 *是否对箱从机
 */
bool is_tws_device_slave();

/*
 *获取对箱主从标识
 *如果对箱连接，则返回主从标识，否则返回0
 *所以该接口亦可以用来判断对箱是否连接
 */
u8 get_tws_device_role();

/*
 *获取数据库已经配对的对箱地址
 *返回0表示没有对箱地址
 *返回1表示存在对箱地址
 */
u8 get_tws_db_addr(u8 *addr);

/*
 *主要用来判断回连设备的类型
 *return 0表示普通设备
 *return 1表示对箱设备
 */
u8 check_dev_type_via_addr(u8 *addr);

/*
 *获取当前连接/断开的设备类型
 *new_status:
 *0=获取最新断开设备的类型
 *1:获取最新连接设备的类型
 */
u8 get_cur_conn_bd(u8 new_status);
void user_monitor_sw(u8 sw);
/***********************sniff*******************************/
extern bool user_sniff_check_req(u8 sniff_cnt_time);
extern bool bt_page_ing(void);
extern bool is_sniff_mode();
extern bool get_edr_status();
/*
 *改变eir_pirv_version进行搜索匹配
 *en 0: change version to  tws_device_indicate
 *en 1: change version not different   tws_device_indicate
 */
extern u8 change_eir_priv_version(u8 en, u8 *change_device_indicate);
extern void user_set_sync_conn_handle(void (*tws_sync_fun_caback_t)(u8 set_cmd, u8 cmd));
extern u8 send_sync_tws_cmd_fun(u8 cmd, u16 slot_time_doing, void (*call_back_fun)());
extern void cfg_rf_rxtx_status_enable(u8 en);
void bredr_fcc_srrc_reset(s16 ofsi, s16 ofsq, s16 cmpuk1, s16 cmpsk2, s16 smpsk3);
extern void iq_trim_info_handle_register(void (*bt_iq_trim_info_handler)(s16, s16));
void bredr_fcc_srrc_reset(s16 ofsi, s16 ofsq, s16 cmpuk1, s16 cmpsk2, s16 smpsk3) ;
void fcc_srrc_trim_step(u8 step);


#endif
