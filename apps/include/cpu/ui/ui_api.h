#ifndef _UI_API_H_
#define _UI_API_H_

#include "sdk_cfg.h"
//#include "dec/decoder_phy.h"

#define UI_TASK_NAME    "UI_TASK_HSECOND"

#define _MENU_LIST_SUPPORT_      (1)

#define UI_POST_MSG_FLAG   (0xF123)

//宏定义
//1、打印控制

#define UI_DEBUG
#ifdef UI_DEBUG
#define ui_puts         puts
#define ui_printf       printf
#else
#define ui_puts(...)
#define ui_printf(...)
#endif

#define UI_NO_ARG   (-1)

//2、显示控制
#define UI_RETURN      3

#if UI_ENABLE//0//
#define UI_DIS_MAIN(x)      ui_dis_main()
#define UI_menu(x,y,z)      ui_dis_menu(x,y,z)//ui_menu_api(x)
#define UI_REFRESH(x)       ui_menu_reflash(x)
#define UI_menu_arg(x,y)    ui_menu_arg_do(x,y)
#define UI_menu_mux(x,y)    ui_menu_spec(x,y)
#define SET_UI_MAIN(x)      ui_set_main(x) // {UI_var.bMainMenu = x;UI_var.rec_opt_state = 0;}//ui_menu_main(x)//
#define SET_UI_LOCK(x)      ui_set_lock(x） // UI_var.bLockMenu = x
#define SET_UI_UNLOCK(x)    ui_set_unlock() //UI_var.bLockMenu = 0
//#define SET_UI_BUF_ADR(x)   UI_var.ui_buf_adr = x
//#define SET_UI_BUF_LEN(x)   UI_var.ui_buf_len = x
#define SET_UI_BUF(x,y)     ui_set_buf(x,y)
#define SET_UI_SYS_VOL(x)   ui_set_sys_vol(x)// UI_var.sys_vol= x
//#define SET_UI_REC_OPT(x)   UI_var.rec_opt_state = x
//#define SET_UI_ECHO_PT(x)   UI_var.echo_pt = (void*)x
#define UI_GET_CUR_MENU(x)   ui_menu_get_arg(x)
#define UI_CLEAR_MSG_BUF()   ui_msg_buf_clear()

#else
#define UI_menu(...)
#define UI_menu_arg(...)
#define UI_menu_mux(...)
#define UI_REFRESH(...)
#define UI_DIS_MAIN(x)
#define SET_UI_MAIN(...)
#define SET_UI_LOCK(...)
#define SET_UI_UNLOCK(...)
#define SET_UI_BUF(...)
#define SET_UI_SYS_VOL(...)
#define SET_UI_REC_OPT(...)
#define SET_UI_ECHO_PT(...)
#define UI_GET_CUR_MENU(...)
#define UI_CLEAR_MSG_BUF(...)

#endif/*UI_ENABLE*/

/*******************UI_SELECT*******************************/

typedef enum {
    UI_LCD_SEG_3X9 = 0,	//39断码屏
    UI_LCD_SEG_4X8,	//48断码屏
    UI_LED,			//数码管
    UI_LCD_128X32,	//12832点阵屏
    UI_LCD_128X64,	//12864点阵屏
} UI_TYPE;

#define UI_SEL_RES       100    //内部上拉为10K

#define UI_ADC_33   (0x3ffL)
#define UI_ADC_30   (0x3ffL*1000/(1000 + UI_SEL_RES))//100K
#define UI_ADC_27   (0x3ffL*510/(510 + UI_SEL_RES))//51K
#define UI_ADC_23   (0x3ffL*240/(240 + UI_SEL_RES))//24K
#define UI_ADC_20   (0x3ffL*150/(150 + UI_SEL_RES))//15K
#define UI_ADC_00   (0)

#define LCD_SEG_3X9_AD          ((UI_ADC_33 + UI_ADC_30)/2)
#define LCD_SEG_4X8_AD 			((UI_ADC_30 + UI_ADC_27)/2)
#define LED_1888_AD      		((UI_ADC_27 + UI_ADC_23)/2)
#define LCD_128X32_AD       	((UI_ADC_23 + UI_ADC_20)/2)
#define LCD_128X64_AD        	((UI_ADC_20 + UI_ADC_00)/2)

/***********************************************************/

enum {
    MENU_POWER_UP = 0,
    MENU_WAIT,
    MENU_BT_MAIN,
    MENU_PC_MAIN,
    MENU_PC_VOL_UP,
    MENU_PC_VOL_DOWN,
    MENU_AUX_MAIN,
    MENU_ALM_UP,
    MENU_DEV_ERR,
    MENU_ECHO_MAIN,
    MENU_POWER_OFF,
    MENU_IDLE,
    MENU_SD,
    MENU_USB,
    MENU_NULL,
    /***以上数据对应menu_string数组，顺序不能修改***/

//    MENU_PLAY,
//    MENU_PLAYMODE,
//    MENU_MAIN_VOL,
//    MENU_EQ,

    MENU_MAIN_VOL,
    MENU_SET_EQ,
    MENU_SET_PLAY_MODE,

    MENU_NOFILE,
    MENU_NODEVICE,
    MENU_PLAY_TIME,
    MENU_FILENUM,
    MENU_INPUT_NUMBER,
    MENU_MUSIC_MAIN,
    MENU_MUSIC_PAUSE,

    MENU_FM_MAIN,
    MENU_FM_DISP_FRE,
    MENU_FM_FIND_STATION,
    MENU_FM_CHANNEL,
    MENU_FM_CLR,

    MENU_USBREMOVE,
    MENU_SDREMOVE,
    MENU_SCAN_DISK,

    MENU_NOP,
    MENU_NUM,

    MENU_RTC_MAIN,
    MENU_RTC_SET,
    MENU_RTC_PWD,
    MENU_ALM_SET,
    MENU_ALM_DISPLAY,
    MENU_ALM2_SET,
    MENU_ALM2_DISPLAY,
    MENU_ALM_UP_DISPLAY,
    MENU_TIME_FORMAT,
    MENU_AUTO_TIME,
    MENU_FM_PRESET_STATION,
    MENU_POWER_OFF_TIME,
    //MENU_ALM_UP,
    MENU_VERSION_2,
    MENU_RESET,
    MENU_ALL_DISPLAY,

    MENU_BT_SEARCH_DEVICE,
    MENU_BT_CONNECT_DEVICE,
    MENU_BT_DEVICE_ADD,
    MENU_BT_DEVICE_NAME,

    // BLUETOOTH_SHOW_DEVICE_ADDR_NAME_DEFAULT,
    // BLUETOOTH_CLEAR_LCD,
    // BLUETOOTH_TEST_A2DP,
    // BLUETOOTH_TEST_HFP,
    ////BLUETOOTH_TEST_HID,
    // BLUETOOTH_TEST_OFFSET,
    // BLUETOOTH_VALUE_OFFSET,

    MENU_RECODE_MAIN,
    MENU_RECODE_ERR,

    MENU_POWER,

    MENU_LIST_DISPLAY,

	//
    MENU_MAIN_VOL_MAX_MIN,
	// 
	MENU_DISPLAY_TEST,

    //MENU_200MS_REFRESH = 0x80,
    //MENU_100MS_REFRESH,
    MENU_SEC_REFRESH = 0x80,
    MENU_REFRESH,
    // MENU_POWER_DOWN,
    // MENU_SPEC,
    // MENU_MAIN,
    // MENU_SUB,
    // MENU_OTHER,
};

enum {
    UI_MSG_NON = 0,
    UI_MSG_MAIN_MENU,
    UI_MSG_REFLASH,
    UI_MSG_OTHER_MENU,
    UI_MSG_SUB_MENU,
    UI_MSG_MENULIST,

    UI_MSG_MENULIST_KEYBACK = 0x10,
    UI_MSG_MENULIST_KEYMODE,
    UI_MSG_MENULIST_KEYPREV,
    UI_MSG_MENULIST_KEYNEXT,
    UI_MSG_MENULIST_KEYEXIT,

    UI_MSG_NULL = 0xff,
};

enum {
    UI_REC_OPT_STOP = 0,
    UI_REC_OPT_START,
    UI_REC_OPT_PAUSE,
    UI_REC_OPT_NODEV,
    UI_REC_OPT_ERR,
};

enum {
    UI_MENU_LIST_MAIN = 0,
    UI_MENU_LIST_ITEM,
    UI_MENU_LIST_BROWSER,
};
enum {
	MENU_0000,
	MENU_1111,
	MENU_2222,
	MENU_3333,
	MENU_4444,
	MENU_5555,
	MENU_6666,
	MENU_7777,
	MENU_8888,
	MENU_9999,
	MENU_VERSION,
#if 0
	MENU_PM,
	MENU_FM,
	MENU_ICON_BT,
	MENU_AU,
	MENU_MON,
	MENU_TUE,
	MENU_WED,
	MENU_THU,
	MENU_FRI,
	MENU_SAT,
	MENU_SUN,
	MENU_A1,
	MENU_A2,
	MENU_DOT_UP,
	MENU_DOT_DN,
	MENU_MHZ,
#else
    #if 0
	MENU_2POINT,
	MENU_MHZ,
	MENU_SLEEP,
	MENU_SNOOZ,
	MENU_PM,
	MENU_AUX,
	MENU_ALM1,
	MENU_ALM1_PM,
	MENU_2POINT_ALM1,
	MENU_ALM2,
	MENU_ALM2_PM,
	MENU_2POINT_ALM2,
	MENU_FM,
	MENU_AM,
	MENU_BT,
	MENU_MHZ_2,
	MENU_KHZ,
	MENU_SD_TEST,
	MENU_USB_TEST,
	MENU_CD,
	MENU_MEMORY,
	MENU_FOLDER,
	MENU_ALL_LOOP,
	MENU_RANDOM,
	MENU_BAT,
	#else
    MENU_2POINT,
    MENU_MHZ,
    MENU_CD,
    MENU_USB_TEST,
    MENU_SD_TEST,
    MENU_BT,
    MENU_FM,
    MENU_AUX,
    MENU_MEMORY,
    MENU_AM,
    MENU_PM,
    MENU_KHZ,
    MENU_FOLDER,
    MENU_ALL_LOOP,
    MENU_RANDOM,
    MENU_ALM1,
    MENU_ALM2,
    MENU_SLEEP,
    MENU_SNOOZ,
    MENU_BAT,
    MENU_MHZ_2,
	#endif
#endif
	MENU_ALL,
	//MENU_CLR,
	MENU_HI,
	MENU_CLOCK,
};

typedef struct _UI_VAR {
    u8  bCurMenu;
    u8  bMainMenu;
    u8  bMenuReturnCnt;
    u8  bMenuReturnmax;
    u8  bLockMenu;
    void *ui_buf_adr; //UI_显示buf，第一个u32 必须是显示类型
    u32 ui_buf_len;
    u8  *sys_vol;
    int *echo_pt;
    u8  rec_opt_state;
    u8  ui_type;
    u8  param_menu;
    u32  param;
} UI_VAR;

typedef struct _UI_DISP_API {
    ///--------common------///
    void (*ui_string_menu)(u8);
    void (*ui_IR_number)(s32);
    void (*ui_vol_set)(s32 vol);
    void (*ui_clear)(void);
    void (*ui_setX)(u8);
    void (*ui_power)(void);

    ///--------music------///
    void (*ui_music_main)(void);
    void (*ui_eq_set)(s32);
    void (*ui_file_num)(void);
    void (*ui_play_mode)(s32);

    ///--------fm------///
    void (*ui_FM_main)(void);
    void (*ui_FM_channel)(void);

    ///--------rtc------///
    void (*ui_RTC_main)(void);
    void (*ui_RTC_set)(void);
    void (*ui_ALM_set)(void);
    void (*ui_ALM2_set)(void);

    ///--------rec------///
    void (*ui_REC_main)(void);

    ///--------echo------///
    void (*ui_echo_main)(u8);


    ///--------aux------///
    void (*ui_AUX_main)(u8);

    ///--------bt------///
    void (*ui_BT_main)(u8);

    ///--------pc------///
    void (*ui_PC_main)(u8);

    void (*ui_menu_list)(u32);

} UI_DISP_API;


//全局变量声明
extern UI_VAR _data UI_var;

//函数声明
//void set_LED_fade_out(void);
//void set_LED_all_on(void);
void ui_init_api(void);
void ui_menu_api(u8 menu, s32 arg);
bool ui_menu_reflash(u8 menu);
bool ui_menu_arg_do(u8 menu, u32 arg);
bool ui_menu_spec(u8 NewMenu, u32 CurMenu);
u32 ui_menu_get_arg(u8 type);
void ui_set_main(u32 menu);
void ui_set_lock(u32 menu);
void ui_set_unlock();
void ui_set_buf(u32 *addr, u32 len);
void ui_set_sys_vol(u8 *vol);
bool ui_dis_main();
bool ui_dis_menu(u8 menu, u32 arg, u8 back_main_flag);
void ui_msg_buf_clear(void);
u8 is_main_menu(void);

extern void UI_run(u32 *msg_buf);
#if UI_ENABLE
extern void volume_display(void);
#else
#define volume_display()
#endif
#endif  /*  _LED_UI_API_H_  */
