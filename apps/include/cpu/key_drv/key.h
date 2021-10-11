/*--------------------------------------------------------------------------*/
/**@file     key.h
   @brief
   @details
   @author
   @date   2011-4-26
   @note
*/
/*----------------------------------------------------------------------------*/
#ifndef _KEY_
#define _KEY_

#include "typedef.h"
#include "sdk_cfg.h"
// #define KEY_UART_DEBUG

#ifdef KEY_UART_DEBUG
#define key_puts           puts
#define key_printf         printf
#define key_buf            put_buf
#else
#define key_puts(...)
#define key_printf(...)
#define key_buf(...)
#endif/*KEY_UART_DEBUG*/

/*按键类型定义*/
#define KEY_AD_RTCVDD_EN       0   ///<采用RTCVDD电源的AD按键使能
#define KEY_AD_VDDIO_EN        1   ///<采用VDDIO电源的AD按键使能
#define KEY_IO_EN              1   ///<IO按键使能
#define KEY_IR_EN              0   ///<红外遥控使能,PA9
#define KEY_TCH_EN             0   ///<触摸按键使能
#define KEY_UART_EN            0   ///<串口按键使能
#define KEY_ENCODER_EN		   1   ///<旋转编码器使能

#if ((KEY_AD_RTCVDD_EN == 1) && (KEY_AD_VDDIO_EN == 1))
#error  "AD_KEY power support only one"
#endif


/*按键类型*/
typedef enum {
    KEY_TYPE_AD,
    KEY_TYPE_IO,
    KEY_TYPE_IR,
    KEY_TYPE_TOUCH,
    KEY_TYPE_UART,
    MAX_TYPE_KEY,
} KEY_TYPE;

/*按键门槛值*/
#define KEY_BASE_CNT  3///4
#define KEY_LONG_CNT  75
#define KEY_HOLD_CNT  11//15
#define KEY_SHORT_CNT 3///7

/*按键状态*/
#define KEY_SHORT_UP    0x0
#define KEY_LONG        0x1
#define KEY_HOLD        0x2
#define KEY_LONG_UP     0x3

#define KEY_DOUBLE_CLICK_CNT    50 //35*10ms

#define NO_KEY          0xff

typedef struct {
    KEY_TYPE key_type;
    void (*key_init)(void);
    u8(*key_get_value)(void);
} key_interface_t;


#define KEY_REG_AD_MAX			(10)
#define KEY_REG_IO_MAX			(10)
#define KEY_REG_IR_MAX			(21)
#define KEY_REG_TOUCH_MAX		(10)
#define KEY_REG_UART_MAX		(10)

typedef struct __KEY_REG {
#if (KEY_AD_RTCVDD_EN||KEY_AD_VDDIO_EN)
    const u16(*_ad)[KEY_REG_AD_MAX];
#endif
#if KEY_IO_EN
    const u16(*_io)[KEY_REG_IO_MAX];
#endif
#if KEY_IR_EN
    const u16(*_ir)[KEY_REG_IR_MAX];
#endif
#if KEY_TCH_EN
    const u16(*_touch)[KEY_REG_TOUCH_MAX];
#endif
#if KEY_UART_EN
    const u16(*_uart)[KEY_REG_UART_MAX];
#endif
} KEY_REG;

void key_msg_reg(const KEY_REG *reg);
u8 __get_key_invalid_flag(void);
#if (KEY_AD_RTCVDD_EN | KEY_AD_VDDIO_EN | KEY_IO_EN |KEY_IR_EN |KEY_TCH_EN |KEY_UART_EN)
#define key_msg_table_reg key_msg_reg
#define get_key_invalid_flag() __get_key_invalid_flag()
#else
#define key_msg_table_reg(...)
#define get_key_invalid_flag() 0
#endif

extern u8 vol_maxmin_play_flag;
extern u16 auto_sleep_time_cnt;
extern u8 dcin_status;
extern u8 is_dcin_poweron;
extern u8 tone_display_flag;
extern u8 mute_flag;
extern u8 music_stop_flag;
extern u16 voltage_value;
extern u8 volume_chang_enable;
extern u8 volume_value;
extern u8 flag_100ms;
extern u8 aux_pp_flag;
extern u8 fm_pp_flag;
extern u8 led7_display_level;
extern u8 alarm_ring_mode_flag;
extern u8 rtc_tone_enable_flag;
extern u8 rtc_display_cnt;
extern const u16 auto_power_off_table[AUTO_TIME_END];
extern u8 auto_power_off_type;
extern u16 auto_power_off_cnt;
extern u8 preset_station_num;
extern u8 alarm_up_flag;
extern u8 display_test_flag;
extern u8 alarm_tone_flag;
extern u8 tone_back_cnt;
extern u8 time_alm_flag;
extern u8 time_alm2_flag;
extern u8 time_format_set_flag;
extern u8 backlight_flag;
extern u8 backlight_cnt;
extern u8 half_second_flash_flag;
extern u8 amp_mute_flag;
extern u8 mode_type;
extern u8 back_light_mode;
extern const u8 back_light_table[];
extern u8 back_light_set_flag;
extern u16 pwm_duty;
extern u32 music_curr_file;
extern u32 music_total_file;
extern u8 music_normal_end_flag;
extern u8 fm_station_cur_temp;
extern u8 fm_station_all_temp;
extern u8 alm_fm_station_temp;
extern u8 preset_station_flag;
extern void fm_module_mute(u8 flag);
extern void delay_2ms(u32 delay_time);
extern void HT1621LCDInit(void);
extern u8 get_tone_status(void);
extern void rtc_alm_coordinate_init(void);
extern void set_timer1_pwm(u32 fre, u8 duty);
extern void set_fm_channel(void);
extern void bt_info_clear(void);
extern void fm_info_clear(void);

#endif
