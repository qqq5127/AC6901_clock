/*********************************************************************************************
    *   Filename        : sdk_cfg.h
    *   Description     : Config for Sound Box Case
    *   Author          : Bingquan
    *   Email           : bingquan_cai@zh-jieli.com
    *   Last modifiled  : 2018-4-07 15:36
    *   Copyright:(c)JIELI  2011-2018  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _CONFIG_
#define _CONFIG_
#include "includes.h"
#include "sdk_const_define.h"
/********               !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!                  ********/
//                修改配置前请认真看看这段功能互斥说明，谢谢
//由于AC692X资源有限，SDK开发目前已知会有以下一些功能不能同时使用的情况。
//1、使用后台一定不能使用芯片内部FM功能（硬件规格确定）
//2、后台，混响，TWS功能互斥，只能存在其中一种功能（RAM资源不足）
//3、独立的拍照模式跟后台不能同时打开（地址和名字需要独立初始化）
//4、开了混响无法再打开软件EQ和BLE（RAM资源不足）
//5、开了TWS 无法使用软件EQ（运算速度限制）
//6、FM不支持录音（RAM资源不足）
//7、不能同时录linein和录mic（硬件规格确定）
//8、新增一拖二（也可支持后台），但与混响，TWS功能互斥，只能存在其中一种功能（RAM资源不足）
/*******************************************************************************/
/********************************************************************************/
//------------------------------调试类配置
/********************************************************************************/
//<开启系统打印调试功能
//#define __DEBUG

#ifdef __DEBUG
//串口打印IO口选则
#define DEBUG_UART_SEL          UART0_TXPA5_RXPA6
//串口波特率选则
#define DEBUG_UART_RATE         460800    //115200
//打印函数
#define log_printf		        printf
#else
#define log_printf(...)
#endif

#define UART_UPDATA_EN          0

/********************************************************************************/
//------------------------------电源类配置
/********************************************************************************/
//<电量监测
#define SYS_LVD_EN              1
#define POWER_EXTERN_DETECT_EN  0	//外部电压电量检测，一般用于点烟器采集车载电瓶电压
//可选配置：PWR_NO_CHANGE / PWR_LDO15 / PWR_DCDC15
#define PWR_MODE_SELECT        PWR_LDO15
///蓝牙无连接自动关机计时，u16类型，0表示不自动关机
#define AUTO_SHUT_DOWN_TIME     10*2*60 //((3*60* 2)/2+10) //除以2 减去进低功耗的时间 概数
///<按键双击功能
#define KEY_DOUBLE_CLICK        0
///<电池电量低，是否切换电源输出配置
#define SWITCH_PWR_CONFIG		0

#define SYS_VDDIO_LEVEL         1
#define SYS_RTCVDD_LEVEL        1

#define SYS_LDO_REDUCE_LEVEL    2

#define LOW_POWER_NOISE_DEAL    0   //低电时底噪处理，该问题存在于VDDIO和HPVDD绑在一起的封装

/********************************************************************************/
//------------------------------音效类配置
/********************************************************************************/
//EQ config.  //more config in audio_stream.h
//EQ选则:EQ_RUN_NULL(不运行EQ) / EQ_RUN_HW (硬件EQ) / EQ_RUN_SW (软件EQ)
#define EQ_RUN_SEL              EQ_RUN_NULL//EQ_RUN_HW    /*更多音效设置请在audio_stream.h头文件配置*/
//EQ 串口在线调试使能
#define EQ_UART_DEBUG           0
//在线调EQ 串口选则.可选:UART1_TXPB0_RXPB1  /  UART1_USB_TXDP_RXDM
#define EQ_DEBUG_UART_SEL       UART1_TXPB0_RXPB1    //EQ_UART_DEBUG 为1时有效
#define EQ_DEBUG_UART_RATE      9600
//<变速变调
//注意该功能开启要考虑芯片性能， 相应提高系统频率，并且尽量不要跟其他cpu占用率高的应用同时打开
#define SPEED_PITCH_EN			0
/********************************************************************************/
//------------------------------DAC配置
/********************************************************************************/
//是否选择VCMO直推耳机
#define VCOMO_EN 	            0
#if 0
//DAC声道选择：DAC_L_R_CHANNEL / DAC_L_CHANNEL / DAC_R_CHANNEL
#define DAC_CHANNEL_SLECT       DAC_L_R_CHANNEL
#endif
//<dac差分输出
#define DAC_DIFF_OUTPUT		 	0
//<dac声道合并
#define DAC_SOUNDTRACK_COMPOUND 0
//DAC声道选择：DAC_L_R_CHANNEL / DAC_L_CHANNEL / DAC_R_CHANNEL
#if DAC_SOUNDTRACK_COMPOUND
	#define DAC_CHANNEL_SLECT       DAC_L_CHANNEL
#else
	#define DAC_CHANNEL_SLECT       DAC_L_R_CHANNEL
#endif
//<dac声道差集
#define DAC_SOUNDTRACK_SUBTRACT 0
//<通话切换成差分输出(前提是VCOMO_EN为1)
#define CALL_USE_DIFF_OUTPUT	0
//<自动mute
#define DAC_AUTO_MUTE_EN	    1
//<按键音
#define TONE_EN     	    	1
//<非0表示使用默认音量
#define SYS_DEFAULT_VOL         20
/*
//<非0表示提示音使用默认音量
#define TONE_DEFAULT_VOL        0
*/
/********************************************************************************/
//------------------------------外设类配置
/********************************************************************************/
//SD0_A	(DAT_CMD_CLK) : CHIP:PA5_PA6_PA7	FPGA:PO5_PO6_PO7
//SD0_B	(DAT_CMD_CLK) : CHIP:PB3_PB4_PB5	FPGA:PP3_PP4_PP5
//SD1_A	(DAT_CMD_CLK) : CHIP:PC3_PC4_PC5	FPGA:PQ3_PQ4_PQ5
//SD1_B	(DAT_CMD_CLK) : CHIP:PB0_PB1_PB2	FPGA:PP0_PP1_PP2
#define SDMMC0_EN               0
#define SDMMC1_EN               0
#define USB_DISK_EN             1
#define USB_PC_EN               0
//设备在线时 上电是否进音乐模式。 1：上电设备插入不进音乐模式   0：上电进入音乐模式
#define POWERUP_DEV_IGNORE      1
//不使用设备的任务是否需要关闭设备,开启要考虑设备兼容性
#define DEV_POWER_OFF_EN        0

//usb_sd引脚复用，需要测试兼容性
#define USB_SD0_MULT_EN     0   //<需要测试兼容性
#define USB_SD1_MULT_EN     0   //<需要测试兼容性

//adkey 和 sd 复用,需要测试兼容性,注意:ADKEY分压较低的电阻值不能用,会对SD卡造成影响,请确保分压大于1.8V以上
#define ADKEY_SD_MULT_EN    0   //0 不复用  1 复用sd0 2 复用sd1

#if(USB_SD0_MULT_EN == 1)||(USB_SD1_MULT_EN == 1)
#undef USB_PC_EN
#define USB_PC_EN       0
#endif



/********************************************************************************/
//------------------------------蓝牙类配置
/********************************************************************************/
#include "bluetooth/bluetooth_api.h"

///可选配置：0(普通音箱)/BT_TWS_TRANSMIT(对箱使能)
///如果仅作为单机使用，建议不开对箱宏，如果开了对箱宏而且做单机使用会占用基带，单机使用性能没不开对箱宏好
#define BT_TWS                  0//BT_TWS_TRANSMIT
#if BT_TWS
#undef  EQ_RUN_SEL
#define EQ_RUN_SEL              EQ_RUN_NULL/*这个宏不修改，其它更多音效设置请在audio_stream.h头文件配置*/
#endif

///蓝牙连接个数选择 1 /2 一拖二
#if BT_TWS
#define BT_CONNTCT_NUM             2
#define BT_TWS_LINEIN              0  //linein 转换成对箱播放
#else
#define BT_CONNTCT_NUM             1
#define BT_TWS_LINEIN              0
#endif

//蓝牙是否开启后台模式
#if (BT_CONNTCT_NUM == 2)
#define BT_BACKGROUND_EN		0
#else
#define BT_BACKGROUND_EN		0
#endif
#if (BT_BACKGROUND_EN== 0)
///<HID拍照的独立模式只支持非后台
#define BT_HID_INDEPENDENT_MODE  0
#endif
//可选配置：NORMAL_MODE/TEST_BQB_MODE/TEST_FCC_MODE/TEST_FRE_OFF_MODE/TEST_BOX_MODE/TEST_PERFOR_MODE
#define BT_MODE             NORMAL_MODE      // TEST_PERFOR_MODE

//模拟配置
#define BT_ANALOG_CFG           0
#define BT_XOSC                 0

//蓝牙晶振频偏设置 0x0~0xf//如果频偏为正，把值改大
#define BT_OSC_INTERNAL_L       0x09
#define BT_OSC_INTERNAL_R       0x09

//------------------------------蓝牙低功耗设置
//使能该功能后只能是纯蓝牙功能，没有显示功能

//可选配置：SNIFF_EN/SNIFF_TOW_CONN_ENTER_POWERDOWN_EN
#define SNIFF_MODE_CONF	       0//	SNIFF_EN
//可选配置：BT_POWER_DOWN_EN/BT_POWER_OFF_EN
#define BT_LOW_POWER_MODE      0// BT_POWER_DOWN_EN
//可选配置：BT_OSC/RTC_OSCH/RTC_OSCL/LRC_32K
#define LOWPOWER_OSC_TYPE     BT_OSC // LRC_32K
//可选配置：32768L//24000000L//32000L
#define LOWPOWER_OSC_HZ       24000000L//  32000L
//可选配置：BT_BREDR_EN/BT_BLE_EN/(BT_BREDR_EN|BT_BLE_EN)

#if BT_TWS

#undef  SNIFF_MODE_CONF
#define SNIFF_MODE_CONF         0

#undef  BT_LOW_POWER_MODE
#define BT_LOW_POWER_MODE       0

#define BLE_BREDR_MODE          (BT_BREDR_EN)//资源充足的情况，tws 可以开启ble
#else
#define BLE_BREDR_MODE          (BT_BREDR_EN)//(BT_BREDR_EN|BT_BLE_EN)//资源问题，开了ble，不能开启一拖二
#endif

#if (BLE_BREDR_MODE&BT_BLE_EN)
//可选配置：O--server ,1--client
#define BLE_GAP_ROLE            0
#endif

#define	BT_PHONE_NUMBER		    0
#define	BT_PHONE_VOL_SYNC       0
//是否需要音量同步功能
#define	BT_MUSIC_VOL_SYNC       0
//如果开了音量同步功能要根据实际情况看要不要配HAVE_VOL_CONTROL_KEY这个宏
#define	HAVE_VOL_CONTROL_KEY
//需要电量显示但是不需要通话功能
#define BT_HFP_EN_SCO_DIS		0
//播放手机自带来电提示音(前提是手机支持该功能)
#define BT_INBAND_RINGTONE		0
//对箱角色切换，连接手机的设备即为主机
#define BT_TWS_ROLE_SWITCH		1

///对耳主从同时按下配对按键才进行配对
#define    BT_TWS_SCAN_ENBLE        0
///主从连接上，同步播连接成功提示音、sync_led_scan
#define    BT_TWS_SYNC_CON_STATE_ENBLE        1
/********************************************************************************/
//------------------------------MUSIC MACRO
/********************************************************************************/
//SMP加密文件支持
#define MUSIC_DECRYPT_EN 		0
//<MP3
#define DEC_TYPE_MP3_ENABLE     1
//<SBC
#define DEC_TYPE_SBC_ENABLE     1
//<AAC
#define DEC_TYPE_AAC_ENABLE		0
//<8K_code_space
#define DEC_TYPE_WAV_ENABLE     1
//<8K_code_space
#define DEC_TYPE_FLAC_ENABLE    1
//<8K_code_space
#define DEC_TYPE_APE_ENABLE     1
//<32K_code_space
#define DEC_TYPE_WMA_ENABLE     1
//<32K_code_space
#define DEC_TYPE_F1A_ENABLE     1
//<2K_code_space
#define DEC_TYPE_NWT_ENABLE     0
//<76K_code_space
#define DEC_TYPE_AMR_ENABLE     0
//<60K_code_space
#define DEC_TYPE_M4A_ENABLE     0
//<112K_code_space
#define DEC_TYPE_DTS_ENABLE     0

/********************************************************************************/
//------------------------------FM MACRO
/********************************************************************************/
//<标准SDK
#define FM_RADIO_EN             1

//dependency
#if (FM_RADIO_EN == 1)
//<RDA5807FM
#define RDA5807                 1
//<BK1080FM
#define BK1080                  0
//<QN8035FM
#define QN8035                  0
//<芯片内部FM
#if BT_BACKGROUND_EN
//蓝牙RF和收音RF共用，使用后台不能用内部收音
#define FM_INSIDE               0
#else
#define FM_INSIDE               0
#endif
#endif

/********************************************************************************/
//------------------------------AUX MACRO
/********************************************************************************/
#define AUX_EN					1 
#define AUX_AD_ENABLE           0
//aux检测使能
#define AUX_DETECT_EN           0

#if AUX_DETECT_EN
#define AUX_DET_MULTI_AD_KEY    0     //ad_key复用aux检测
#endif

#if AUX_DET_MULTI_AD_KEY
#define ADC_AUX_IN					(((0x3dfL*30/33)+(0x3dfL*27/33))/2)
#define AUX_DIR_SET					//(AUX_IO_PORT->DIR |= AUX_IO_BIT)
#define AUX_PU_SET					//(AUX_IO_PORT->PU |= AUX_IO_BIT)
#else
#define AUX_IO_BIT					BIT(11)
#define AUX_IO_PORT					JL_PORTA
#define AUX_DIR_SET					(AUX_IO_PORT->DIR |= AUX_IO_BIT)
#define AUX_PU_SET					(AUX_IO_PORT->PU |= AUX_IO_BIT)
#define AUX_IN_CHECK				(AUX_IO_PORT->IN & AUX_IO_BIT)
#endif
/********************************************************************************/
//------------------------------REC MACRO
///********************************************************************************/
///<标准SDK录音功能
#define REC_EN              0


#if (REC_EN == 1)
///<MIC录音使能
#define MIC_REC_EN		1
#define AUX_REC_EN      1
#define FM_REC_EN       1
#define BT_REC_EN       1

#if (AUX_REC_EN == 1)  //AUX必须为数字通道
#undef AUX_AD_ENABLE
#define AUX_AD_ENABLE   1
#endif

#define REC_PLAY_EN     1
#endif

/********************************************************************************/
//------------------------------RTC MACRO
/********************************************************************************/
//标准SDK RTC时钟模式
#define RTC_CLK_EN              1

//dependency
#if (RTC_CLK_EN == 1)
//<RTC闹钟模式
#define RTC_ALM_EN              1
#endif

/********************************************************************************/
//------------------------------ECHO MACRO
/********************************************************************************/
//混响使能, 打开后可在BT/FM/MUSIC/LINEIN下开混响功能.
#define ECHO_EN                 0

#if ECHO_EN
//ECHO_EN为1时, 以下配置才有效
#define PITCH_EN                1
//单独混响模式使能
#define TASK_ECHO_EN            1

//混响一般不自动MUTE DAC
#undef DAC_AUTO_MUTE_EN
#define  DAC_AUTO_MUTE_EN       0

//混响不能开AUX的AD通道
#undef AUX_AD_ENABLE
#define AUX_AD_ENABLE   0

//混响与AUX录音有冲突，不能同时开
#undef AUX_REC_EN
#define AUX_REC_EN      0
#endif

/********************************************************************************/
//------------------------------SPDIF MACRO
/********************************************************************************/
//支持同轴，光纤输入的spdif task
#define SPDIF_EN                 0



/********************************************************************************/
//------------------------------FM TRANSMITTER MACRO
///********************************************************************************/
////FM发射器配置，点烟器用
#define FMTX_EN       0

#if (FMTX_EN)
//QN8027
#define QN8027                 	1
//QN8007
#define QN8007					0
#endif

/********************************************************************************/
//------------------------------UI MACRO
/********************************************************************************/
///<LED指示使能
#define LED_EN                  0

#define UI_ENABLE               1
#if (BT_LOW_POWER_MODE || SNIFF_MODE_CONF)            //进低功耗模式 not support ui
#undef  UI_ENABLE
#define UI_ENABLE               0
#endif
///dependency
#if (UI_ENABLE == 1)
#define UI_SEL_BY_RES_EN		0	  ///是否通过选屏电阻进行选屏
#define LED_7_EN                1     ///<led 支持
#define THEME_LEDSEG_7PIN		0
#define LED_1888_EN             0	  ///6脚数码管，点烟器一般用这个
#define LCD_128X64_EN           0	  ///点阵屏LCD
#define BT_FLASH_ICON			1	//蓝牙未连接的时候，蓝牙图标是否闪烁
#else
///都不支持
#define LED_7_EN                0
#define LED_1888_EN             0
#define LCD_128X64_EN           0
#endif

#if (LCD_128X64_EN == 1)
#define LRC_LYRICS_EN			1	  ///LRC歌词显示
#else
#define LRC_LYRICS_EN           0
#endif

//
#define MUSIC_AUTO_STANDBY_EN		0	//音乐播放任务下，是否需要自动进入休眠待机
#define FM_AUTO_STANDBY_EN			0	//FM收音任务下，是否需要自动进入休眠待机
#define BT_AUTO_STANDBY_EN			1	//BT任务下，是否需要自动进入休眠待机
#define AUX_AUTO_STANDBY_EN			0	//AUX任务下，是否需要自动进入休眠待机

#define BT_NOPLAY_STANDBY_EN		1	//蓝牙已连接，但没播放，也自动关机

#define LED_RED_BLINK				0	//是否使用红灯闪烁
#define LED_BLUE_BLINK				1	//是否使用蓝灯闪烁（双LED时，蓝灯主要用于蓝牙模式）

#define KL_PLAY_DISCONNECT			1	//长按PLAY键断开蓝牙
#define USE_DISPLAY_WAIT			0	//0:转模式时不显现"LOD"
#define USE_MUSIC_STOP				0	//是否使用STOP功能
#define USE_MULTI_REMOTE			0	//是否使用2(FF00和807F)个遥控
#define USE_TWO_PLAYMODE			1	//是否使用两个播放模式
#define USE_MUTE_FLASH				0	//是否静音时屏闪烁
#define USE_SHOW_DEVICE				0	//显示SD或USB，前提是要有显示屏
#define USE_MUTE_PALYTONE_ENABLE	0	//静音时是否能出提示音的声音
#define USE_ADD_N_FUN				0	//是否需要10+功能
#define ADD_NUBER					10	//
extern u8 ui_mode_wait_flag;
#define ALARM_TIME					30*60*2	//闹钟闹响时间，120/2=60s=1min
#define MINUTE_ALARM_BELL			9	//每次闹铃的重响时间(单位:分钟)
#define ALARM_REBELL_TIMES			7	//闹钟重响次数
#define RTC_WAKEUP_VALUE			4
#define RTC_SET_BACK_CNT		    2
#define RTC_DISPLAY_BACK_CNT		10
#define RTC_COORDINATE_BACK_CNT		10//20
#define USE_AD_CHECK_VOLTAGE		1    //用AD检测电压
#if USE_AD_CHECK_VOLTAGE
    #define AD_CHECK_VOLTAGE_CH            AD_CH_PC4
    #define AD_CHECK_VOLTAGE_IO_BIT        BIT(4)
    #define AD_CHECK_VOLTAGE_IO_PORT       JL_PORTC
#endif
#define USE_SHOW_BAT				1    //是否显示电量图标
#define USE_SHOW_BAT_1				1    //满电和低电两个图标
#define USE_SHOW_BAT_2				0    //电量图标有3 格
enum
{
	battery_low,   //3.3V
	battery_1,     //3.4V
	battery_2,     //3.5V
	battery_3,     //3.6V
	battery_4,     //3.7V
	battery_5,     //3.8V
	battery_6,     //3.9V
	battery_7,     //4.0V
	battery_8,     //4.1V
	battery_full   //4.2V
};
#define USE_AD_TUNER_VOLUME            0    //是否用电位器调音量
#if USE_AD_TUNER_VOLUME
    #define AD_TUNER_VOL_CH         AD_CH_PC3
    #define AD_TUNER_VOL_IO_BIT     BIT(3)
    #define AD_TUNER_VOL_IO_PORT    JL_PORTC
#endif

#define LOWPOWER_WARNING_VOLTAGE	170//360
#define LOWPOWER_VOLTAGE			160//350

//开机音使能
//<非0表示提示音使用默认音量
#define TONE_DEFAULT_VOL        	8//15//20

#define WARNING_POWER_ON			1	//开机提示音
#define WARNING_SD_USB				0	//是否需要播放SD或USB提示音
#define WARNING_MUSIC				1	//
#define WARNING_TASK_BT             1   //蓝牙任务提示音
#define WARNING_TASK_CLOCK          0   //时钟任务提示音
#define WARNING_TASK_AUX            1   //AUX任务提示音
#define WARNING_TASK_FM             1   //FM任务提示音
//#define WARNING_TASK_VOICE          1   //录音播放任务提示音
//#define WARNING_TASK_SPEAKER        1   //扩音器任务提示音
//#define WARNING_TASK_MIC            1   //MIC录音任务提示音
#define WARNING_TASK_USBDEV         1   //PC任务提示音
#define WARNING_BT_CONNECT          1   //蓝牙连接提示音
#define WARNING_BT_DISCONNECT       1   //蓝牙断开连接提示音
#define WARNING_LOWPOWER			0	//低电提示（不关机）
#define WARNING_POWER_OFF           0   //关机或低电关机提示音
#define WARNING_VOL_MIN				1	//是否需要音量调节到最小时提示音
#define WARNING_VOL_MAX				1	//是否需要音量调节到最大时提示音
#define WARNING_VOL_ONCE			0	//提示音只出一次
//
#define USE_16_LEVEL_VOLUME			1	//是否使用16级音量
#if USE_16_LEVEL_VOLUME
#define MAX_SYS_VOL_TEMP			30//32
extern u8 volume_temp;
extern const u8 volume_table[MAX_SYS_VOL_TEMP+1];
extern const u8 volume_table_2[MAX_SYS_VOL_TEMP+1];
#define WARNING_ALM_VOLUME_START	15    //闹钟的音量
#define USE_TWO_VOLUME_TABLE        1
#else
//#define MAX_SYS_VOL_TEMP			30
#define WARNING_ALM_VOLUME_START	30    //闹钟的音量
#endif
enum
{
    AUTO_TIME_5,
    AUTO_TIME_15,
    AUTO_TIME_30,
    AUTO_TIME_45,
    AUTO_TIME_60,
    AUTO_TIME_90,
    AUTO_TIME_120,
    AUTO_TIME_OFF,
    AUTO_TIME_END,
};

//功放控制
#define MUTE_TYPE					0	//0:低MUTE，1:高MUTE
#define MUTE_IO_BIT					BIT(13)
#define MUTE_IO_PORT				JL_PORTA
//
#define MUTE_2_IO_BIT				BIT(2)
#define MUTE_2_IO_PORT				JL_PORTC
#if MUTE_TYPE
#define AMP_CHECK()					((MUTE_IO_PORT->OUT & MUTE_IO_BIT))
#define AMP_MUTE()					if (!AMP_CHECK()){MUTE_IO_PORT->DIR &=~ MUTE_IO_BIT;MUTE_IO_PORT->OUT |= MUTE_IO_BIT;}
#define AMP_UNMUTE()				if (AMP_CHECK()){MUTE_IO_PORT->DIR &=~ MUTE_IO_BIT;MUTE_IO_PORT->OUT &=~ MUTE_IO_BIT;}
#define AMP_INIT()					{MUTE_IO_PORT->DIR &=~ MUTE_IO_BIT;MUTE_IO_PORT->OUT |= MUTE_IO_BIT;}
#define AMP_UNMUTE_CTL()			{MUTE_IO_PORT->DIR &=~ MUTE_IO_BIT;MUTE_IO_PORT->OUT &=~ MUTE_IO_BIT;}
#else
//#define AMP_CHECK()					(!(MUTE_IO_PORT->OUT & MUTE_IO_BIT))
//#define AMP_MUTE()					if (!AMP_CHECK()){MUTE_IO_PORT->DIR &=~ MUTE_IO_BIT;MUTE_IO_PORT->OUT &=~ MUTE_IO_BIT;}
//#define AMP_UNMUTE()				if (AMP_CHECK()){MUTE_IO_PORT->DIR &=~ MUTE_IO_BIT;MUTE_IO_PORT->OUT |= MUTE_IO_BIT;}
//#define AMP_INIT()					{MUTE_IO_PORT->DIR &=~ MUTE_IO_BIT;MUTE_IO_PORT->OUT &=~ MUTE_IO_BIT;}
//#define AMP_UNMUTE_CTL()			{MUTE_IO_PORT->DIR &=~ MUTE_IO_BIT;MUTE_IO_PORT->OUT |= MUTE_IO_BIT;}
#endif
extern void AMP_UNMUTE_CTL(void);
extern void AMP_MUTE(void);
extern void AMP_UNMUTE(void);
extern void AMP_INIT(void);

//AB/D类
#define AB_D_IO_BIT					//BIT(2)
#define AB_D_IO_PORT				//JL_PORTC
#define AMP_D()						//{AB_D_IO_PORT->DIR &=~ AB_D_IO_BIT;AB_D_IO_PORT->OUT |= AB_D_IO_BIT;}
#define AMP_AB()					//{AB_D_IO_PORT->DIR &=~ AB_D_IO_BIT;AB_D_IO_PORT->OUT &=~ AB_D_IO_BIT;}

//升压
#define BOOST_IO_BIT				//BIT(5)
#define BOOST_IO_PORT				//JL_PORTC
#define BOOST_ENABEL()				//{BOOST_IO_PORT->DIR &=~ BOOST_IO_BIT;BOOST_IO_PORT->OUT |= BOOST_IO_BIT;}
#define BOOST_DISABEL()				//{BOOST_IO_PORT->DIR &=~ BOOST_IO_BIT;BOOST_IO_PORT->OUT &=~ BOOST_IO_BIT;}

#define BACKLIGHT_BIT				//BIT(1)
#define BACKLIGHT_PORT				//JL_PORTA
#define BACKLIGHT_ENABLE()			//{BACKLIGHT_PORT->DIR &=~ BACKLIGHT_BIT;BACKLIGHT_PORT->OUT |= BACKLIGHT_BIT;}
#define BACKLIGHT_DISABLE()			//{BACKLIGHT_PORT->DIR &=~ BACKLIGHT_BIT;BACKLIGHT_PORT->OUT &=~ BACKLIGHT_BIT;}

enum
{
    BACK_LIGHT_0,
    BACK_LIGHT_1,
    BACK_LIGHT_2,
    BACK_LIGHT_3,
    BACK_LIGHT_4,
    BACK_LIGHT_5,
};

#define SOFT_POWER_ON_OFF_INSIDE	1		//内置软开关机
#if SOFT_POWER_ON_OFF_INSIDE
#define clr_PINR_ctl_soft()			//{rtc_module_port_4s_reset(PORTR2 , 0 , 0 );}
#endif

#define SOFT_POWER_ON_OFF			0		//外置软开关机
#define USE_IOKEY_POWER_ON			0
#if SOFT_POWER_ON_OFF			//是外部软关机
	#define CTL_IO_BIT						BIT(5)
	#define CTL_IO_PORT						JL_PORTC
	#define SOFT_POWER_CTL_ON()				CTL_IO_PORT->OUT |= CTL_IO_BIT
	#define SOFT_POWER_CTL_OFF()			CTL_IO_PORT->OUT &=~ CTL_IO_BIT
	#define SOFT_POWER_CTL_INIT()			{CTL_IO_PORT->DIR &=~ CTL_IO_BIT; SOFT_POWER_CTL_OFF();}

	#if USE_IOKEY_POWER_ON
		#define CHK_IO_BIT					BIT(8)
		#define CHK_IO_PORT					JL_PORTC
		#define SOFT_POWER_CHK_INIT()		{CHK_IO_PORT->OUT |= CHK_IO_BIT;CHK_IO_PORT->PU |= CHK_IO_BIT;}
		#define SOFT_POWER_CHK_IS_ON()		(!(CHK_IO_PORT->IN & CHK_IO_BIT))
	#else
		#define SOFT_POWER_CHK_INIT()
		#define SOFT_POWER_CHK_IS_ON()		(1)
	#endif
#else
	#define SOFT_POWER_CTL_INIT()
	#define SOFT_POWER_CTL_ON()
	#define SOFT_POWER_CTL_OFF()
	#define SOFT_POWER_CHK_INIT()
	#define SOFT_POWER_CHK_IS_ON()			(1)
#endif

#define DCIN_DECT_IO			1
#if DCIN_DECT_IO
#define DCIN_SLEEP				0		//关机插入充电是否需要进入假关机模式
#define DCIN_BIT				BIT(3)
#define DCIN_PORT				JL_PORTC
#define DCIN_INIT()				DCIN_PORT->PU &=~ DCIN_BIT;DCIN_PORT->PD &=~ DCIN_BIT;DCIN_PORT->DIR |= DCIN_BIT;//PORTR_DIR(PORTR1, 1);PORTR_PU(PORTR1, 0);PORTR_PD(PORTR1, 0);
#define IS_DCIN()				(DCIN_PORT->IN & DCIN_BIT)//(PORTR_IN(PORTR1))
#define clr_PINR1_ctl()			//{rtc_module_port_4s_reset(PORTR1 , 0 , 0 );}
#else
#define DCIN_INIT()
#define IS_DCIN()				0
#define clr_PINR1_ctl()
#endif

#if SOFT_POWER_ON_OFF_INSIDE
#define POWER_KEY				PORTR2

#define POWER_KEY_INIT()        do{PORTR_PU(POWER_KEY,1);PORTR_PD(POWER_KEY,0);PORTR_DIR(POWER_KEY,1);\
  							      }while(0)
#define IS_POWER_KEY_DOWN()    	(!PORTR_IN(POWER_KEY))
#elif SOFT_POWER_ON_OFF
#define POWER_KEY_INIT()
#define IS_POWER_KEY_DOWN()    	SOFT_POWER_CHK_IS_ON()
#else
#define POWER_KEY_INIT()
#define IS_POWER_KEY_DOWN()    	1
#endif
/********************************************************************************/
//------------------------------系统时钟等配置
/********************************************************************************/
//时钟配置  //more config in clock_interface.h
#define OSC_Hz                  24000000L	//fpga:12M / chip:24M

///<SYS_CLK   //不同工作状态的系统时钟选则
#define BT_CALL_Hz		        160000000L	//phone call clock
#define BT_REC_Hz		        192000000L	//bt rec clock

#if (EQ_RUN_SEL == EQ_RUN_SW || SPEED_PITCH_EN || ECHO_EN )    //Software EQ need Run 192M
#define MUSIC_DECODE_Hz         192000000L
#define SYS_Hz		            192000000L  //120000000L//96000000L
#else
#define MUSIC_DECODE_Hz         160000000L
#if BT_TWS
#define SYS_Hz		            192000000L
#else
#define SYS_Hz		            120000000L
#endif
#endif


/********************************************************************************/
//------------------------------有冲突的宏处理
/********************************************************************************/
//USB口用于调试时, 关闭USB_DISK/PC功能.
#if ( (defined(__DEBUG) && (DEBUG_UART_SEL == UART1_USB_TXDP_RXDM)) || \
       ( EQ_UART_DEBUG && (EQ_DEBUG_UART_SEL == UART1_USB_TXDP_RXDM) ))
#undef  USB_DISK_EN
#undef  USB_PC_EN
#define USB_DISK_EN             0
#define USB_PC_EN               0
#endif

//调试用的串口 和 EQ在线调试串口冲突
#if ( (defined(__DEBUG) && (DEBUG_UART_SEL == UART1_USB_TXDP_RXDM)) && \
       ( EQ_UART_DEBUG && (EQ_DEBUG_UART_SEL == UART1_USB_TXDP_RXDM) ))
#error "DEBUG_UART_SEL same with EQ_DEBUG_UART_SEL"
#endif

//没有内部收音也没有外部收音时不定义收音模式
#if ( (RDA5807 == 0 ) && (BK1080 == 0 ) && (QN8035 == 0) && (FM_INSIDE==0) )
#undef FM_RADIO_EN
#define FM_RADIO_EN             0
#endif

//没有FM发射芯片的时候不开点烟器的宏
#if ( (QN8007 == 0 ) && (QN8027 == 0 ) )
#undef FMTX_EN
#define FMTX_EN       0
#endif

//点烟器不需要FM收音模式
#if (FMTX_EN == 1)
#undef FM_RADIO_EN
#define FM_RADIO_EN             0
#endif

#if (ECHO_EN == 1)
#undef  DAC_AUTO_MUTE_EN
#define DAC_AUTO_MUTE_EN		0
#endif

#if (SNIFF_MODE_CONF == SNIFF_EN)
#undef  DEV_POWER_OFF_EN
#define DEV_POWER_OFF_EN    1
#endif

#endif  //end of _SDK_CFG_
