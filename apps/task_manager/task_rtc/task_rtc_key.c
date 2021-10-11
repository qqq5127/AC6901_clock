#include "common/msg.h"
#include "task_rtc_key.h"

#define ADKEY_RTC_SHORT     \
                        /*00*/    MSG_ALARM1_SET,\
                        /*01*/    MSG_ALARM2_SET,\
                        /*02*/    MSG_RTC_SET/*MSG_BACK_LIGHT_SET*/,\
                        /*03*/    MSG_DIMMER,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,


#define ADKEY_RTC_LONG      \
                        /*00*/    MSG_ALARM1_SET_OK,\
                        /*01*/    MSG_ALARM2_SET_OK,\
                        /*02*/    MSG_RTC_SET_OK,\
                        /*03*/    NO_MSG/*MSG_ALARM_STOP*/,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,


#define ADKEY_RTC_HOLD      \
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    MSG_RESET_HOLD,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define ADKEY_RTC_LONG_UP   \
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

const u16 task_rtc_ad_table[4][KEY_REG_AD_MAX] = {
    /*短按*/        {ADKEY_RTC_SHORT},
    /*长按*/        {ADKEY_RTC_LONG},
    /*连按*/        {ADKEY_RTC_HOLD},
    /*长按抬起*/    {ADKEY_RTC_LONG_UP},
};

/*******************************************************************
                            I/O按键表
*******************************************************************/
#define IOKEY_RTC_SHORT     \
                        /*00*/    MSG_CHANGE_WORKMODE_PRE,\
                        /*01*/    NO_MSG,\
                        /*02*/    MSG_CHANGE_WORKMODE_PRE,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define IOKEY_RTC_LONG      \
                        /*00*/    MSG_POWER_OFF/*MSG_RTC_SET_OK*/,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,


#define IOKEY_RTC_HOLD      \
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define IOKEY_RTC_LONG_UP   \
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

const u16 task_rtc_io_table[4][KEY_REG_IO_MAX] = {
    /*短按*/        {IOKEY_RTC_SHORT},
    /*长按*/        {IOKEY_RTC_LONG},
    /*连按*/        {IOKEY_RTC_HOLD},
    /*长按抬起*/    {IOKEY_RTC_LONG_UP},
};

/*******************************************************************
                            IR按键表
*******************************************************************/
#define IRFF00_RTC_KEY_SHORT            \
				MSG_POWER_OFF,	MSG_CHANGE_WORKMODE,	MSG_MUTE,			\
				MSG_RTC_SET,	MSG_RTC_MINUS,			MSG_RTC_PLUS,		\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG

#define IRFF00_RTC_KEY_LONG         \
				NO_MSG,			NO_MSG,					NO_MSG,				\
				MSG_RTC_SET_OK,	MSG_RTC_MINUS,			MSG_RTC_PLUS,		\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG

#define IRFF00_RTC_KEY_HOLD         \
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			MSG_RTC_MINUS,			MSG_RTC_PLUS,		\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG

#define IRFF00_RTC_KEY_LONG_UP      \
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG
const u16 task_rtc_ir_table[4][KEY_REG_IR_MAX] = {
    /*短按*/        {IRFF00_RTC_KEY_SHORT},
    /*长按*/        {IRFF00_RTC_KEY_LONG},
    /*连按*/        {IRFF00_RTC_KEY_HOLD},
    /*长按抬起*/    {IRFF00_RTC_KEY_LONG_UP},
};


/*******************************************************************
                            touchkey按键表
*******************************************************************/
#define TOUCHKEY_RTC_SHORT      \
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define TOUCHKEY_RTC_LONG       \
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,


#define TOUCHKEY_RTC_HOLD       \
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define TOUCHKEY_RTC_LONG_UP    \
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

const u16 task_rtc_touch_table[4][KEY_REG_TOUCH_MAX] = {
    /*短按*/        {TOUCHKEY_RTC_SHORT},
    /*长按*/        {TOUCHKEY_RTC_LONG},
    /*连按*/        {TOUCHKEY_RTC_HOLD},
    /*长按抬起*/    {TOUCHKEY_RTC_LONG_UP},
};

const KEY_REG task_rtc_key = {
#if (KEY_AD_RTCVDD_EN||KEY_AD_VDDIO_EN)
    ._ad = task_rtc_ad_table,
#endif
#if KEY_IO_EN
    ._io = task_rtc_io_table,
#endif
#if KEY_IR_EN
    ._ir = task_rtc_ir_table,
#endif
#if KEY_TCH_EN
    ._touch = task_rtc_touch_table,
#endif
#if KEY_UART_EN
    ._uart = task_rtc_io_table,//task_bt_touch_table,
#endif
};

//alarm1 key
/*******************************************************************
                            AD按键表
*******************************************************************/
#define ADKEY_ALARM_SHORT		\
                        /*00*/    NO_MSG/*MSG_ALARM1_SET*/,\
                        /*01*/    NO_MSG/*MSG_ALARM2_SET*/,\
                        /*02*/    MSG_BACK_LIGHT_SET,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    MSG_ALARM_SNOOZE,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define ADKEY_ALARM_LONG		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    MSG_ALARM_STOP,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define ADKEY_ALARM_HOLD		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define ADKEY_ALARM_LONG_UP	\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

const u16 task_alarm_ad_table[4][KEY_REG_AD_MAX] = {
    /*短按*/	    {ADKEY_ALARM_SHORT},
    /*长按*/		{ADKEY_ALARM_LONG},
    /*连按*/		{ADKEY_ALARM_HOLD},
    /*长按抬起*/	{ADKEY_ALARM_LONG_UP},
};

/*******************************************************************
                            I/O按键表
*******************************************************************/
#define IOKEY_ALARM_SHORT		\
                        /*00*/    MSG_CHANGE_WORKMODE_PRE,\
                        /*01*/    NO_MSG,\
                        /*02*/    MSG_CHANGE_WORKMODE_PRE,\
                        /*03*/    MSG_VOL_UP_SHORT,\
                        /*04*/    MSG_VOL_DOWN_SHORT,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define IOKEY_ALARM_LONG		\
                        /*00*/    MSG_RTC_MODE,\
                        /*01*/    NO_MSG,\
                        /*02*/    MSG_RTC_MODE,\
                        /*03*/    MSG_VOL_UP,\
                        /*04*/    MSG_VOL_DOWN,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,


#define IOKEY_ALARM_HOLD		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    MSG_VOL_UP,\
                        /*04*/    MSG_VOL_DOWN,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define IOKEY_ALARM_LONG_UP	\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    MSG_VOL_UP_HOLD_UP,\
                        /*04*/    MSG_VOL_DOWN_HOLD_UP,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

const u16 task_alarm_io_table[4][KEY_REG_IO_MAX] = {
    /*短按*/	    {IOKEY_ALARM_SHORT},
    /*长按*/		{IOKEY_ALARM_LONG},
    /*连按*/		{IOKEY_ALARM_HOLD},
    /*长按抬起*/	{IOKEY_ALARM_LONG_UP},
};

/*******************************************************************
                            IR按键表
*******************************************************************/
#define IRFF00_ALARM_KEY_SHORT			\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG

#define IRFF00_ALARM_KEY_LONG			\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG

#define IRFF00_ALARM_KEY_HOLD			\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG

#define IRFF00_ALARM_KEY_LONG_UP 		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG
const u16 task_alarm_ir_table[4][KEY_REG_IR_MAX] = {
    /*短按*/	    {IRFF00_ALARM_KEY_SHORT},
    /*长按*/		{IRFF00_ALARM_KEY_LONG},
    /*连按*/		{IRFF00_ALARM_KEY_HOLD},
    /*长按抬起*/	{IRFF00_ALARM_KEY_LONG_UP},
};

/*******************************************************************
                            touchkey按键表
*******************************************************************/
#define TOUCHKEY_ALARM_SHORT		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define TOUCHKEY_ALARM_LONG		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define TOUCHKEY_ALARM_HOLD		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define TOUCHKEY_ALARM_LONG_UP	\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

const u16 task_alarm_touch_table[4][KEY_REG_TOUCH_MAX] = {
    /*短按*/	    {TOUCHKEY_ALARM_SHORT},
    /*长按*/		{TOUCHKEY_ALARM_LONG},
    /*连按*/		{TOUCHKEY_ALARM_HOLD},
    /*长按抬起*/	{TOUCHKEY_ALARM_LONG_UP},
};

/*******************************************************************
                            按键总驱动表
*******************************************************************/
const KEY_REG task_alarm_key = {
#if (KEY_AD_RTCVDD_EN||KEY_AD_VDDIO_EN)
    ._ad = task_alarm_ad_table,
#endif
#if KEY_IO_EN
    ._io = task_alarm_io_table,
#endif
#if KEY_IR_EN
    ._ir = task_alarm_ir_table,
#endif
#if KEY_TCH_EN
    ._touch = task_alarm_touch_table,
#endif
#if KEY_UART_EN
    ._uart = task_alarm_io_table,
#endif
};

