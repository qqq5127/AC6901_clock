#ifndef _RTC_SETTING_H_
#define _RTC_SETTING_H_

#include "includes.h"
#include "rtc/rtc_api.h"
#include "rtc/rtc_api_tools.h"
#include "flash_api.h"

#define RTC_SETTING_CNT   10///RTC 设置超时时间，500unit
#define ALM_SETTING_CNT   10///ALARM 设置超时时间，500unit

///1--once,2--every day,3-- one day for every week
#define ALARM_MODE_ONCE                         1
#define ALARM_MODE_EVERY_DAY                    2
#define ALARM_MODE_EVERY_WEEK                   3

//RTC显示类型
typedef enum {
    RTC_DISPLAY = 0,    //RTC显示
    ALM_DISPLAY,        //闹钟显示
    RTC_SET_MODE,        //R设置显示
    ALM_SET_MODE,      //闹钟设置显示
    ALM2_DISPLAY,
    ALM2_SET_MODE,
} ENUM_RTC_UI;

typedef struct _ALM_SET_ {
    RTC_TIME *curr_alm_time;   //当前闹钟时间
    RTC_TIME *curr_alm2_time;  //当前闹钟2时间
    u8 coordinate;            //闹钟设置位
    u8 alm_set_cnt;           //闹钟设置超时计数

    u8 alarm_flag;          //闹钟响闹标志
    u8 alarm2_flag;         //闹钟2响闹标志
    u8 alarm_sw;            //闹钟开关
    u8 alarm2_sw;            //闹钟2开关
    u8 bAlarmOnCnt ;          //闹钟响闹计时

    u8 current_alarm_mode;   ///1--once,2--every day,3-- one day for every week
    u8 current_alarm2_mode;   ///1--once,2--every day,3-- one day for every week
//    u8 alm_ring_mode;         //闹钟响闹模式
//    u8 alm_music_type;        //闹钟响闹提示音类型
} ALM_SET;

typedef struct _RTC_SET_ {
    RTC_TIME *curr_rtc_time;   //当前万年历时间
    u8 coordinate;            //RTC设置位
    u8 rtc_set_cnt;           //rtc设置超时计数
} RTC_SET;

typedef struct _RTC_SET_MODE_ {
    ALM_SET alarm_set;          //闹钟设置
    RTC_SET calendar_set;       //万年历设置
    u8 rtc_set_mode;            //RTC设置模式
} RTC_SETTING;

enum
{
	FORMAT_12,
	FORMAT_24
};

enum
{
	ALARM_OFF,
	ALARM_ON,
};

enum
{
	ALARM_RING_BELL,
	ALARM_RING_FM
};

enum
{
	ALARM_UP_1_5,
	ALARM_UP_1_7,
	ALARM_UP_6_7,
};

enum
{
	WEEKDAY_6,
	WEEKDAY_7,
	WEEKDAY_1,
	WEEKDAY_2,
	WEEKDAY_3,
	WEEKDAY_4,
	WEEKDAY_5,
};

/*
**********************************************************************
*                           VARIABLES DECLARE
**********************************************************************
*/
extern RTC_SETTING rtc_set;
//extern RTC_SETTING rtc_set_temp;
extern RTC_TIME current_time;
extern RTC_TIME current_alarm;
extern RTC_TIME current_alarm2;
extern RTC_TIME current_alarm_temp;
extern RTC_TIME current_alarm2_temp;
extern RTC_TIME rtc_time_set_temp;
extern u8 coordinate_temp;
//extern u8 alm_time_cnt;
//extern u8 alm_times;
//extern u8 sys_volume_temp;
//extern u8 rtc_time_set_cnt;
/*
**********************************************************************
*                           FUNCTIONS DECLARE
**********************************************************************
*/
void rtc_setting(int msg);
void reset_irtc(RTC_TIME *curr_time, RTC_TIME *curr_alm, RTC_TIME *curr_alm2);
void rtc_dac_channel_on(void);
void rtc_dac_channel_off(void);
RTC_SETTING *rtc_info_init();
void rtc_stop_alm(RTC_SETTING *rtc_set);
void alarm_time_reset(void);
void auto_set_next_alarm(RTC_SETTING *rtc_set);
void rtc_set_setting_mode(RTC_SETTING *rtc_set, ENUM_RTC_UI mode);
void rtc_set_alarm_sw(RTC_SETTING *rtc_set, u8 sw);
void rtc_set_alarm2_sw(RTC_SETTING *rtc_set, u8 sw);
void rtc_set_alarm_mode(RTC_SETTING *rtc_set, u8 mode);
void rtc_set_alarm2_mode(RTC_SETTING *rtc_set, u8 mode);
void rtc_start_setting(RTC_SETTING *rtc_set, u8 set_mode);
void rtc_stop_alm_api(void);

//void rtc_stop_setting(void);

u8  rtc_get_alarm_flag(RTC_SETTING *rtc_set);
u8  rtc_get_alarm2_flag(RTC_SETTING *rtc_set);
s16 rtc_get_alarm_mode(RTC_SETTING *rtc_set);
s16 rtc_get_alarm2_mode(RTC_SETTING *rtc_set);
u8  rtc_get_alarm_sw(RTC_SETTING *rtc_set);
u8  rtc_get_alarm2_sw(RTC_SETTING *rtc_set);
bool rtc_get_setting_flag(RTC_SETTING *rtc_set);
void rtc_set_alarm_flag(RTC_SETTING *rtc_set, u8 flag);
void rtc_set_alarm2_flag(RTC_SETTING *rtc_set, u8 flag);
s16 rtc_get_alarm_mode(RTC_SETTING *rtc_set);
s16 rtc_get_alarm2_mode(RTC_SETTING *rtc_set);
u8  rtc_get_alarm_sw(RTC_SETTING *rtc_set);
u8  rtc_get_alarm2_sw(RTC_SETTING *rtc_set);


void rtc_write_datetime(RTC_TIME *curr_time);
void rtc_read_datetime(RTC_TIME *curr_time);
void rtc_write_alarmtime(RTC_TIME *curr_time, u8 alarm_mode);
void rtc_read_alarmtime(RTC_TIME *curr_time);
void rtc_update_time(RTC_SETTING *rtc_set);

extern u8 time_format_flag;
//extern u8 alarm_alarm2_flag;
extern u8 rtc_display_cnt;
extern u8 sys_volume_temp;
extern u16 alm_time_cnt;
extern u8 alm_times;
extern u8 alm2_times;
extern u8 display_rtc_cnt;
extern u8 sw_set_flag;
extern u8 sw_set_cnt;
extern u8 setok_exit_flag;
//extern u8 rtc_set_temp;
extern u8 rtc_time_set_cnt;
extern u8 rtc_alarm_flag;
extern u8 alarm_sw_set_flag;
extern u8 alarm_ring_type;
extern u8 alarm2_ring_type;
extern u8 alarm_ring_volume;
extern u8 alarm2_ring_volume;
extern u8 alarm_up_mode;
extern u8 alarm2_up_mode;
extern u8 coordinate_back_cnt;
extern u8 rtc_week;
extern u8 alm_vol_set_sys_volume_temp;
extern u8 alm_ring_fm_set_flag;
extern u8 alm_fm_station_temp;
extern u8 alm_fm_station;
extern u8 alm2_fm_station;
extern u8 alm_fm_station_set_flag;
extern void alarm_time_set(void);
extern void alarm_temp_time_set(void);
extern void alarm1_temp_time_update(void);
extern void alarm2_temp_time_update(void);
extern void alarm_restart_times(void);
extern void alarm2_restart_times(void);
extern void alarm_time_update(void);
extern void alarm2_time_update(void);
extern void alarm_alarm2_check(void);
extern void alarm_time_reset(void);
extern void alarm2_time_reset(void);
extern void alarm_time_reset_spc(void);
extern void alarm2_time_reset_spc(void);
extern void auto_set_next_alarm(RTC_SETTING *rtc_set);
extern void auto_set_next_alarm2(RTC_SETTING *rtc_set);
extern void alarm_temp_reset(void);
extern void alarm1_temp_time_update_SW(void);
extern void alarm2_temp_time_update_SW(void);
extern void alm_wake_up_set(void);
extern u8 alarm_check(void);
extern u8 alarm2_check(void);
extern u8 alarm_up_mode_check(void);
extern u8 alarm2_up_mode_check(void);
extern u8 rtc_week_check(RTC_TIME *time_in);
extern void alarm_ring_volme_set(void);
extern void alm_ring_fm_init(void);
extern void alm_ring_fm_exit(void);
extern u8 alm_ring_fm_radio_init(void);
extern void alm_ring_set_fm_channel(void);

#endif
