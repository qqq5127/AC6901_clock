#ifndef POWER_MAMAGE_API_H
#define POWER_MAMAGE_API_H

#include "typedef.h"

/************************need to init before entern lowpower***************************/
void set_lowpower_mode_config(u8 config);
void set_wheather_can_enter_lowpower_callback(u32(*fun)(void));
void set_lowpower_osc(u8 osc_type, u32 osc_hz);
void set_lowpower_delay_arg(u32 sys_freq);
void set_lowpower_keep_32K_osc_flag(u8 flag);
void set_lowpower_pd_ldo_level(u32 vddio, u32 rtcvdd, u32 dvdd, u32 dvdd_cur);
void set_lowpower_btosc_dis(u8 flag);
void set_lowpower_io_status_set_callback(void(*fun)(u8, u32));
void set_lowpower_wakeup_io_callback(void (*handle)(), void (*sleep_io_handle)());
void set_lowpower_poweroff_wakeup_callback(void (*fun)(void));
void bt_power_reset_dual_mode(void(*reset_dual_mode)(u8));
void set_get_osc_callback(void(*fun)(u32 *, u32 *));


/************************power set function***************************/
u16 get_ldo_bt();
void set_sys_pwrmd(u8 mode);
void enter_sys_soft_poweroff();
void soft_power_ctl(u8 ctl);
void enter_sys_sleep_mode(void);
void enter_sys_powerdown_mode(void);
void enter_sys_poweroff_mode(void);

u8 bt_power_is_poweroff_probe(void);
u8 bt_power_is_poweroff_post(void);

void set_vddio_level(u8 level);
u8 get_vddio_level(void);
void set_rtcvdd_level(u8 level);
u8 get_rtcvdd_level(void);
void set_vdd_level(u8 level);
u8 get_vdd_level(void);
void set_dvdda_level(u8 level);
u8 get_dvdda_level(void);

//sw:0 disable  1:enable
//0(2.0v)  1(2.1v) 2(2.2v) 3(2.3v) 4(2.4v) 5(2.5v) 6(2.6v) 7(2.7v)
void set_lvd_mode(u8 sw, u8 lev);

void set_sys_ldo_level(u8 vddio, u8 rtcvdd);
void set_fm_ldo_level(u8 level);

void power_off_demo_init(void);
void power_off_demo_close(void);
void power_off_demo_test(void);

void lowpower_test_demo(void);
u32 get_jiffies(u8 mode, u32 timer_ms);

void set_power_off_lock_ext(void);
void set_power_off_ulock_ext(void);
#endif  //POWER_MAMAGE_API_H
