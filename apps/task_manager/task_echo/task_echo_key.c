#include "sdk_cfg.h"
#include "task_echo_key.h"
#include "common/msg.h"

#if TASK_ECHO_EN
/*******************************************************************
                            AD按键表
*******************************************************************/
#define ADKEY_ECHO_SHORT	\
                        /*00*/    NO_MSG,\
                        /*01*/    MSG_ECHO_STOP,\
                        /*02*/    MSG_ECHO_START,\
                        /*03*/    MSG_ECHO_MUSIC_VOL_SW,\
                        /*04*/    MSG_ECHO_DEEP_SET,\
                        /*05*/    MSG_ECHO_PITCH_SET,\
                        /*06*/    MSG_ECHO_DECAY_SET,\
                        /*07*/    MSG_ECHO_MIC_VOL_SET,\
                        /*08*/    MSG_ECHO_MUSIC_VOL_SET,\
                        /*09*/    NO_MSG,

#define ADKEY_ECHO_LONG		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    MSG_ECHO_MUSIC_VOL_SW,\
                        /*04*/    MSG_ECHO_DEEP_SET,\
                        /*05*/    MSG_ECHO_PITCH_SET,\
                        /*06*/    MSG_ECHO_DECAY_SET,\
                        /*07*/    MSG_ECHO_MIC_VOL_SET,\
                        /*08*/    MSG_ECHO_MUSIC_VOL_SET,\
                        /*09*/    MSG_CHANGE_WORKMODE,

#define ADKEY_ECHO_HOLD		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    MSG_ECHO_MUSIC_VOL_SW,\
                        /*04*/    MSG_ECHO_DEEP_SET,\
                        /*05*/    MSG_ECHO_PITCH_SET,\
                        /*06*/    MSG_ECHO_DECAY_SET,\
                        /*07*/    MSG_ECHO_MIC_VOL_SET,\
                        /*08*/    MSG_ECHO_MUSIC_VOL_SET,\
                        /*09*/    NO_MSG,

#define ADKEY_ECHO_LONG_UP	\
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

const u16 task_echo_ad_table[4][KEY_REG_AD_MAX] = {
    /*短按*/	    {ADKEY_ECHO_SHORT},
    /*长按*/		{ADKEY_ECHO_LONG},
    /*连按*/		{ADKEY_ECHO_HOLD},
    /*长按抬起*/	{ADKEY_ECHO_LONG_UP},
};

/*******************************************************************
                            I/O按键表
*******************************************************************/
#define IOKEY_ECHO_SHORT		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define IOKEY_ECHO_LONG		\
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


#define IOKEY_ECHO_HOLD		\
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

#define IOKEY_ECHO_LONG_UP	\
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

const u16 task_echo_io_table[4][KEY_REG_IO_MAX] = {
    /*短按*/	    {IOKEY_ECHO_SHORT},
    /*长按*/		{IOKEY_ECHO_LONG},
    /*连按*/		{IOKEY_ECHO_HOLD},
    /*长按抬起*/	{IOKEY_ECHO_LONG_UP},
};

/*******************************************************************
                            IR按键表
*******************************************************************/
#define IRFF00_ECHO_KEY_SHORT			\
                                /*00*/    MSG_ECHO_SW,\
							    /*01*/    MSG_CHANGE_WORKMODE,\
								/*02*/    MSG_MUTE,\
								/*03*/    MSG_ECHO_MUSIC_VOL_SW,\
								/*04*/    MSG_ECHO_MUSIC_VOL_SET,\
								/*05*/    MSG_ECHO_MIC_VOL_SET,\
								/*06*/    NO_MSG,\
								/*07*/    MSG_ECHO_DECAY_SET,\
								/*08*/    MSG_ECHO_PITCH_SET,\
								/*09*/    MSG_0,\
                                /*10*/    MSG_ECHO_DEEP_SET,\
								/*11*/    NO_MSG,\
								/*12*/    MSG_1,\
                                /*13*/    NO_MSG ,\
							    /*14*/    NO_MSG,\
								/*15*/    NO_MSG,\
								/*16*/    NO_MSG,\
								/*17*/    NO_MSG ,\
								/*18*/    NO_MSG ,\
								/*19*/    NO_MSG,\
								/*20*/    NO_MSG,

#define IRFF00_ECHO_KEY_LONG			\
								/*00*/    NO_MSG,\
                                /*01*/    NO_MSG,\
								/*02*/    NO_MSG,\
								/*03*/    NO_MSG,\
								/*04*/    NO_MSG,\
								/*05*/    NO_MSG,\
								/*06*/    NO_MSG,\
								/*07*/    NO_MSG,\
								/*08*/    NO_MSG,\
								/*09*/    NO_MSG,\
                                /*10*/    NO_MSG,\
								/*11*/    NO_MSG,\
								/*12*/    NO_MSG,\
								/*13*/    NO_MSG,\
								/*14*/    NO_MSG,\
								/*15*/    NO_MSG,\
								/*16*/    NO_MSG,\
								/*17*/    NO_MSG,\
								/*18*/    NO_MSG,\
								/*19*/    NO_MSG,\
								/*20*/    NO_MSG

#define IRFF00_ECHO_KEY_HOLD			\
								/*00*/    NO_MSG,\
                                /*01*/    NO_MSG,\
								/*02*/    NO_MSG,\
								/*03*/    NO_MSG,\
								/*04*/    NO_MSG,\
								/*05*/    NO_MSG,\
								/*06*/    NO_MSG,\
								/*07*/    NO_MSG,\
								/*08*/    NO_MSG,\
								/*09*/    NO_MSG,\
                                /*10*/    NO_MSG,\
								/*11*/    NO_MSG,\
								/*12*/    NO_MSG,\
								/*13*/    NO_MSG,\
								/*14*/    NO_MSG,\
								/*15*/    NO_MSG,\
								/*16*/    NO_MSG,\
								/*17*/    NO_MSG,\
								/*18*/    NO_MSG,\
								/*19*/    NO_MSG,\
								/*20*/    NO_MSG


#define IRFF00_ECHO_KEY_LONG_UP 		\
								/*00*/    NO_MSG,\
                                /*01*/    NO_MSG,\
								/*02*/    NO_MSG,\
								/*03*/    NO_MSG,\
								/*04*/    NO_MSG,\
								/*05*/    NO_MSG,\
								/*06*/    NO_MSG,\
								/*07*/    NO_MSG,\
								/*08*/    NO_MSG,\
								/*09*/    NO_MSG,\
								/*10*/    NO_MSG,\
								/*11*/    NO_MSG,\
								/*12*/    NO_MSG,\
								/*13*/    NO_MSG,\
                                /*14*/    NO_MSG,\
								/*15*/    NO_MSG,\
								/*16*/    NO_MSG,\
								/*17*/    NO_MSG,\
								/*18*/    NO_MSG,\
								/*19*/    NO_MSG,\
								/*20*/    NO_MSG
const u16 task_echo_ir_table[4][KEY_REG_IR_MAX] = {
    /*短按*/	    {IRFF00_ECHO_KEY_SHORT},
    /*长按*/		{IRFF00_ECHO_KEY_LONG},
    /*连按*/		{IRFF00_ECHO_KEY_HOLD},
    /*长按抬起*/	{IRFF00_ECHO_KEY_LONG_UP},
};


/*******************************************************************
                            touchkey按键表
*******************************************************************/
#define TOUCHKEY_ECHO_SHORT		\
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

#define TOUCHKEY_ECHO_LONG		\
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


#define TOUCHKEY_ECHO_HOLD		\
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

#define TOUCHKEY_ECHO_LONG_UP	\
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

const u16 task_echo_touch_table[4][KEY_REG_TOUCH_MAX] = {
    /*短按*/	    {TOUCHKEY_ECHO_SHORT},
    /*长按*/		{TOUCHKEY_ECHO_LONG},
    /*连按*/		{TOUCHKEY_ECHO_HOLD},
    /*长按抬起*/	{TOUCHKEY_ECHO_LONG_UP},
};

/*******************************************************************
                            按键总驱动表
*******************************************************************/
const KEY_REG task_echo_key = {
#if (KEY_AD_RTCVDD_EN||KEY_AD_VDDIO_EN)
    ._ad = task_echo_ad_table,
#endif
#if KEY_IO_EN
    ._io = task_echo_io_table,
#endif
#if KEY_IR_EN
    ._ir = task_echo_ir_table,
#endif
#if KEY_TCH_EN
    ._touch = task_echo_touch_table,
#endif
#if KEY_UART_EN
    ._uart = NULL,
#endif
};

#endif
