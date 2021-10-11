#include "board_init.h"
#include "rtc_setting.h"
#include "task_rtc.h"
#include "msg.h"
#include "sys_detect.h"
#include "audio/dac_api.h"
#include "audio/audio.h"
#include "dac.h"
#include "ui_api.h"
#include "warning_tone.h"

#if RTC_CLK_EN

#define RTC_SET_DEBUG_ENABLE
#ifdef RTC_SET_DEBUG_ENABLE
#define rtc_set_printf log_printf
#else
#define rtc_set_printf(...)
#endif

RTC_SETTING rtc_set;
//RTC_SETTING rtc_set_temp;
RTC_TIME    current_time;
RTC_TIME    current_alarm;
RTC_TIME    current_alarm2;
RTC_TIME    current_alarm_temp;
RTC_TIME    current_alarm2_temp;
RTC_TIME    rtc_time_set_temp;
u8 coordinate_temp;
u8 time_format_flag=FORMAT_24;		//0:12小时制式，1:24小时制式
//u8 alarm_alarm2_flag=ALARM_1;		//0:当前为闹钟1，1:当前为闹钟2
u8 rtc_display_cnt=RTC_DISPLAY_BACK_CNT;
u8 sys_volume_temp;					//保存音量
u8 is_rtc_wakeup=0;
u16 alm_time_cnt=0;					//闹钟响的时间
u8 alm_times=0;						//闹钟响的次数
u8 alm2_times=0;					//闹钟2响的次数
u8 sw_set_flag=0;
u8 sw_set_cnt=0;
u8 setok_exit_flag=0;
//u8 rtc_set_temp;
u8 rtc_time_set_cnt=0;
u8 rtc_alarm_flag=0;				//0：RTC显示，1：ALARM显示
u8 alarm_sw_set_flag=0;
u8 alarm_ring_type=ALARM_RING_BELL;
u8 alarm2_ring_type=ALARM_RING_BELL;
u8 alarm_ring_volume=10;
u8 alarm2_ring_volume=10;
u8 alarm_up_mode=ALARM_UP_1_5;
u8 alarm2_up_mode=ALARM_UP_1_5;
u8 coordinate_back_cnt=0;
u8 rtc_week=0;
u8 alm_vol_set_sys_volume_temp;
u8 alm_ring_fm_set_flag=0;
u8 alm_fm_station_temp=0;
u8 alm_fm_station=0;
u8 alm2_fm_station=0;
u8 alm_fm_station_set_flag=0;
extern void music_tone_stop(void);

void puts_time(RTC_TIME *time)
{
    rtc_set_printf("time,%d-%d-%d, %d:%d:%d\n", time->dYear, time->bMonth, time->bDay, \
                   time->bHour, time->bMin, time->bSec);
}
/*----------------------------------------------------------------------------*/
/** @brief: 写入rtc时间值
    @param: 当前时间
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void rtc_write_datetime(RTC_TIME *curr_time)
{
    struct rtc_data hw_date;
    RTC_TIME_to_rtc_data(&hw_date, curr_time);
    set_rtc_time(&hw_date);
}
/*----------------------------------------------------------------------------*/
/** @brief: 读取rtc时间值
    @param: 当前时间
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void rtc_read_datetime(RTC_TIME *curr_time)
{
    struct rtc_data hw_date;
    get_rtc_time(&hw_date);
    rtc_data_to_RTC_TIME(curr_time, &hw_date);
}
/*----------------------------------------------------------------------------*/
/** @brief: 写入闹钟时间值
    @param: 闹钟时间
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void rtc_write_alarmtime(RTC_TIME *alarm_time, u8 alarm_mode)
{
    struct rtc_data hw_date;
    alarm_time->bSec = 0;//alarm_mode & 0x03; ///save mode for  alarming power on
    RTC_TIME_to_rtc_data(&hw_date, alarm_time);
    set_rtc_alarm(&hw_date);
}
/*----------------------------------------------------------------------------*/
/** @brief: 读取闹钟时间值
    @param: 闹钟时间
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void rtc_read_alarmtime(RTC_TIME *alarm_time)
{
    struct rtc_data hw_date;
    get_rtc_alarm(&hw_date);
    rtc_data_to_RTC_TIME(alarm_time, &hw_date);
}

void rtc_update_time(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
        rtc_read_datetime(rtc_set->calendar_set.curr_rtc_time);
        //rtc_read_alarmtime(rtc_set->alarm_set.curr_alm_time);
        puts_time(rtc_set->calendar_set.curr_rtc_time);
    }
}

/*----------------------------------------------------------------------------*/
/** @brief: 获取正在设置时钟标志
    @param: null
    @return: true 正在设置   false 不在设置模式
    @note:
*/
/*----------------------------------------------------------------------------*/
bool rtc_get_setting_flag(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
        if ((rtc_set->rtc_set_mode != RTC_DISPLAY)
			&&(rtc_set->rtc_set_mode != ALM_DISPLAY)
			&&(rtc_set->rtc_set_mode != ALM2_DISPLAY)) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

inline u8 rtc_get_alarm_flag(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
        return rtc_set->alarm_set.alarm_flag;
    }
    return 0;
}

inline u8 rtc_get_alarm2_flag(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
        return rtc_set->alarm_set.alarm2_flag;
    }
    return 0;
}

inline s16 rtc_get_alarm_mode(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
        return rtc_set->alarm_set.current_alarm_mode;
    }
    return -1;
}

inline s16 rtc_get_alarm2_mode(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
        return rtc_set->alarm_set.current_alarm2_mode;
    }
    return -1;
}

inline u8 rtc_get_alarm_sw(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
        return rtc_set->alarm_set.alarm_sw;
    }
    return 0;
}

inline u8 rtc_get_alarm2_sw(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
        return rtc_set->alarm_set.alarm2_sw;
    }
    return 0;
}

inline void rtc_set_alarm_mode(RTC_SETTING *rtc_set, u8 mode)
{
    if (rtc_set) {
        rtc_set->alarm_set.current_alarm_mode = mode;
    }
}

inline void rtc_set_alarm2_mode(RTC_SETTING *rtc_set, u8 mode)
{
    if (rtc_set) {
        rtc_set->alarm_set.current_alarm2_mode = mode;
    }
}

inline void rtc_set_setting_mode(RTC_SETTING *rtc_set, ENUM_RTC_UI mode)
{
    if (rtc_set) {
        rtc_set->rtc_set_mode = mode;
    }
}

inline void rtc_set_alarm_flag(RTC_SETTING *rtc_set, u8 flag)
{
    if (rtc_set) {
        rtc_set->alarm_set.alarm_flag = flag;
    }
}

inline void rtc_set_alarm2_flag(RTC_SETTING *rtc_set, u8 flag)
{
    if (rtc_set) {
        rtc_set->alarm_set.alarm2_flag = flag;
    }
}

inline void rtc_set_alarm_sw(RTC_SETTING *rtc_set, u8 sw)
{
    if (rtc_set) {
        rtc_set->alarm_set.alarm_sw = sw;
        //alarm_sw(rtc_set->alarm_set.alarm_sw);
        rtc_set_printf("alm sw %d\n", rtc_set->alarm_set.alarm_sw);
    }
}

inline void rtc_set_alarm2_sw(RTC_SETTING *rtc_set, u8 sw)
{
    if (rtc_set) {
        rtc_set->alarm_set.alarm2_sw = sw;
        //alarm_sw(rtc_set->alarm_set.alarm2_sw);
        rtc_set_printf("alm sw %d\n", rtc_set->alarm_set.alarm2_sw);
    }
}

/*----------------------------------------------------------------------------*/
/** @brief: 进入设置模式
    @param: null
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void rtc_start_setting(RTC_SETTING *rtc_set, u8 set_mode)
{
    if (rtc_set == NULL) {
        return;
    }
    rtc_set->calendar_set.coordinate = RTC_HOUR_SETTING;
    rtc_set->alarm_set.coordinate = ALM_COORDINATE_MIN;

    rtc_set->calendar_set.rtc_set_cnt = 0;
    rtc_set->alarm_set.alm_set_cnt = 0;
	
	//rtc_set_temp = 0;
	rtc_time_set_cnt = 0;
	alarm_sw_set_flag = 0;
	time_format_set_flag = 0;
	alm_fm_station_set_flag = 0;
	alm_fm_station_temp = 1;
	
    rtc_set->rtc_set_mode = set_mode;
    if (rtc_set->rtc_set_mode == RTC_SET_MODE) {
    	SET_UI_MAIN(MENU_RTC_MAIN);
        //UI_DIS_MAIN();
    } else if (rtc_set->rtc_set_mode == ALM_SET_MODE) {
    	SET_UI_MAIN(MENU_ALM_SET);
        //UI_DIS_MAIN();
    } else if (rtc_set->rtc_set_mode == ALM2_SET_MODE) {
    	SET_UI_MAIN(MENU_ALM2_SET);
        //UI_DIS_MAIN();
    }
}

void rtc_stop_alm(RTC_SETTING *rtc_set)
{
    if (rtc_set) {
		if (rtc_set->alarm_set.alarm_flag)
		{
        	rtc_set->alarm_set.alarm_flag = 0;
		}
		else
		{
	    	rtc_set->alarm_set.alarm2_flag = 0;
		}
    }
    UI_DIS_MAIN();
}

void rtc_stop_alm_api()
{
    rtc_stop_alm(&rtc_set);
}

void rtc_reset_irtc()
{
    set_rtc_enable();
    rtc_clr_power_flag();
}

//
void alm_ring_bell_volume(void)
{
	sound.vol.sys_vol_l = volume_table[alarm_ring_volume];
	sound.vol.sys_vol_r = sound.vol.sys_vol_l;
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    tone_play(TONE_ALARM_RING, 1);
}
void alm_ring_fm_volume(void)
{
	sound.vol.sys_vol_l = volume_table[alarm_ring_volume];
	sound.vol.sys_vol_r = sound.vol.sys_vol_l;
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    alm_ring_fm_init();
}
void alm2_ring_bell_volume(void)
{
	sound.vol.sys_vol_l = volume_table[alarm2_ring_volume];
	sound.vol.sys_vol_r = sound.vol.sys_vol_l;
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    tone_play(TONE_ALARM_RING, 1);
}
void alm2_ring_fm_volume(void)
{
	sound.vol.sys_vol_l = volume_table[alarm2_ring_volume];
	sound.vol.sys_vol_r = sound.vol.sys_vol_l;
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    alm_ring_fm_init();
}
/*----------------------------------------------------------------------------*/
/** @brief: 时钟消息设置
    @param: msg:消息
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void rtc_setting(int msg)
{
    if ((MSG_HALF_SECOND != msg) &&
        (msg != MSG_ONE_SECOND)) {
        rtc_set.calendar_set.rtc_set_cnt = 0;
        rtc_set.alarm_set.alm_set_cnt = 0;
    }

    if (rtc_set.rtc_set_mode == RTC_SET_MODE)
    {
        switch (msg) {
        case MSG_RTC_SET://移动光标到下一项
        //case MSG_BACK_LIGHT_SET:
    		rtc_time_set_cnt = 0;
		#if 0
			rtc_set.calendar_set.coordinate++;
			if (rtc_set.calendar_set.coordinate > COORDINATE_MAX)
			{
    			rtc_set.calendar_set.coordinate = COORDINATE_MIN;
                goto _rtc_setting_exit;
			}
		#else
		    if (time_format_set_flag)
		    {
    			if (rtc_set.calendar_set.coordinate == RTC_HOUR_SETTING)
    			{
        			rtc_set.calendar_set.coordinate = RTC_MIN_SETTING;
    		        UI_DIS_MAIN();
    			}
    			#if 0
    			else if (rtc_set.calendar_set.coordinate == RTC_MIN_SETTING)
    			{
        			rtc_set.calendar_set.coordinate = RTC_YEAR_SETTING;
    			}
    			else if (rtc_set.calendar_set.coordinate == RTC_YEAR_SETTING)
    			{
        			rtc_set.calendar_set.coordinate = RTC_MONTH_SETTING;
    			}
    			else if (rtc_set.calendar_set.coordinate == RTC_MONTH_SETTING)
    			{
        			rtc_set.calendar_set.coordinate = RTC_DAT_SETTING;
    			}
    			else
    			{
                    goto _rtc_setting_exit;
    			}
    			#else
    			else if (rtc_set.calendar_set.coordinate == RTC_MIN_SETTING)
    			{
        			//time_format_set_flag = 1;
	                //UI_menu(MENU_TIME_FORMAT, 0, 0);
                    goto _rtc_setting_exit;
    			}
    			#endif
    		}
    		else
    		{
    			time_format_set_flag = 1;
    			rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
		        UI_DIS_MAIN();
                //goto _rtc_setting_exit;
    		}
		#endif
    		break;

        //case MSG_RTC_SET_OK:
        //    goto _rtc_setting_exit;
    	//	break;
        case MSG_RTC_PLUS:
        case MSG_RTC_PLUS_HOLD:
        	rtc_time_set_cnt = RTC_SET_BACK_CNT;
        	if (time_format_set_flag)
        	{
                calendar_time_plus(&rtc_time_set_temp, rtc_set.calendar_set.coordinate);
                //rtc_write_datetime(rtc_time_set_temp.calendar_set.curr_rtc_time);
                puts_time(&rtc_time_set_temp);
    		    UI_DIS_MAIN();
            }
            else
            {
				if (time_format_flag == FORMAT_24)
					time_format_flag = FORMAT_12;
				else
					time_format_flag = FORMAT_24;
	            UI_menu(MENU_TIME_FORMAT, 0, 0);
            }
            rtc_set_printf("rtc plus\n");
            break;
    
        case MSG_RTC_MINUS:
        case MSG_RTC_MINUS_HOLD:
        	rtc_time_set_cnt = RTC_SET_BACK_CNT;
        	if (time_format_set_flag == 0)
        	{
                calendar_time_minus(&rtc_time_set_temp, rtc_set.calendar_set.coordinate);
                //rtc_write_datetime(rtc_time_set_temp.calendar_set.curr_rtc_time);
                //rtc_clk_out(rtc_time_set_temp.calendar_set.curr_rtc_time);
                puts_time(&rtc_time_set_temp);
        		UI_DIS_MAIN();
            }
            else
            {
				if (time_format_flag == FORMAT_24)
					time_format_flag = FORMAT_12;
				else
					time_format_flag = FORMAT_24;
	            UI_menu(MENU_TIME_FORMAT, 0, 0);
            }
            rtc_set_printf("rtc minus\n");
            break;
    #if 0
        case MSG_ALM_SW:
            //按键开关闹钟时候，自动改为设置闹钟模式
            rtc_time_set_temp.rtc_set_mode = ALM_SET_MODE;
            if (rtc_time_set_temp.alarm_set.alarm_sw) {
                rtc_time_set_temp.alarm_set.alarm_sw = 0;
            } else {
                rtc_time_set_temp.alarm_set.alarm_sw = 1;
            }
            alarm_sw(rtc_time_set_temp.alarm_set.alarm_sw);
            UI_menu(MENU_ALM_SET, 0, 0);
            rtc_set_printf("alm sw %d\n", rtc_time_set_temp.alarm_set.alarm_sw);
            break;
    #endif
        case MSG_HALF_SECOND:
            //时间设置计时
            rtc_set.calendar_set.rtc_set_cnt++;
            //时间设置超时退出
            if (rtc_set.calendar_set.rtc_set_cnt == RTC_SETTING_CNT) {
                rtc_set.calendar_set.rtc_set_cnt = 0;
                rtc_set_printf("calendar_set timeout\n");
                goto _rtc_setting_exit;
            }
            if (rtc_time_set_cnt)
    			rtc_time_set_cnt--;
            UI_REFRESH(MENU_REFRESH);
            break;
    	case MSG_RTC_POWER_DOWN:
    		task_post_msg(NULL, 1, MSG_POWER_OFF);
    		break;
        default:
            break;
        }
    }
    else if (rtc_set.rtc_set_mode == ALM_SET_MODE)
    {
        switch (msg) {
        case MSG_ALARM1_SET://移动光标到下一项
    		rtc_time_set_cnt = 0;
    		if (rtc_set.alarm_set.coordinate == ALARM_RING_TYPE)
    		{
			    if (alarm_ring_type == ALARM_RING_BELL)
			    {
			        rtc_set.alarm_set.coordinate++;
			    }
			    else if (alarm_ring_type == ALARM_RING_FM)
			    {
			        alm_fm_station_set_flag++;
			        if (alm_fm_station_set_flag > 1)
			        {
			            rtc_set.alarm_set.coordinate++;
			        }
			    }
    		}
    		else
    		{
			    rtc_set.alarm_set.coordinate++;
			}
    		if (rtc_set.alarm_set.coordinate > ALM_COORDINATE_MAX)
			{
    			rtc_set.alarm_set.coordinate = ALM_COORDINATE_MIN;
                goto _rtc_setting_exit;
			}
			if (rtc_set.alarm_set.coordinate == ALARM_RING_VOLUME)
			{
			    if (alarm_ring_type == ALARM_RING_BELL)
			    {
			        alm_ring_bell_volume();
			    }
			    else if (alarm_ring_type == ALARM_RING_FM)
			    {
			        alm_ring_fm_volume();
			    }
			}
    		UI_DIS_MAIN();
    		break;

        //case MSG_RTC_SET_OK:
        //    goto _rtc_setting_exit;
    	//	break;
        case MSG_RTC_PLUS:
            if (rtc_set.alarm_set.coordinate == ALARM_RING_TYPE)
            {
                if (alm_fm_station_set_flag == 1)
                {
                    alm_fm_station_temp++;
                    if (alm_fm_station_temp > fm_station_all_temp)
                    {
                        alm_fm_station_temp = 1;
                    }
                    alm_ring_set_fm_channel();
                }
                else
                {
                    if (alarm_ring_type == ALARM_RING_BELL)
                    {
                        alarm_ring_type = ALARM_RING_FM;
                    }
    				else
                    {
                        alarm_ring_type = ALARM_RING_BELL;
                    }
                }
            }
			else if (rtc_set.alarm_set.coordinate == ALARM_MODE)
			{
			    alarm_up_mode++;
				if (alarm_up_mode > ALARM_UP_6_7)
					alarm_up_mode = ALARM_UP_1_5;
			}
        case MSG_RTC_PLUS_HOLD:
			if (rtc_set.alarm_set.coordinate == ALARM_RING_VOLUME)
			{
			    alarm_ring_volume++;
				if (alarm_ring_volume > MAX_SYS_VOL_TEMP)
					alarm_ring_volume = MAX_SYS_VOL_TEMP;
				else
				{
            		sound.vol.sys_vol_l = volume_table[alarm_ring_volume];
            		sound.vol.sys_vol_r = sound.vol.sys_vol_l;
                    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
                }
			}
			else if (rtc_set.alarm_set.coordinate == ALARM_HOUR_SETTING)
			{
			    rtc_time_set_temp.bHour++;
				if (rtc_time_set_temp.bHour > 23)
					rtc_time_set_temp.bHour = 0;
			}
			else if (rtc_set.alarm_set.coordinate == ALARM_MIN_SETTING)
			{
			    rtc_time_set_temp.bMin++;
				if (rtc_time_set_temp.bMin > 59)
					rtc_time_set_temp.bMin = 0;
			}
        	rtc_time_set_cnt = RTC_SET_BACK_CNT;
            //calendar_time_plus(rtc_time_set_temp.alarm_set.curr_alm_time, rtc_time_set_temp.alarm_set.coordinate);
            //rtc_write_alarmtime(rtc_time_set_temp.alarm_set.curr_alm_time, rtc_time_set_temp.alarm_set.current_alarm_mode);
            //puts_time(rtc_time_set_temp.alarm_set.curr_alm_time);
    		UI_DIS_MAIN();
            rtc_set_printf("rtc plus\n");
            break;
    
        case MSG_RTC_MINUS:
            if (rtc_set.alarm_set.coordinate == ALARM_RING_TYPE)
            {
                if (alm_fm_station_set_flag == 1)
                {
                    if (alm_fm_station_temp)
                    {
                        alm_fm_station_temp--;
                        if (alm_fm_station_temp == 0)
                        {
                            alm_fm_station_temp = fm_station_all_temp;
                        }
                    }
                    if (alm_fm_station_temp == 0)
                    {
                        alm_fm_station_temp = 1;
                    }
                    alm_ring_set_fm_channel();
                }
                else
                {
                    if (alarm_ring_type == ALARM_RING_BELL)
                    {
                        alarm_ring_type = ALARM_RING_FM;
                    }
    				else
                    {
                        alarm_ring_type = ALARM_RING_BELL;
                    }
                }
            }
			else if (rtc_set.alarm_set.coordinate == ALARM_MODE)
			{
			    if (alarm_up_mode)
			    {
			        alarm_up_mode--;
			    }
				else
				{
			        alarm_up_mode = ALARM_UP_6_7;
				}
			}
        case MSG_RTC_MINUS_HOLD:
			if (rtc_set.alarm_set.coordinate == ALARM_RING_VOLUME)
			{
			    if (alarm_ring_volume)
			    {
			        alarm_ring_volume--;
			        //
            		sound.vol.sys_vol_l = volume_table[alarm_ring_volume];
            		sound.vol.sys_vol_r = sound.vol.sys_vol_l;
                    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
			    }
			}
			else if (rtc_set.alarm_set.coordinate == ALARM_HOUR_SETTING)
			{
			    if (rtc_time_set_temp.bHour)
					rtc_time_set_temp.bHour--;
				else
					rtc_time_set_temp.bHour = 23;
			}
			else if (rtc_set.alarm_set.coordinate == ALARM_MIN_SETTING)
			{
			    if (rtc_time_set_temp.bMin)
					rtc_time_set_temp.bMin--;
				else
					rtc_time_set_temp.bMin = 59;
			}
        	rtc_time_set_cnt = RTC_SET_BACK_CNT;
            //calendar_time_minus(rtc_time_set_temp.alarm_set.curr_alm_time, rtc_time_set_temp.alarm_set.coordinate);
            //rtc_write_alarmtime(rtc_time_set_temp.alarm_set.curr_alm_time, rtc_time_set_temp.alarm_set.current_alarm_mode);
            //rtc_clk_out(rtc_time_set_temp.alarm_set.curr_alm_time);
            //puts_time(rtc_time_set_temp.alarm_set.curr_alm_time);
    		UI_DIS_MAIN();
            rtc_set_printf("rtc minus\n");
            break;
    #if 0
        case MSG_ALM_SW:
            //按键开关闹钟时候，自动改为设置闹钟模式
            rtc_time_set_temp.rtc_set_mode = ALM_SET_MODE;
            if (rtc_time_set_temp.alarm_set.alarm_sw) {
                rtc_time_set_temp.alarm_set.alarm_sw = 0;
            } else {
                rtc_time_set_temp.alarm_set.alarm_sw = 1;
            }
            alarm_sw(rtc_time_set_temp.alarm_set.alarm_sw);
            UI_menu(MENU_ALM_SET, 0, 0);
            rtc_set_printf("alm sw %d\n", rtc_time_set_temp.alarm_set.alarm_sw);
            break;
    #endif
        case MSG_HALF_SECOND:
            //时间设置计时
            rtc_set.alarm_set.alm_set_cnt++;
            if (rtc_set.alarm_set.alm_set_cnt == ALM_SETTING_CNT) {
                rtc_set.alarm_set.alm_set_cnt = 0;
                rtc_set_printf("Alarm Set TimeOut\n");
                goto _rtc_setting_exit;
            }
            if (rtc_time_set_cnt)
    			rtc_time_set_cnt--;
            UI_REFRESH(MENU_REFRESH);
			if (rtc_set.alarm_set.coordinate == ALARM_RING_VOLUME)
			{
			    if (alarm_ring_type == ALARM_RING_BELL)
			    {
        			if (get_tone_status() == 0)
        			{
                        tone_play(TONE_ALARM_RING, 1);
        			}
			    }
			}
            break;
    	case MSG_RTC_POWER_DOWN:
    		task_post_msg(NULL, 1, MSG_POWER_OFF);
    		break;
        default:
            break;
        }
    }
    else if (rtc_set.rtc_set_mode == ALM2_SET_MODE)
    {
        switch (msg) {
        case MSG_ALARM2_SET://移动光标到下一项
    		rtc_time_set_cnt = 0;
    		if (rtc_set.alarm_set.coordinate == ALARM_RING_TYPE)
    		{
			    if (alarm_ring_type == ALARM_RING_BELL)
			    {
			        rtc_set.alarm_set.coordinate++;
			    }
			    else if (alarm_ring_type == ALARM_RING_FM)
			    {
			        alm_fm_station_set_flag++;
			        if (alm_fm_station_set_flag > 1)
			        {
			            rtc_set.alarm_set.coordinate++;
			        }
			    }
    		}
    		else
    		{
			    rtc_set.alarm_set.coordinate++;
    		}
    		if (rtc_set.alarm_set.coordinate > ALM_COORDINATE_MAX)
			{
    			rtc_set.alarm_set.coordinate = ALM_COORDINATE_MIN;
                goto _rtc_setting_exit;
			}
			if (rtc_set.alarm_set.coordinate == ALARM_RING_VOLUME)
			{
			    if (alarm2_ring_type == ALARM_RING_BELL)
			    {
			        alm2_ring_bell_volume();
			    }
			    else if (alarm2_ring_type == ALARM_RING_FM)
			    {
			        alm2_ring_fm_volume();
			    }
			}
    		UI_DIS_MAIN();
    		break;

        //case MSG_RTC_SET_OK:
        //    goto _rtc_setting_exit;
    	//	break;
        case MSG_RTC_PLUS:
            if (rtc_set.alarm_set.coordinate == ALARM_RING_TYPE)
            {
                if (alm_fm_station_set_flag == 1)
                {
                    alm_fm_station_temp++;
                    if (alm_fm_station_temp > fm_station_all_temp)
                    {
                        alm_fm_station_temp = 1;
                    }
                    alm_ring_set_fm_channel();
                }
                else
                {
                    if (alarm2_ring_type == ALARM_RING_BELL)
                    {
                        alarm2_ring_type = ALARM_RING_FM;
                    }
    				else
                    {
                        alarm2_ring_type = ALARM_RING_BELL;
                    }
                }
            }
			else if (rtc_set.alarm_set.coordinate == ALARM_MODE)
			{
			    alarm2_up_mode++;
				if (alarm2_up_mode > ALARM_UP_6_7)
					alarm2_up_mode = ALARM_UP_1_5;
			}
        case MSG_RTC_PLUS_HOLD:
			if (rtc_set.alarm_set.coordinate == ALARM_RING_VOLUME)
			{
			    alarm2_ring_volume++;
				if (alarm2_ring_volume > MAX_SYS_VOL_TEMP)
					alarm2_ring_volume = MAX_SYS_VOL_TEMP;
				else
				{
            		sound.vol.sys_vol_l = volume_table[alarm2_ring_volume];
            		sound.vol.sys_vol_r = sound.vol.sys_vol_l;
                    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
                }
			}
			else if (rtc_set.alarm_set.coordinate == ALARM_HOUR_SETTING)
			{
			    rtc_time_set_temp.bHour++;
				if (rtc_time_set_temp.bHour > 23)
					rtc_time_set_temp.bHour = 0;
			}
			else if (rtc_set.alarm_set.coordinate == ALARM_MIN_SETTING)
			{
			    rtc_time_set_temp.bMin++;
				if (rtc_time_set_temp.bMin > 59)
					rtc_time_set_temp.bMin = 0;
			}
        	rtc_time_set_cnt = RTC_SET_BACK_CNT;
            //calendar_time_plus(rtc_time_set_temp.alarm_set.curr_alm_time, rtc_time_set_temp.alarm_set.coordinate);
            //rtc_write_alarmtime(rtc_time_set_temp.alarm_set.curr_alm_time, rtc_time_set_temp.alarm_set.current_alarm_mode);
            //puts_time(rtc_time_set_temp.alarm_set.curr_alm_time);
    		UI_DIS_MAIN();
            rtc_set_printf("rtc plus\n");
            break;
    
        case MSG_RTC_MINUS:
            if (rtc_set.alarm_set.coordinate == ALARM_RING_TYPE)
            {
                if (alm_fm_station_set_flag == 1)
                {
                    if (alm_fm_station_temp)
                    {
                        alm_fm_station_temp--;
                        if (alm_fm_station_temp == 0)
                        {
                            alm_fm_station_temp = fm_station_all_temp;
                        }
                    }
                    if (alm_fm_station_temp == 0)
                    {
                        alm_fm_station_temp = 1;
                    }
                    alm_ring_set_fm_channel();
                }
                else
                {
                    if (alarm2_ring_type == ALARM_RING_BELL)
                    {
                        alarm2_ring_type = ALARM_RING_FM;
                    }
    				else
                    {
                        alarm2_ring_type = ALARM_RING_BELL;
                    }
                }
            }
			else if (rtc_set.alarm_set.coordinate == ALARM_MODE)
			{
			    if (alarm2_up_mode)
			    {
			        alarm2_up_mode--;
			    }
				else
				{
			        alarm2_up_mode = ALARM_UP_6_7;
				}
			}
        case MSG_RTC_MINUS_HOLD:
			if (rtc_set.alarm_set.coordinate == ALARM_RING_VOLUME)
			{
			    if (alarm2_ring_volume)
			    {
			        alarm2_ring_volume--;
			        //
            		sound.vol.sys_vol_l = volume_table[alarm2_ring_volume];
            		sound.vol.sys_vol_r = sound.vol.sys_vol_l;
                    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
			    }
			}
			else if (rtc_set.alarm_set.coordinate == ALARM_HOUR_SETTING)
			{
			    if (rtc_time_set_temp.bHour)
					rtc_time_set_temp.bHour--;
				else
					rtc_time_set_temp.bHour = 23;
			}
			else if (rtc_set.alarm_set.coordinate == ALARM_MIN_SETTING)
			{
			    if (rtc_time_set_temp.bMin)
					rtc_time_set_temp.bMin--;
				else
					rtc_time_set_temp.bMin = 59;
			}
        	rtc_time_set_cnt = RTC_SET_BACK_CNT;
            //calendar_time_minus(rtc_time_set_temp.alarm_set.curr_alm_time, rtc_time_set_temp.alarm_set.coordinate);
            //rtc_write_alarmtime(rtc_time_set_temp.alarm_set.curr_alm_time, rtc_time_set_temp.alarm_set.current_alarm_mode);
            //rtc_clk_out(rtc_time_set_temp.alarm_set.curr_alm_time);
            //puts_time(rtc_time_set_temp.alarm_set.curr_alm_time);
    		UI_DIS_MAIN();
            rtc_set_printf("rtc minus\n");
            break;
    #if 0
        case MSG_ALM_SW:
            //按键开关闹钟时候，自动改为设置闹钟模式
            rtc_time_set_temp.rtc_set_mode = ALM_SET_MODE;
            if (rtc_time_set_temp.alarm_set.alarm_sw) {
                rtc_time_set_temp.alarm_set.alarm_sw = 0;
            } else {
                rtc_time_set_temp.alarm_set.alarm_sw = 1;
            }
            alarm_sw(rtc_time_set_temp.alarm_set.alarm_sw);
            UI_menu(MENU_ALM_SET, 0, 0);
            rtc_set_printf("alm sw %d\n", rtc_time_set_temp.alarm_set.alarm_sw);
            break;
    #endif
        case MSG_HALF_SECOND:
            //时间设置计时
            rtc_set.alarm_set.alm_set_cnt++;
            if (rtc_set.alarm_set.alm_set_cnt == ALM_SETTING_CNT) {
                rtc_set.alarm_set.alm_set_cnt = 0;
                rtc_set_printf("Alarm Set TimeOut\n");
                goto _rtc_setting_exit;
            }
            if (rtc_time_set_cnt)
    			rtc_time_set_cnt--;
            UI_REFRESH(MENU_REFRESH);
			if (rtc_set.alarm_set.coordinate == ALARM_RING_VOLUME)
			{
			    if (alarm2_ring_type == ALARM_RING_BELL)
			    {
        			if (get_tone_status() == 0)
        			{
                        tone_play(TONE_ALARM_RING, 1);
        			}
			    }
			}
            break;
    	case MSG_RTC_POWER_DOWN:
    		task_post_msg(NULL, 1, MSG_POWER_OFF);
    		break;
        default:
            break;
        }
    }
    return;

_rtc_setting_exit:
    puts("rtc_setting_exit\n");
    if (rtc_set.rtc_set_mode == RTC_SET_MODE)
    {
        rtc_time_set_temp.bSec = 0;
        memcpy(&current_time, &rtc_time_set_temp, sizeof(rtc_time_set_temp));
    }
	else if (rtc_set.rtc_set_mode == ALM_SET_MODE)
    {
        memcpy(&current_alarm, &rtc_time_set_temp, sizeof(rtc_time_set_temp));
	    alm_times = 0;
	    memcpy(&current_alarm_temp,rtc_set.alarm_set.curr_alm_time,sizeof(RTC_TIME));
    	vm_write(VM_ALARM_TIME,&current_alarm_temp,VM_ALARM_TIME_LEN);
    	vm_write(VM_ALARM_MODE,&alarm_up_mode,VM_ALARM_MODE_LEN);
    	vm_write(VM_ALARM_VOLUME,&alarm_ring_volume,VM_ALARM_VOLUME_LEN);
    	vm_write(VM_ALARM_RING,&alarm_ring_type,VM_ALARM_RING_LEN);
        rtc_set_alarm_sw(&rtc_set, 1);
    	vm_write(VM_ALARM_SWITCH,&rtc_set.alarm_set.alarm_sw,VM_ALARM_SWITCH_LEN);
    	//
    	music_tone_stop();
    	if (alm_ring_fm_set_flag)
    	{
    	    alm_ring_fm_exit();
    	    alm_fm_station = alm_fm_station_temp;
    	    vm_write(VM_ALARM_RING_FM_STATION,&alm_fm_station,VM_ALARM_RING_FM_STATION_LEN);
    	}
        //volume_temp = alm_vol_set_sys_volume_temp;
		sound.vol.sys_vol_l = volume_table[volume_temp];
		sound.vol.sys_vol_r = sound.vol.sys_vol_l;
        set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    }
	else if (rtc_set.rtc_set_mode == ALM2_SET_MODE)
    {
        memcpy(&current_alarm2, &rtc_time_set_temp, sizeof(rtc_time_set_temp));
	    alm2_times = 0;
	    memcpy(&current_alarm2_temp,rtc_set.alarm_set.curr_alm2_time,sizeof(RTC_TIME));
    	vm_write(VM_ALARM2_TIME,&current_alarm2_temp,VM_ALARM2_TIME_LEN);
    	vm_write(VM_ALARM2_MODE,&alarm2_up_mode,VM_ALARM2_MODE_LEN);
    	vm_write(VM_ALARM2_VOLUME,&alarm2_ring_volume,VM_ALARM2_VOLUME_LEN);
    	vm_write(VM_ALARM2_RING,&alarm2_ring_type,VM_ALARM2_RING_LEN);
        rtc_set_alarm2_sw(&rtc_set, 1);
    	vm_write(VM_ALARM2_SWITCH,&rtc_set.alarm_set.alarm2_sw,VM_ALARM2_SWITCH_LEN);
    	//
    	music_tone_stop();
    	if (alm_ring_fm_set_flag)
    	{
    	    alm_ring_fm_exit();
    	    alm2_fm_station = alm_fm_station_temp;
    	    vm_write(VM_ALARM2_RING_FM_STATION,&alm2_fm_station,VM_ALARM2_RING_FM_STATION_LEN);
    	}
        //volume_temp = alm_vol_set_sys_volume_temp;
		sound.vol.sys_vol_l = volume_table[volume_temp];
		sound.vol.sys_vol_r = sound.vol.sys_vol_l;
        set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    }

	//alm_times = 0;
	//alm2_times = 0;
	//memcpy(&current_alarm_temp,rtc_set.alarm_set.curr_alm_time,sizeof(RTC_TIME));
	//memcpy(&current_alarm2_temp,rtc_set.alarm_set.curr_alm2_time,sizeof(RTC_TIME));
	//vm_write(VM_ALARM_TIME,&current_alarm_temp,VM_ALARM_TIME_LEN);
	//vm_write(VM_ALARM2_TIME,&current_alarm2_temp,VM_ALARM2_TIME_LEN);
	//vm_write(VM_ALARM_MODE,&alarm_up_mode,VM_ALARM_MODE_LEN);
	//vm_write(VM_ALARM2_MODE,&alarm2_up_mode,VM_ALARM2_MODE_LEN);
	//vm_write(VM_ALARM_VOLUME,&alarm_ring_volume,VM_ALARM_VOLUME_LEN);
	//vm_write(VM_ALARM2_VOLUME,&alarm2_ring_volume,VM_ALARM2_VOLUME_LEN);
    //rtc_write_alarmtime(rtc_set.alarm_set.curr_alm_time, rtc_set.alarm_set.current_alarm_mode);

    //rtc_write_alarmtime(rtc_set.alarm_set.curr_alm_time, rtc_set.alarm_set.current_alarm_mode);
    if (rtc_set.rtc_set_mode == RTC_SET_MODE)
    {
        rtc_write_datetime(rtc_set.calendar_set.curr_rtc_time);
    }
    rtc_read_datetime(rtc_set.calendar_set.curr_rtc_time);
	//if (rtc_set.rtc_set_mode == ALM_SET_MODE)
	{
		//rtc_set_alarm_sw(&rtc_set, 1);
	}
    rtc_set.rtc_set_mode = RTC_DISPLAY;
    rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
    rtc_set.alarm_set.coordinate = ALARM_HOUR_SETTING;
    SET_UI_MAIN(MENU_RTC_MAIN);
    UI_DIS_MAIN();
    time_alm_flag = 0;
    time_alm2_flag = 0;
	AMP_D();
    //return;
}

/*----------------------------------------------------------------------------*/
/** @brief: rtc中断回调函数
    @param: flag:中断类型标志
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
static void rtc_isr_user_handler(u8 flag)
{
#if 0
    if (RTC_ISR_ALARM_ON == flag) {
        rtc_set_printf("--ALM-ON-ISR--\n");
        rtc_set.alarm_set.alarm_sw = 0;

        alarm_sw(rtc_set.alarm_set.alarm_sw);
        rtc_set.alarm_set.alarm_flag = 1;
        task_post_msg(NULL, 1, MSG_ALM_UP);
		alm_time_cnt = 0;
    }
    if (RTC_ISR_PCNT == flag) {
        rtc_set_printf("--TYPE_PCNT_OVF--\n");
    }
    if (RTC_ISR_LDO5V == flag) {
        rtc_set_printf("--LDO5V DET--\n");
    }
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief 	复位RTC时间
   @param 	void
   @return  void
   @note  	void reset_irtc(void)，根据当前时钟有效值，决定是否复位时钟
*/
/*----------------------------------------------------------------------------*/
void reset_irtc(RTC_TIME *curr_time, RTC_TIME *curr_alm, RTC_TIME *curr_alm2)
{
    rtc_read_datetime(curr_time);
    //rtc_read_alarmtime(curr_alm);
	vm_read(VM_ALARM_TIME,curr_alm,VM_ALARM_TIME_LEN);
	vm_read(VM_ALARM2_TIME,curr_alm2,VM_ALARM2_TIME_LEN);

    rtc_set.calendar_set.curr_rtc_time = curr_time;
    rtc_set.alarm_set.curr_alm_time = curr_alm;
    rtc_set.alarm_set.curr_alm2_time = curr_alm2;
    if (((curr_time->dYear > 2100)
         || (curr_time->dYear < 2000))
        || (curr_time->bMonth >= 12)
        || (curr_time->bHour >= 24)
        || (curr_time->bMin >= 60)
        || (curr_time->bSec >= 60)
        || rtc_get_power_flag()) {
        puts("\n--------RTC RESET--------\n");
        rtc_set_printf("old time %d/%d/%d %d:%d:%d\n", curr_time->dYear, curr_time->bMonth, curr_time->bDay, curr_time->bHour, curr_time->bMin, curr_time->bSec);
        curr_time->dYear = 2020;
        curr_time->bMonth = 1;
        curr_time->bDay = 29;
        curr_time->bHour = 0;
        curr_time->bMin = 0;
        curr_time->bSec = 0;
        //curr_time->bWeekday = 0;

        //puts("\n--------ALM RESET--------\n");
        memcpy(curr_alm, curr_time, sizeof(RTC_TIME));
        memcpy(curr_alm2,curr_time,sizeof(RTC_TIME));
        curr_alm->bHour = 7;
        curr_alm->bMin = 0;
        curr_alm2->bHour = 7;
        curr_alm2->bMin = 30;

        ///update date
        rtc_write_datetime(curr_time);
        //rtc_write_alarmtime(curr_alm, rtc_set.alarm_set.current_alarm_mode);
        rtc_reset_irtc();
    	vm_write(VM_ALARM_TIME,curr_alm,VM_ALARM_TIME_LEN);
    	vm_write(VM_ALARM2_TIME,curr_alm2,VM_ALARM2_TIME_LEN);
        rtc_set.alarm_set.alarm_sw = 0;
    	vm_write(VM_ALARM_SWITCH,&rtc_set.alarm_set.alarm_sw,VM_ALARM_SWITCH_LEN);
        rtc_set.alarm_set.alarm2_sw = 0;
    	vm_write(VM_ALARM2_SWITCH,&rtc_set.alarm_set.alarm2_sw,VM_ALARM2_SWITCH_LEN);
        alarm_sw(rtc_set.alarm_set.alarm_sw);
    } else {
        rtc_reset_irtc();
    }
    //rtc_set.alarm_set.alarm_sw = get_rtc_alarm_en();//rtc_module_get_alarm_flag();
    vm_read(VM_ALARM_SWITCH,&rtc_set.alarm_set.alarm_sw,VM_ALARM_SWITCH_LEN);
    if (rtc_set.alarm_set.alarm_sw > 1)
		rtc_set.alarm_set.alarm_sw = 0;
	vm_read(VM_ALARM2_SWITCH,&rtc_set.alarm_set.alarm2_sw,VM_ALARM2_SWITCH_LEN);
	if (rtc_set.alarm_set.alarm2_sw > 1)
		rtc_set.alarm_set.alarm2_sw = 0;
    vm_read(VM_ALARM_RING,&alarm_ring_type,VM_ALARM_RING_LEN);
    if (alarm_ring_type > ALARM_RING_FM)
		alarm_ring_type = ALARM_RING_BELL;
	vm_read(VM_ALARM2_RING,&alarm2_ring_type,VM_ALARM2_RING_LEN);
    if (alarm2_ring_type > ALARM_RING_FM)
		alarm2_ring_type = ALARM_RING_BELL;
    vm_read(VM_ALARM_MODE,&alarm_up_mode,VM_ALARM_MODE_LEN);
    if (alarm_up_mode > ALARM_UP_6_7)
		alarm_up_mode = ALARM_UP_1_5;
	vm_read(VM_ALARM2_MODE,&alarm2_up_mode,VM_ALARM2_MODE_LEN);
    if (alarm2_up_mode > ALARM_UP_6_7)
		alarm2_up_mode = ALARM_UP_1_5;
    vm_read(VM_ALARM_VOLUME,&alarm_ring_volume,VM_ALARM_VOLUME_LEN);
    if (alarm_ring_volume > 32)
		alarm_ring_volume = 15;
	vm_read(VM_ALARM2_VOLUME,&alarm2_ring_volume,VM_ALARM2_VOLUME_LEN);
    if (alarm2_ring_volume > 32)
		alarm2_ring_volume = 15;
    vm_read(VM_ALARM_RING_FM_STATION,&alm_fm_station,VM_ALARM_RING_FM_STATION_LEN);
    //if (alm_fm_station > 32)
	//	alm_fm_station = 15;
    vm_read(VM_ALARM2_RING_FM_STATION,&alm2_fm_station,VM_ALARM2_RING_FM_STATION_LEN);
    //if (alm2_fm_station > 32)
	//	alm2_fm_station = 15;
    memcpy(&current_alarm_temp, curr_alm, sizeof(RTC_TIME));
    memcpy(&current_alarm2_temp,curr_alm2,sizeof(RTC_TIME));
    alarm_sw(0);
    rtc_week = rtc_week_check(rtc_set.calendar_set.curr_rtc_time);
}

/****************************************************

    note        :open rtc hardware
******************************************************/
static void irtc_hw_init(void)
{
    //if (check_alarm_out()) {
        //rtc_set.alarm_set.alarm_flag = 1;
        //update mode ,maybe powerup alarming
        //rtc_set.alarm_set.current_alarm_mode = ALARM_MODE_EVERY_DAY;//current_alarm.bSec & 0x03;
    //} else {
        //rtc_set.alarm_set.alarm_flag = 0;
    //}
    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
    rtc_set.alarm_set.current_alarm_mode = ALARM_MODE_EVERY_DAY;
	rtc_set.alarm_set.current_alarm2_mode = ALARM_MODE_EVERY_DAY;
    rtc_clk_enable(WAKE_UP_DISENABLE, rtc_isr_user_handler);

    reset_irtc(&current_time, &current_alarm, &current_alarm2);
}
no_sequence_initcall(irtc_hw_init);
/*----------------------------------------------------------------------------*/
/**@brief  RTC DAC通道选择，开启
   @param  无
   @return 无
   @note   void aux_dac_channel_on(void)
*/
/*----------------------------------------------------------------------------*/
void rtc_dac_channel_on(void)
{
    dac_mute(1, 1);
    dac_channel_on(RTC_CHANNEL, FADE_ON);
    //delay(15);// os_time_dly(15);  //这个参数会影响到上电第一次进入line in的冲击声。可以根据样机调整
    dac_mute(0, 1);
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
}

/*----------------------------------------------------------------------------*/
/**@brief  RTC DAC通道选择，关闭
   @param  无
   @return 无
   @note   void aux_dac_channel_off(void)
*/
/*----------------------------------------------------------------------------*/
void rtc_dac_channel_off(void)
{
    dac_channel_off(RTC_CHANNEL, FADE_ON);
    dac_mute(0, 1);
}
/*----------------------------------------------------------------------------*/
/**@brief  RTC初始化
   @param  void
   @return 无
   @note   void rtc_info_init(void)
*/
/*----------------------------------------------------------------------------*/
RTC_SETTING *rtc_info_init()
{
    //alarm_sw(rtc_set.alarm_set.alarm_sw);
    rtc_dac_channel_on();
    //rtc_set.rtc_set_mode = RTC_DISPLAY;
    //rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
    //rtc_set.alarm_set.coordinate = ALARM_HOUR_SETTING;
    return &rtc_set;
}

/*----------------------------------------------------------------------------*/
/**@brief  根据闹钟模式设置循环闹钟的日期
   @param  none
   @return none
   @note
*/
/*----------------------------------------------------------------------------*/
void auto_set_next_alarm(RTC_SETTING *rtc_set)
{
    if (rtc_set == NULL) {
        return ;
    }
    if (rtc_set->alarm_set.current_alarm_mode == ALARM_MODE_EVERY_DAY) {
        //every day
        rtc_calculate_next_day(&current_alarm);
    } else {
        //one day for every week
        rtc_calculate_next_weekday(&current_alarm);
    }

    rtc_set_printf("auto_set_alarm:%d-%d-%d,%d:%d:%d\n", current_alarm.dYear, current_alarm.bMonth, current_alarm.bDay, \
                   current_alarm.bHour, current_alarm.bMin, current_alarm.bSec);

    rtc_write_alarmtime(&current_alarm, rtc_set->alarm_set.current_alarm_mode);
    rtc_set->alarm_set.alarm_sw = 1;

    alarm_sw(rtc_set->alarm_set.alarm_sw);
	alm_times = 0;
    memcpy(&current_alarm_temp,&current_alarm,sizeof(RTC_TIME));
}

void alarm_time_reset(void)
{
    alm_times = 0;
    rtc_set.alarm_set.alarm_flag = 0;
 	current_alarm_temp.bHour = rtc_set.alarm_set.curr_alm_time->bHour;
 	current_alarm_temp.bMin = rtc_set.alarm_set.curr_alm_time->bMin;
}

void alarm2_time_reset(void)
{
    alm2_times = 0;
    rtc_set.alarm_set.alarm2_flag = 0;
 	current_alarm2_temp.bHour = rtc_set.alarm_set.curr_alm2_time->bHour;
 	current_alarm2_temp.bMin = rtc_set.alarm_set.curr_alm2_time->bMin;
}

void alarm_time_reset_spc(void)
{
    alm_times = 0;
    //rtc_set.alarm_set.alarm_flag = 0;
 	current_alarm_temp.bHour = rtc_set.alarm_set.curr_alm_time->bHour;
 	current_alarm_temp.bMin = rtc_set.alarm_set.curr_alm_time->bMin;
}

void alarm2_time_reset_spc(void)
{
    alm2_times = 0;
    //rtc_set.alarm_set.alarm2_flag = 0;
 	current_alarm2_temp.bHour = rtc_set.alarm_set.curr_alm2_time->bHour;
 	current_alarm2_temp.bMin = rtc_set.alarm_set.curr_alm2_time->bMin;
}

void resave_almtime(void)
{
	//alarm_time_set();
}

void alarm_restart(void)
{
#if 0
	current_alarm_temp.bMin += MINUTE_ALARM_BELL;
	if (current_alarm_temp.bMin >= 60)
	{
 		current_alarm_temp.bMin -= 60;
 		current_alarm_temp.bHour++;
         if (current_alarm_temp.bHour >= 24)
         {
		 	current_alarm_temp.bHour = 0;
         }
	}
#else
    u8 minute_temp,hour_temp;
    rtc_read_datetime(rtc_set.calendar_set.curr_rtc_time);
    minute_temp = rtc_set.calendar_set.curr_rtc_time->bMin;
    hour_temp = rtc_set.calendar_set.curr_rtc_time->bHour;
    
	minute_temp += MINUTE_ALARM_BELL;
	if (minute_temp >= 60)
	{
 		minute_temp -= 60;
 		hour_temp++;
        if (hour_temp >= 24)
        {
			hour_temp = 0;
        }
	}
	current_alarm_temp.bMin = minute_temp;
	current_alarm_temp.bHour = hour_temp;
#endif
    rtc_set_printf("alarm_restart:%d-%d-%d,%d:%d:%d\n",current_alarm_temp.dYear,current_alarm_temp.bMonth,current_alarm_temp.bDay,\
           current_alarm_temp.bHour,current_alarm_temp.bMin,current_alarm_temp.bSec);
}

void alarm2_restart(void)
{
#if 0
	current_alarm2_temp.bMin += MINUTE_ALARM_BELL;
	if (current_alarm2_temp.bMin >= 60)
	{
 		current_alarm2_temp.bMin -= 60;
 		current_alarm2_temp.bHour++;
         if (current_alarm2_temp.bHour >= 24)
         {
		 	current_alarm2_temp.bHour = 0;
         }
	}
#else
    u8 minute_temp,hour_temp;
    rtc_read_datetime(rtc_set.calendar_set.curr_rtc_time);
    minute_temp = rtc_set.calendar_set.curr_rtc_time->bMin;
    hour_temp = rtc_set.calendar_set.curr_rtc_time->bHour;
    
	minute_temp += MINUTE_ALARM_BELL;
	if (minute_temp >= 60)
	{
 		minute_temp -= 60;
 		hour_temp++;
        if (hour_temp >= 24)
        {
			hour_temp = 0;
        }
	}
	current_alarm2_temp.bMin = minute_temp;
	current_alarm2_temp.bHour = hour_temp;
#endif
    rtc_set_printf("alarm_restart:%d-%d-%d,%d:%d:%d\n",current_alarm_temp.dYear,current_alarm_temp.bMonth,current_alarm_temp.bDay,\
           current_alarm_temp.bHour,current_alarm_temp.bMin,current_alarm_temp.bSec);
}

void alarm_restart_times(void)
{
	alm_times++;
	if (alm_times < ALARM_REBELL_TIMES)
	{
		alarm_restart();
	}
	else
	{
	    alm_times = 0;
 		current_alarm_temp.bHour = rtc_set.alarm_set.curr_alm_time->bHour;
 		current_alarm_temp.bMin = rtc_set.alarm_set.curr_alm_time->bMin;
	}
}

void alarm2_restart_times(void)
{
	alm2_times++;
	if (alm2_times < ALARM_REBELL_TIMES)
	{
		alarm2_restart();
	}
	else
	{
	    alm2_times = 0;
 		current_alarm2_temp.bHour = rtc_set.alarm_set.curr_alm2_time->bHour;
 		current_alarm2_temp.bMin = rtc_set.alarm_set.curr_alm2_time->bMin;
	}
}

u8 rtc_week_check(RTC_TIME *time_in)
{
    struct rtc_data hw_alarmtempdate;
    RTC_TIME_to_rtc_data(&hw_alarmtempdate, time_in);
	return (hw_alarmtempdate.days % 7);
}

#endif  /*RTC_CLK_EN*/
