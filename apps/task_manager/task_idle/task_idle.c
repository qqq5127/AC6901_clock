#include "task_idle.h"
#include "task_idle_key.h"
#include "msg.h"
#include "task_manager.h"
#include "task_common.h"
#include "audio/dac_api.h"
#include "dac.h"
#include "power_manage_api.h"
#include "dev_manage.h"
#include "warning_tone.h"
#include "led.h"
#include "ui/ui_api.h"
#include "common.h"


extern void clear_wdt(void);
extern int vm_cache_submit(void);
extern void alarm_time_reset(void);
extern u32 idle_dev_detect_ignore(u32 msg);
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif

#define TASK_IDLE_DEBUG_ENABLE

#ifdef TASK_IDLE_DEBUG_ENABLE
#define task_idle_printf log_printf
#else
#define task_idle_printf(...)
#endif// TASK_IDLE_DEBUG_ENABLE

extern u32 os_time_get(void);
//
//static tbool task_idle_skip_check(void)
//{
//    task_idle_printf("task_idle_skip_check !!\n");
//    return false;//true;//
//}
//

static void *task_idle_init(void *priv)
{
    /* dac_channel_on(MUSIC_CHANNEL, 0); */
#if UI_ENABLE
	ui_mode_wait_flag = 0;
#endif
    led_mode_on();
    task_idle_printf("task_idle_init !!\n");
    if (priv == IDLE_POWER_OFF) {
		SET_UI_MAIN(MENU_POWER_OFF);
    	SET_UI_BUF(NULL, 0);
    	UI_DIS_MAIN();
	#if WARNING_POWER_OFF
        tone_play(TONE_POWER_OFF, 0);
	#else
	    task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
	#endif
    } else if (priv == IDLE_POWER_UP) {
        /*
        tone_play(TONE_POWER_ON, 0);
        do{
            task_common_msg_deal(NULL, NO_MSG);
        }while(get_tone_status());
        */
    }
#if WARNING_POWER_ON
	else if (priv == IDLE_POWER_UP_TONE)
	{
        tone_play(TONE_POWER_ON, 0);
	}
#endif
    return priv;
}

static void task_idle_exit(void **hdl)
{
    task_idle_printf("task_idle_exit !!\n");
	is_dcin_poweron = 2;
    task_clear_all_message();
#if UI_ENABLE
#if (USE_DISPLAY_WAIT == 0)
	ui_mode_wait_flag = 1;
#endif
    UI_CLEAR_MSG_BUF();
#endif
}

extern void delay(u32 d);
static void task_idle_deal(void *hdl)
{
    int error = MSG_NO_ERROR;
    int msg = NO_MSG;
    task_idle_printf("task_idle_deal !!\n");
    u32 i = 0;
    u8 flag = 0;

    /* log_printf("-----------------------------start------------------------------\n"); */
    /* tone_play(0); */
    /* delay(1000); */
    /* tone_play(1); */
    /* delay(100); */
    /* tone_play(2); */
    /* delay(100); */
    /* tone_play(3); */
    /* delay(100); */
    /* tone_play(4); */
    /* tone_play(5); */
    /* log_printf("-----------------------------end------------------------------\n"); */
	if (is_dcin_poweron == 0)	//正常按键开机
	{
	    while (1) {

	        error = task_get_msg(0, 1, &msg);
		#if WARNING_POWER_ON
			if (hdl == IDLE_POWER_UP_TONE)
			{
			#if 0
			    //task_common_msg_deal(hdl, NO_MSG);
			#else
				resource_manage_schedule();
				msg = idle_dev_detect_ignore(msg);
			#endif
			}
			else
		#endif
			{
		        if (task_common_msg_deal(hdl, msg) == false) {
		            return ;
		        }
			}
	        if (NO_MSG == msg) {
	            continue;
	    	}

	        task_idle_printf("idle msg = %x\n", msg);
	        switch (msg) {
	        case MSG_HALF_SECOND:
	            /* task_idle_printf("-I_H-"); */
			#if WARNING_POWER_ON
	            if ((os_time_get() > 100) /*&& (os_time_get() < 500)*/) {	///2~5s
			        if (dev_online_mount(sd0))
			        {
			            task_switch(TASK_ID_MUSIC, sd0);
			        }
			        else if (dev_online_mount(sd1))
					{
			            task_switch(TASK_ID_MUSIC, sd1);
			        }
			        else if (dev_online_mount(usb))
					{
			            task_switch(TASK_ID_MUSIC, usb);
			        }
					else
					{
	                	task_switch(TASK_ID_BT, NULL);
					}
	                return;
	            }
			#else
	            if ((os_time_get() > 100) /*&& (os_time_get() < 500)*/) {	///2~5s
	                //task_time_out, run default task
	                task_switch(TASK_ID_RTC, NULL);
	                return;
	            }
			#endif
		        UI_REFRESH(MENU_REFRESH);
	            break;
		    case MSG_ONE_SECOND:
		        vm_cache_submit();
		        UI_REFRESH(MENU_SEC_REFRESH);
				break;

	        ///test ------------------------
	        case SYS_EVENT_DEC_END:
	            /* task_idle_printf("\n---------------sbc notice SYS_EVENT_DEC_END\n"); */
	            /* sbc_notice_stop(sbc_hdl); */
	            /* sbc_notice_play(i); */
	            /* i++; */
	            break;

	        case SYS_EVENT_PLAY_SEL_END:
	            task_idle_printf("SYS_EVENT_PLAY_TONE_END\n");
			#if 0
	            if (hdl == IDLE_POWER_OFF) {
	                task_idle_printf("idle enter soft_poweroff\n");
					soft_power_ctl(PWR_OFF);
	            }
			#endif
			#if 0//WARNING_POWER_ON
				//else
				{
			        if (dev_online_mount(sd0))
			        {
			            task_switch(TASK_ID_MUSIC, sd0);
			        }
			        else if (dev_online_mount(sd1))
					{
			            task_switch(TASK_ID_MUSIC, sd1);
			        }
			        else if (dev_online_mount(usb))
					{
			            task_switch(TASK_ID_MUSIC, usb);
			        }
					else
					{
	                	task_switch(TASK_ID_BT, NULL);
					}
					return;
				}
			#endif
	            break;

	        case MSG_IDLE_POWER_OFF:
				if (os_time_get() > 200)
				{
		            task_idle_printf("idle power off\n");
				//	soft_power_ctl(PWR_OFF);
				}
	            break;

	        default:
	            break;
	        }
	    }
	}
	else if (is_dcin_poweron == 1)	//插入充电线开机
	{
	    while (1) {
	        error = task_get_msg(0, 1, &msg);
		#if 0
	        if (task_common_msg_deal(hdl, msg) == false) {
	            return ;
	        }
		#else
			resource_manage_schedule();
			msg = idle_dev_detect_ignore(msg);
		#endif
	        if (NO_MSG == msg) {
	            continue;
	    	}

	        task_idle_printf("idle msg = %x\n", msg);
	        switch (msg) {
	        case MSG_HALF_SECOND:
				if (dcin_status == false)
				{
					soft_power_ctl(PWR_OFF);
				}
		        UI_REFRESH(MENU_REFRESH);
	            break;
		    case MSG_ONE_SECOND:
		        vm_cache_submit();
		        UI_REFRESH(MENU_SEC_REFRESH);
				break;

	        case SYS_EVENT_DEC_END:
	            break;

	        case SYS_EVENT_PLAY_SEL_END:
	            task_idle_printf("SYS_EVENT_PLAY_TONE_END\n");
			#if 0
	            if (hdl == IDLE_POWER_OFF) {
	                task_idle_printf("idle enter soft_poweroff\n");
					AMP_MUTE();
					SET_UI_MAIN(MENU_NULL);
			    	SET_UI_BUF(NULL, 0);
			    	UI_DIS_MAIN();
	            }
			#endif
	            break;

	        case MSG_IDLE_POWER_OFF:
				if (os_time_get() > 200)
				{
		            task_idle_printf("idle power off\n");
					AMP_UNMUTE();
					SET_UI_MAIN(MENU_POWER_UP);
			    	SET_UI_BUF(NULL, 0);
			    	UI_DIS_MAIN();
					delay_n10ms(100);
			        if (dev_online_mount(sd0))
			        {
			            task_switch(TASK_ID_MUSIC, sd0);
			        }
			        else if (dev_online_mount(sd1))
					{
			            task_switch(TASK_ID_MUSIC, sd1);
			        }
			        else if (dev_online_mount(usb))
					{
			            task_switch(TASK_ID_MUSIC, usb);
			        }
					else
					{
	                	task_switch(TASK_ID_BT, NULL);
					}
					return;
				}
	            break;

		#if RTC_ALM_EN
		    case MSG_ALM_UP:
				AMP_UNMUTE();
				SET_UI_MAIN(MENU_POWER_UP);
		    	SET_UI_BUF(NULL, 0);
		    	UI_DIS_MAIN();
				delay_n10ms(100);
                task_switch(TASK_ID_RTC, NULL);
				return;
				break;
		#endif

	        default:
	            break;
	        }
	    }
	}
	else	//正常开机工作后关机
	{
		if (dcin_status == 0)	//没有插入充电线时关机
		{
		    while (1) {

		        error = task_get_msg(0, 1, &msg);
				{
					//if (hdl == IDLE_POWER_OFF)
					{
					#if 0
				        //task_common_msg_deal(hdl, NO_MSG);
					#else
						resource_manage_schedule();
					#endif
					}
				}
		        if (NO_MSG == msg) {
		            continue;
		    	}

		        task_idle_printf("idle msg = %x\n", msg);
		        switch (msg) {
		        case MSG_HALF_SECOND:
		            /* task_idle_printf("-I_H-"); */
			        UI_REFRESH(MENU_REFRESH);
		            break;
			    case MSG_ONE_SECOND:
			        vm_cache_submit();
			        UI_REFRESH(MENU_SEC_REFRESH);
					break;

		        ///test ------------------------
		        case SYS_EVENT_DEC_END:
		            /* task_idle_printf("\n---------------sbc notice SYS_EVENT_DEC_END\n"); */
		            /* sbc_notice_stop(sbc_hdl); */
		            /* sbc_notice_play(i); */
		            /* i++; */
		            break;

		        case SYS_EVENT_PLAY_SEL_END:
		            task_idle_printf("SYS_EVENT_PLAY_TONE_END\n");
		            if (hdl == IDLE_POWER_OFF) {
		                task_idle_printf("idle enter soft_poweroff\n");
						soft_power_ctl(PWR_OFF);
		            }
				#if 0//WARNING_POWER_ON
					else
					{
				        if (dev_online_mount(sd0))
				        {
				            task_switch(TASK_ID_MUSIC, sd0);
				        }
				        else if (dev_online_mount(sd1))
						{
				            task_switch(TASK_ID_MUSIC, sd1);
				        }
				        else if (dev_online_mount(usb))
						{
				            task_switch(TASK_ID_MUSIC, usb);
				        }
						else
						{
		                	task_switch(TASK_ID_BT, NULL);
						}
						return;
					}
				#endif
		            break;

		        case MSG_IDLE_POWER_OFF:
					if (os_time_get() > 200)
					{
			            task_idle_printf("idle power off\n");
						soft_power_ctl(PWR_OFF);
					}
		            break;

		        default:
		            break;
		        }
		    }
		}
		else	//插入充电线时关机
		{
		    while (1) {
		        error = task_get_msg(0, 1, &msg);
			#if 0
		        if (task_common_msg_deal(hdl, msg) == false) {
		            return ;
		        }
			#else
				resource_manage_schedule();
				msg = idle_dev_detect_ignore(msg);
			#endif
		        if (NO_MSG == msg) {
		            continue;
		    	}

		        task_idle_printf("idle msg = %x\n", msg);
		        switch (msg) {
		        case MSG_HALF_SECOND:
					if (dcin_status == false)
					{
						soft_power_ctl(PWR_OFF);
					}
			        UI_REFRESH(MENU_REFRESH);
		            break;
			    case MSG_ONE_SECOND:
			        vm_cache_submit();
			        UI_REFRESH(MENU_SEC_REFRESH);
					break;

		        case SYS_EVENT_DEC_END:
		            break;

		        case SYS_EVENT_PLAY_SEL_END:
		            task_idle_printf("SYS_EVENT_PLAY_TONE_END\n");
		            if (hdl == IDLE_POWER_OFF) {
		                task_idle_printf("idle enter soft_poweroff\n");
						AMP_MUTE();
						SET_UI_MAIN(MENU_NULL);
				    	SET_UI_BUF(NULL, 0);
				    	UI_DIS_MAIN();
		            }
		            break;

		        case MSG_IDLE_POWER_OFF:
					if (os_time_get() > 50)
					{
			            task_idle_printf("idle power off\n");
						AMP_UNMUTE();
						SET_UI_MAIN(MENU_POWER_UP);
				    	SET_UI_BUF(NULL, 0);
				    	UI_DIS_MAIN();
						delay_n10ms(100);
				        if (task_switch(TASK_ID_TYPE_LAST, NULL) == true)
						{
				            return;
				        }
						else
						{
					        if (dev_online_mount(sd0))
					        {
					            task_switch(TASK_ID_MUSIC, sd0);
					        }
					        else if (dev_online_mount(sd1))
							{
					            task_switch(TASK_ID_MUSIC, sd1);
					        }
					        else if (dev_online_mount(usb))
							{
					            task_switch(TASK_ID_MUSIC, usb);
					        }
							else
							{
			                	task_switch(TASK_ID_BT, NULL);
							}
							return;
						}
					}
		            break;

			#if RTC_ALM_EN
			    case MSG_ALM_UP:
					AMP_UNMUTE();
					SET_UI_MAIN(MENU_POWER_UP);
			    	SET_UI_BUF(NULL, 0);
			    	UI_DIS_MAIN();
					delay_n10ms(100);
	                task_switch(TASK_ID_RTC, NULL);
					return;
					break;
			#endif

		        default:
		            break;
		        }
		    }
		}
	}
}

const TASK_APP task_idle_info = {
    /* .name 		= TASK_APP_IDLE, */
    .skip_check = NULL,//task_idle_skip_check,//
    .init 		= task_idle_init,
    .exit 		= task_idle_exit,
    .task 		= task_idle_deal,
    .key 		= &task_idle_key,
};

