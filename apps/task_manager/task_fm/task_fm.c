#include "sdk_cfg.h"
#include "task_fm.h"
#include "task_fm_key.h"
#include "fm_api.h"
#include "common/msg.h"
#include "audio/dac_api.h"
#include "key_drv/key.h"
#include "clock_api.h"
#include "common/common.h"
#include "task_common.h"
#include "fm_inside.h"
#include "clock.h"
#include "audio/audio.h"
#include "os/embedded/embedded_os.h"
#include "warning_tone.h"
#include "music_player.h"
#include "fm_ui.h"
#include "ui_api.h"
#include "fm/fm_inside_api.h"
#include "led.h"
#include "task_echo_key.h"
#include "rec_api.h"
#include "fat_io.h"
#include "tone.h"
#include "msg.h"
#include "flash_api.h"


#include "echo_api.h"
#include "audio/dac.h"
#include "rtc_setting.h"

#if FM_RADIO_EN

/* RECORD_OP_API * rec_fm_api = NULL; */
void set_fm_channel(void);
static void del_fm_channel(u8 chl_num);
static void save_fm_channel(u16 save_freq);

FM_INFO *fm_info = NULL;
FM_MODE_VAR *fm_mode_var = NULL;
RECORD_OP_API *fm_rec_api = NULL;

u8 fm_pp_flag;
#if (USE_FM_SAVE_FRE_POINT == 0)
u8 preset_station_flag=0;
u8 preset_station_num=0;
u8 preset_station_cnt=0;
u8 fm_fre_cur_temp;
u8 fm_station_cur_temp;
u8 fm_station_all_temp;
u8 fm_fre_buf[MAX_STATION];
#endif
////---------------test_code------------------//
extern bool fm_test_set_freq(u16 freq);
/*----------------------------------------------------------------------------*/
/**@brief  FM频点测试，直接吧数组里面频点循环设置如FM中
   @param  NULL
   @return NULL
   @note   用于直接测试听台效果
*/
/*----------------------------------------------------------------------------*/
const u16 fm_test_freq_arr[] = {
    875,
    878,
    888,
    898,
    905,
    915,
    918,
    928,
    939,
    951,
    967,
    971,
    980,
    991,
    1007,
    1012,
    1030,
    1038,
    1043,
    1049,
    1071
};

/*----------------------------------------------------------------------------*/
/**@brief  FM频点报号
   @param  freq:875-1080
   @return NULL
   @note   static bool fm_msg_filter(int msg)
   @note   FM频点报号
*/
/*----------------------------------------------------------------------------*/
static void fm_frq_play(u16 freq)
{
    /* void* a, *b, *c, *d; */

    /* if(play_sel_busy()) */
    /* { */
    /* play_sel_stop(); */
    /* fm_prompt_break = 1; */
    /* } */
    /* fm_module_mute(1); */
    /* clear_dac_buf(1); */

    /* a = tone_number_get_name(freq%10000/1000); */
    /* b = tone_number_get_name(freq%1000/100); */
    /* c = tone_number_get_name(freq%100/10); */
    /* d = tone_number_get_name(freq%10); */

    /* printf("a:%s	b:%s	c:%s	d:%s\n",a,b,c,d); */

    /* if(freq%10000/1000) */
    /* { */
    /* tone_play_by_name(FM_TASK_NAME,4,a, b, c, d); */
    /* } */
    /* else */
    /* { */
    /* tone_play_by_name(FM_TASK_NAME,3, b, c, d); */
    /* } */
}

/*----------------------------------------------------------------------------*/
/**@brief  FM频点设置
   @param  freq:875-1080
   @return NULL
   @note   static bool fm_msg_filter(int msg)
   @note   FM频点设置，循环从表中获取频点
*/
/*----------------------------------------------------------------------------*/
static void fm_test_freq_fun(void)
{
    static u8 i = 0;

    u8 max = sizeof(fm_test_freq_arr) / sizeof(fm_test_freq_arr[0]);

    if (i >= max) {
        i = 0;
    }

    fm_frq_play(fm_test_freq_arr[i]);
    fm_test_set_freq(fm_test_freq_arr[i]);
    i++;
}


/*----------------------------------------------------------------------------*/
/**@brief  FM录音过程消息过滤函数
   @param  msg；接收到的消息
   @return 1：不需要过滤，0：过滤
   @note   static bool fm_msg_filter(int msg)
   @note   FM不同工作状态时，部分消息不能进行处理，如还没初始化不能搜台等操作
*/
/*----------------------------------------------------------------------------*/
#if FM_REC_EN
static bool fm_rec_msg_filter(RECORD_OP_API *rec_fm_api, int msg)
{
    if (fm_rec_api) {
        if (rec_get_enc_sta(fm_rec_api) != ENC_STOP) { //正在录音，不响应以下消息
            if ((msg == MSG_FM_SCAN_ALL_INIT)
                || (msg == MSG_FM_SCAN_ALL)
                || (msg == MSG_FM_SCAN_ALL_UP)
                || (msg == MSG_FM_SCAN_ALL_DOWN)) {
                return 0;
            }
        }
    }
    return 1;
}
#endif

/*----------------------------------------------------------------------------*/
/**@brief  FM消息过滤函数
   @param  msg；接收到的消息
   @return 1：不需要过滤，0：过滤
   @note   static bool fm_msg_filter(int msg)
   @note   FM不同工作状态时，部分消息不能进行处理，如还没初始化不能搜台等操作
*/
/*----------------------------------------------------------------------------*/
static bool fm_msg_filter(int msg)
{
    ///FM 任何情况，都必须响应SYS_EVENT_DEL_TASK消息

    if (fm_mode_var->scan_mode == FM_UNACTIVE) { ///FM 还没初始化完成(正在播放提示音)，不响应其他消息
        if (msg == SYS_EVENT_PLAY_SEL_END) {
            return 1;
        } else {
            return 0;
        }
    } else if (fm_mode_var->scan_mode >= FM_SCAN_BUSY) { ///FM正在搜台，只响应部分按键
        if ((msg == SYS_EVENT_PLAY_SEL_END) || \
            (msg == MSG_FM_SCAN_ALL) || \
            (msg == MSG_FM_SCAN_ALL_SEMI) || \
            (msg == MSG_FM_SCAN_ALL_INIT) || \
            (msg == MSG_FM_PP)) {
            return 1;
        } else {
            return 0;
        }
    } else {        ///一般情况下，不进行过滤
        return 1;
    }
}


static u8 fm_radio_start(void)
{
    u8 ret = FALSE;
    fm_puts("fm_radio_start!!\n");
    ret = fm_radio_init();
    if (ret) {
        mutex_resource_apply("fm", 3, fm_mutex_init, fm_mutex_stop, 0);
    } else {
        task_post_msg(NULL, 1, MSG_CHANGE_WORKMODE);
        if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
    	{
    	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
    	}
        else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
    	{
    	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
    	}
    }
    return ret;
}

/*----------------------------------------------------------------------------*/
/**@brief  FM模式主线程初始化
   @param  priv：NULL
   @return NULL
   @note   static void fm_radio_task_init(void *priv)
*/
/*----------------------------------------------------------------------------*/
static void *task_fm_init(void *priv)
{
    u32 res;
    fm_printf("task_fm_init\n");
	//BOOST_DISABEL();
	//AMP_AB();
	mode_type = TASK_ID_FM;

#if FM_REC_EN
    fat_init();
#endif
#if (USE_FM_SAVE_FRE_POINT == 0)
	preset_station_flag = 0;
#endif
#if USE_TWO_VOLUME_TABLE
    sound.vol.sys_vol_l = volume_table_2[volume_temp];
    sound.vol.sys_vol_r = sound.vol.sys_vol_l;
	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
#endif
    fm_arg_open();
    res =  fm_mode_read_id(); //fm_radio_init();
    if (res) {
        if ((rtc_set.alarm_set.alarm_flag == ALARM_ON)||(rtc_set.alarm_set.alarm2_flag == ALARM_ON))
        {
            alarm_ring_volme_set();
            set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
	        task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
        }
		else
		{
	#if WARNING_TASK_FM
        tone_play(TONE_RADIO_MODE, 0);
	#else
	    task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
	#endif
		}
    } else {
        fm_puts("init_fm_err\n");
        task_post_msg(NULL, 1, MSG_CHANGE_WORKMODE);
        if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
    	{
    	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
    	}
        else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
    	{
    	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
    	}
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
/**@brief  FM模式退出
   @param  NULL
   @return NULL
   @note   static void fm_radio_task_exit(void)
*/
/*----------------------------------------------------------------------------*/
static void task_fm_exit(void **priv)
{
    /*先关闭FM模块线程，再关闭FM变采样线程*/
    fm_printf("task_fm_exit !!\n");
    mutex_resource_release("fm");

#if SWITCH_PWR_CONFIG
    extern void fm_ldo_level(u8 level);
    if (get_pwr_config_flag()) {
        fm_ldo_level(FM_LDO_REDUCE_LEVEL);
    }
#endif
#if (FM_INSIDE == 0)
    task_common_msg_deal(NULL, NO_MSG);
#if FM_AD_ENABLE
    ladc_ch_close(LADC_LINLR_CHANNEL);
#endif
    dac_channel_off(FM_IIC_CHANNEL, FADE_ON);
    dac_channel_on(MUSIC_CHANNEL, FADE_ON);
#endif
    fm_radio_powerdown();
    task_clear_all_message();
    fm_arg_close();
    ui_close_fm();
#if FM_REC_EN
    rec_exit(&fm_rec_api);
    mutex_resource_release("record_play");
    fat_del();
#endif
	BOOST_ENABEL();
	AMP_D();
#if USE_TWO_VOLUME_TABLE
    sound.vol.sys_vol_l = volume_table[volume_temp];
    sound.vol.sys_vol_r = sound.vol.sys_vol_l;
	set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
#endif
	mode_type = 0;
}

void set_fm_channel(void)
{
    if (fm_mode_var->wTotalChannel == 0) {
        fm_mode_var->wFreChannel = 0;
        fm_mode_var->wFreq = 875;
        fm_module_set_fre(FM_CUR_FRE);
        fm_module_mute(0);
        return;
    }
#if USE_FM_SAVE_FRE_POINT
    if ((fm_mode_var->wFreChannel == 0) || (fm_mode_var->wFreChannel == 0xff)) {
        fm_mode_var->wFreChannel = fm_mode_var->wTotalChannel;
    } else if (fm_mode_var->wFreChannel > fm_mode_var->wTotalChannel) {
        fm_mode_var->wTotalChannel = get_total_mem_channel();
        fm_mode_var->wFreChannel = 1;
    }
    fm_mode_var->wLastwTotalChannel = fm_mode_var->wTotalChannel;
    fm_mode_var->wFreq = get_fre_via_channle(fm_mode_var->wFreChannel) + MIN_FRE;				//根据台号找频点
    fm_module_set_fre(FM_CUR_FRE);
    fm_info->dat[FM_FRE] = fm_mode_var->wFreq - MIN_FRE;
    fm_info->dat[FM_CHAN] = fm_mode_var->wFreChannel;
    fm_save_info();
#else
    if ((fm_mode_var->wFreChannel == 0) || (fm_mode_var->wFreChannel == 0xff)) {
        fm_mode_var->wFreChannel = fm_mode_var->wTotalChannel;
    } else if (fm_mode_var->wFreChannel > fm_mode_var->wTotalChannel) {
        get_station_all();
        fm_mode_var->wTotalChannel = fm_station_all_temp;
        fm_mode_var->wFreChannel = 1;
    }
    fm_mode_var->wLastwTotalChannel = fm_mode_var->wTotalChannel;
    fm_mode_var->wFreq = fm_fre_buf[fm_mode_var->wFreChannel - 1] + MIN_FRE;				//根据台号找频点
    fm_module_set_fre(FM_CUR_FRE);
	fm_fre_cur_temp = fm_mode_var->wFreq - MIN_FRE;
	fm_station_cur_temp = fm_mode_var->wFreChannel;
	save_fre_cur();
	save_station_cur();
    fm_printf("!!!fm_fre_cur_temp = %d\n", fm_fre_cur_temp);
    fm_printf("!!!fm_station_all_temp = %d\n", fm_station_all_temp);
    fm_printf("!!!fm_station_cur_temp = %d\n", fm_station_cur_temp);
#endif
    fm_module_mute(0);
}
#if (USE_FM_SAVE_FRE_POINT == 0)
static void set_fm_preset_channel(void)
{
    if (fm_mode_var->wTotalChannel == 0) {
        //fm_mode_var->wFreChannel = 0;
        //fm_mode_var->wFreq = 875;
        //fm_module_set_fre(FM_CUR_FRE);
        //fm_module_mute(0);
        return;
    }
    if ((preset_station_num == 0) || (preset_station_num == 0xff)) {
        //fm_mode_var->wFreChannel = fm_mode_var->wTotalChannel;
        return;
    } else if (preset_station_num > MAX_STATION/*fm_mode_var->wTotalChannel*/) {
        //get_station_all();
        //fm_mode_var->wTotalChannel = fm_station_all_temp;
        //fm_mode_var->wFreChannel = 1;
        return;
    }
    //fm_mode_var->wLastwTotalChannel = fm_mode_var->wTotalChannel;
    fm_fre_buf[preset_station_num - 1] = fm_mode_var->wFreq - MIN_FRE;
	fm_mode_var->wFreChannel = preset_station_num;
	if (preset_station_num > fm_mode_var->wTotalChannel)
	{
	    fm_mode_var->wTotalChannel = preset_station_num;
	    fm_station_all_temp = fm_mode_var->wTotalChannel;
	    save_station_all();
	}
    fm_mode_var->wFreq = fm_fre_buf[fm_mode_var->wFreChannel - 1] + MIN_FRE;				//根据台号找频点
    fm_module_set_fre(FM_CUR_FRE);
	fm_fre_cur_temp = fm_mode_var->wFreq - MIN_FRE;
	fm_station_cur_temp = fm_mode_var->wFreChannel;
	save_fre_cur();
	save_station_cur();
	save_fre_all();
    fm_printf("###fm_fre_cur_temp = %d\n", fm_fre_cur_temp);
    fm_printf("###fm_station_all_temp = %d\n", fm_station_all_temp);
    fm_printf("###fm_station_cur_temp = %d\n", fm_station_cur_temp);
    fm_module_mute(0);
}
#endif
static void del_fm_channel(u8 chl_num)
{
    u8 i = 0;
    u8 j = 0;
    u8 byte = 0;
    u8 channel = 0;

    if (fm_mode_var->wTotalChannel == 0) {
        fm_module_mute(0);
        return;
    }
    if ((chl_num == 0) || (chl_num > fm_mode_var->wTotalChannel)) {
        return;
    }

    //找到channel 有效频点 标记位
    for (i = 0; i < MEM_FM_LEN; i++) {
        byte = fm_info->dat[FM_CHANNL + i];
        for (j = 0; j < 8; j++) {
            if ((byte & (1 << j)) != 0) {
                channel++;
                if (chl_num == channel) {
                    fm_info->dat[FM_CHANNL + i] &= (~(1 << j));
                    goto del_end;
                }
            }
        }
    }

    return;

del_end:

    fm_save_info();

    fm_mode_var->wTotalChannel--;
    if (fm_mode_var->wTotalChannel == 0) {
        fm_mode_var->wFreq = 0;
        fm_module_mute(0);
    } else {
        if (chl_num < fm_mode_var->wFreChannel) {
            //删除当前播放的之前电台
            fm_mode_var->wFreChannel--;
        }

        if (fm_mode_var->wFreChannel > fm_mode_var->wTotalChannel) {
            fm_mode_var->wFreChannel = 1;
        }
    }

    set_fm_channel();

}

static void save_fm_channel(u16 save_freq)
{
    u8 i = 0;
    u8 j = 0;
    u8 byte = 0, byte1;
    u8 channel = 0;

    byte = save_freq - MIN_FRE;
    i = (byte >> 3);
    if (i >= MEM_FM_LEN) {
        return;
    }

    j = (byte & 0x07);

    if ((fm_info->dat[FM_CHANNL + i] & (1 << j)) != 0) {
        //already save
        return;
    }
    fm_info->dat[FM_CHANNL + i] |= (1 << j);

    fm_save_info();

    //确定 channel num
    for (i = 0; i < MEM_FM_LEN; i++) {
        byte1 = fm_info->dat[FM_CHANNL + i];
        for (j = 0; j < 8; j++) {
            if ((byte1 & (1 << j)) != 0) {
                channel++;
                if (byte == (i * 8 + j)) {
                    goto save_end;
                }
            }
        }
    }

save_end:

    fm_mode_var->wTotalChannel++;
    fm_mode_var->wFreChannel = channel;
    set_fm_channel();

}

static void fm_unmute(void)
{
    if (is_dac_mute()) {
		if (sound.vol.sys_vol_l > 0)
        	dac_mute(0, 1);
    }
	fm_pp_flag = 0;
    mute_flag = 0;
    if(fm_mode_var->fm_mute == 1)
    {
		if (sound.vol.sys_vol_l > 0)
		{
        	fm_module_mute(0);
		}
    }
#if (FM_INSIDE == 0)
	if (linein_mute_status())
	{
		if (sound.vol.sys_vol_l > 0)
		{
	    	linein_mute(0);
		}
	}
#endif
	led_fm_play();
}
/*----------------------------------------------------------------------------*/
/**@brief  FM模式主线程
   @param  p：NULL
   @return NULL
   @note   static void fm_radio_task(void *p)
*/
/*----------------------------------------------------------------------------*/
static void task_fm_deal(void *p)
{
    int msg = NO_MSG;
    int error = MSG_NO_ERROR;
    u8 scan_counter = 0;
    u32 fm_input_num = 0;
    u32 res;

    led_fm_play();//led_fre_set(C_BLED_SLOW_MODE);
    fm_puts("\n***********************RADIO TASK********************\n");

    while (1) {
        error = task_get_msg(0, 1, &msg);
        if (task_common_msg_deal(p, msg) == false) {
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

#if FM_REC_EN
        if ((msg == MSG_REC_START) || (msg == MSG_REC_DEL)) {
            if (FALSE == is_cur_resource("fm")) { //当前资源不属fm所有，不允许录音，防止资源冲突
                continue;
            }
        }
        rec_msg_deal_api(&fm_rec_api, msg); //record 流程
#endif

        if (NO_MSG == msg) {
            continue;
        }

        //if ((rtc_set.alarm_set.alarm_flag == ALARM_ON)||(rtc_set.alarm_set.alarm_flag = ALARM_ON))
        //{
        //    if ((msg == MSG_FM_PREV_STATION)||(msg == MSG_FM_NEXT_STATION)
		//		||(msg == MSG_FM_PP)||(msg == MSG_FM_SCAN_ALL_INIT)
		//		||(msg == MSG_VOL_UP_SHORT)||(msg == MSG_VOL_DOWN_SHORT)||(msg == MSG_VOL_UP)||(msg == MSG_VOL_DOWN)
		//		)
		//		continue;
        //}

        if ((msg != MSG_HALF_SECOND) && (msg != MSG_ONE_SECOND) && (msg != MSG_FM_SCAN_ALL)) {
            printf("\nfm_msg=   0x%x\n", msg);
        }
    #if (USE_FM_SAVE_FRE_POINT == 0)
		if (preset_station_flag)
		{
		    if ((msg == MSG_FM_PRESET)||(msg == MSG_FM_PRESET_DOWN)||(msg == MSG_FM_PRESET_UP)
				||(msg == MSG_HALF_SECOND)||(msg == MSG_ONE_SECOND)||(msg == MSG_RTC_SET)
				||(msg == MSG_FM_PREV_STATION)||(msg == MSG_FM_NEXT_STATION)
				||(msg == MSG_FM_PRESET_OK))
		    {
		    }
			else
		    {
		        preset_station_flag = 0;
		    }
		}
    #endif
        if (!fm_msg_filter(msg)) { //根据FM状态，过滤消息
            continue;
        }
        if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
        {
            if ((msg == MSG_FM_PP)||(msg == MSG_HALF_SECOND)||(msg == MSG_ONE_SECOND)
                ||(msg == MSG_ALARM1_SET)||(msg == MSG_DIMMER)
                ||(msg == MSG_CHANGE_WORKMODE_PRE)||(msg == MSG_RTC_MODE)
                ||(msg == SYS_EVENT_PLAY_SEL_END)||(msg == MSG_ALARM_STOP))
            {
            }
            else
            {
                continue;
            }
        }
        else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
        {
            if ((msg == MSG_FM_PP)||(msg == MSG_HALF_SECOND)||(msg == MSG_ONE_SECOND)
                ||(msg == MSG_ALARM2_SET)||(msg == MSG_DIMMER)
                ||(msg == MSG_CHANGE_WORKMODE_PRE)||(msg == MSG_RTC_MODE)
                ||(msg == SYS_EVENT_PLAY_SEL_END)||(msg == MSG_ALARM_STOP))
            {
            }
            else
            {
                continue;
            }
        }

        switch (msg) {
        case SYS_EVENT_PLAY_SEL_END:
            printf("MUSIC SYS_EVENT_PLAY_TONE_END\n");
            if (FM_UNACTIVE == fm_mode_var->scan_mode) {
				AMP_MUTE();
				BOOST_DISABEL();
				AMP_AB();
                fm_radio_start();
                if ((rtc_set.alarm_set.alarm_flag == ALARM_ON)||(rtc_set.alarm_set.alarm2_flag == ALARM_ON))
                {
                    if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
                    {
                        fm_mode_var->wFreChannel = alm_fm_station;
                    }
                    else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
                    {
                        fm_mode_var->wFreChannel = alm2_fm_station;
                    }
                    set_fm_channel();
                }
				AMP_UNMUTE();
                ui_open_fm(&fm_mode_var, sizeof(FM_MODE_VAR **));
            }
			else
			{
			#if USE_MUTE_PALYTONE_ENABLE
				if (mute_flag)
				{
					//AMP_MUTE();
	                dac_mute(1, 1);
	                fm_module_mute(1);
	                led_fm_pause();
				}
			#endif
			}
            //ui_open_fm(&fm_mode_var, sizeof(FM_MODE_VAR **));


            break;
			
		case MSG_FM_SCAN_ALL_SEMI:
			led_fm_scan();
			mute_flag = 0;
            fm_pp_flag = 0;
			AMP_MUTE();
			goto SEEK_AUTO;
			break;
        case  MSG_FM_SCAN_ALL_INIT:
            fm_printf("MSG_FM_SCAN_ALL_INIT\n");
            /* led_fre_set(5,0); */
            if (fm_mode_var->scan_mode == FM_SCAN_STOP) {
                led_fm_scan();//led_fre_set(C_BLED_FAST_MODE);
				mute_flag = 0;
	            fm_pp_flag = 0;
				AMP_MUTE();
                delay_n10ms(50);
			#if USE_FM_SAVE_FRE_POINT
                fm_info->dat[FM_CHAN] = 0;
                fm_info->dat[FM_FRE] = 0;
			#else
			    fm_fre_cur_temp = 0;
			    fm_station_cur_temp = 0;
			#endif
                clear_all_fm_point();
                fm_mode_var->wTotalChannel = 0;
                fm_mode_var->wFreChannel = 0;
                fm_mode_var->wFreq = MIN_FRE - 1;//自动搜索从最低的频点开始
                scan_counter = MAX_CHANNL;
                fm_mode_var->scan_mode = FM_SCAN_ALL;
            } else {
                scan_counter = 1;//再搜索一个频点就停止
            }

        case  MSG_FM_SCAN_ALL:
SEEK_AUTO:
			AMP_MUTE();
			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
            //fm_printf("MSG_FM_SCAN_ALL\n");
            if (fm_radio_scan(fm_mode_var->scan_mode)) {
                fm_printf("\nFIND FM[%d] total_ch = %d, cnt=%d\n", fm_mode_var->wFreq, fm_mode_var->wTotalChannel, scan_counter);
    		#if (FM_INSIDE == 0)
    			if (linein_mute_status())
    			{
    			    linein_mute(0);
    			}
    		#endif
				if (is_dac_mute())
				{
				    dac_mute(0,1);
				}
				mute_flag = 0;
                fm_pp_flag = 0;
				AMP_UNMUTE();
				led_fm_play();
                if (fm_mode_var->scan_mode == FM_SCAN_ALL) {
                    // printf("wait 1s \n");
                    res = 10;
                    while (res) {
                        delay_n10ms(10);
                        res--;
                    }

                } else {                                                  //找到一个台，半自动搜索，播放当前频点
                    fm_mode_var->scan_mode = FM_SCAN_STOP;
                    set_fm_channel();
                    fm_module_mute(0);
                    // UI_menu(MENU_REFRESH);
                    UI_DIS_MAIN();
                    //led_fre_set(C_BLED_SLOW_MODE);
                    /* led_fre_set(15,0); */
                    break;
                }
				//AMP_MUTE();
				led_fm_scan();
            }
            scan_counter--;
            if (scan_counter == 0) {
                if (fm_mode_var->scan_mode == FM_SCAN_ALL) {               //全频点搜索结束，播放第一个台
                    task_post_msg(NULL, 1, MSG_FM_PRESET_UP);
                    fm_mode_var->scan_mode = FM_SCAN_STOP;
                    fm_printf("FM_SCAN_OVER \n");
                    //led_fre_set(C_BLED_SLOW_MODE);
                    //fm_insice_scan_info_printf(875,1080);
                } else if ((fm_mode_var->scan_mode == FM_SCAN_NEXT) || (fm_mode_var->scan_mode == FM_SCAN_PREV)) {
                    fm_mode_var->scan_mode = FM_SCAN_STOP;
                    //led_fre_set(C_BLED_SLOW_MODE);
                    fm_printf("FM_SCAN_OVER \n");
                }
			#if (USE_FM_SAVE_FRE_POINT == 0)
				save_station_all();
				save_fre_all();
			#endif
				fm_module_mute(0);
    		#if (FM_INSIDE == 0)
    			if (linein_mute_status())
    			{
    			    linein_mute(0);
    			}
    		#endif
				if (is_dac_mute())
				{
				    dac_mute(0,1);
				}
				mute_flag = 0;
                fm_pp_flag = 0;
				AMP_UNMUTE();
                led_fm_play();//led_fre_set(15,0);
            } else {   //搜索过程
                delay_2ms(2);
                if (fm_mode_var->scan_mode != FM_SCAN_STOP) {
                    res = task_post_msg(NULL, 1, MSG_FM_SCAN_ALL);
                    if (res == OS_Q_FULL) {
                        task_post_msg(NULL, 1, MSG_FM_SCAN_ALL);
                    }
                    /* os_sched_unlock(); */
                    UI_DIS_MAIN();
                }
            }
            break;

        case MSG_FM_SCAN_ALL_DOWN:
            fm_printf("MSG_FM_SCAN_ALL_DOWN\n");
            /* fm_mode_var->scan_mode = FM_SCAN_NEXT; */
            fm_mode_var->scan_mode = FM_SCAN_PREV;
            scan_counter = MAX_CHANNL;
            task_post_msg(NULL, 1, MSG_FM_SCAN_ALL_SEMI/*MSG_FM_SCAN_ALL*/);
            break;

        case MSG_FM_SCAN_ALL_UP:
            fm_printf("MSG_FM_SCAN_ALL_UP\n");
            /* fm_mode_var->scan_mode = FM_SCAN_PREV; */
            fm_mode_var->scan_mode = FM_SCAN_NEXT;
            scan_counter = MAX_CHANNL;
            task_post_msg(NULL, 1, MSG_FM_SCAN_ALL_SEMI/*MSG_FM_SCAN_ALL*/);
            break;

        case  MSG_FM_PREV_STEP:
            fm_printf("MSG_FM_PREV_STEP\n");
            res = fm_module_set_fre(FM_FRE_DEC);
		#if USE_FM_SAVE_FRE_POINT
            if (res) {
                save_fm_point(fm_mode_var->wFreq - MIN_FRE);
                fm_mode_var->wTotalChannel = get_total_mem_channel();
            }
            fm_mode_var->wFreChannel = get_channel_via_fre(fm_mode_var->wFreq - MIN_FRE);
            fm_info->dat[FM_FRE] = fm_mode_var->wFreq - MIN_FRE;
            fm_info->dat[FM_CHAN] = fm_mode_var->wFreChannel;
            fm_save_info();
            fm_module_mute(0);
            if (!res) {
                fm_mode_var->wFreChannel = 0;
            }
		#else
            fm_module_mute(0);
        #endif
            fm_unmute();
			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
            UI_DIS_MAIN();
            break;

        case MSG_FM_NEXT_STEP:
            fm_printf("MSG_FM_NEXT_STEP\n");
            res = fm_module_set_fre(FM_FRE_INC);
		#if USE_FM_SAVE_FRE_POINT
            if (res) {
                save_fm_point(fm_mode_var->wFreq - MIN_FRE);
                fm_mode_var->wTotalChannel = get_total_mem_channel();
            }
            fm_mode_var->wFreChannel = get_channel_via_fre(fm_mode_var->wFreq - MIN_FRE);
            fm_info->dat[FM_FRE] = fm_mode_var->wFreq - MIN_FRE;
            fm_info->dat[FM_CHAN] = fm_mode_var->wFreChannel;
            fm_save_info();
            fm_module_mute(0);
            if (!res) {
                fm_mode_var->wFreChannel = 0;
            }
		#else
            fm_module_mute(0);
        #endif
            fm_unmute();
			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
            UI_DIS_MAIN();
            break;
#if 0
        case MSG_FM_PREV_STATION:
            fm_printf("MSG_FM_PREV_STATION\n");

            //if (fm_mode_var->wTotalChannel == 0) {
                //if (preset_station_flag == 0)
                    task_post_msg(NULL, 1, MSG_FM_PREV_STEP);
            //    break;
            //}
		#if 0//(USE_FM_SAVE_FRE_POINT == 0)
            if (preset_station_flag)
            {
            	preset_station_cnt = 10;
				if (fm_mode_var->wTotalChannel)
				{
                	preset_station_num--;
    				if ((preset_station_num == 0)||(preset_station_num == 0xFF))
    					preset_station_num = fm_mode_var->wTotalChannel;
				}
                UI_menu(MENU_FM_PRESET_STATION, 0, 6);
                fm_printf("~~~preset_station_num=%d\n",preset_station_num);
            }
			else
		#endif
			{
                //fm_mode_var->wFreChannel--;//fm_mode_var->wFreChannel -= 2;
                //set_fm_channel();
                //UI_menu(MENU_FM_FIND_STATION, 0, 2);
			}
            break;

        case MSG_FM_NEXT_STATION:
            fm_printf("MSG_FM_NEXT_STATION\n");

            //if (fm_mode_var->wTotalChannel == 0) {
                //if (preset_station_flag == 0)
                    task_post_msg(NULL, 1, MSG_FM_NEXT_STEP);
            //    break;
            //}
		#if 0//(USE_FM_SAVE_FRE_POINT == 0)
            if (preset_station_flag)
            {
            	preset_station_cnt = 10;
				if (fm_mode_var->wTotalChannel)
				{
                	preset_station_num++;
    				if (preset_station_num > fm_mode_var->wTotalChannel)
    					preset_station_num = 1;
				}
                UI_menu(MENU_FM_PRESET_STATION, 0, 6);
                fm_printf("!!!preset_station_num=%d\n",preset_station_num);
            }
			else
		#endif
			{
                //fm_mode_var->wFreChannel++;
                //set_fm_channel();
                //UI_menu(MENU_FM_FIND_STATION, 0, 2);
			}
            break;
#endif
        case MSG_FM_PREV_STATION:
        case MSG_FM_PRESET_DOWN:
            if (preset_station_flag)
            {
            	preset_station_cnt = 10;
				if (fm_mode_var->wTotalChannel)
				{
                	preset_station_num--;
    				if ((preset_station_num == 0)||(preset_station_num == 0xFF))
    					preset_station_num = MAX_STATION;//fm_mode_var->wTotalChannel;
				}
                UI_menu(MENU_FM_PRESET_STATION, 0, 6);
                fm_printf("~~~preset_station_num=%d\n",preset_station_num);
            }
			else
			{
                if (fm_mode_var->wTotalChannel > 0) {
                    fm_mode_var->wFreChannel--;//fm_mode_var->wFreChannel -= 2;
                    set_fm_channel();
                    fm_unmute();
                    UI_menu(MENU_FM_FIND_STATION, 0, 2);
    			}
    			else
    			{
    			    task_post_msg(NULL, 1, MSG_FM_PREV_STEP);
    			}
			}
            break;
        case MSG_FM_NEXT_STATION:
        case MSG_FM_PRESET_UP:
            if (preset_station_flag)
            {
            	preset_station_cnt = 10;
				if (fm_mode_var->wTotalChannel)
				{
                	preset_station_num++;
    				if (preset_station_num > MAX_STATION/*fm_mode_var->wTotalChannel*/)
    					preset_station_num = 1;
				}
                UI_menu(MENU_FM_PRESET_STATION, 0, 6);
                fm_printf("!!!preset_station_num=%d\n",preset_station_num);
            }
			else
			{
                if (fm_mode_var->wTotalChannel > 0) {
                    fm_mode_var->wFreChannel++;
                    set_fm_channel();
                    fm_unmute();
                    UI_menu(MENU_FM_FIND_STATION, 0, 2);
    			}
    			else
    			{
    			    task_post_msg(NULL, 1, MSG_FM_NEXT_STEP);
    			}
			}
            break;

        case MSG_INPUT_NUMBER_END:
        case MSG_INPUT_TIMEOUT: {
            if (get_input_number(&fm_input_num)) {
		    #if USE_FM_SAVE_FRE_POINT
                if (fm_input_num < 100)
			#else
                if (fm_input_num < MAX_STATION/*100*/) { //选择台号
            #endif
                    if (fm_input_num > fm_mode_var->wTotalChannel) {
                        break;
                    }
                    fm_mode_var->wFreChannel = fm_input_num;
                    set_fm_channel();
                } else {  //选择频率
                    if ((fm_input_num < 875) || (fm_input_num > 1080)) {
                        break;
                    }
                    fm_mode_var->wFreq = fm_input_num;
                    res = fm_module_set_fre(FM_CUR_FRE);
                }
            }
            fm_module_mute(0);
            UI_DIS_MAIN();
        }
        break;
    #if (USE_FM_SAVE_FRE_POINT == 0)
        case MSG_FM_PRESET:
			if (preset_station_flag == 0)
			{
			    preset_station_flag = 1;
				if (fm_mode_var->wTotalChannel)
			        preset_station_num = fm_mode_var->wFreChannel;
				else
			        preset_station_num = 0;
			    preset_station_cnt = 10;
                fm_printf("~~~MSG_FM_PRESET_START\n");
                UI_menu(MENU_FM_PRESET_STATION, 0, 6);
			}
			else
			{
			    preset_station_flag = 0;
			    set_fm_preset_channel();
                fm_printf("~~~MSG_FM_PRESET_OVER\n");
    			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
    		    UI_DIS_MAIN();
			}
			break;
	#endif
        case MSG_FM_PP:
        	if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
        	{
			    task_post_msg(NULL, 1, MSG_ALARM1_SET);
            	break;
        	}
			else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
        	{
			    task_post_msg(NULL, 1, MSG_ALARM2_SET);
            	break;
        	}
			if ((fm_mode_var->scan_mode == FM_SCAN_ALL)||
				(fm_mode_var->scan_mode == FM_SCAN_NEXT)||
				(fm_mode_var->scan_mode == FM_SCAN_PREV))
			{
				task_post_msg(NULL, 1, MSG_FM_SCAN_ALL_INIT);
				break;
			}
		#if UI_ENABLE
            if (UI_var.bCurMenu == MENU_INPUT_NUMBER)   //暂停和播放
            {
                task_post_msg(NULL, 1, MSG_INPUT_TIMEOUT);
                break;
            }
		#endif
		    //break;
		case MSG_MUTE:
            if (mute_flag == 0) {//(fm_mode_var->fm_mute == 0) {
				mute_flag = 1;
                fm_pp_flag = 1;
				//AMP_MUTE();
                dac_mute(1, 1);
                fm_module_mute(1);
			#if (FM_INSIDE == 0)
                linein_mute(1);
			#endif
                led_fm_pause();
            } else {
				mute_flag = 0;
                fm_pp_flag = 0;
                fm_module_mute(0);
			#if (FM_INSIDE == 0)
                linein_mute(0);
			#endif
                dac_mute(0, 1);
				//AMP_UNMUTE();
                led_fm_play();
            }
			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
		    UI_DIS_MAIN();
            break;
	#if 0
        case MSG_VOL_UP:
        case MSG_VOL_DOWN:
            if (sound.vol.sys_vol_l && (get_tone_status() == 0)) {
                fm_module_mute(0);
            }
            //fm_printf("fm_vol recv\n");
            break;
	#endif

        case MSG_PROMPT_PLAY:
        //case MSG_LOW_POWER:
            //puts("fm_low_power\n");
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
#if FMTX_EN
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
	            //    linein_mute(0);
	            //}
	        }
			fm_pp_flag = 0;
	        mute_flag = 0;
            if(fm_mode_var->fm_mute == 1)
            {
            	//led_fm_play();
                fm_module_mute(0);
				//AMP_UNMUTE();
            }
		#if (FM_INSIDE == 0)
			if (linein_mute_status())
			{
			    linein_mute(0);
			}
		#endif
        	led_fm_play();
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
#if FMTX_EN
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
	            //    linein_mute(0);
	            //}
	        }
			fm_pp_flag = 0;
	        mute_flag = 0;
            if(fm_mode_var->fm_mute == 1)
            {
            	//led_fm_play();
				if (sound.vol.sys_vol_l > 0)
				{
                	fm_module_mute(0);
				}
				//if (sound.vol.sys_vol_l > 0)
				//	AMP_UNMUTE();
            }
		#if (FM_INSIDE == 0)
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
		#endif
        	led_fm_play();
			//if (sound.vol.sys_vol_l > 0)
			//	AMP_UNMUTE();
#endif
	        volume_display();//UI_menu(MENU_MAIN_VOL, 0, 3);
	        break;

        case MSG_FM_TEST1:
            fm_mode_var->wFreq = 875;
            fm_module_set_fre(FM_CUR_FRE);
            fm_module_mute(0);
            fm_printf("fm_mode_var->wFreq = %d\n", fm_mode_var->wFreq);
            break;


        case MSG_HALF_SECOND:
		#if DAC_AUTO_MUTE_EN
			if (fm_mode_var->scan_mode == FM_SCAN_STOP)
			{
				if ((sound.vol.sys_vol_r == 0)||(fm_mode_var->fm_mute)||(is_dac_mute())||(is_auto_mute()))
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
			}
		#endif
		#if (FM_AUTO_STANDBY_EN && AUTO_SHUT_DOWN_TIME)
			if ((sound.vol.sys_vol_r == 0)||(fm_mode_var->fm_mute)||(is_dac_mute())||(is_auto_mute()))
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
        #if (USE_FM_SAVE_FRE_POINT == 0)
		    if (preset_station_flag)
		    {
        	    if (preset_station_cnt)
        	    {
        			preset_station_cnt--;
        			if (preset_station_cnt == 0)
        			{
        			    preset_station_flag = 0;
    					//fm_mode_var->wFreq = fm_fre_buf[fm_mode_var->wFreChannel - 1] + MIN_FRE;
        			    set_fm_preset_channel();
                        fm_printf("~~~MSG_FM_PRESET_OVER\n");
            			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
        			}
        	    }
		    }
		#endif
		    if ((rtc_set.alarm_set.alarm_flag == ALARM_ON)||(rtc_set.alarm_set.alarm2_flag == ALARM_ON))
    		{
    			alm_time_cnt++;
    			if (alm_time_cnt >= ALARM_TIME)
    			{
            	    if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
                	{
                	    rtc_set.alarm_set.alarm_flag = ALARM_OFF;
        				//if (alm_times)
        				{
				            alarm_time_reset();
        				    rtc_tone_enable_flag = 0;
        			        task_post_msg(NULL, 1, MSG_RTC_MODE);
        				}
        				//else
        				{
        			    //    task_post_msg(NULL, 1, MSG_CHANGE_WORKMODE);
        				}
                	}
    			    else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
                	{
                	    rtc_set.alarm_set.alarm2_flag = ALARM_OFF;
        				//if (alm2_times)
        				{
				            alarm2_time_reset();
        				    rtc_tone_enable_flag = 0;
        			        task_post_msg(NULL, 1, MSG_RTC_MODE);
        				}
        				//else
        				{
        			    //    task_post_msg(NULL, 1, MSG_CHANGE_WORKMODE);
        				}
                	}
    			}
    		}
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
        case MSG_DIMMER:
        #if 0
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
			else
		#endif
        	{
        	#if 0
        	    led7_display_level++;
				if (led7_display_level > LED7_LEVEL_OFF)
					led7_display_level = LED7_LEVEL_MAX;
                set1628Display();
			    rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
		        UI_DIS_MAIN();
		    #else
			    task_post_msg(NULL, 1, MSG_SLEEP_SET);
		    #endif
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
			else
			{
			    task_post_msg(NULL, 1, MSG_CHANGE_WORKMODE);
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
			    task_post_msg(NULL, 1, MSG_FM_SCAN_ALL_INIT);
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
		case MSG_FM_PRESET_OK:
			if (preset_station_flag)
			{
			    preset_station_flag = 0;
			    set_fm_preset_channel();
                fm_printf("~~~MSG_FM_PRESET_OVER\n");
    			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
		        UI_DIS_MAIN();
			}
		    break;
        case MSG_RTC_SET:
        #if 0
			if (preset_station_flag)
			{
			    preset_station_flag = 0;
			    set_fm_preset_channel();
                fm_printf("~~~MSG_FM_PRESET_OVER\n");
    			rtc_display_cnt = RTC_DISPLAY_BACK_CNT;
			}
			else
			{
                rtc_set.calendar_set.coordinate++;
    			if (rtc_set.calendar_set.coordinate == RTC_DAT_SETTING)
                    rtc_set.calendar_set.coordinate++;
    			if (rtc_set.calendar_set.coordinate >= COORDINATE_MAX)
    			{
        			rtc_set.calendar_set.coordinate = COORDINATE_MIN;
    			}
    			rtc_display_cnt = 0;
    			coordinate_back_cnt = RTC_COORDINATE_BACK_CNT;
			}
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
        case MSG_FM_CLR:
            clear_all_fm_point();
			fm_station_all_temp = 0;
            fm_mode_var->wTotalChannel = 0;
			save_station_all();
			fm_station_cur_temp = 0;
            fm_mode_var->wFreChannel = 0;
			save_station_cur();
			fm_fre_cur_temp = 0;
            fm_mode_var->wFreq = MIN_FRE;
			save_fre_cur();
			
			fm_module_set_fre(FM_CUR_FRE);
			fm_pp_flag = 0;
	        mute_flag = 0;
            if(fm_mode_var->fm_mute == 1)
            {
            	//led_fm_play();
				if (sound.vol.sys_vol_l > 0)
				{
                	fm_module_mute(0);
				}
				//if (sound.vol.sys_vol_l > 0)
				//	AMP_UNMUTE();
            }
		#if (FM_INSIDE == 0)
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
		#endif
        	led_fm_play();
			
			UI_menu(MENU_FM_CLR, 0, 2);
			break;

        default:
            break;
        }
    }
}

const TASK_APP task_fm_info = {
    .skip_check = NULL,//
    .init 		= task_fm_init,
    .exit 		= task_fm_exit,
    .task 		= task_fm_deal,
    .key 		= &task_fm_key,
};

//
void alm_ring_fm_init(void)
{
    u8 ret = FALSE;
    fm_arg_open();
    ret = fm_mode_read_id(); //fm_radio_init();
    if (ret)
    {
    	AMP_MUTE();
    	BOOST_DISABEL();
    	AMP_AB();
        //fm_radio_start();
        ret = alm_ring_fm_radio_init();
        if (ret) {
            fm_mutex_init(NULL);//mutex_resource_apply("fm", 3, fm_mutex_init, fm_mutex_stop, 0);
        } else {
        }
    	AMP_UNMUTE();
	}
	alm_ring_fm_set_flag = 1;
}
void alm_ring_fm_exit(void)
{
#if (FM_INSIDE == 0)
    task_common_msg_deal(NULL, NO_MSG);
#if FM_AD_ENABLE
    ladc_ch_close(LADC_LINLR_CHANNEL);
#endif
    dac_channel_off(FM_IIC_CHANNEL, FADE_ON);
    dac_channel_on(MUSIC_CHANNEL, FADE_ON);
#endif
    fm_radio_powerdown();
    //task_clear_all_message();
    fm_arg_close();
    //ui_close_fm();
#if FM_REC_EN
    rec_exit(&fm_rec_api);
    mutex_resource_release("record_play");
    fat_del();
#endif
	BOOST_ENABEL();
	AMP_D();
	alm_ring_fm_set_flag = 0;
}
void alm_ring_set_fm_channel(void)
{
    if (fm_mode_var->wTotalChannel == 0) {
        fm_mode_var->wFreChannel = 0;
        fm_mode_var->wFreq = 875;
        fm_module_set_fre(FM_CUR_FRE);
        fm_module_mute(0);
        return;
    }
#if USE_FM_SAVE_FRE_POINT
    if ((fm_mode_var->wFreChannel == 0) || (fm_mode_var->wFreChannel == 0xff)) {
        fm_mode_var->wFreChannel = fm_mode_var->wTotalChannel;
    } else if (fm_mode_var->wFreChannel > fm_mode_var->wTotalChannel) {
        fm_mode_var->wTotalChannel = get_total_mem_channel();
        fm_mode_var->wFreChannel = 1;
    }
    fm_mode_var->wLastwTotalChannel = fm_mode_var->wTotalChannel;
    fm_mode_var->wFreq = get_fre_via_channle(fm_mode_var->wFreChannel) + MIN_FRE;				//根据台号找频点
    fm_module_set_fre(FM_CUR_FRE);
    fm_info->dat[FM_FRE] = fm_mode_var->wFreq - MIN_FRE;
    fm_info->dat[FM_CHAN] = fm_mode_var->wFreChannel;
    fm_save_info();
#else
    fm_mode_var->wFreq = fm_fre_buf[alm_fm_station - 1] + MIN_FRE;				//根据台号找频点
    fm_module_set_fre(FM_CUR_FRE);
#endif
    fm_module_mute(0);
}

void fm_info_clear(void)
{
    clear_all_fm_point();
	fm_station_all_temp = 0;
    //fm_mode_var->wTotalChannel = 0;
	save_station_all();
	fm_station_cur_temp = 0;
    //fm_mode_var->wFreChannel = 0;
	save_station_cur();
	fm_fre_cur_temp = 0;
    //fm_mode_var->wFreq = MIN_FRE;
	save_fre_cur();
}
#endif
