#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#include "typedef.h"
#include "sdk_cfg.h"
#include "cpu/audio_param.h"

/*
*********************************************************************
*			AUDIO EFFECT TYPE (Bit definition for AUDIO_EFFECT)
*
*Note(s):(1)如果一种音效有软件和硬件两种做法，两个不能同时打开
*		 (2)HW(硬件实现),SW(软件实现)
		 (3)由于硬件同步和硬件SRC都使用了SRC模块，所以一个音效里面不
		    能同时使能
*********************************************************************
*/
#define AUDIO_EFFECT_NULL		0x00u
#define AUDIO_EFFECT_HW_EQ		0x01u	/*Hardware EQ			*/
#define AUDIO_EFFECT_SW_EQ		0x02u	/*Software EQ			*/
#define AUDIO_EFFECT_HW_SYNC	0x04u	/*Hardware Sync			*/
#define AUDIO_EFFECT_SW_SYNC	0x08u	/*Software Sync			*/
#define AUDIO_EFFECT_PITCH		0x10u	/*PitchShifter			*/
#define AUDIO_EFFECT_HW_SRC		0x20u	/*Hardware SRC			*/
#define AUDIO_EFFECT_TWS_SYNC	0x40u	/*TWS SYNC				*/

#if (EQ_RUN_SEL == EQ_RUN_HW)
#define AUDIO_EFFECT_A2DP	    (AUDIO_EFFECT_HW_SYNC | AUDIO_EFFECT_HW_EQ)
#define AUDIO_EFFECT_MUSIC	   	(AUDIO_EFFECT_HW_EQ | AUDIO_EFFECT_PITCH)
#define AUDIO_EFFECT_SCO		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_KTV		(AUDIO_EFFECT_HW_SRC)
#define AUDIO_EFFECT_PC			(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_LINEIN		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_FM			(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_ECHO		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_SPDIF	    (AUDIO_EFFECT_HW_SYNC)
#elif(EQ_RUN_SEL == EQ_RUN_SW)
#define AUDIO_EFFECT_A2DP	    (AUDIO_EFFECT_HW_SYNC | AUDIO_EFFECT_SW_EQ)
#define AUDIO_EFFECT_MUSIC	   	(AUDIO_EFFECT_SW_EQ)
#define AUDIO_EFFECT_SCO		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_KTV		(AUDIO_EFFECT_HW_SRC)
#define AUDIO_EFFECT_PC			(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_LINEIN		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_FM			(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_ECHO		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_SPDIF	    (AUDIO_EFFECT_HW_SYNC)
#else   //EQ_RUN_NULL
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
#define AUDIO_EFFECT_A2DP	    (AUDIO_EFFECT_TWS_SYNC|AUDIO_EFFECT_HW_SYNC)//对箱同步用AUDIO_EFFECT_TWS_SYNC|AUDIO_EFFECT_HW_SYNC,对箱支持软件eq,硬件eq <<软件eq支持限幅器 >>
// #define AUDIO_EFFECT_A2DP	    (AUDIO_EFFECT_TWS_SYNC|AUDIO_EFFECT_HW_SYNC|AUDIO_EFFECT_SW_EQ)//对箱支持软件eq,硬件eq <<软件eq支持限幅器 >>
#else
#define AUDIO_EFFECT_A2DP	    (AUDIO_EFFECT_HW_SYNC)
#endif //BT_TWS
#define AUDIO_EFFECT_MUSIC	   	(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_SCO		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_KTV		(AUDIO_EFFECT_HW_SRC)
#define AUDIO_EFFECT_PC			(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_LINEIN		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_FM			(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_ECHO		(AUDIO_EFFECT_NULL)
#define AUDIO_EFFECT_SPDIF	    (AUDIO_EFFECT_HW_SYNC)
#endif




#define AUDIO_EFFECT	( AUDIO_EFFECT_A2DP	| AUDIO_EFFECT_MUSIC	| AUDIO_EFFECT_SCO 	|	\
						  AUDIO_EFFECT_PC 	| AUDIO_EFFECT_LINEIN 	| AUDIO_EFFECT_FM 	| 	\
						  AUDIO_EFFECT_ECHO | AUDIO_EFFECT_KTV	)

#if ((AUDIO_EFFECT & AUDIO_EFFECT_HW_EQ) && (AUDIO_EFFECT & AUDIO_EFFECT_SW_EQ))
#error "Hardware EQ and Software EQ can't open at the same time!!!"
#endif
/*
*********************************************************************
*			AUDIO STREAM DATA STRUCT
*********************************************************************
*/
typedef struct __AUDIO_STREAM_PARAM {
    u8 ch;
    u16 sr;
    u16 ef;
} AUDIO_STREAM_PARAM;

typedef struct __AUDIO_STREAM {
    void  *priv;
    u32(*output)(void *priv, void *buf, u32 len);
    void (*clear)(void *priv);
    tbool(*check)(void *priv);
    u32(*free_len)(void *priv);
    u32(*data_len)(void *priv);
    void (*set_sr)(void *priv, u16 sr, u8 wait);
} AUDIO_STREAM;

typedef struct __AUDIO_FILE {
    void *priv;
    bool (*seek)(void *priv, u8 type, u32 offset);
    u32(*read)(void *priv, u8 *buf, u16 len);
    u32(*get_size)(void *priv);
} AUDIO_FILE;

/*
*********************************************************************
*					Function Declaration
*********************************************************************
*/
AUDIO_STREAM *audio_stream_init(AUDIO_STREAM_PARAM *param, void *priv);
s32 audio_stream_exit(AUDIO_STREAM_PARAM *param, void *priv);


#endif// __AUDIO_STREAM_H__

