#include "task_idle_key.h"
#include "common/msg.h"/*******************************************************************
                            AD按键表
*******************************************************************/

#define ADKEY_IDLE_SHORT		\
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

#define ADKEY_IDLE_LONG		\
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

#define ADKEY_IDLE_HOLD		\
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

#define ADKEY_IDLE_LONG_UP	\
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

const u16 task_idle_ad_table[4][KEY_REG_AD_MAX] = {
    /*短按*/	    {ADKEY_IDLE_SHORT},
    /*长按*/		{ADKEY_IDLE_LONG},
    /*连按*/		{ADKEY_IDLE_HOLD},
    /*长按抬起*/	{ADKEY_IDLE_LONG_UP},
};

/*******************************************************************
                            I/O按键表
*******************************************************************/
#define IOKEY_IDLE_SHORT		\
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

#define IOKEY_IDLE_LONG		\
                        /*00*/    MSG_IDLE_POWER_OFF,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,


#define IOKEY_IDLE_HOLD		\
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

#define IOKEY_IDLE_LONG_UP	\
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

const u16 task_idle_io_table[4][KEY_REG_IO_MAX] = {
    /*短按*/	    {IOKEY_IDLE_SHORT},
    /*长按*/		{IOKEY_IDLE_LONG},
    /*连按*/		{IOKEY_IDLE_HOLD},
    /*长按抬起*/	{IOKEY_IDLE_LONG_UP},
};

/*******************************************************************
                            IR按键表
*******************************************************************/
#define IRFF00_IDLE_KEY_SHORT			\
				MSG_IDLE_POWER_OFF,	NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG

#define IRFF00_IDLE_KEY_LONG			\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG

#define IRFF00_IDLE_KEY_HOLD			\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG

#define IRFF00_IDLE_KEY_LONG_UP 		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG,		\
				NO_MSG,				NO_MSG,			NO_MSG
const u16 task_idle_ir_table[4][KEY_REG_IR_MAX] = {
    /*短按*/	    {IRFF00_IDLE_KEY_SHORT},
    /*长按*/		{IRFF00_IDLE_KEY_LONG},
    /*连按*/		{IRFF00_IDLE_KEY_HOLD},
    /*长按抬起*/	{IRFF00_IDLE_KEY_LONG_UP},
};


/*******************************************************************
                            touchkey按键表
*******************************************************************/
#define TOUCHKEY_IDLE_SHORT		\
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

#define TOUCHKEY_IDLE_LONG		\
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


#define TOUCHKEY_IDLE_HOLD		\
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

#define TOUCHKEY_IDLE_LONG_UP	\
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

const u16 task_idle_touch_table[4][KEY_REG_TOUCH_MAX] = {
    /*短按*/	    {TOUCHKEY_IDLE_SHORT},
    /*长按*/		{TOUCHKEY_IDLE_LONG},
    /*连按*/		{TOUCHKEY_IDLE_HOLD},
    /*长按抬起*/	{TOUCHKEY_IDLE_LONG_UP},
};

/*******************************************************************
                            按键总驱动表
*******************************************************************/
const KEY_REG task_idle_key = {
#if (KEY_AD_RTCVDD_EN||KEY_AD_VDDIO_EN)
    ._ad = task_idle_ad_table,
#endif
#if KEY_IO_EN
    ._io = task_idle_io_table,
#endif
#if KEY_IR_EN
    ._ir = task_idle_ir_table,
#endif
#if KEY_TCH_EN
    ._touch = task_idle_touch_table,
#endif
#if KEY_UART_EN
    ._uart = task_idle_io_table,
#endif
};

