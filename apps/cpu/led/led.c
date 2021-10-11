#include "sdk_cfg.h"
#include "led.h"
#include "timer.h"
#include "clock_api.h"
#include "common.h"
#include "board_init.h"
#include "bluetooth/bt_led.h"
#include "key.h"
#include "common/msg.h"
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif

#define LED_SCAN_MS   10  //led_scan 10ms
#define LED_INT  2  //led_scan 2ms
static u16 g_led_fre = 0; ///<led fre
static u8 g_led_flash = 0;///<b_led and r_led flash

static volatile u8 g_led_scan_en = 1;

void set_led_scan(u8 en)
{
    g_led_scan_en = en;
}

static u32 g_led_cnt = 0;
static volatile bool led_rb_flag = 0;
static u32 pre_led_flash = 0;
void lower_power_led_flash(u8 control, u32 led_flash)
{
    if (control) {
        led_fre_set(pre_led_flash);

    } else {
        if (led_flash != C_RLED_LOWER_POWER_MODE) {
            pre_led_flash = led_flash ;
        }
    }
}
void led_cnt(u32 cnt)
{
    g_led_cnt += cnt;
}

void led_scan(void)
{
    u8 led_doble_flag = 0;
    if (!g_led_scan_en) {
        return;
    }

#if LED_EN

    u32 led_cnt;
///进入powndown为了达到省功耗，红灯、蓝灯或者红蓝灯要一直亮时，需要修改 in_low_pwr_port_deal函数，对应的点灯口不控制关,LEDB_PORT LEDR_PORT选择
    switch (g_led_flash) {

    case C_ALL_OFF :    ///全灭
        powndown_led_fre_set((10 * LED_SCAN_MS), (500 * LED_SCAN_MS)); //sleep(5000ms)
        B_LED_OFF();
        R_LED_OFF();
        break;
    case C_ALL_ON  :    ///全亮
        powndown_led_fre_set((10 * LED_SCAN_MS), (500 * LED_SCAN_MS)); //sleep(5000ms)
        B_LED_ON();
        R_LED_ON();
        break;
    case C_BLED_ON :    ///蓝亮
        powndown_led_fre_set((10 * LED_SCAN_MS), (500 * LED_SCAN_MS)); //sleep(5000ms)
        B_LED_ON();
        break;
    case C_BLED_OFF :    ///蓝灭
        powndown_led_fre_set((5 * LED_SCAN_MS), (500 * LED_SCAN_MS)); //sleep(5000ms)
        B_LED_OFF();
        break;
    case C_RLED_ON  :    ///红亮
        powndown_led_fre_set((5 * LED_SCAN_MS), (500 * LED_SCAN_MS)); //sleep(5000ms)
        R_LED_ON();
        break;
    case C_RLED_OFF :    ///红灭
        powndown_led_fre_set((5 * LED_SCAN_MS), (500 * LED_SCAN_MS)); //sleep(5000ms)
        R_LED_OFF();
        break;
    case C_BLED_SLOW :    ///蓝慢闪
    case C_BLED_FAST :    ///蓝快闪
        powndown_led_fre_set(((g_led_fre + LED_INT)*LED_SCAN_MS), ((g_led_fre << 1)*LED_SCAN_MS));
        led_cnt = (g_led_cnt % (g_led_fre << 1));
        if (led_cnt > g_led_fre) {
            B_LED_OFF();
        } else if (led_cnt > 0) {
            B_LED_ON();
        }
        break;
    case C_BLED_FAST_DOBLE_5S :    ///蓝灯5秒闪连闪两下
        led_doble_flag = 1;
    case C_BLED_FAST_ONE_5S:       ///蓝灯5秒闪连闪1下
        powndown_led_fre_set((45 + LED_INT) * LED_SCAN_MS, (g_led_fre * LED_SCAN_MS)); //sleep(4550ms)
        led_cnt = (g_led_cnt % g_led_fre);
        if (led_cnt > 45 && led_doble_flag) {
            B_LED_OFF();
        } else if (led_cnt > 30 && led_doble_flag) {
            B_LED_ON();
        } else if (led_cnt > 15) {
            B_LED_OFF();
        } else if (led_cnt > 0) {
            B_LED_ON();
        }
        break;
    case C_RLED_SLOW :    ///红慢闪
    case C_RLED_FAST :   ///红快闪
        powndown_led_fre_set((g_led_fre + LED_INT)*LED_SCAN_MS, ((g_led_fre << 1)*LED_SCAN_MS));
        led_cnt = (g_led_cnt % (g_led_fre << 1));
        if (led_cnt > g_led_fre) {
            if (led_cnt >= g_led_fre * 2) {
            }
            R_LED_OFF();
        } else if (led_cnt > 0) {
            R_LED_ON();
        }
        break;
    case C_RLED_FAST_DOBLE_5S  :   ///红灯5秒闪连闪两下
        led_doble_flag = 1;
    case C_RLED_FAST_ONE_5S:    //////红灯5秒闪连闪1下
        powndown_led_fre_set((45 + LED_INT) * LED_SCAN_MS, (g_led_fre * LED_SCAN_MS)); //sleep(4550ms)
        led_cnt = (g_led_cnt % g_led_fre);
        if (led_cnt > 45 && led_doble_flag) {
            R_LED_OFF();
        } else if (led_cnt > 30 && led_doble_flag) {
            R_LED_ON();
        } else if (led_cnt > 15) {
            R_LED_OFF();
        } else if (led_cnt > 0) {
            R_LED_ON();
        }
        break;
    case C_RB_FAST :    ///红蓝交替闪（快闪）
    case C_RB_SLOW:     ///红蓝交替闪（慢闪）
        powndown_led_fre_set(((g_led_fre / 4) + LED_INT)*LED_SCAN_MS, (g_led_fre / 4 - LED_INT)*LED_SCAN_MS); //sleep (g_led_fre/4*10ms)
        led_cnt = (g_led_cnt % g_led_fre);
        if (led_cnt > ((g_led_fre * 3) / 4)) {
            B_LED_OFF();
            R_LED_OFF();
        } else if (led_cnt > (g_led_fre / 2)) {
            B_LED_OFF();
            R_LED_ON();
        } else if (led_cnt > (g_led_fre / 4)) {
            B_LED_OFF();
            R_LED_OFF();
        } else if (led_cnt > 0) {
            if (led_rb_flag) {
                B_LED_OFF();
                R_LED_ON();
            } else {
                R_LED_OFF();
                B_LED_ON();
            }
        }
        break;
    default:
        break;
    }
#endif
}

static void led_timer_scan(void)
{
#if(((LED_TYPE_SEL == LED_PRX) && (LED_CONTROL_SEL == LED_PWM_TYPE)))
    led_pwm_portr_fre_set(g_led_flash);
    return;
#endif
    led_cnt(1);
    led_scan();
}
LOOP_UI_REGISTER(led_detect) = {
    .time = 5,
    .fun  = led_timer_scan,
};
/*----------------------------------------------------------------------------*/
/**@brief   LED初始化函数
   @param   void
   @return  void
   @note    void led_init(void)
*/
/*----------------------------------------------------------------------------*/
void led_fre_set(u32 fre_type)
{
    static u8  last_g_led_flash = 0;
#if LED_EN
    g_led_flash = fre_type & 0x00ff;
    g_led_fre = fre_type >> 8;
    if (last_g_led_flash == g_led_flash) {      //不重复设置led闪烁类型
        return;
    }

    last_g_led_flash = g_led_flash;
    //LED_INIT_EN();

#if((LED_TYPE_SEL == LED_PRX) && (LED_CONTROL_SEL == LED_PWM_TYPE))
    led_pwm_portr_fre_set(g_led_flash);
#else
    B_LED_OFF();
    R_LED_OFF();
#endif

    lower_power_led_flash(0, fre_type);
    led_rb_flag = 0;
#endif
}
static void __led_fre_set(void)
{
    /* led_fre_set(C_RLED_FAST_DOBLE_MODE); */
    /* led_fre_set(C_ALL_ON_MODE); */
}
no_sequence_initcall(__led_fre_set);

#if 0
static void set_timer1_pwm(u32 fre, u8 duty)
{
#if (LED_CONTROL_SEL == LED_PWM_TYPE)

    SFR(JL_IOMAP->CON1, 8, 4, 3);
    JL_TIMER1->CON = 0x0000;
    JL_TIMER1->PRD = OSC_Hz / (4 * fre);
    JL_TIMER1->PWM = (JL_TIMER1->PRD * duty) / 100;
    JL_TIMER1->CON = (1 << 4) | (1 << 8) | (1 << 0) | (2 << 2);
#endif
}

void led_pwm_init(void)
{
#if (LED_CONTROL_SEL == LED_PWM_TYPE)
    set_timer1_pwm(500L, 20);
#endif
}

void led_pwm_close(void)
{
#if (LED_CONTROL_SEL == LED_PWM_TYPE)
    set_timer1_pwm(0, 0);
#endif
}
#endif


/*----------------------------------------------------------------------------*/
/**@brief   LED_in_bt_sniff
   @param   void
   @return  void
   @note    void led_init(void)
*/
/*----------------------------------------------------------------------------*/
void led_sniff_cb(void)
{
    if (!get_key_invalid_flag()) {
        g_led_cnt = 0;
        led_rb_flag = !led_rb_flag;
    } else {
        if (led_rb_flag == 1) {
            g_led_cnt = 0;
        }
        led_rb_flag = 0;
    }
}

void clear_led_cnt(void)
{
    g_led_cnt = 0;
}
void clear_led_rb_flag(void)
{
    led_rb_flag = 0;
}
void led_bt_sniff_init(void)
{
#if LED_EN
    //LED_INIT_EN();
#if(LED_CONTROL_SEL == LED_SNIFF_TYPE)
    bt_led_init(led_sniff_cb, 500, 5000 - 500); //C_RLED_FAST_DOBLE
#endif
#endif
}

////关机红灯闪三下
void set_r_led_on_cnt(u8 cnt)
{
    extern void delay_2ms(u32 delay_time);
    static u8 power_off = 0;
    u8 cnt_led;
    cnt_led = cnt * 2;
    if (power_off) {
        return;
    }
    B_LED_OFF();
    while (cnt_led) {
        if (cnt_led % 2) {
            R_LED_OFF();
        } else {
            R_LED_ON();
        }
        delay_2ms(150);
        cnt_led--;
    }
    power_off = 1;
}


void led_test(void)
{
#if 0
    extern void close_wdt(void);
    close_wdt();

    extern void delay(u32 d);

    LED_INIT_EN();
    while (1) {
        B_LED_ON();
        R_LED_ON();
        delay(4800000);
        B_LED_OFF();
        R_LED_OFF();
        delay(4800000);
    }
#endif

}

#if (LED_RED_BLINK&&LED_BLUE_BLINK)
void led_mode_on(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
void led_mode_off(void)
{
	//led_status2 = C_ALL_OFF_MODE;
	led_fre_set(C_ALL_OFF_MODE);
}
//void led_bt_none(void)
//{
//}
//void led_bt_voice(void)
//{
//}
void led_bt_idle(void)
{
	//led_status2 = C_RB_FAST_MODE;
	led_fre_set(C_RB_FAST_MODE);
}
void led_bt_connect(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
void led_bt_play(void)
{
	//led_status2 = C_RB_SLOW_MODE;
	led_fre_set(C_BLED_SLOW_MODE);
}
void led_bt_update_name_end(void)
{
	//led_status2 = C_ALL_ON_MODE;
	led_fre_set(C_ALL_ON_MODE);
}
void led_idle(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
//void led_busy(void)
//{
//}
void led_music_play(void)
{
	//led_status2 = C_BLED_SLOW_MODE;
	led_fre_set(C_RLED_SLOW_MODE);
}
void led_music_pause(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_RLED_ON_MODE);
}
void led_music_idle(void)
{
	//led_status2 = C_RLED_ON_MODE;
	led_fre_set(C_RLED_ON_MODE);
}
void led_aux_play(void)
{
	//led_status2 = C_RLED_SLOW_MODE;
	led_fre_set(C_RLED_SLOW_MODE);
}
void led_aux_pause(void)
{
	//led_status2 = C_RLED_ON_MODE;
	led_fre_set(C_RLED_ON_MODE);
}
void led_fm_play(void)
{
	//led_status2 = C_RLED_SLOW_MODE;
	led_fre_set(C_RLED_SLOW_MODE);
}
void led_fm_pause(void)
{
	//led_status2 = C_RLED_ON_MODE;
	led_fre_set(C_RLED_ON_MODE);
}
void led_fm_scan(void)
{
	//led_status2 = C_RLED_FAST_MODE;
	led_fre_set(C_RLED_FAST_MODE);
}
#elif LED_BLUE_BLINK
void led_mode_on(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
void led_mode_off(void)
{
	//led_status2 = C_ALL_OFF_MODE;
	led_fre_set(C_ALL_OFF_MODE);
}
//void led_bt_none(void)
//{
//}
//void led_bt_voice(void)
//{
//}
void led_bt_idle(void)
{
	//led_status2 = C_BLED_FAST_MODE;
	led_fre_set(C_BLED_FAST_MODE);
}
void led_bt_connect(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
void led_bt_play(void)
{
	//led_status2 = C_BLED_SLOW_MODE;
	led_fre_set(C_BLED_SLOW_MODE);
}
void led_bt_update_name_end(void)
{
	//led_status2 = C_ALL_ON_MODE;
	led_fre_set(C_ALL_ON_MODE);
}
void led_idle(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
//void led_busy(void)
//{
//}
void led_music_play(void)
{
	//led_status2 = C_BLED_SLOW_MODE;
	led_fre_set(C_BLED_SLOW_MODE);
}
void led_music_pause(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
void led_music_idle(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
void led_aux_play(void)
{
	//led_status2 = C_BLED_SLOW_MODE;
	led_fre_set(C_BLED_SLOW_MODE);
}
void led_aux_pause(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
void led_fm_play(void)
{
	//led_status2 = C_BLED_SLOW_MODE;
	led_fre_set(C_BLED_SLOW_MODE);
}
void led_fm_pause(void)
{
	//led_status2 = C_BLED_ON_MODE;
	led_fre_set(C_BLED_ON_MODE);
}
void led_fm_scan(void)
{
	//led_status2 = C_BLED_FAST_MODE;
	led_fre_set(C_BLED_FAST_MODE);
}
#endif

#if DCIN_DECT_IO
void dcin_detect(void)
{
	static u8 dc_in_online_cnt;
	u8 temp;

    DCIN_INIT();
	if (IS_DCIN())
		temp = true;
	else
		temp = false;

	if (dcin_status == temp)
		dc_in_online_cnt = 0;
	else
	{
		dc_in_online_cnt++;
		if (dc_in_online_cnt > 20)
		{
			dc_in_online_cnt = 0;
			dcin_status = temp;
			if (dcin_status == true)
			{
			    if (back_light_set_flag == 0)
			    {
                    set_timer1_pwm(500,back_light_table[back_light_mode]);//BACKLIGHT_ENABLE();
                    backlight_cnt = 11;
                }
			}
			else
			{
			    task_post_msg(NULL, 1, MSG_POWER_OFF_AUTO);
			}
		}
	}
}
LOOP_DETECT_REGISTER(dcin_detect_loop) = {
    .time = 5,
    .fun  = dcin_detect,
};
#endif

