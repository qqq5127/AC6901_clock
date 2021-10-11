#include "task_linein.h"
#include "task_linein_key.h"
#include "msg.h"
#include "task_manager.h"
#include "task_common.h"
#include "audio/dac_api.h"
#include "audio/audio.h"
#include "dac.h"
#include "power_manage_api.h"
#include "dev_manage.h"
#include "common/common.h"
#include "warning_tone.h"
#include "audio/linein_api.h"
#include "audio/ladc.h"
#include "linein_ui.h"
#include "music_player.h"
#include "led.h"
#include "task_echo_key.h"
#include "rec_api.h"
#include "fat_io.h"

#include "echo_api.h"
#include "tone.h"
#include "msg.h"
#include "flash_api.h"
#include "rtc_setting.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif

#define TASK_LINEIN_DEBUG_ENABLE

#ifdef TASK_LINEIN_DEBUG_ENABLE
#define task_linein_printf log_printf
#else
#define task_linein_printf(...)
#endif// TASK_LINEIN_DEBUG_ENABLE

AUX_VAR g_aux_var = {0}; ///<line in检测变量
RECORD_OP_API *linein_rec_api = NULL;
u8 aux_pp_flag;

/*----------------------------------------------------------------------------*/
/**@brief  供AD_KEY 复用AUX检测时设置当前aux状态和获取aux状态*/
/*----------------------------------------------------------------------------*/
u8   get_aux_sta(void)
{
    return g_aux_var.bDevOnline;
}

void set_aux_sta(u8 sta)
{
    g_aux_var.cur_status = sta;
}

/*----------------------------------------------------------------------------*/
/**@brief  LINE IN 在线检测实体函数
   @param  cnt：检测滤波次数
   @return 在线情况
   @note   AUX_STATUS linein_check(u8 cnt)
*/
/*----------------------------------------------------------------------------*/
AUX_STATUS linein_check(u8 cnt)
{
#if AUX_DET_MULTI_AD_KEY
    g_aux_var.cur_status = g_aux_var.cur_status;
#else
    g_aux_var.cur_status = AUX_IN_CHECK; //获取当前AUX状态
#endif
    if (g_aux_var.cur_status != g_aux_var.pre_status) {
        g_aux_var.pre_status = g_aux_var.cur_status;
        g_aux_var.status_cnt = 0;
    } else {
        g_aux_var.status_cnt++;
    }

    if (g_aux_var.status_cnt < cnt) { //消抖
        return NULL_AUX;
    }
    g_aux_var.status_cnt = 0;

    ///检测到AUX插入
    if ((AUX_OFF == g_aux_var.bDevOnline) && (!g_aux_var.pre_status)) {
        g_aux_var.bDevOnline = AUX_ON;
        return AUX_ON;
    } else if ((AUX_ON == g_aux_var.bDevOnline) && g_aux_var.pre_status) {
        g_aux_var.bDevOnline = AUX_OFF;
        return AUX_OFF;
    }

    return NULL_AUX;
}

/*----------------------------------------------------------------------------*/
/**@brief  LINE IN 在线检测调度函数
   @param  无
   @return 在线情况
   @note   s32 aux_detect(void)
*/
/*----------------------------------------------------------------------------*/
void aux_detect(void)
{
    AUX_STATUS res;

    AUX_DIR_SET;
    AUX_PU_SET;

    res = linein_check(20); //aux在线检测，去抖计数为50
    if (AUX_ON == res) {
        task_post_msg(NULL, 1, MSG_AUX_ONLINE);
    } else if (AUX_OFF == res) {
        task_post_msg(NULL, 1, MSG_AUX_OFFLINE);
    }
}

#if AUX_DETECT_EN
LOOP_DETECT_REGISTER(aux_detect_loop) = {
    .time = 5,
    .fun  = aux_detect,
};
#endif
void aux_detect_init(void)
{
	g_aux_var.cur_status = AUX_ON;
}

static tbool task_linein_skip_check(void **priv)
{
    task_linein_printf("task_linein_skip_check !!\n");
#if AUX_DETECT_EN
    return    g_aux_var.bDevOnline;
#else
    return    true;
#endif
}


//u8 linein_mutex_init_flag=0;
static void *task_linein_init(void *priv)
{
    task_linein_printf("task_linein_init !!\n");
    sound_automute_set(AUTO_MUTE_CFG, 4, 3600, 200);
#if AUX_REC_EN
    fat_init();
#endif
#if USE_TWO_VOLUME_TABLE
    sound.vol.sys_vol_l = volume_table_2[volume_temp];
    sound.vol.sys_vol_r = sound.vol.sys_vol_l;
	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
#endif
#if WARNING_TASK_AUX
    tone_play(TONE_LINEIN_MODE, 0);
#else
	task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
#endif
    led_aux_play();//led_fre_set(C_BLED_SLOW_MODE);
    ui_open_aux(&linein_rec_api, sizeof(RECORD_OP_API **));
	//linein_mutex_init_flag = 0;

    return NULL;
}

static void linein_channel_init(void)
{
	dac_mute(1, 1);
    if (LINEIN_CHANNEL == DAC_AMUX0) {
        /* PB4(L)PB5(R) */
        JL_PORTB->DIR |= BIT(4) | BIT(5);
    } else if (LINEIN_CHANNEL == DAC_AMUX1) {
        /* PA3(L)PA4(R) */
        JL_PORTA->DIR |= BIT(3) | BIT(4);
    } else if (LINEIN_CHANNEL == DAC_AMUX2) {
        /* PB6(L)PB3(R) */
        JL_PORTB->DIR |= BIT(6) | BIT(3);
    }
#if AUX_AD_ENABLE
    /*
     *如果是dac其中一个通道做aux输入，由于dac通道自身阻抗
     *导致输入略小于普通aux通道。这中情况下，可以讲作为aux
     *输入的通道增益减小。比如DAC_Left作为aux输入：
     *set_sys_vol(1, sound.vol.sys_vol_r, FADE_ON);
     */
    linein_channel_open(LINEIN_CHANNEL, 0);
    ladc_ch_open(LADC_LINLR_CHANNEL, SR44100);
#else
    dac_channel_off(MUSIC_CHANNEL, FADE_ON);
    delay_2ms(20);
    dac_channel_on(LINEIN_CHANNEL, FADE_ON);
#endif
    linein_mute(1);
    delay_2ms(50);
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    //delay_2ms(50);
	//dac_mute(0, 1);
}

static void task_linein_exit(void **hdl)
{
    task_clear_all_message();
    mutex_resource_release("linein");
    task_common_msg_deal(NULL, NO_MSG);
#if AUX_AD_ENABLE
    ladc_ch_close(LADC_LINLR_CHANNEL);
#endif
    dac_channel_off(LINEIN_CHANNEL, FADE_ON);
    dac_channel_on(MUSIC_CHANNEL, FADE_ON);
    if (is_dac_mute()) {
        dac_mute(0, 0);
    }
    ui_close_aux();

#if AUX_REC_EN
    rec_exit(&linein_rec_api);
    mutex_resource_release("record_play");
    fat_del();
#endif
#if USE_TWO_VOLUME_TABLE
    sound.vol.sys_vol_l = volume_table[volume_temp];
    sound.vol.sys_vol_r = sound.vol.sys_vol_l;
	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
#endif
    sound_automute_set(AUTO_MUTE_CFG, 4, 1200, 200);
    task_linein_printf("task_linein_exit !!\n");
}

void linein_mutex_init(void *priv)
{
//#if (WARNING_VOL_MAX || WARNING_VOL_MIN)
//	if (vol_maxmin_play_flag == 0)
//#endif
	{
		//linein_mutex_init_flag = 1;
        linein_channel_init();
        delay_2ms(50);
		if (sound.vol.sys_vol_l)
		{
            linein_mute(0);
		}
        delay_2ms(50);
    	dac_mute(0, 1);
	}
}

void linein_mutex_stop(void *priv)
{
	//linein_mutex_init_flag = 0;
#if AUX_REC_EN
    rec_exit(&linein_rec_api);
#endif
    dac_channel_off(LINEIN_CHANNEL, FADE_OFF);
#if AUX_AD_ENABLE
    ladc_ch_close(LADC_LINLR_CHANNEL);
#endif
}

static void task_linein_deal(void *hdl)
{
    int error = MSG_NO_ERROR;
    int msg = NO_MSG;
    u8 linein_start = 0;

	aux_pp_flag = 0;
    task_linein_printf("task_linein_deal !!\n");

    while (1) {

        error = task_get_msg(0, 1, &msg);
        if (task_common_msg_deal(hdl, msg) == false) {
            music_tone_stop();
            task_common_msg_deal(NULL, NO_MSG);
            return ;
        }

#if AUX_REC_EN
        if (msg == MSG_REC_START) {
            if (FALSE == is_cur_resource("linein")) { //当前资源不属lienin所有，不允许录音，防止资源冲突
                continue;
            }
        }
        rec_msg_deal_api(&linein_rec_api, msg); //record 流程
#endif


        if (NO_MSG == msg) {
            continue;
        }

        //task_linein_printf("linein msg = %x\n", msg);
        switch (msg) {
        case MSG_HALF_SECOND:
            //task_linein_printf("-H-");
			#if DAC_AUTO_MUTE_EN
				if ((sound.vol.sys_vol_r == 0)||(is_dac_mute())
				#if AUX_AD_ENABLE
					||(is_auto_mute())
				#endif
					)
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
				}
			#endif
			#if (AUX_AUTO_STANDBY_EN && AUTO_SHUT_DOWN_TIME)
				if ((sound.vol.sys_vol_r == 0)||(is_dac_mute())||(is_auto_mute()))
				{
					auto_sleep_time_cnt++;
					if (auto_sleep_time_cnt > AUTO_SHUT_DOWN_TIME)
					{
						task_post_msg(NULL,1,MSG_POWER_OFF);
					}
				}
				else
				{
					auto_sleep_time_cnt = 0;
				}
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
            UI_REFRESH(MENU_REFRESH);
            break;

        case SYS_EVENT_PLAY_SEL_END:
            if (linein_start == 0) {
                linein_start = 1;
                mutex_resource_apply("linein", 3, linein_mutex_init, linein_mutex_stop, 0);
            }
			else
			{
			#if USE_MUTE_PALYTONE_ENABLE
				if (mute_flag)
				{
					//AMP_MUTE();
	                dac_mute(1, 1);
	                linein_mute(1);
	                led_aux_pause();
				}
			#endif
			}

            //ui_open_aux(&linein_rec_api, sizeof(RECORD_OP_API **));
            task_linein_printf("Linein SYS_EVENT_DEC_END\n");
            break;

		case MSG_MUTE:
        case MSG_AUX_MUTE:
            task_linein_printf("MSG_AUX_MUTE\n");
            if (mute_flag) {//(is_dac_mute()) {
				mute_flag = 0;
				aux_pp_flag = 0;
                linein_mute(0);
                dac_mute(0, 1);
				//AMP_UNMUTE();
                led_aux_play();//led_fre_set(C_BLED_SLOW_MODE);
            } else {
				mute_flag = 1;
				aux_pp_flag = 1;
				//AMP_MUTE();
                dac_mute(1, 1);
                linein_mute(1);
                led_aux_pause();//led_fre_set(C_BLED_ON_MODE);
            }
            break;

        case MSG_INPUT_NUMBER_END:
        case MSG_INPUT_TIMEOUT:
            get_input_number(NULL);
            break;

        case MSG_PROMPT_PLAY:
            tone_play(TONE_LOW_POWER,0);
            break;
		case MSG_PROMPT_VOL_MAXMIN:
		case MSG_PROMPT_VOL_MAXMIN_SHORT:
			if (get_tone_status())
				break;
			//if (vol_maxmin_play_flag == 0)
			//	break;
			if (msg == MSG_PROMPT_VOL_MAXMIN_SHORT)
			{
			    tone_play(TONE_VOLMAXMIN, 0);
			}
			else
			{
			    tone_play(TONE_VOLMAXMIN,1);
			}
			break;

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
			#if USE_TWO_VOLUME_TABLE
			sound.vol.sys_vol_l = volume_table_2[volume_temp];
			#else
			sound.vol.sys_vol_l = volume_table[volume_temp];
			#endif
		#else
	        sound.vol.sys_vol_l++;
		#endif
#if 0
	        if (sound.vol.sys_vol_l >= get_max_sys_vol(0)) {
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
	        //if (get_call_status() != BT_CALL_HANGUP) {
	        //    user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_UP, 0, NULL);
	        //} else {
	        //    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);
	        //}
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
	                //linein_mute(0);
	            //}
	        }
			if (linein_mute_status())
			{
	        	linein_mute(0);
			}
            led_aux_play();
			aux_pp_flag = 0;
            mute_flag = 0;
			//AMP_UNMUTE();
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
			#if USE_TWO_VOLUME_TABLE
			sound.vol.sys_vol_l = volume_table_2[volume_temp];
			#else
			sound.vol.sys_vol_l = volume_table[volume_temp];
			#endif
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
	        //if (get_call_status() != BT_CALL_HANGUP) {
	        //    user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_DOWN, 0, NULL);
	        //} else {
	        //    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);
	        //}
#if (SYS_DEFAULT_VOL == 0)
	        vm_cache_write(VM_SYS_VOL, &sound.vol.sys_vol_l, 2);
#endif
vol_down_end:
#if (BT_TWS_LINEIN==0)
	        if (is_dac_mute()) {
				if (sound.vol.sys_vol_l > 0)
	            	dac_mute(0, 1);
	            //if (task_get_cur() == TASK_ID_LINEIN) {
	                //linein_mute(0);
	            //}
	        }
			if (linein_mute_status())
			{
    			if (sound.vol.sys_vol_l > 0)
    			{
    	            linein_mute(0);
    			}
			}
			if (sound.vol.sys_vol_l == 0)
			{
			    linein_mute(1);
			}
            led_aux_play();
			aux_pp_flag = 0;
            mute_flag = 0;
			//if (sound.vol.sys_vol_l > 0)
			//    AMP_UNMUTE();
#endif
	        volume_display();//UI_menu(MENU_MAIN_VOL, 0, 3);
	        break;
        //case MSG_VOL_DOWN_HOLD_UP:
        //case MSG_VOL_UP_HOLD_UP:
		//	if (linein_mutex_init_flag == 0)
		//	{
		//	    if (get_tone_status() == 0)
		//		{
		//		    linein_mutex_init(NULL);
		//		}
		//	}
		//	break;
        case MSG_DIMMER:
        	//if (rtc_set.rtc_set_mode == RTC_DISPLAY)
        	{
        	#if 0
        	    led7_display_level++;
				if (led7_display_level > LED7_LEVEL_OFF)
					led7_display_level = LED7_LEVEL_MAX;
                set1628Display();
		    #else
			    task_post_msg(NULL, 1, MSG_SLEEP_SET);
		    #endif
        	}
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
            break;
        }
    }
}

const TASK_APP task_linein_info = {
    /* .name 		= TASK_APP_linein, */
    .skip_check = task_linein_skip_check,
    .init 		= task_linein_init,
    .exit 		= task_linein_exit,
    .task 		= task_linein_deal,
    .key 		= &task_linein_key,
};

