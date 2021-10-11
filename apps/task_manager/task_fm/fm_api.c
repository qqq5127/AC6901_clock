/*--------------------------------------------------------------------------*/
/**@file    fm_api.c
   @brief   FM 模式功能接口函数
   @details
   @author  guowei
   @date    2014-9-15
   @note    BC51
*/
/*----------------------------------------------------------------------------*/
#include "sdk_cfg.h"

#if FM_RADIO_EN

#include "task_fm/task_fm.h"
#include "task_fm/fm_api.h"
#include "common/msg.h"
#include "audio/dac.h"
#include "common/flash_api.h"
#include "audio/ladc.h"
#include "clock_api.h"
#include "common/common.h"
#include "audio/src_api.h"
#include "ui_api.h"
#include "rec_api.h"
#include "audio/audio.h"
#include "dac.h"

#if	RDA5807
#include "RDA5807.h"
#endif

#if	BK1080
#include "BK1080.h"
#endif

#if	QN8035
#include "QN8035.h"
#endif

#if	FM_INSIDE
#include "fm_inside.h"
#endif

/* extern RECORD_OP_API * rec_fm_api; */

static FM_INFO fm_info_buf sec_used(.fm_mem)__attribute__((aligned(4)));
static FM_MODE_VAR fm_mode_var_buf sec_used(.fm_mem)__attribute__((aligned(4)));

extern RECORD_OP_API *fm_rec_api;

void (* const fm_arr_init[])(void)  = {

#if	RDA5807
    rda5807_init,
#endif

#if	BK1080
    bk1080_init,
#endif

#if	QN8035
    qn8035_init,
#endif

#if	FM_INSIDE
    fm_inside_init,
#endif

};

bool (* const fm_arr_set_fre[])(u16) = {

#if	RDA5807
    rda5807_set_fre,
#endif

#if	BK1080
    bk1080_set_fre,
#endif

#if	QN8035
    qn8035_set_fre,
#endif

#if	FM_INSIDE
    fm_inside_set_fre,
#endif

};


void (* const fm_arr_powerdown[])(void) = {

#if	RDA5807
    rda5807_powerdown,
#endif

#if	 BK1080
    bk1080_powerdown,
#endif

#if	QN8035
    qn8035_powerdown,
#endif

#if	FM_INSIDE
    fm_inside_powerdown,
#endif

};


bool (* const fm_arr_read_id[])(void) = {

#if	RDA5807
    rda5807_read_id,
#endif

#if	 BK1080
    bk1080_read_id,
#endif

#if	QN8035
    qn8035_read_id,
#endif

#if	FM_INSIDE
    fm_inside_read_id,
#endif

};


void (* const fm_arr_mute[])(u8) = {

#if	RDA5807
    rda5807_mute,
#endif

#if	BK1080
    bk1080_mute,
#endif

#if	QN8035
    qn8035_mute,
#endif

#if	FM_INSIDE
    fm_inside_mute,
#endif

};

/*----------------------------------------------------------------------------*/
/**@brief  FM变量初始化函数
   @param  无
   @return 无
   @note   void fm_drv_open(void)
*/
/*----------------------------------------------------------------------------*/
void fm_arg_open(void)
{
    memset(&fm_mode_var_buf, 0x00, sizeof(FM_MODE_VAR));
    fm_mode_var = &fm_mode_var_buf;
    memset(&fm_info_buf, 0x00, sizeof(FM_INFO));
    fm_info = &fm_info_buf;
}

/*----------------------------------------------------------------------------*/
/**@brief  FM变量初始化函数
   @param  无
   @return 无
   @note   void fm_open(void)
*/
/*----------------------------------------------------------------------------*/
void fm_arg_close(void)
{
    memset(&fm_mode_var_buf, 0x00, sizeof(FM_MODE_VAR));
    memset(&fm_info_buf, 0x00, sizeof(FM_INFO));
}

/*----------------------------------------------------------------------------*/
/**@brief   FM模块初始化接口函数
   @param
   @return
   @note    void init_fm_rev(void)
*/
/*----------------------------------------------------------------------------*/
bool fm_mode_init(void)
{
    for (fm_mode_var->bAddr = 0; fm_mode_var->bAddr < (sizeof(fm_arr_read_id) / sizeof(fm_arr_read_id[0])); fm_mode_var->bAddr++) {
        if ((*fm_arr_read_id[fm_mode_var->bAddr])()) {
            (* fm_arr_init[fm_mode_var->bAddr])();
            return TRUE;
        }
    }
    return FALSE;
}

/*----------------------------------------------------------------------------*/
/**@brief  FM ID read.
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
bool fm_mode_read_id(void)
{
    if (NULL == fm_mode_var) {
        return FALSE;
    }
    for (fm_mode_var->bAddr = 0; fm_mode_var->bAddr < (sizeof(fm_arr_read_id) / sizeof(fm_arr_read_id[0])); fm_mode_var->bAddr++) {
        if ((*fm_arr_read_id[fm_mode_var->bAddr])()) {
            // (* fm_arr_init[fm_mode_var->bAddr])();
            return TRUE;
        }
    }

    return FALSE;
}

/*----------------------------------------------------------------------------*/
/**@brief   关闭FM模块电源
   @param
   @return
   @note    void fm_mode_powerdown(void
*/
/*----------------------------------------------------------------------------*/
void fm_mode_powerdown(void)
{
    (* fm_arr_powerdown[fm_mode_var->bAddr])();
}

/*----------------------------------------------------------------------------*/
/**@brief   FM模块Mute开关
   @param   flag：Mute使能位 0：开声音  1：关声音
   @return  无
   @note    void fm_module_mute(u8 flag)
*/
/*----------------------------------------------------------------------------*/
void fm_module_mute(u8 flag)
{
	//if (flag == 0)
	{
		//mute_flag = 0;
		//fm_pp_flag = 0;
	}
    fm_mode_var->fm_mute = flag;
    (* fm_arr_mute[fm_mode_var->bAddr])(flag);
}

/*----------------------------------------------------------------------------*/
/**@brief   设置一个FM频点的接口函数
   @param   mode = 0: 使用frequency中的值，= 1:频点加1， = 2:频点减1
   @return  1：有台；0：无台
   @note    bool fm_module_set_fre(u16 fre, u8 mode)
*/
/*----------------------------------------------------------------------------*/
bool fm_module_set_fre(u8 mode)
{
    fm_module_mute(1);

    vm_check_api(0);

    if (mode == FM_FRE_INC) {
        fm_mode_var->wFreq++;
    } else if (mode == FM_FRE_DEC) {
        fm_mode_var->wFreq--;
    }
    if (fm_mode_var->wFreq > MAX_FRE) {
        fm_mode_var->wFreq = MIN_FRE;
    }
    if (fm_mode_var->wFreq < MIN_FRE) {
        fm_mode_var->wFreq = MAX_FRE;
    }

    return (* fm_arr_set_fre[fm_mode_var->bAddr])(fm_mode_var->wFreq);
}

/*----------------------------------------------------------------------------*/
/**@brief   设置一个FM频点的接口函数
   @param   freq:875-1080
   @return  NULL
   @note    void fm_test_set_freq(u16 freq)
*/
/*----------------------------------------------------------------------------*/
void fm_test_set_freq(u16 freq)
{
    u8 ret;
    ret = (* fm_arr_set_fre[fm_mode_var->bAddr])(freq);
    fm_printf("fm_test_set_freq:%d = %d\n", freq, ret);
}

/*----------------------------------------------------------------------------*/
/**@brief    全频段搜索
   @param    mode: 0全频段, 1:上一个有效频点， 2:下一个有效频点
   @return   0:未找到频点 1:搜索完成 2:退出FM模式
   @note     bool fm_radio_scan(u8 mode)
*/
/*----------------------------------------------------------------------------*/
bool fm_radio_scan(u8 mode)
{
    bool tres;

    if (mode == FM_SCAN_PREV) {
        tres = fm_module_set_fre(FM_FRE_DEC);
    } else if ((mode == FM_SCAN_NEXT) || (mode == FM_SCAN_ALL)) {
        tres = fm_module_set_fre(FM_FRE_INC);
    } else { //搜台状态错误
        return FALSE;
    }
#if USE_FM_SAVE_FRE_POINT
    fm_mode_var->wFreChannel = 0;
#endif
    //全频搜索频点显示
    /* SET_UI_LOCK(MENU_FM_DISP_FRE); */
#if LED_7_EN
    UI_menu(MENU_FM_DISP_FRE, 0, 0);
#endif // LED_7_EN
#if LCD_128X64_EN
    if (MENU_FM_MAIN == UI_GET_CUR_MENU(1)) {
        UI_REFRESH(MENU_REFRESH);
    }
#endif // LCD_128X64_EN
    if (tres) {					//找到一个台
        if (fm_mode_var->fm_mute) {
            fm_module_mute(0);
        }
	#if USE_FM_SAVE_FRE_POINT
        fm_info->dat[FM_FRE] = fm_mode_var->wFreq - MIN_FRE;
        save_fm_point(fm_mode_var->wFreq - MIN_FRE);
        fm_mode_var->wFreChannel = get_channel_via_fre(fm_mode_var->wFreq - MIN_FRE);
        fm_mode_var->wTotalChannel = get_total_mem_channel();
    #else
		if (fm_mode_var->wFreChannel < MAX_STATION)
		{
    	    fm_fre_cur_temp = fm_mode_var->wFreq - MIN_FRE;
    	    fm_fre_buf[fm_mode_var->wFreChannel] = fm_mode_var->wFreq - MIN_FRE;
            fm_printf("+++fm_fre_buf[fm_mode_var->wFreChannel] = %d\n", fm_fre_buf[fm_mode_var->wFreChannel],fm_mode_var->wFreChannel);
            fm_mode_var->wFreChannel++;
    		fm_mode_var->wTotalChannel = fm_mode_var->wFreChannel;
    		fm_station_all_temp = fm_mode_var->wTotalChannel;
            fm_printf("+++fm_fre_cur_temp = %d\n", fm_fre_cur_temp);
            fm_printf("+++fm_station_all_temp = %d\n", fm_station_all_temp);
            UI_menu(MENU_FM_FIND_STATION, 0, 0);
		}
	#endif
        //显示搜到的频道
        /* SET_UI_LOCK(MENU_FM_FIND_STATION); */
#if LED_7_EN
        //UI_menu(MENU_FM_FIND_STATION, 0, 0);
#endif // LED_7_EN
#if LCD_128X64_EN
        UI_REFRESH(MENU_REFRESH);
#endif // LCD_128X64_EN
        return TRUE;
    }

    return FALSE;
}

/*----------------------------------------------------------------------------*/
/**@brief  FM 模式信息初始化
   @param  无
   @return 无
   @note   void fm_info_init(void)
*/
/*----------------------------------------------------------------------------*/
static void fm_info_init(void)
{
    fm_mode_var->scan_mode = FM_ACTIVE;
#if USE_FM_SAVE_FRE_POINT
    fm_read_info();
    fm_puts("after_fm_read_info\n");
    fm_mode_var->wFreq = fm_info->dat[FM_FRE];
    if (fm_mode_var->wFreq > (MAX_FRE - MIN_FRE)) {
        fm_mode_var->wFreq = MIN_FRE;
    } else {
        fm_mode_var->wFreq += MIN_FRE;
    }

    fm_mode_var->wTotalChannel = get_total_mem_channel();
    fm_printf("wTotalChannel = %d\n", fm_mode_var->wTotalChannel);
    if (!fm_mode_var->wTotalChannel) {
        fm_mode_var->wFreq = 875;
        fm_mode_var->wFreChannel = 0;
        fm_mode_var->wTotalChannel = 0;
        fm_module_set_fre(FM_CUR_FRE);
        fm_module_mute(0);
        fm_mode_var->scan_mode = FM_SCAN_STOP;
        return;
    }

    fm_mode_var->wFreChannel = fm_info->dat[FM_CHAN];
    if (fm_mode_var->wFreChannel > MAX_CHANNL) {				//台号为1;总台数为1
        fm_mode_var->wFreChannel = 1;
        fm_mode_var->wTotalChannel = 1;
    } else if (!fm_mode_var->wFreChannel) {
        fm_mode_var->wFreChannel = 1;
    }

    fm_mode_var->wFreChannel = get_channel_via_fre(fm_mode_var->wFreq - MIN_FRE);
    if (0x00 == fm_mode_var->wFreChannel) {
        fm_puts("no channel\n");
        /* fm_mode_var->wFreChannel = 1; */
        /* save_fm_point(fm_mode_var->wFreq - MIN_FRE); */
        /* fm_save_info(); */
    }
#else
    get_fre_all();
    get_fre_cur();
    fm_mode_var->wFreq = fm_fre_cur_temp;
    fm_printf("~~~fm_fre_cur_temp = %d\n", fm_fre_cur_temp);
    if (fm_mode_var->wFreq > (MAX_FRE - MIN_FRE)) {
        fm_mode_var->wFreq = MIN_FRE;
    } else {
        fm_mode_var->wFreq += MIN_FRE;
    }

    get_station_all();
    fm_mode_var->wTotalChannel = fm_station_all_temp;
    fm_printf("~~~fm_station_all_temp = %d\n", fm_station_all_temp);
    if (!fm_mode_var->wTotalChannel) {
        fm_mode_var->wFreq = 875;
        fm_mode_var->wFreChannel = 0;
        fm_mode_var->wTotalChannel = 0;
        fm_module_set_fre(FM_CUR_FRE);
        fm_module_mute(0);
        fm_mode_var->scan_mode = FM_SCAN_STOP;
        return;
    }

    get_station_cur();
    fm_mode_var->wFreChannel = fm_station_cur_temp;
    fm_printf("~~~fm_station_cur_temp = %d\n", fm_station_cur_temp);
    if (fm_mode_var->wFreChannel > MAX_STATION) {				//台号为1;总台数为1
        fm_mode_var->wFreChannel = 1;
        fm_mode_var->wTotalChannel = 1;
    } else if (!fm_mode_var->wFreChannel) {
        fm_mode_var->wFreChannel = 1;
    }
#endif
    //fm_mode_var->wFreq = 875;
    fm_module_set_fre(FM_CUR_FRE);
    fm_module_mute(0);
    fm_mode_var->scan_mode = FM_SCAN_STOP;

    /*---------FM MAIN UI--------------*/
//    set_ui_main(MENU_FM_MAIN);
//    ui_menu_api(MENU_FM_MAIN);
}

//
static void alm_ring_fm_info_init(void)
{
    fm_mode_var->scan_mode = FM_ACTIVE;
#if USE_FM_SAVE_FRE_POINT
    fm_read_info();
    fm_puts("after_fm_read_info\n");
    fm_mode_var->wFreq = fm_info->dat[FM_FRE];
    if (fm_mode_var->wFreq > (MAX_FRE - MIN_FRE)) {
        fm_mode_var->wFreq = MIN_FRE;
    } else {
        fm_mode_var->wFreq += MIN_FRE;
    }

    fm_mode_var->wTotalChannel = get_total_mem_channel();
    fm_printf("wTotalChannel = %d\n", fm_mode_var->wTotalChannel);
    if (!fm_mode_var->wTotalChannel) {
        fm_mode_var->wFreq = 875;
        fm_mode_var->wFreChannel = 0;
        fm_mode_var->wTotalChannel = 0;
        fm_module_set_fre(FM_CUR_FRE);
        fm_module_mute(0);
        fm_mode_var->scan_mode = FM_SCAN_STOP;
        return;
    }

    fm_mode_var->wFreChannel = fm_info->dat[FM_CHAN];
    if (fm_mode_var->wFreChannel > MAX_CHANNL) {				//台号为1;总台数为1
        fm_mode_var->wFreChannel = 1;
        fm_mode_var->wTotalChannel = 1;
    } else if (!fm_mode_var->wFreChannel) {
        fm_mode_var->wFreChannel = 1;
    }

    fm_mode_var->wFreChannel = get_channel_via_fre(fm_mode_var->wFreq - MIN_FRE);
    if (0x00 == fm_mode_var->wFreChannel) {
        fm_puts("no channel\n");
        /* fm_mode_var->wFreChannel = 1; */
        /* save_fm_point(fm_mode_var->wFreq - MIN_FRE); */
        /* fm_save_info(); */
    }
#else
    get_fre_all();
    #if 0
    get_fre_cur();
    fm_mode_var->wFreq = fm_fre_cur_temp;
    fm_printf("~~~fm_fre_cur_temp = %d\n", fm_fre_cur_temp);
    if (fm_mode_var->wFreq > (MAX_FRE - MIN_FRE)) {
        fm_mode_var->wFreq = MIN_FRE;
    } else {
        fm_mode_var->wFreq += MIN_FRE;
    }
    #endif
    get_station_all();
    fm_mode_var->wTotalChannel = fm_station_all_temp;
    fm_printf("~~~fm_station_all_temp = %d\n", fm_station_all_temp);
    if (!fm_mode_var->wTotalChannel) {
        fm_mode_var->wFreq = 875;
        fm_mode_var->wFreChannel = 0;
        fm_mode_var->wTotalChannel = 0;
        //fm_module_set_fre(FM_CUR_FRE);
        //fm_module_mute(0);
        //fm_mode_var->scan_mode = FM_SCAN_STOP;
        //return;
    }
    else
    {
        fm_mode_var->wFreq = fm_fre_buf[alm_fm_station_temp - 1] + MIN_FRE;
    }
#endif
    fm_module_set_fre(FM_CUR_FRE);
    fm_module_mute(0);
    fm_mode_var->scan_mode = FM_SCAN_STOP;
}

void fm_channel_init(void)
{
	dac_mute(1, 1);
    if (FM_IIC_CHANNEL == DAC_AMUX0) {
        /* PB4(L)PB5(R) */
        JL_PORTB->DIR |= BIT(4) | BIT(5);
    } else if (FM_IIC_CHANNEL == DAC_AMUX1) {
        /* PA3(L)PA4(R) */
        JL_PORTA->DIR |= BIT(3) | BIT(4);
    } else if (FM_IIC_CHANNEL == DAC_AMUX2) {
        /* PB6(L)PB3(R) */
        JL_PORTB->DIR |= BIT(6) | BIT(3);
    }
#if FM_AD_ENABLE
    /*
     *如果是dac其中一个通道做aux输入，由于dac通道自身阻抗
     *导致输入略小于普通aux通道。这中情况下，可以讲作为aux
     *输入的通道增益减小。比如DAC_Left作为aux输入：
     *set_sys_vol(1, sound.vol.sys_vol_r, FADE_ON);
     */
    linein_channel_open(FM_IIC_CHANNEL, 0);
    ladc_ch_open(LADC_LINLR_CHANNEL, SR44100);
#else
    dac_channel_off(MUSIC_CHANNEL, FADE_ON);
    delay_2ms(20);
    dac_channel_on(FM_IIC_CHANNEL, FADE_ON);
#endif
    delay_2ms(50);
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    //delay_2ms(50);
	//dac_mute(0, 1);
}

void fm_mutex_init(void *priv)
{
#if	FM_INSIDE
    fm_inside_mutex_init();
#endif
#if (FM_INSIDE == 0)
	fm_channel_init();
    delay_2ms(50);
    linein_mute(0);
    delay_2ms(50);
	dac_mute(0, 1);
#endif
}

void fm_mutex_stop(void *priv)
{
#if	FM_INSIDE
    fm_inside_mutex_stop();
#endif
#if FM_REC_EN
//    rec_msg_deal_api(&fm_rec_api, MSG_REC_STOP); //record 流程
    rec_exit(&fm_rec_api);
#endif
#if (FM_INSIDE == 0)
    dac_channel_off(FM_IIC_CHANNEL, FADE_OFF);
#if FM_AD_ENABLE
    ladc_ch_close(LADC_LINLR_CHANNEL);
#endif
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief  FM 模式关闭总接口
   @param  无
   @return 无
   @note   void fm_radio_powerdown(void)
*/
/*----------------------------------------------------------------------------*/
void fm_radio_powerdown(void)
{
    fm_mode_powerdown();
    fm_arg_close();
}


/*----------------------------------------------------------------------------*/
/**@brief  FM 模式初始化总接口
   @param  无
   @return 无
   @note   void fm_radio_init(void)
*/
/*----------------------------------------------------------------------------*/
u8 fm_radio_init(void)
{
    if (fm_mode_init()) {
        fm_puts("fm_mode_init_ok\n");
        fm_info_init();
        fm_puts("after fm_info init\n");
        return TRUE;
    } else {
        fm_puts("fm_mode_init_fail\n");
        fm_radio_powerdown();
        return FALSE;
    }
}

u8 alm_ring_fm_radio_init(void)
{
    if (fm_mode_init()) {
        fm_puts("fm_mode_init_ok\n");
        alm_ring_fm_info_init();
        fm_puts("after fm_info init\n");
        return TRUE;
    } else {
        fm_puts("fm_mode_init_fail\n");
        fm_radio_powerdown();
        return FALSE;
    }
}

#endif
