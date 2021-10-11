#ifndef POWER_H
#define POWER_H

#include "typedef.h"
#include "audio/dac_api.h"
enum {
    POWER_ON_CNT_SET = 1,
    POWER_ON_CNT_INC = 2,
    POWER_ON_CNT_GET = 3,
};
#define GOINT_POWER_OFF_START_CNT 1
#define GOINT_POWER_OFF_END_CNT   4    //控制按键关机时间
#define GOINT_POWER_OFF_START     0XFF //立即关机
#if 0
#define POWER_KEY				PORTR2

#define POWER_KEY_INIT()        do{PORTR_PU(POWER_KEY,1);PORTR_PD(POWER_KEY,0);PORTR_DIR(POWER_KEY,1);\
  							      }while(0)
#define IS_POWER_KEY_DOWN()    	(!PORTR_IN(POWER_KEY))
#endif
#if POWER_EXTERN_DETECT_EN
#define AD_POWER_EXTERN_IO_BIT		BIT(4)
#define AD_POWER_EXTERN_IO_PORT		PORTR2
#define AD_POWER_EXTERN_CH			AD_CH_PR2
#endif

u16 get_battery_level(void);
void power_init_app(u8 mode, u8 chargeV);
void power_on_detect_deal(void);
u16 get_power_external_value(void);
u8 get_low_power_external_flag(void);

u32 bt_noconn_pwr_down_in(void);
u32 bt_noconn_pwr_down_out(void);
void battery_check(void);
extern u16 control_power_on_cnt(u8 mode, u16 poweron_cnt);
void pa_umute(void);
void pa_mute(void);
extern void *get_power_manage_op_str();
/*function in bt libs*/
extern void edr_power_manage_struct_init(void *power_op);
extern void ble_power_manage_struct_init(void *power_op);
extern void rf_reset_dual_mode(u8 dual_mode);
void set_main_ldo_en(u8 en);
#endif      //POWER_H
