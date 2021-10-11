#include "task_pc_key.h"
#include "common/msg.h"/*******************************************************************
                            AD按键表
*******************************************************************/

#define ADKEY_PC_SHORT		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    MSG_PC_VOL_DOWN,\
                        /*06*/    MSG_PC_VOL_UP,\
                        /*07*/    MSG_PC_PLAY_PREV,\
                        /*08*/    MSG_PC_PLAY_NEXT,\
                        /*09*/    MSG_PC_PP,

#define ADKEY_PC_LONG		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    MSG_PC_VOL_DOWN,\
                        /*08*/    MSG_PC_VOL_UP,\
                        /*09*/    MSG_CHANGE_WORKMODE,

#define ADKEY_PC_HOLD		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    NO_MSG,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    MSG_PC_VOL_DOWN,\
                        /*08*/    MSG_PC_VOL_UP,\
                        /*09*/    NO_MSG,


#define ADKEY_PC_LONG_UP	\
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

const u16 task_pc_ad_table[4][KEY_REG_AD_MAX] = {
    /*短按*/	    {ADKEY_PC_SHORT},
    /*长按*/		{ADKEY_PC_LONG},
    /*连按*/		{ADKEY_PC_HOLD},
    /*长按抬起*/	{ADKEY_PC_LONG_UP},
};

/*******************************************************************
                            I/O按键表
*******************************************************************/
#define IOKEY_PC_SHORT		\
                        /*00*/    NO_MSG,\
                        /*01*/    NO_MSG,\
                        /*02*/    NO_MSG,\
                        /*03*/    MSG_CHANGE_WORKMODE,\
                        /*04*/    NO_MSG,\
                        /*05*/    NO_MSG,\
                        /*06*/    NO_MSG,\
                        /*07*/    NO_MSG,\
                        /*08*/    NO_MSG,\
                        /*09*/    NO_MSG,

#define IOKEY_PC_LONG		\
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



#define IOKEY_PC_HOLD		\
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


#define IOKEY_PC_LONG_UP	\
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

const u16 task_pc_io_table[4][KEY_REG_IO_MAX] = {
    /*短按*/	    {IOKEY_PC_SHORT},
    /*长按*/		{IOKEY_PC_LONG},
    /*连按*/		{IOKEY_PC_HOLD},
    /*长按抬起*/	{IOKEY_PC_LONG_UP},
};

/*******************************************************************
                            IR按键表
*******************************************************************/
#define IRFF00_PC_KEY_SHORT			\
                            /*00*/    NO_MSG,\
                            /*01*/    MSG_CHANGE_WORKMODE,\
                            /*02*/    MSG_PC_MUTE,\
                            /*03*/    MSG_PC_PP,\
                            /*04*/    MSG_PC_PLAY_PREV,\
                            /*05*/    MSG_PC_PLAY_NEXT,\
                            /*06*/    NO_MSG,\
                            /*07*/    MSG_PC_VOL_DOWN,\
                            /*08*/    MSG_PC_VOL_UP,\
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


#define IRFF00_PC_KEY_LONG			\
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


#define IRFF00_PC_KEY_HOLD			\
								/*00*/    NO_MSG,\
                                /*01*/    NO_MSG,\
								/*02*/    NO_MSG,\
								/*03*/    NO_MSG,\
								/*04*/    NO_MSG,\
								/*05*/    NO_MSG,\
								/*06*/    NO_MSG,\
								/*07*/    MSG_PC_VOL_DOWN,\
								/*08*/    MSG_PC_VOL_UP,\
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


#define IRFF00_PC_KEY_LONG_UP 		\
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
const u16 task_pc_ir_table[4][KEY_REG_IR_MAX] = {
    /*短按*/	    {IRFF00_PC_KEY_SHORT},
    /*长按*/		{IRFF00_PC_KEY_LONG},
    /*连按*/		{IRFF00_PC_KEY_HOLD},
    /*长按抬起*/	{IRFF00_PC_KEY_LONG_UP},
};


/*******************************************************************
                            touchkey按键表
*******************************************************************/
#define TOUCHKEY_PC_SHORT		\
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

#define TOUCHKEY_PC_LONG		\
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


#define TOUCHKEY_PC_HOLD		\
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

#define TOUCHKEY_PC_LONG_UP	\
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

const u16 task_pc_touch_table[4][KEY_REG_TOUCH_MAX] = {
    /*短按*/	    {TOUCHKEY_PC_SHORT},
    /*长按*/		{TOUCHKEY_PC_LONG},
    /*连按*/		{TOUCHKEY_PC_HOLD},

    /*长按抬起*/	{TOUCHKEY_PC_LONG_UP},
};

const KEY_REG task_pc_key = {
#if (KEY_AD_RTCVDD_EN||KEY_AD_VDDIO_EN)
    ._ad = task_pc_ad_table,
#endif
#if KEY_IO_EN
    ._io = task_pc_io_table,
#endif
#if KEY_IR_EN
    ._ir = task_pc_ir_table,
#endif
#if KEY_TCH_EN
    ._touch = task_pc_touch_table,
#endif
#if KEY_UART_EN
    ._uart = NULL, //task_pc_io_table,//task_pc_touch_table,
#endif
};

