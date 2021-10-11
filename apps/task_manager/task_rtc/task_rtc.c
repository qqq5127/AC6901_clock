#include "sdk_cfg.h"
#include "msg.h"
#include "task_common.h"
#include "wdt.h"
#include "task_rtc.h"
#include "task_rtc_key.h"
#include "rtc_setting.h"
#include "sys_detect.h"
#include "key_voice.h"
#include "warning_tone.h"
#include "music_player.h"
#include "rtc_ui.h"
#include "ui_api.h"
#include "led.h"
#include "audio/tone.h"
#include "audio/dac_api.h"
#include "audio/audio.h"
#include "audio/dac.h"
#include "bluetooth/avctp_user.h"


#if RTC_CLK_EN
#define RTC_TASK_DEBUG_ENABLE
#ifdef RTC_TASK_DEBUG_ENABLE
#define rtc_task_printf log_printf
#else
#define rtc_task_printf(...)
#endif
#define delay_n10ms(x)  delay_2ms(5*(x))
void rtc_alm_coordinate_init(void)
{
    rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
    rtc_set.alarm_set.coordinate = ALARM_HOUR_SETTING;
}

static void *task_rtc_init(void *priv)
{
    rtc_task_printf("rtc task init\n");
    //alm_vol_set_sys_volume_temp = volume_temp;
	//sound.vol.sys_vol_l = volume_table[alm_vol_set_sys_volume_temp];
	//sound.vol.sys_vol_r = sound.vol.sys_vol_l;
	auto_power_off_type = AUTO_TIME_OFF;
    auto_power_off_cnt = 0;
	mode_type = TASK_ID_RTC;
#if 0
#if USE_16_LEVEL_VOLUME
	sys_volume_temp = volume_temp;
	sound.vol.sys_vol_l = volume_table[WARNING_ALM_VOLUME_START];
	sound.vol.sys_vol_r = sound.vol.sys_vol_l;
#else
    sys_volume_temp = sound.vol.sys_vol_l;
	sound.vol.sys_vol_l = WARNING_ALM_VOLUME_START;
	sound.vol.sys_vol_r = sound.vol.sys_vol_l;
#endif
    led_idle();//led_fre_set(C_BLED_SLOW_MODE);
    if (rtc_set.alarm_set.alarm_flag)
    {
    	task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
	    rtc_set.rtc_set_mode = ALM_DISPLAY;
	    UI_menu(MENU_ALM_DISPLAY, 0, 0);
    }
	else
	{
	    tone_play(TONE_RTC_MODE, 0);
	    rtc_set.rtc_set_mode = RTC_DISPLAY;
	}
#else
    //rtc_dac_channel_on();
    #if 0
    if (rtc_tone_enable_flag)
    {
        tone_play(TONE_RTC_MODE, 0);
    }
	else
	#endif
	{
	    task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
	}
	rtc_tone_enable_flag = 1;
    rtc_set.rtc_set_mode = RTC_DISPLAY;
#endif
    rtc_update_time(&rtc_set);
    rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
    rtc_set.alarm_set.coordinate = ALARM_HOUR_SETTING;
    ui_open_rtc(&rtc_set, sizeof(RTC_SETTING));
    return NULL;
}

static void task_rtc_exit(void **hdl)
{
    rtc_task_printf("rtc task exit\n");
	//alarm_time_reset();
    rtc_dac_channel_off();
    //dac_channel_on(MUSIC_CHANNEL, FADE_ON);
    task_clear_all_message();
    ui_close_rtc();
#if USE_16_LEVEL_VOLUME
	//volume_temp = sys_volume_temp;
	//sound.vol.sys_vol_l = volume_table[volume_temp];
	//sound.vol.sys_vol_r = sound.vol.sys_vol_l;
#else
    //sound.vol.sys_vol_l = sys_volume_temp;
	//sound.vol.sys_vol_r = sound.vol.sys_vol_l;
#endif
	auto_power_off_type = AUTO_TIME_OFF;
    rtc_set.rtc_set_mode = RTC_DISPLAY;
    //volume_temp = alm_vol_set_sys_volume_temp;
	sound.vol.sys_vol_l = volume_table[volume_temp];
	sound.vol.sys_vol_r = sound.vol.sys_vol_l;
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
	AMP_D();
	mode_type = 0;
}

static void task_rtc_deal(void *hdl)
{
    int msg;
    RTC_SETTING *rtc_api = NULL;
    int msg_error = MSG_NO_ERROR;
    tbool ret = true;

    printf("****************RTC TSAK*********************\n");
    while (1) {
        clear_wdt();
        msg_error = task_get_msg(0, 1, &msg);
        if (task_common_msg_deal(hdl, msg) == false) {
            music_tone_stop();
            if (alm_ring_fm_set_flag)
            {
    	        alm_ring_fm_exit();
    	    }
            //rtc_set_alarm_flag(rtc_api, 0);
            task_common_msg_deal(NULL, NO_MSG);
            return;
        }
        if (NO_MSG == msg) {
            continue;
        }

        if (rtc_get_setting_flag(rtc_api) == true) { //进入设置模式
            rtc_setting(msg);
            //continue;
        }
		else
		{
		#if 0
	        if (rtc_get_alarm_flag(rtc_api)) {
	            if (msg == MSG_RTC_SET) {
	                rtc_task_printf("stop alarm_ui\n");
	                rtc_stop_alm(rtc_api);
				#if 0
	                if (rtc_get_alarm_mode(rtc_api) > ALARM_MODE_ONCE) {
	                    auto_set_next_alarm(rtc_api);
	                }
				#else
					alarm_restart_times();
				#endif
	                continue;
	            }
	        }
        #endif

	        switch (msg) {
	        case SYS_EVENT_PLAY_SEL_END:
	            rtc_api = rtc_info_init();
	            rtc_update_time(rtc_api);
	            ui_open_rtc(rtc_api, sizeof(RTC_SETTING));
	            break;
#if 0
	        case MSG_RTC_SETTING:                  ///RTC时间设置
	            rtc_task_printf("MSG_RTC_SETTING\n");
	            rtc_start_setting(rtc_api, RTC_SET_MODE);
	            break;

#if RTC_ALM_EN
	        case MSG_ALM_SETTING:                     ///闹钟时间设置
	            rtc_task_printf("MSG_ALM_SETTING\n");
	            rtc_set_alarm_sw(rtc_api, 1);
	            rtc_set_alarm_mode(rtc_api, ALARM_MODE_ONCE);
	            rtc_start_setting(rtc_api, ALM_SET_MODE);
	            break;
#endif
#else
	        case MSG_RTC_SET:
	        #if 0
	            rtc_set.calendar_set.coordinate++;
				if (rtc_set.calendar_set.coordinate == RTC_DAT_SETTING)
	                rtc_set.calendar_set.coordinate++;
    			if (rtc_set.calendar_set.coordinate >= COORDINATE_MAX)
    			{
        			rtc_set.calendar_set.coordinate = COORDINATE_MIN;
    			}
				coordinate_back_cnt = RTC_COORDINATE_BACK_CNT;
    		    UI_DIS_MAIN();
    		#else
			    task_post_msg(NULL, 1, MSG_BACK_LIGHT_SET);//(NULL, 1, MSG_TIME_FORMAT);
    		#endif
	            break;
	        case MSG_ALARM1_SET:
				if (UI_var.bCurMenu != MENU_ALM_DISPLAY)
				{
				}
				else
				{
    	            if (rtc_get_alarm_sw(rtc_api)) {
    	                rtc_set_alarm_sw(rtc_api, 0);
    	            } else {
    	                rtc_set_alarm_sw(rtc_api, 1);
				        alarm_time_reset();
    	            }
                	vm_write(VM_ALARM_SWITCH,&rtc_set.alarm_set.alarm_sw,VM_ALARM_SWITCH_LEN);
				}
	            UI_menu(MENU_ALM_DISPLAY, 0, 6);
        		rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
	            break;
	        case MSG_ALARM2_SET:
				if (UI_var.bCurMenu != MENU_ALM2_DISPLAY)
				{
				}
				else
				{
    	            if (rtc_get_alarm2_sw(rtc_api)) {
    	                rtc_set_alarm2_sw(rtc_api, 0);
    	            } else {
    	                rtc_set_alarm2_sw(rtc_api, 1);
				        alarm2_time_reset();
    	            }
                	vm_write(VM_ALARM2_SWITCH,&rtc_set.alarm_set.alarm2_sw,VM_ALARM2_SWITCH_LEN);
				}
	            UI_menu(MENU_ALM2_DISPLAY, 0, 6);
        		rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
	            break;
	        case MSG_RTC_SET_OK:                  ///RTC时间或ALM时间设置
	        	//if (rtc_set.rtc_set_mode == RTC_DISPLAY)
	        	{
	            	rtc_start_setting(rtc_api, RTC_SET_MODE);
                    memcpy(&rtc_time_set_temp, rtc_set.calendar_set.curr_rtc_time, sizeof(RTC_TIME));
	        	}
				//else
				{
	            //	rtc_set_alarm_sw(rtc_api, 0);	//设置闹钟时间前，关闭闹钟
	            //	rtc_set_alarm_mode(rtc_api, rtc_set.alarm_set.current_alarm_mode);
	            //	rtc_start_setting(rtc_api, ALM_SET_MODE);
				}
    		    UI_menu(MENU_TIME_FORMAT, 0, 0);//UI_DIS_MAIN();
	        	break;
	        case MSG_ALARM1_SET_OK:
            	//rtc_set_alarm_sw(rtc_api, 0);
            	//rtc_set_alarm_mode(rtc_api, rtc_set.alarm_set.current_alarm_mode);
            	rtc_start_setting(rtc_api, ALM_SET_MODE);
                memcpy(&rtc_time_set_temp, rtc_set.alarm_set.curr_alm_time, sizeof(RTC_TIME));
    		    UI_DIS_MAIN();
	        	break;
	        case MSG_ALARM2_SET_OK:
            	//rtc_set_alarm_sw(rtc_api, 0);
            	//rtc_set_alarm_mode(rtc_api, rtc_set.alarm_set.current_alarm2_mode);
            	rtc_start_setting(rtc_api, ALM2_SET_MODE);
                memcpy(&rtc_time_set_temp, rtc_set.alarm_set.curr_alm2_time, sizeof(RTC_TIME));
    		    UI_DIS_MAIN();
	        	break;
			case MSG_TIME_FORMAT:
				if (time_format_flag == FORMAT_24)
					time_format_flag = FORMAT_12;
				else
					time_format_flag = FORMAT_24;
	            UI_menu(MENU_TIME_FORMAT, 0, 5);
        		rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
				break;
            case MSG_DIMMER:
	        	if (rtc_set.rtc_set_mode == RTC_DISPLAY)
	        	{
	        	#if 0
	        	    led7_display_level++;
					if (led7_display_level > LED7_LEVEL_OFF)
						led7_display_level = LED7_LEVEL_MAX;
                    set1628Display();
    		    #else
    			    //task_post_msg(NULL, 1, MSG_SLEEP_SET);
    		    #endif
	        	}
				break;
#endif

	        case MSG_ALM_SW:
	            if (rtc_get_alarm_sw(rtc_api)) {
	                rtc_set_alarm_sw(rtc_api, 0);
	            } else {
	                rtc_set_alarm_sw(rtc_api, 1);
	            }
	            UI_menu(MENU_ALM_SET, 0, 2);
	            break;

	        case MSG_HALF_SECOND:
			#if DAC_AUTO_MUTE_EN
				if ((is_dac_mute())||(is_auto_mute()))
				{
					//if (rtc_set.alarm_set.alarm_flag)
					{
					//	AMP_UNMUTE();
					}
					//else
					{
						if ((!get_tone_status())&&(!is_sin_tone_busy()))
							AMP_MUTE();
					}
				}
				else
				{
					AMP_UNMUTE();
				}
			#endif
			#if 0
	            if (rtc_get_alarm_flag(rtc_api)) {
	                puts("alarm ring\n");
	                sin_tone_play(250);
					alm_time_cnt++;
					if (alm_time_cnt > ALARM_TIME)
					{
						rtc_stop_alm_api();
						alarm_restart_times();
						//if (alarm_off_back)
						{
							//task_post_msg(0, 1, MSG_LAST_WORKMOD);
						}
					}
	                //UI_menu(MENU_ALM_UP, 0, 0);
	            } //else {
	        #endif
	                rtc_update_time(rtc_api);
	                UI_REFRESH(MENU_REFRESH);
	            //}
	            if (coordinate_back_cnt)
	            {
	                coordinate_back_cnt--;
					if (coordinate_back_cnt == 0)
					{
        			    rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
					}
	            }
	            break;

	        case MSG_INPUT_NUMBER_END:
	        case MSG_INPUT_TIMEOUT:
	            get_input_number(NULL);
	            break;
			case MSG_RTC_POWER_DOWN:
				task_post_msg(NULL, 1, MSG_POWER_OFF);
				break;
			case MSG_CHANGE_WORKMODE_PRE:
				task_post_msg(NULL, 1, MSG_LAST_WORKMOD);
				break;

            case MSG_ALARM_STOP:
    			alarm_time_reset();
    			alarm2_time_reset();
                break;

            case MSG_SYS_RESET:
                bt_info_clear();
                fm_info_clear();
            #if 0
                volume_temp = SYS_DEFAULT_VOL;
            	sound.vol.sys_vol_l = volume_table[volume_temp];
            	sound.vol.sys_vol_r = sound.vol.sys_vol_l;
                set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
            #endif
	            UI_menu(MENU_RESET, 0, 0);
	            delay_n10ms(200);
	            UI_menu(MENU_ALL_DISPLAY, 0, 0);
	            delay_n10ms(200);
	            UI_menu(MENU_VERSION_2, 0, 0);
	            delay_n10ms(200);
    		    task_post_msg(NULL, 1, MSG_POWER_OFF);
                break;
	        default:
	            break;
	        }
		}
    }
}

u8 alarm_check(void)
{
    rtc_read_datetime(rtc_set.calendar_set.curr_rtc_time);
	if (rtc_set.calendar_set.curr_rtc_time->bHour == current_alarm_temp.bHour)
	{
	    if (rtc_set.calendar_set.curr_rtc_time->bMin == current_alarm_temp.bMin)
	    {
	        if (rtc_set.calendar_set.curr_rtc_time->bSec <= 10)
	        {
	            //rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
	            //alarm_restart_times();
	            //if (rtc_set.alarm_set.alarm_flag == ALARM_OFF)
	            {
	            //    alm_time_cnt = 0;
	            //    rtc_set.alarm_set.alarm_flag = ALARM_ON;
	                if (time_alm_flag == 0)
	                {
    	                time_alm_flag = 1;
    	                return 1;
	                }
	            }
	        }
	    }
	}
	return 0;
}

u8 alarm2_check(void)
{
    rtc_read_datetime(rtc_set.calendar_set.curr_rtc_time);
	if (rtc_set.calendar_set.curr_rtc_time->bHour == current_alarm2_temp.bHour)
	{
	    if (rtc_set.calendar_set.curr_rtc_time->bMin == current_alarm2_temp.bMin)
	    {
	        if (rtc_set.calendar_set.curr_rtc_time->bSec <= 10)
	        {
	            //rtc_set.alarm_set.alarm_flag = ALARM_OFF;
	            //alarm2_restart_times();
	            //if (rtc_set.alarm_set.alarm2_flag == ALARM_OFF)
	            {
	            //    alm_time_cnt = 0;
	            //    rtc_set.alarm_set.alarm2_flag = ALARM_ON;
	                if (time_alm2_flag == 0)
	                {
    	                time_alm2_flag = 1;
	                    return 1;
	                }
	            }
	        }
	    }
	}
	return 0;
}

u8 alarm_up_mode_check(void)
{
#if 0
    //if (rtc_set.alarm_set.alarm_sw == 0)
    //    return 0;
	//if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status())
    //    return 0;
    if (alarm_up_mode == ALARM_UP_1_5)
    {
        if ((rtc_week >= WEEKDAY_1)
			&&(rtc_week <= WEEKDAY_5))
        {
	        return 1;
        }
    }
	else if (alarm_up_mode == ALARM_UP_1_7)
    {
        if ((rtc_week >= WEEKDAY_6)
			&&(rtc_week <= WEEKDAY_5))
        {
	        return 1;
        }
    }
	else if (alarm_up_mode == ALARM_UP_6_7)
    {
        if ((rtc_week >= WEEKDAY_6)
			&&(rtc_week <= WEEKDAY_7))
        {
	        return 1;
        }
    }
    return 0;
#else
    return 1;
#endif
}

u8 alarm2_up_mode_check(void)
{
#if 0
    //if (rtc_set.alarm_set.alarm2_sw == 0)
    //    return 0;
	//if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status())
    //    return 0;
    if (alarm2_up_mode == ALARM_UP_1_5)
    {
        if ((rtc_week >= WEEKDAY_1)
			&&(rtc_week <= WEEKDAY_5))
        {
	        return 1;
        }
    }
	else if (alarm2_up_mode == ALARM_UP_1_7)
    {
        if ((rtc_week >= WEEKDAY_6)
			&&(rtc_week <= WEEKDAY_5))
        {
	        return 1;
        }
    }
	else if (alarm2_up_mode == ALARM_UP_6_7)
    {
        if ((rtc_week >= WEEKDAY_6)
			&&(rtc_week <= WEEKDAY_7))
        {
	        return 1;
        }
    }
    return 0;
#else
    return 1;
#endif
}

const TASK_APP task_rtc_info = {
    .skip_check  = NULL,
    .init        = task_rtc_init,
    .exit        = task_rtc_exit,
    .task        = task_rtc_deal,
    .key         = &task_rtc_key,
};

//
void alarm_ring_volme_set(void)
{
    sys_volume_temp = volume_temp;
	if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
	    volume_temp = alarm_ring_volume;
	else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
	    volume_temp = alarm2_ring_volume;
    sound.vol.sys_vol_l = volume_table[volume_temp];
    sound.vol.sys_vol_r = volume_table[volume_temp];
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
}
static void *task_alarm_init(void *priv)
{
    alarm_ring_mode_flag = 1;
    rtc_set.rtc_set_mode = ALM_DISPLAY;
    rtc_update_time(&rtc_set);
    rtc_set.calendar_set.coordinate = RTC_HOUR_SETTING;
    rtc_set.alarm_set.coordinate = ALARM_HOUR_SETTING;
#if 0
    ui_open_rtc(&rtc_set, sizeof(RTC_SETTING));
#else
	ui_mode_wait_flag = 0;
    SET_UI_MAIN(MENU_ALM_UP_DISPLAY);
    SET_UI_BUF((u32 *)&rtc_set, sizeof(RTC_SETTING));
    UI_DIS_MAIN();
    rtc_task_printf("MENU_ALM_UP_DISPLAY\n");
#endif

    rtc_task_printf("task_alarm1_init !!\n");
	//task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
	alarm_ring_volme_set();
    return NULL;
}

static void task_alarm_exit(void **hdl)
{
    rtc_task_printf("task_alarm1_exit !!\n");
    task_clear_all_message();
    //sound.vol.sys_vol_l = volume_table[sys_volume_temp];
    //sound.vol.sys_vol_r = volume_table[sys_volume_temp];
    //set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    alarm_ring_mode_flag = 0;
    rtc_set.rtc_set_mode = RTC_DISPLAY;
}

static void task_alarm_deal(void *hdl)
{
    int error = MSG_NO_ERROR;
    int msg = NO_MSG;
    rtc_task_printf("task_alarm1_deal !!\n");
    tone_play(TONE_ALARM_RING, 1);

    while (1) {

        error = task_get_msg(0, 1, &msg);
        if (task_common_msg_deal(hdl, msg) == false) {
            music_tone_stop();
            task_common_msg_deal(NULL, NO_MSG);
			if (alarm_up_flag == 0)
			{
                if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
            	{
                    alarm_time_reset();
            	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
            	}
                else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
            	{
                    alarm2_time_reset();
            	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
            	}
			}
			alarm_up_flag = 0;
            return ;
        }
        if (NO_MSG == msg) {
            continue;
    	}

        rtc_task_printf("alarm1 msg = %x\n", msg);
        switch (msg) {
        case MSG_HALF_SECOND:
	        UI_REFRESH(MENU_REFRESH);
			alm_time_cnt++;
			if (alm_time_cnt >= ALARM_TIME)
			{
        	    if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
            	{
				    alarm_time_reset();
            	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
            	}
			    else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
            	{
				    alarm2_time_reset();
            	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
            	}
    			rtc_tone_enable_flag = 0;
			    task_post_msg(NULL, 1, MSG_RTC_MODE);
			}
			if (get_tone_status() == 0)
			{
			    if (tone_display_flag == 0)
			    {
                    if (tone_back_cnt)
                        tone_back_cnt--;
                    if (tone_back_cnt == 0)
                    {
                        tone_play(TONE_ALARM_RING, 1);
                    }
                }
			}
            break;

        case SYS_EVENT_DEC_END:
            break;

        case SYS_EVENT_PLAY_SEL_END:
            rtc_task_printf("SYS_EVENT_PLAY_TONE_END\n");
            break;

        case MSG_ALARM_SNOOZE:
        	if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
        	{
        	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
				//if (alm_times)
				{
				    rtc_tone_enable_flag = 0;
			        task_post_msg(NULL, 1, MSG_RTC_MODE);
				}
				//else
				{
			    //    task_post_msg(NULL, 1, MSG_RTC_MODE);//task_post_msg(NULL, 1, MSG_CHANGE_WORKMODE);
				}
            	alarm_restart_times();
        	}
			else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
        	{
        	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
				//if (alm2_times)
				{
				    rtc_tone_enable_flag = 0;
			        task_post_msg(NULL, 1, MSG_RTC_MODE);
				}
				//else
				{
			    //    task_post_msg(NULL, 1, MSG_RTC_MODE);//task_post_msg(NULL, 1, MSG_CHANGE_WORKMODE);
				}
            	alarm2_restart_times();
        	}
			break;

		case MSG_CHANGE_WORKMODE_PRE:
        	if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
        	{
        	    rtc_set.alarm_set.alarm_sw = 0;
    	        vm_write(VM_ALARM_SWITCH,&rtc_set.alarm_set.alarm_sw,VM_ALARM_SWITCH_LEN);
    	        //
        	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
				alarm_time_reset();
				rtc_tone_enable_flag = 0;
			    task_post_msg(NULL, 1, MSG_RTC_MODE);
        	}
			else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
			{
        	    rtc_set.alarm_set.alarm2_sw = 0;
    	        vm_write(VM_ALARM2_SWITCH,&rtc_set.alarm_set.alarm2_sw,VM_ALARM2_SWITCH_LEN);
    	        //
        	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
				alarm2_time_reset();
				rtc_tone_enable_flag = 0;
			    task_post_msg(NULL, 1, MSG_RTC_MODE);
			}
			break;
		case MSG_ALARM_STOP:
        	if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
        	{
        	    rtc_set.alarm_set.alarm_sw = 0;
    	        vm_write(VM_ALARM_SWITCH,&rtc_set.alarm_set.alarm_sw,VM_ALARM_SWITCH_LEN);
    	        //
        	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
				alarm_time_reset();
				rtc_tone_enable_flag = 0;
			    task_post_msg(NULL, 1, MSG_RTC_MODE);
        	}
			else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
        	{
        	    rtc_set.alarm_set.alarm2_sw = 0;
    	        vm_write(VM_ALARM2_SWITCH,&rtc_set.alarm_set.alarm2_sw,VM_ALARM2_SWITCH_LEN);
    	        //
        	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
				alarm2_time_reset();
				rtc_tone_enable_flag = 0;
			    task_post_msg(NULL, 1, MSG_RTC_MODE);
        	}
        	else
        	{
    			//alarm_time_reset();
    			//alarm2_time_reset();
        	}
			break;
		case MSG_ALARM1_SET:
        	if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
        	{
        	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
				alarm_time_reset();
				rtc_tone_enable_flag = 0;
			    task_post_msg(NULL, 1, MSG_RTC_MODE);
        	}
        	else
        	{
    			//alarm_time_reset();
        	}
		    break;
		case MSG_ALARM2_SET:
			if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
        	{
        	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
				alarm2_time_reset();
				rtc_tone_enable_flag = 0;
			    task_post_msg(NULL, 1, MSG_RTC_MODE);
        	}
        	else
        	{
    			//alarm2_time_reset();
        	}
		    break;

		case MSG_PROMPT_VOL_MAXMIN:
		case MSG_PROMPT_VOL_MAXMIN_SHORT:
			if ((get_tone_status())&&(alarm_tone_flag == 0))
				break;
			//if (vol_maxmin_play_flag == 0)
			//	break;
			if (msg == MSG_PROMPT_VOL_MAXMIN_SHORT)
			{
			    tone_play(TONE_VOLMAXMIN, 0);
			    if (volume_temp == MAX_SYS_VOL_TEMP)
			        vol_maxmin_play_flag = MSG_VOL_UP_SHORT;
			    else //if (volume_temp == 0)
			        vol_maxmin_play_flag = MSG_VOL_DOWN_SHORT;
			}
			else
			{
			    tone_play(TONE_VOLMAXMIN,1);
			}
			break;

	    case MSG_VOL_UP_TWS:
	    case MSG_VOL_UP:
	    case MSG_VOL_UP_SHORT:
			//if ((get_tone_status())&&(alarm_tone_flag == 0))
			//	break;
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
			if ((get_tone_status())&&(alarm_tone_flag == 0))
				break;
		#if USE_16_LEVEL_VOLUME
			if (volume_temp < MAX_SYS_VOL_TEMP)
				volume_temp++;
			sound.vol.sys_vol_l = volume_table[volume_temp];
		#else
	        sound.vol.sys_vol_l++;
		#endif

	        //if (sound.vol.sys_vol_l > get_max_sys_vol(0)) {
	        //    sound.vol.sys_vol_l = get_max_sys_vol(0);
	        //}

	        log_printf("VOL+:%d\n", sound.vol.sys_vol_l);
	        sound.vol.sys_vol_r = sound.vol.sys_vol_l;
			//if (get_tone_status() == 0)
			{
	        	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
			}
#if (SYS_DEFAULT_VOL == 0)
	        vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
vol_up_end:
	        if (is_dac_mute()) {                //dac mute时调节音量解mute
	            dac_mute(0, 1);
	        }
	        mute_flag = 0;
	        volume_display();//UI_menu(MENU_MAIN_VOL, 0, 3);
	        break;

	    case MSG_VOL_DN_TWS:
	    case MSG_VOL_DOWN:
	    case MSG_VOL_DOWN_SHORT:
			//if ((get_tone_status())&&(alarm_tone_flag == 0))
			//	break;
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
			if ((get_tone_status())&&(alarm_tone_flag == 0))
				break;
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
			//if (get_tone_status() == 0)
			{
	        	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
			}
#if (SYS_DEFAULT_VOL == 0)
	        vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
vol_down_end:
	        if (is_dac_mute()) {
				if (sound.vol.sys_vol_l)
				{
	                dac_mute(0, 1);
				}
	        }
	        mute_flag = 0;
	        volume_display();//UI_menu(MENU_MAIN_VOL, 0, 3);
	        break;

        default:
            break;
        }
    }	
}

const TASK_APP task_alarm_info = {
    /* .name 		= TASK_APP_IDLE, */
    .skip_check = NULL,//task_idle_skip_check,//
    .init 		= task_alarm_init,
    .exit 		= task_alarm_exit,
    .task 		= task_alarm_deal,
    .key 		= &task_alarm_key,
};

#endif
