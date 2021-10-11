/*******************************************************************************************
File Name: 	SD_Host_API.h

Version: 	1.00

Discription SD Host 接口函数

Author: 	Caibingquan

Email:  	change.tsai@gmail.com

Date:		2013.01.02

Copyright (c) 2010 - 2012 JieLi. All Rights Reserved.
*******************************************************************************************/
#ifndef _SD_HOST_API_H_
#define _SD_HOST_API_H_

#include "typedef.h"

typedef enum {
    SD_CONTROLLER_0 = 1, //SD0控制器
    SD_CONTROLLER_1,     //SD1控制器
} SD_CONTROLLER_TYPE;

/*SD读写函数选择*/
typedef enum {
    SD_READ_WRITE_WAIT = 1, //SD卡读写等待完成
    SD_READ_WRITE_NOT_WAIT, //SD卡读写不等待完成
} SD_RW_WAY;

/*SD控制器状态*/
typedef enum {
    SD_CTLLER_NOT_BUSY = 0,
    SD_CTLLER_BUSY,
} SD_CTLLER_STATUS;

/*SD CMD设置*/
typedef enum {
    SD_CMD_UNLOCK = 0,
    SD_CMD_LOCK,
} SD_CMD_SET;



//----------------------------------
// IO CONTROL CMD
//----------------------------------
enum {
    SDMMC_GET_VAR_ADDR = 1,
    SDMMC_GET_CTRLER_STATUS,	//获取SD控制器繁忙状态
    SDMMC_GET_CTRLER_FLAG,
    SDMMC_GET_CMD_LOCK,			//设置CMD线是否锁定
    SDMMC_GET_BUSY,				//获取SD BUSY状态
    // SDMMC_MEM_FREE,				//释放SD申请的变量空间
    SDMMC_CLOSE_CONTROLLER,		//suspend SD卡并关闭SD控制器(不再占用IO口)
    SDMMC_RELEASE_CONTROLLER,	//重新打开SD控制器和占据相应IO口
};

/*SD四线模式设置*/
typedef enum {
    SD_4WIRE_MODE = 0,
    SD_1WIRE_MODE = BIT(0),
    SD_HI_SPEED = BIT(1),
} SD_NWRITE_SPEED_MODE;

/*SD控制器对应IO出口*/
typedef enum {
    SD0_IO_A = 1, //sd0控制器A出口
    SD0_IO_B,     //sd0控制器B出口

    SD1_IO_A,     //sd1控制器A出口
    SD1_IO_B,     //sd1控制器B出口

    SD0_IO_C,     //sd0控制器C出口
} SD_CONTROLLER_IO;

/*SD在线检测方式*/
typedef enum {
    SD_CMD_CHECK = 1, //SD卡命令检测
    SD_CLK_CHECK,     //SD卡CLK检测
    SD_IO_CHECK,      //SD卡IO检测
} SD_CHECK_WAY;

typedef struct _SD_API_SET {
    u8  controller_io;     //<控制器IO出口
    u8  online_check_way;  //<sd卡在线检测方式：cmd和clk
    u8  max_data_baud;     //<数据传输最大波特率
    u8  hc_mode;           //<sd卡四线模式和高速模式设置
    u8(*io_det_func)(void);   //<io口检测函数
    u8  rw_way;               //读写函数选择：默认选择“等待读写完函数”
} sSD_API_SET __attribute__((aligned(4)));


/**---------------------enum类----------------------**/
/**---------------------------------------------------**/
/*sd调用失败列表*/
enum {
    SD_SUCC = 0x0,
    SD_ERR_NOT_MOUNTED,             ///<设备未挂载
    SD_ERR_OVER_CAPACITY,           ///<读容量超出范围
    SD_ERR_UNKNOWN_CLASS,           ///<设备未知 Class
    SD_ERR_DEV_BUSY,                ///<控制器繁忙

    SD_ERR_READ = 0x10,             ///<读出错
    SD_ERR_READ_MULTIPLE_BLOCK,

    SD_ERR_WRITE = 0x20,            ///<写出错
    SD_ERR_WRITE_MULTIPLE_BLOCK,

    SD_ERR_TIMEOUT = 0x30,          ///<传输超时
    SD_ERR_TIMEOUT_COMMAND,         ///<命令回应超时
    SD_ERR_TIMEOUT_DATA,            ///<数据传输超时
    SD_ERR_TIMEOUT_READ,            ///<传输超时 读数据阶段
    SD_ERR_TIMEOUT_WRITE,           ///<传输超时 写数据阶段

    SD_ERR_OFFLINE = 0x40,          ///<设备不在线
    SD_ERR_OFFLINE_READ,
    SD_ERR_OFFLINE_WRITE,

    SD_ERR_CRC16 = 0x50,
    SD_ERR_CRC16_COMMAND,
    SD_ERR_CRC16_DATA,
    SD_ERR_CRC16_READ,
    SD_ERR_CRC16_WRITE,

    SD_STOP_ERR = 0x60,            ///<暂停
    SD_STOP_ERR_READ,
    SD_STOP_ERR_WRITE,
    SD_IDENTIFICATION_RETRY_ERR,   ///<发送无效 控制传输阶段
};



//upboot updata use this struct , change FORBIDDEN
typedef struct _SD_NOT_CLEAR { /*在sd模块生存周期里不会被清0的变量*/
    u8  control_type;      //<控制器类型(sd0或者sd1)
    u8  control_io;        //<控制器IO出口
    u8  online_check_way;  //<sd卡在线检测方式：cmd和clk
    u8  max_data_baud;     //<数据传输最大波特率
    u16 wDevTimeOutMax;    //<设备操作超时上限
    u8  per_online_status; //<sd卡前一次在线状态变量
    u8  hc_mode;           //<sd卡四线模式和高速模式设置
    u8(*io_det_func)(void);   //<io口检测函数
} sSD_NOT_CLEAR; /*按照4个bytes对齐：共8个bytes*/






/**---------------------struct类----------------------**/
/**---------------------------------------------------**/

typedef struct _SD_HOST_VAR {
    /*-----设备公共属性-------*/
    u32 dwDevCapacity;          //<设备容量
    u32 dwDevCurLBA;            //<当前设备操作逻辑地址
    u8  bDevOperation;          //<设备操作状态
    u8  bDevStatus;
    volatile u8 bDevBusy;       //<控制器繁忙（新增volatile 声明，防止编译器对while的优化）
    u8  bDevError;              //<设备操作错误

    /*-----设备特有属性-------*/
    volatile u8  bDevOnline;
    u8  bClass;
    u8  bmFeat;                 //<SD 卡特性 [0]SDMMC Hispeed [1]SDMMC 4 wires [2]SDMMC HC
    u8  bBusWidth;              //<SD Bus Width support

    u8  bWCRCError;
    u8  bRCRCError;
    u16 wDevRCA;

    u32 dect_init_flag;
    u8  bSDSpec;
    u8	bCardReader_popup;
    u8  align_reserve[2];  //align byte
} sSD_HOST_VAR; /*按照4个bytes对齐：共28个bytes*/

typedef struct _SD_CMD_DETECT {
    u8  bCheckCnt;
    u8  bCMDStatus;
    u8  bOcState;               //<Command 检测状态机
    u8  bLockCMD;               //<Command 检测
    u8  bCheckStep;
    volatile u8 bOnceCheck;
    u8  bBaudMax;           //<SD 传输速度上限
    u8  bBaudMin;           //<SD 传输速度下限
} sSD_CMD_DETECT; /*按照4个bytes对齐：共8个bytes*/

typedef struct _SD_VAR {
    sSD_NOT_CLEAR    var_not_clr; //在sd模块生存周期里不会被清0的变量
    sSD_HOST_VAR     host_var;    //普通sd变量
    sSD_CMD_DETECT   cmd_detect;  //cmd检测变量

    // OS_SEM  sd_sem;     //SD信号量

    u8  sd_ctller_flag; //IIC复用标志位
    u8  rw_way;         //读写函数选择：默认选择“等待读写完函数”
} sSD_VAR __attribute__((aligned(4)));






/*****************************
        Function Declare
*****************************/
/*----------------------------------------------------------------------------
@brief   SD/SDIO/MMC 模块初始化函数,包括IO,寄存器初始化,不包括鉴定卡流程

   @param	sd_nwire_mode:单/四线模式选择
   @param	clk：SD CLK
   @return	无
   @note	void sd_lowlevel_init(sSD_VAR *sd_var)
----------------------------------------------------------------------------*/
void sd_lowlevel_init(sSD_VAR *sd_var);

/*----------------------------------------------------------------------------*/
/**@brief   SD Host 挂载(函数实现有判断保护)

   @param	delay：操作超时上限
   @return  0：挂载成功；etc：挂载失败,详细可见错误列表
   @note	u8  sd_mount(sSD_VAR *sd_var)
*/
/*----------------------------------------------------------------------------*/
u8  sd_mount(sSD_VAR *sd_var);

/*----------------------------------------------------------------------------*/
/**@brief   SD Host 初始化，控制器复位，变量复位

   @param	BaudMax：波特率上限
   @param	BaudMin：波特率下限
   @return  无
   @note    void sd_host_init(sSD_VAR *sd_var)
*/
/*----------------------------------------------------------------------------*/
void sd_host_init(sSD_VAR *sd_var);
/*----------------------------------------------------------------------------*/
/**@brief    设置SD 在线状态,初始化状态机，触发系统事件(函数实现有判断保护)

   @param   status：0:设置离线   1：设置在线
   @return  void
   @note	void sd_host_status(u8 status, sSD_VAR *sd_var)
*/
/*----------------------------------------------------------------------------*/
void sd_host_status(u8 status, sSD_VAR *sd_var);

/*----------------------------------------------------------------------------*/
/**@brief   SD Host 命令鉴定流程

   @return	0：鉴定成功  etc：鉴定失败，详细见错误列表
   @note	u8 sd_init_card(sSD_VAR *sd_var)
*/
/*----------------------------------------------------------------------------*/
u8 sd_init_card(sSD_VAR *sd_var);

/*----------------------------------------------------------------------------*/
/**@brief   SD Host 命令检测初始化函数
   @param   无
   @return  无
   @note	void sd_cmd_det_init(sSD_VAR *sd_var)
*/
/*----------------------------------------------------------------------------*/
void sd_cmd_det_init(sSD_VAR *sd_var);

/*----------------------------------------------------------------------------*/
/**@brief   SD Host 操作超时保护，在线保护
   @param	jiffies：计算超时起始值:由 sd_host_get_jiffies获得
   @return  0：操作正常；etc：操作失败，详细见错误列表
   @note	u8 sd_host_timeout(u32 jiffies, sSD_VAR *sd_var)
*/
/*----------------------------------------------------------------------------*/
u8 sd_host_timeout(u32 jiffies, sSD_VAR *sd_var);

/*usb读卡器接口*/
/*----------------------------------------------------------------------------*/
/**@brief   SD/MMC 模块初始化函数,包括IO,寄存器初始化,鉴定卡流程
   @param	*pram:单/四线模式设置
   @return	成功返回0,etc：设置失败，详细见错误列表
   @note	s32 sd0_mount_api(void *pram)
*/
/*----------------------------------------------------------------------------*/
s32 sd0_mount_api(void *pram);
s32 sd1_mount_api(void *pram);
/*----------------------------------------------------------------------------*/
/**@brief   SD/MMC 模块参数读取与设置函数(eg.DEV_GET_BLOCK_NUM/DEV_GET_STATUS/DEV_GET_BLOCK_SIZE)
   @param	*pram:参数设置(只定义了SD传输波特率设置)
   @param	cmd:参数设置(只定义了SD传输波特率设置)
   @return	读取参数返回
   @note	s32 sd0_mount_api(void *pram)
*/
/*----------------------------------------------------------------------------*/
s32 sd0_ioctl_api(void *parm, u32 cmd);
s32 sd1_ioctl_api(void *parm, u32 cmd);

u32 sd_cmd_check(sSD_VAR *sd_var, u32 det_delay);
void sd_chk_ctl(u8 cmd, u8 cnt, sSD_VAR *sd_var);
u8 sd_wait_idle(u8 flag, sSD_VAR *sd_var);
void sd_cmd_det(sSD_VAR *sd_var, u32 det_delay);
void sd_clk_det(sSD_VAR *sd_var, u32 det_delay);
void sd_io_det(sSD_VAR *sd_var, u32 det_delay);
s32 sd_read(u8 *buf, u32 addr, sSD_VAR *sd_var);
s32 sd_write(u8 *buf, u32 addr, sSD_VAR *sd_var);
u8 sd_host_sd_status(sSD_VAR *sd_var);
s32 sd_set_bus_width(sSD_VAR *sd_var);
u8 sd_host_switch_func(void *buf, u32 arg, sSD_VAR *sd_var);
u8 sd_host_set_hispeed(sSD_VAR *sd_var);
void sd_write_start(void *buffer, u16 len, sSD_VAR *sd_var);
void sd_read_start(void *buffer, u16 len, sSD_VAR *sd_var);
void sd_active_check(sSD_VAR *sd_var);
void sd_idle_check(sSD_VAR *sd_var);

s32 sd0_read_go(u8 *buf, u32 lba); //不等待read完成
s32 sd1_read_go(u8 *buf, u32 lba);
s32 sd0_write_go(u8 *buf, u32 lba); //不等待write完成
s32 sd1_write_go(u8 *buf, u32 lba);
u8 sd0_rw_wait_idle(u8 rw_ctl, u8 retry); //读卡器wait idle
u8 sd1_rw_wait_idle(u8 rw_ctl, u8 retry);

//controller_type function
u8 get_sdmmc_dev_status(u8 controller_type);
u8 get_sdmmc_popup_status(u8 controller_type);
u8 sd_host_identification(u8 controller_type, sSD_VAR *sd_var);
s32 sd_get_sector_unit(u8 controller_type);
u32 sd_dev_det_flag(u8 controller_type);
void sd_dev_mult_det(u8 controller_type);
sSD_VAR *sd_return_var_addr(u8 controller_type);
u32 get_sd_dev_busy_status(u8 controller_type);
void close_sd_controller(u8 controller_type);
void release_sd_controller(u8 controller_type);
u32 get_sd_ctller_status(u8 controller_type, u8 flag);
u32 sd_cmd_lock_set(u8 controller_type, u8 lock_set);
u32 sd_ctller_flag_set(u8 controller_type, u8 flag);
u32 sd_dev_ioctl(u8 controller_type, void *parm, u32 cmd);
u32 sd_dev_power(u8 controller_type, u32 mode);
u32 sd_dev_mount(u8 controller_type, void *parm);
u32 sd_dev_unmount(u8 controller_type);
u32 sd_dev_read(u8 controller_type, u8 *buf, u32 addr, u32 len);
u32 sd_dev_write(u8 controller_type, u8 *buf, u32 addr, u32 len);
u32 sd_dev_det(u8 controller_type);
u32 hook_sd_var(u8 controller_type, void *var_p, void *parm);
u32 sd_get_capacity(u8 controller_type);
u8 sd_is_online(u8 controller_type);
s32 sd_suspend(u8 controller_type);
s32 sd_init(u8 controller_type);
s32 sd_unmount(u8 controller_type);
s32 sd_read_api(u8 controller_type, u8 *buf, u32 addr, u8 *dma_pbuf);
s32 sd_write_api(u8 controller_type, u8 *buf, u32 addr, u8 *dma_pbuf);

#endif

