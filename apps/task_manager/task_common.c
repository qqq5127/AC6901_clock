#include "sdk_cfg.h"
#include "common/common.h"
#include "task_common.h"
#include "task_manager.h"
#include "uart.h"
#include "msg.h"
#include "bluetooth/avctp_user.h"
#include "warning_tone.h"
#include "audio/dac_api.h"
#include "audio/dac.h"
#include "audio/audio.h"
#include "audio/tone.h"
#include "audio/eq.h"
#include "audio/linein_api.h"
#include "wdt.h"
#include "led.h"
#include "dev_manage.h"
#include "timer.h"
#include "ui_api.h"
#include "flash_api.h"
#include "power.h"
#include "echo_api.h"
#include "fmtx_api.h"
#include "music_player.h"
#include "rtc_setting.h"
#include "updata.h"
#include "task_idle.h"
#include "bt_tws.h"
#include "task_fm.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".task_common_bss")
#pragma data_seg(	".task_common_data")
#pragma const_seg(	".task_common_const")
#pragma code_seg(	".task_common_code")
#endif

u32 powerup_dev_detect_ignore(u32 msg)
{
    if (os_time_get() < 200) {
        switch (msg) {
        case MSG_SD0_ONLINE:
            dev_online_mount(sd0);
            dev_power_off_spec_dev(sd0);
            return NO_MSG;

        case MSG_SD1_ONLINE:
            dev_online_mount(sd1);
            dev_power_off_spec_dev(sd1);
            return NO_MSG;

        case MSG_HUSB_ONLINE:
            dev_online_mount(usb);
            dev_power_off_spec_dev(usb);
            return NO_MSG;

        case MSG_PC_ONLINE:
            return NO_MSG;

        case MSG_AUX_ONLINE:
            return NO_MSG;
        }
    }

    return msg;
}

u32 idle_dev_detect_ignore(u32 msg)
{
    //if (os_time_get() < 200) {
        switch (msg) {
        case MSG_SD0_ONLINE:
            dev_online_mount(sd0);
            dev_power_off_spec_dev(sd0);
            return NO_MSG;

        case MSG_SD1_ONLINE:
            dev_online_mount(sd1);
            dev_power_off_spec_dev(sd1);
            return NO_MSG;

        case MSG_HUSB_ONLINE:
            dev_online_mount(usb);
            dev_power_off_spec_dev(usb);
            return NO_MSG;

        case MSG_PC_ONLINE:
            return NO_MSG;

        case MSG_AUX_ONLINE:
            return NO_MSG;
        }
    //}

    return msg;
}

static u32 input_number;	    ///<输入的数字的实际值
static u8 input_number_cnt;   //输入数字个数
static u8 input_number_time_cnt;   //输入数字超时计数
void input_number_deal(u32 num)
{
    if (num < 10) {
        input_number = input_number * 10 + num;
        input_number_cnt++;
        input_number_time_cnt = 0;
#if LED_1888_EN
        if ((input_number > 1999) || (input_number_cnt > 4)) {
#else
        if ((input_number > 9999) || (input_number_cnt > 4)) {
#endif
            input_number = num;
            input_number_cnt = 1;
        }
        UI_menu(MENU_INPUT_NUMBER, input_number | input_number_cnt << 24, 4);
        printf("input_number = %u   time_cnt = %d\n", input_number, input_number_time_cnt);
    } else {
        if (input_number_cnt) {
            if (input_number_time_cnt < 3) {
                input_number_time_cnt++;
                if (input_number_time_cnt == 3) {
                    task_post_msg(NULL, 1, MSG_INPUT_TIMEOUT);
                    UI_DIS_MAIN();
                }
            }
        }
    }
}

void input_number_n_deal(u32 num)
{
	//if (num == ADD_NUBER){
        input_number = input_number + num;
        //input_number_cnt++;
		if ((input_number > 0)&&(input_number < 10))
			input_number_cnt = 1;
		else if (input_number < 100)
			input_number_cnt = 2;
		else if (input_number < 1000)
			input_number_cnt = 3;
		else if (input_number < 10000)
			input_number_cnt = 4;
		else
			input_number_cnt = 5;
        input_number_time_cnt = 0;
#if LED_1888_EN
        if ((input_number > 1999) || (input_number_cnt > 4)) {
#else
        if ((input_number > 9999) || (input_number_cnt > 4)) {
#endif
            input_number = num;
            input_number_cnt = 1;
        }
        UI_menu(MENU_INPUT_NUMBER, input_number | input_number_cnt << 24, 4);
    //} else {
    //    if (input_number_cnt) {
    //        if (input_number_time_cnt < 3) {
    //            input_number_time_cnt++;
    //            if (input_number_time_cnt == 3) {
    //                task_post_msg(NULL, 1, MSG_INPUT_TIMEOUT);
    //                UI_DIS_MAIN();
    //            }
    //        }
    //    }
    //}
}

/*----------------------------------------------------------------------------*/
/**@brief  获取IR输入数字值
   @param  函数会将input_num通过该指针返回
   @return 输入数字个数
   @note   input_cnt = get_input_number(&input_num)
*/
/*----------------------------------------------------------------------------*/
u32 get_input_number(u32 *num)
{
    u32 ret = input_number_cnt;
    if (num) {
        *num = input_number;
    }
    input_number_cnt = 0;
    input_number = 0;
    return ret;
}
/*----------------------------------------------------------------------------*/
/**@brief  tbool task_common_msg_deal(void *hdl, u32 msg)
  @param
  @return
   @note
 */
/*----------------------------------------------------------------------------*/
extern void resource_manage_schedule();
extern u8 is_rtc_wakeup;
tbool task_common_msg_deal(void *hdl, u32 msg)
{
    tbool exit = true;
    static u8 power_off_doing = 0;
    static u8 power_off_cnt = 0;
    u8 tws_lienin_state = 0;
    u32 dev_status;
    s32 dev_status_temp;
    //u8 test_box_vol_up = 0x41;
    //u8 test_box_vol_down = 0x42;
    resource_manage_schedule();
#if POWERUP_DEV_IGNORE
    msg = powerup_dev_detect_ignore(msg);
#else
	#if RTC_CLK_EN
		if (is_rtc_wakeup == RTC_WAKEUP_VALUE)
		{
    		msg = powerup_dev_detect_ignore(msg);
		}
	#endif
#endif
#if USE_AD_TUNER_VOLUME
	if (get_tone_status() == 0)
	{
		if (flag_100ms)
		{
			flag_100ms = 0;
			if (volume_chang_enable)
			{
				if (volume_value < sound.vol.sys_vol_l)
				{
					if (sound.vol.sys_vol_l)
					{
						sound.vol.sys_vol_l--;
						sound.vol.sys_vol_r = sound.vol.sys_vol_l;
			        	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
					}
				}
				else if (volume_value > sound.vol.sys_vol_l)
				{
					if (sound.vol.sys_vol_l < get_max_sys_vol(0))
					{
						sound.vol.sys_vol_l++;
						sound.vol.sys_vol_r = sound.vol.sys_vol_l;
			        	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
					}
				}
				else
				{
					volume_chang_enable = 0;
				}
	            //user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);
	        //__volume_set_exit:
		        mute_flag = 0;
				aux_pp_flag = 0;
				fm_pp_flag = 0;
	            if (sound.vol.sys_vol_l > 0)
		        {
		        	//mute_flag = 0;
			        if (is_dac_mute())
					{
			            dac_mute(0, 1);
			        }
		            if (task_get_cur() == TASK_ID_LINEIN)
					{
						if (linein_channel_status())
						{
		                	linein_mute(0);
						}
						//aux_pp_flag = 0;
		            }
		            if (task_get_cur() == TASK_ID_FM)
					{
						if (fm_mode_var->fm_mute)
						{
		                	fm_module_mute(0);
						}
						//fm_pp_flag = 0;
		            }
		        }
			}
		}
	}
#endif
    if (msg == NO_MSG) {
        return exit;
    }


    switch (msg) {
    case MSG_ACTIVE_TASK:
        puts("resource_manage_schedule_ok\n");
        break;

    case MSG_CHANGE_WORKMODE:
        puts("---------------task_switch---------------\n");
        if (task_switch(TASK_ID_TYPE_NEXT, NULL) == true) {
            exit = false;
        }
        break;
    case MSG_SD0_ONLINE:
        puts("MSG_SD0_ONLINE\n");
        if (dev_online_mount(sd0)) {
            if (task_get_cur() == TASK_ID_MUSIC) {           //当前为音乐任务不切换,把该消息给music任务处理
                puts("MUSIC :MSG_SD0_ONLINE_SUCC\n");
                task_post_event(NULL, 1, EVENT_SD0_ON_SUCC);
                break;
            }

            if (task_switch(TASK_ID_MUSIC, sd0) == true) {
                exit = false;
            }
        }
        break;

    case MSG_SD0_OFFLINE:
        puts("MSG_SD0_OFFLINE\n");
        dev_offline_unmount(sd0);
        break;

	case MSG_MUSIC_SD:
		dev_status_temp = dev_get_online_status(sd0, &dev_status);
		printf("~~~dev_status=%d\n",dev_status);
		printf("~~~dev_status_temp=%d\n",dev_status_temp);
		if (dev_status)
		{
            if (task_get_cur() == TASK_ID_MUSIC)
			{
			    task_post_msg(NULL, 1, MSG_MUSIC_SD_ENTER);
                break;
            }
            if (task_switch(TASK_ID_MUSIC, sd0) == true)
			{
                exit = false;
            }
		}
		break;
    case MSG_SD1_ONLINE:
        puts("MSG_SD1_ONLINE\n");
        if (dev_online_mount(sd1)) {
            if (task_get_cur() == TASK_ID_MUSIC) {           //当前为音乐任务不切换,把该消息给music任务处理
                puts("MUSIC :MSG_SD1_ONLINE_SUCC\n");
                task_post_event(NULL, 1, EVENT_SD1_ON_SUCC);
                break;
            }

            if (task_switch(TASK_ID_MUSIC, sd1) == true) {
                exit = false;
            }
        }
        break;

    case MSG_SD1_OFFLINE:
        puts("MSG_SD1_OFFLINE\n");
        dev_offline_unmount(sd1);
        break;

	case MSG_MUSIC_USB:
		dev_status_temp = dev_get_online_status(usb, &dev_status);
		printf("!!!dev_status=%d\n",dev_status);
		printf("!!!dev_status_temp=%d\n",dev_status_temp);
		if (dev_status)
		{
            if (task_get_cur() == TASK_ID_MUSIC)
			{
			    task_post_msg(NULL, 1, MSG_MUSIC_USB_ENTER);
                break;
            }
            if (task_switch(TASK_ID_MUSIC, usb) == true)
			{
                exit = false;
            }
		}
		break;
    case MSG_HUSB_ONLINE:
        puts("MSG_HUSB_ONLINE\n");
        if (dev_online_mount(usb)) {
            if (task_get_cur() == TASK_ID_MUSIC) {           //当前为音乐任务不切换,把该消息给music任务处理
                puts("MUSIC :MSG_HUSB_ONLINE_SUCC\n");
                task_post_event(NULL, 1, EVENT_USB_ON_SUCC);
                break;
            }
            #if 1
            if (task_get_cur() == TASK_ID_RTC)
            {
                dev_power_off_spec_dev(usb);
            }
            else
            {
                if (task_switch(TASK_ID_MUSIC, usb) == true) {
                    puts("usb music task false\n");
                    exit = false;
                }
            }
            #endif
        }
        break;

    case MSG_HUSB_OFFLINE:
        puts("MSG_HUSB_OFFLINE\n");
        dev_offline_unmount(usb);
        break;

    case MSG_PC_ONLINE:
        puts("MSG_PC_ONLINE\n");
#if USB_PC_EN
        if (task_switch(TASK_ID_PC, NULL) == true) {
            exit = false;
        }
#endif
        break;
#if AUX_EN
    case MSG_AUX_ONLINE:
        puts("MSG_AUX_IN\n");
#if BT_TWS_LINEIN
        tws_lienin_state = bt_tws_linein_open();
        if (TWS_LINEIN_TASK_STATE == tws_lienin_state) {
            break;
        } else if (TWS_LINEIN_TASK_BT == tws_lienin_state) {
            task_post_msg(NULL, 1, MSG_BT_TONE_LINEIN);
        } else if (TWS_LINEIN_TASK_AUX == tws_lienin_state) {

        }
#else
		if (os_time_get() > 200)
		{
	        if (task_switch(TASK_ID_LINEIN, NULL) == true) {
	            exit = false;
	        }
		}
#endif
        break;

    case MSG_AUX_OFFLINE:
        puts("MSG_AUX_OUT\n");
#if BT_TWS_LINEIN
        bt_tws_linein_close();
#else
        if (task_get_cur() == TASK_ID_LINEIN) {
            if (task_switch(TASK_ID_TYPE_NEXT, NULL) == true) {
                exit = false;
            }
        }
#endif
        break;
#endif
	case MSG_BT_MODE:
        if (task_get_cur() != TASK_ID_BT)
		{
            if (task_switch(TASK_ID_BT, NULL) == true)
			{
                exit = false;
            }
        }
		break;
#if FM_RADIO_EN
	case MSG_FM_MODE:
        if (task_get_cur() != TASK_ID_FM)
		{
            if (task_switch(TASK_ID_FM, NULL) == true)
			{
                exit = false;
            }
        }
		break;
#endif
#if AUX_EN
	case MSG_AUX_MODE:
        if (task_get_cur() != TASK_ID_LINEIN)
		{
            if (task_switch(TASK_ID_LINEIN, NULL) == true)
			{
                exit = false;
            }
        }
		break;
#endif
	case MSG_RESET:
		break;

#if 0
    case MSG_VOL_UP_TWS:
    case MSG_VOL_UP:
#if FMTX_EN
        if (fmtx_get_state() == FREQ_SETTING) {
            fmtx_setfre(FREQ_NEXT, 0);
            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
            break;
        }
#endif
        sound.vol.sys_vol_l++;

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
            sin_tone_play(250);
        }

        log_printf("VOL+:%d\n", sound.vol.sys_vol_l);
        sound.vol.sys_vol_r = sound.vol.sys_vol_l;
        set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
        if (get_call_status() != BT_CALL_HANGUP) {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_UP, 0, NULL);
        } else {
            user_send_cmd_prepare(USER_CTRL_CMD_SYNC_VOL_INC, 0, NULL);
        }
        user_send_cmd_prepare(USER_CTRL_TEST_KEY, 1, &test_box_vol_up); //音量加
#if (SYS_DEFAULT_VOL == 0)
        vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
#if (BT_TWS_LINEIN==0)
        if (is_dac_mute()) {                //dac mute时调节音量解mute
            dac_mute(0, 1);
            if (task_get_cur() == TASK_ID_LINEIN) {
                linein_mute(0);
            }
        }
#endif
        UI_menu(MENU_MAIN_VOL, 0, 3);
        break;

    case MSG_VOL_DN_TWS:
    case MSG_VOL_DOWN:
#if FMTX_EN
        if (fmtx_get_state() == FREQ_SETTING) {
            fmtx_setfre(FREQ_PREV, 0);
            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
            break;
        }
#endif
        if (sound.vol.sys_vol_l) {
            sound.vol.sys_vol_l--;
        }
        log_printf("VOL-:%d\n", sound.vol.sys_vol_l);
        sound.vol.sys_vol_r = sound.vol.sys_vol_l;
        set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_OFF);
        if (get_call_status() != BT_CALL_HANGUP) {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_DOWN, 0, NULL);
        } else {
            user_send_cmd_prepare(USER_CTRL_CMD_SYNC_VOL_DEC, 0, NULL);
        }
        user_send_cmd_prepare(USER_CTRL_TEST_KEY, 1, &test_box_vol_down); //音量减
#if (SYS_DEFAULT_VOL == 0)
        vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
#if (BT_TWS_LINEIN==0)
        if (is_dac_mute()) {
            dac_mute(0, 1);
            if (task_get_cur() == TASK_ID_LINEIN) {
                linein_mute(0);
            }
        }
#endif
        UI_menu(MENU_MAIN_VOL, 0, 3);
        break;
#endif
    case MSG_BT_TWS_VOL_KEY_UP:
    case MSG_VOL_KEY_UP:
    case MSG_VOL_DOWN_HOLD_UP:
    case MSG_VOL_UP_HOLD_UP:
        puts("MSG_VOL_KEY_UP\n");
	#if (WARNING_VOL_MAX || WARNING_VOL_MIN)
        music_tone_stop();
	#endif
        break;
#if USE_ADD_N_FUN
	case MSG_ADD_N:
		msg = ADD_NUBER;
        input_number_n_deal(msg);
		break;
#endif
    case MSG_0:
    case MSG_1:
    case MSG_2:
    case MSG_3:
    case MSG_4:
    case MSG_5:
    case MSG_6:
    case MSG_7:
    case MSG_8:
    case MSG_9:
        input_number_deal(msg);
        break;
    case MSG_ONE_SECOND:
        vm_cache_submit();
		if ((rtc_set.rtc_set_mode == RTC_DISPLAY)
			||(rtc_set.rtc_set_mode == ALM_DISPLAY)
			||(rtc_set.rtc_set_mode == ALM2_DISPLAY))
    	{
            rtc_read_datetime(rtc_set.calendar_set.curr_rtc_time);
            if (rtc_set.calendar_set.curr_rtc_time->bSec > 10)
            {
                if (time_alm_flag)
                    time_alm_flag = 0;
                if (time_alm2_flag)
                    time_alm2_flag = 0;
            }
            rtc_week = rtc_week_check(rtc_set.calendar_set.curr_rtc_time);
	        printf("~~~rtc_week=%d\n",rtc_week);
            if (rtc_set.alarm_set.alarm_sw)
            {
    		    if (alarm_check())
        		{
        	        if ((alarm_up_mode_check())||(alm_times))
        	        {
            	        //alarm_restart_times();
                    	//if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status())
                    	{
        	            //    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
                        }
                        //else
                        {
                            if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
                            {
                    	        alarm2_time_reset();
            	                rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
            	            }
            	            
            	            if (rtc_set.alarm_set.alarm_flag == ALARM_OFF)
            	            {
            	                alm_time_cnt = 0;
            	                rtc_set.alarm_set.alarm_flag = ALARM_ON;
            	                
            		            if (alarm_ring_type == ALARM_RING_BELL)
                    		    {
                    		        if (task_get_cur() != TASK_ID_ALARM)
                            		{
                                        if (task_switch(TASK_ID_ALARM, NULL) == true)
                            			{
                            			    alarm_up_flag = 1;
                                            exit = false;
                                            return exit;
                                        }
                                    }
            						else
            						{
                    		            alarm_ring_volme_set();
            						}
                    		    }
            			        else if (alarm_ring_type == ALARM_RING_FM)
                    		    {
                    		        if (task_get_cur() != TASK_ID_FM)
                            		{
                                        if (task_switch(TASK_ID_FM, NULL) == true)
                            			{
                            			    alarm_up_flag = 1;
                                            exit = false;
                                            return exit;
                                        }
                                    }
            						else
            						{
                                        fm_mode_var->wFreChannel = alm_fm_station;
                                        set_fm_channel();
                    		            alarm_ring_volme_set();
            						}
                    		    }
            	            }
                		}
        	        }	        
    				else
    				{
				        alarm_time_reset_spc();
    	                //rtc_set.alarm_set.alarm_flag = ALARM_OFF;
    				}
        		}
    		}
            if (rtc_set.alarm_set.alarm2_sw)
            {
    		    if (alarm2_check())
        		{
        	        if ((alarm2_up_mode_check())||(alm2_times))
        	        {
            	        //alarm2_restart_times();
                    	//if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status())
                    	{
        	            //    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
                        }
                        //else
                        {
                            if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
                            {
                    	        alarm_time_reset();
            	                rtc_set.alarm_set.alarm_flag = ALARM_OFF;
            	            }
            	            
            	            if (rtc_set.alarm_set.alarm2_flag == ALARM_OFF)
            	            {
            	                alm_time_cnt = 0;
            	                rtc_set.alarm_set.alarm2_flag = ALARM_ON;
            	                
        		                if (alarm2_ring_type == ALARM_RING_BELL)
                    		    {
                    		        if (task_get_cur() != TASK_ID_ALARM)
                            		{
                                        if (task_switch(TASK_ID_ALARM, NULL) == true)
                            			{
                            			    alarm_up_flag = 1;
                                            exit = false;
                                            return exit;
                                        }
                                    }
            						else
            						{
                    		            alarm_ring_volme_set();
            						}
                    		    }
        			            else if (alarm2_ring_type == ALARM_RING_FM)
                    		    {
                    		        if (task_get_cur() != TASK_ID_FM)
                            		{
                                        if (task_switch(TASK_ID_FM, NULL) == true)
                            			{
                            			    alarm_up_flag = 1;
                                            exit = false;
                                            return exit;
                                        }
                                    }
            						else
            						{
                                        fm_mode_var->wFreChannel = alm2_fm_station;
                                        set_fm_channel();
                    		            alarm_ring_volme_set();
            						}
                    		    }
                		    }
            		    }
        	        }
    				else
    				{
				        alarm2_time_reset_spc();
    	                //rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
    				}
        		}
    		}
    	}
		if (rtc_display_cnt)
			rtc_display_cnt--;
		if (auto_power_off_type != AUTO_TIME_OFF)
		{
    		if (auto_power_off_cnt)
    		{
    		    auto_power_off_cnt--;
    		    if (auto_power_off_cnt == 0)
    		    {
    			    task_post_msg(NULL, 1, MSG_RTC_MODE);
    		    }
    		}
    		//else
    		{
    			//task_post_msg(NULL, 1, MSG_RTC_MODE);
    		}
		}
        if (dcin_status)
        {
            if ((pwm_duty == 0)&&(back_light_mode != BACK_LIGHT_0))
            {
                back_light_set_flag = 1;
                set_timer1_pwm(500,back_light_table[back_light_mode]);//BACKLIGHT_ENABLE();
                backlight_cnt = 11;
                back_light_set_flag = 0;
            }
        }
        else
        {
    		if (backlight_cnt)
    		{
    		    backlight_cnt--;
    		    if (backlight_cnt == 0)
    		    {
                    back_light_set_flag = 1;
                    //set_timer1_pwm(500,back_light_table[0]);//BACKLIGHT_DISABLE();
                    back_light_set_flag = 0;
    		    }
    		}
		}
        UI_REFRESH(MENU_SEC_REFRESH);
        break;
	case MSG_SLEEP_SET:
	    if ((UI_var.bCurMenu == MENU_AUTO_TIME)||(UI_var.bCurMenu == MENU_POWER_OFF_TIME)||(auto_power_off_cnt == 0))
	    {
    		auto_power_off_type++;
    		if (auto_power_off_type >= AUTO_TIME_END)
    			auto_power_off_type = AUTO_TIME_5;
    		auto_power_off_cnt = auto_power_off_table[auto_power_off_type];
            UI_menu(MENU_AUTO_TIME, 0, 5);
        }
        else
        {
            UI_menu(MENU_POWER_OFF_TIME, 0, 5);
        }
		break;
    case MSG_BACK_LIGHT_SET:
        back_light_mode++;
        if (back_light_mode > BACK_LIGHT_5)
            back_light_mode = BACK_LIGHT_0;
        back_light_set_flag = 1;
        set_timer1_pwm(500,back_light_table[back_light_mode]);//BACKLIGHT_ENABLE();
        backlight_cnt = 11;
        back_light_set_flag = 0;
        break;
#if 0
    case MSG_LOW_POWER_VOICE:
        log_printf("MSG_LOW_POWER_VOICE\n");
        tone_play(TONE_WARNING, 0);
        led_fre_set(C_RLED_LOWER_POWER_MODE);
        break;
#endif
    case MSG_LOW_POWER:
    case MSG_POWER_OFF_AUTO:
        log_printf("MSG_POWER_OFF_AUTO\n");
        if (task_get_cur() != TASK_ID_IDLE)
		{
	        if (task_switch(TASK_ID_IDLE, IDLE_POWER_OFF) == true) {
	            /* set_r_led_on_cnt(3); */
	            exit = false;
	        }
        }
        break;
#if 1
    case MSG_POWER_OFF:
        puts("MSG_POWER_OFF\n");
		if (os_time_get() > 200)
		{
        	if (task_get_cur() != TASK_ID_IDLE)
			{
		        if (task_switch(TASK_ID_IDLE, IDLE_POWER_OFF) == true) {
		            exit = false;
		        }
	        }
		}
        break;
#else
    case MSG_POWER_OFF:
        puts("MSG_POWER_OFF\n");
        set_going_to_pwr_off(GOINT_POWER_OFF_START_CNT);
        break;

    case MSG_POWER_OFF_HOLD:
        puts("MSG_POWER_OFF_HOLD\n");
        if (get_going_to_pwr_off() && power_off_doing == 0) {
            if (power_off_cnt >= GOINT_POWER_OFF_END_CNT) {
                power_off_doing = 1;
                set_led_scan(0);
            }
            power_off_cnt++;
        }
        if (power_off_doing) {
            power_off_doing++;
            if (power_off_doing > 2) {
                power_off_doing = 0;
                task_post_msg(NULL, 1, MSG_POWER_KEY_UP);
            }
        }
        break;
    case MSG_POWER_KEY_UP:
        puts("MSG_POWER_KEY_UP\n");
        if (power_off_cnt >= GOINT_POWER_OFF_END_CNT) {
            set_r_led_on_cnt(3);
            if (task_switch(TASK_ID_IDLE, IDLE_POWER_OFF) == true) {
                exit = false;
            }
            break;
        }
        set_led_scan(1);
        set_going_to_pwr_off(0);
        power_off_cnt = 0;
        power_off_doing = 0;
        break;
#endif
#if 0
    case MSG_MUTE:
        puts("MSG_MUTE\n");
        if (is_dac_mute()) {
            dac_mute(0, 1);
        } else {
            dac_mute(1, 1);
        }
        break;
#endif
    case MSG_EQ_MODE_TWS:
    case MSG_EQ_MODE:
        sound.eq_mode++;
        if (sound.eq_mode > EQ_MODE_MAX) {
            sound.eq_mode = 0;
        }
        printf("eq_mode:%d\n", sound.eq_mode);
        eq_mode_switch(sound.eq_mode);
        break;
    case MSG_BACK_TO_BT:
        if (task_switch(TASK_ID_BT, NULL) == true) {
            exit = false;
        }
        break;
    case MSG_LAST_WORKMOD:
        if (task_switch(TASK_ID_TYPE_LAST, NULL) == true) {
            exit = false;
        }
        else
        {
            task_post_msg(NULL, 1, MSG_BACK_TO_BT);
        }
        break;

#if FMTX_EN
    case MSG_FMTX_FREQ_SET:
        puts("MSG_FMTX_FREQ_SET\n");
        if (input_number != 0) {
            if ((input_number <= FREQ_MAX) && (input_number >= FREQ_MIN)) {
                fmtx_setfre(FREQ_SEL, input_number);
            }
            input_number = 0;
            input_number_cnt = 0;
            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
            break;
        }
        if (fmtx_get_state() == FREQ_SETTING) {
            fmtx_set_state(FREQ_TRANSMIT);
            UI_DIS_MAIN();
        } else {
            fmtx_set_state(FREQ_SETTING);
            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
        }
        break;
    case MSG_FMTX_FREQ_UP:
        puts("MSG_FMTX_FREQ_UP\n");
        if (input_number != 0) {
            if ((input_number <= FREQ_MAX) && (input_number >= FREQ_MIN)) {
                fmtx_setfre(FREQ_SEL, input_number);
            }
            input_number = 0;
            input_number_cnt = 0;
            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
            break;
        }
        fmtx_setfre(FREQ_NEXT, 0);
        UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
        break;
    case MSG_FMTX_FREQ_DOWN:
        puts("MSG_FMTX_FREQ_DOWN\n");
        if (input_number != 0) {
            if ((input_number <= FREQ_MAX) && (input_number >= FREQ_MIN)) {
                fmtx_setfre(FREQ_SEL, input_number);
            }
            input_number = 0;
            input_number_cnt = 0;
            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
            break;
        }
        fmtx_setfre(FREQ_PREV, 0);
        UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
        break;
    case MSG_FMTX_FREQ_DOWN_CHECK:
        if (fmtx_get_state() == FREQ_SETTING) {
            fmtx_setfre(FREQ_PREV, 0);
            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
        }
        break;
    case MSG_FMTX_FREQ_UP_CHECK:
        if (fmtx_get_state() == FREQ_SETTING) {
            fmtx_setfre(FREQ_NEXT, 0);
            UI_menu(MENU_FM_DISP_FRE, 0, UI_FREQ_RETURN);
        }
        break;
#endif
#if 0//RTC_ALM_EN
    case MSG_ALM_UP:
        if (task_get_cur() != TASK_ID_RTC) {
            if (BT_STATUS_TAKEING_PHONE != get_bt_connect_status()) {
                if (task_switch(TASK_ID_RTC, NULL) == true) {
                    exit = false;
                }
            } else {
                alarm_time_reset();
            }
        }
        break;
#endif
    case MSG_RTC_MODE:
        if (task_get_cur() != TASK_ID_RTC) {
            if (task_switch(TASK_ID_RTC, NULL) == true) {
                exit = false;
            }
        }
        break;
    case MSG_UART_UPDATA:
        uart_updata_receive_cmd();
	#if ECHO_EN
		echo_stop();
	#endif
        updata_mode_api(UART_UPDATA);
        break;
    default:
#if ECHO_EN
        echo_msg_deal(hdl, msg);
#endif
        break;
    }
    return exit;
}

