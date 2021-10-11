#include "task_bt_key.h"
#include "common/msg.h"
/*******************************************************************
                            AD按键表
*******************************************************************/
#define ADKEY_BT_SHORT		\
                        /*00*/    MSG_ALARM1_SET,\
                        /*01*/    MSG_ALARM2_SET,\
                        /*02*/    MSG_BACK_LIGHT_SET,\
                        /*03*/    MSG_DIMMER,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    MSG_CHANGE_WORKMODE,\
                        /*07*/    MSG_BT_PP,\
                        /*08*/    NO_MSG/*MSG_BT_CLR*/,\
                        /*09*/    NO_MSG,

#define ADKEY_BT_LONG		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG/*MSG_ALARM_STOP*/,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    MSG_BT_PP_LONG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define ADKEY_BT_HOLD		\
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

#define ADKEY_BT_LONG_UP	\
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


const u16 task_bt_ad_table[4][KEY_REG_AD_MAX] = {
    /*短按*/	    {ADKEY_BT_SHORT},
    /*长按*/		{ADKEY_BT_LONG},
    /*连按*/		{ADKEY_BT_HOLD},
    /*长按抬起*/	{ADKEY_BT_LONG_UP},
};

/*******************************************************************
                            I/O按键表
MSG_BT_TRAIN_SCAN_DEVICE\MSG_BT_TRAIN_DEVICE\MSG_BT_SEARCH_DEVICE
*******************************************************************/
#define IOKEY_BT_SHORT		\
                        /*00*/    MSG_RTC_MODE,\
                        /*01*/    NO_MSG,\
                        /*02*/    MSG_CHANGE_WORKMODE,\
                        /*03*/    MSG_VOL_UP_SHORT,\
                        /*04*/    MSG_VOL_DOWN_SHORT,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG ,

#define IOKEY_BT_LONG		\
                        /*00*/    MSG_POWER_OFF/*MSG_RTC_MODE MSG_BT_PP_LONG*/,\
                        /*01*/    NO_MSG,\
                        /*02*/    MSG_RTC_MODE,\
                        /*03*/    MSG_VOL_UP,\
                        /*04*/    MSG_VOL_DOWN,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*09*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,


#define IOKEY_BT_HOLD		\
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

#define IOKEY_BT_LONG_UP	\
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

const u16 task_bt_io_table[4][KEY_REG_IO_MAX] = {
    /*短按*/	    {IOKEY_BT_SHORT},
    /*长按*/		{IOKEY_BT_LONG},
    /*连按*/		{IOKEY_BT_HOLD},
    /*长按抬起*/	{IOKEY_BT_LONG_UP},
};

/*******************************************************************
                            IR按键表
*******************************************************************/
#define IRFF00_BT_KEY_SHORT			\
				MSG_POWER_OFF,	MSG_CHANGE_WORKMODE,	MSG_MUTE,			\
				MSG_BT_PP,		MSG_BT_PREV_FILE,		MSG_BT_NEXT_FILE,	\
				MSG_EQ_MODE,	MSG_VOL_DOWN_SHORT,		MSG_VOL_UP_SHORT,	\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG

#define IRFF00_BT_KEY_LONG			\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				MSG_BT_PP_LONG,	NO_MSG,					NO_MSG,				\
				NO_MSG,			MSG_VOL_DOWN,			MSG_VOL_UP,			\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG

#define IRFF00_BT_KEY_HOLD			\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			MSG_VOL_DOWN,			MSG_VOL_UP,			\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG

#define IRFF00_BT_KEY_LONG_UP 		\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			MSG_VOL_DOWN_HOLD_UP,	MSG_VOL_UP_HOLD_UP,	\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG,				\
				NO_MSG,			NO_MSG,					NO_MSG
const u16 task_bt_ir_table[4][KEY_REG_IR_MAX] = {
    /*短按*/	    {IRFF00_BT_KEY_SHORT},
    /*长按*/		{IRFF00_BT_KEY_LONG},
    /*连按*/		{IRFF00_BT_KEY_HOLD},
    /*长按抬起*/	{IRFF00_BT_KEY_LONG_UP},
};


/*******************************************************************
                            touchkey按键表
*******************************************************************/
#define TOUCHKEY_BT_SHORT		\
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

#define TOUCHKEY_BT_LONG		\
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


#define TOUCHKEY_BT_HOLD		\
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

#define TOUCHKEY_BT_LONG_UP	\
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

const u16 task_bt_touch_table[4][KEY_REG_TOUCH_MAX] = {
    /*短按*/	    {TOUCHKEY_BT_SHORT},
    /*长按*/		{TOUCHKEY_BT_LONG},
    /*连按*/		{TOUCHKEY_BT_HOLD},

    /*长按抬起*/	{TOUCHKEY_BT_LONG_UP},
};



/*******************************************************************
                            按键总驱动表
*******************************************************************/
const KEY_REG task_bt_key = {
#if (KEY_AD_RTCVDD_EN||KEY_AD_VDDIO_EN)
    ._ad = task_bt_ad_table,
#endif
#if KEY_IO_EN
    ._io = task_bt_io_table,
#endif
#if KEY_IR_EN
    ._ir = task_bt_ir_table,
#endif
#if KEY_TCH_EN
    ._touch = task_bt_touch_table,
#endif
#if KEY_UART_EN
    ._uart = task_bt_io_table,//task_bt_touch_table,
#endif
};

