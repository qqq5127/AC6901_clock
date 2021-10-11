/*******************************************************************************************
File Name: USB_Host_API.h

Version: 1.00

Discription

Author: Caibingquan

Email:  change.tsai@gmail.com

Date:2012.02.06

Copyright:(c) 2012 , All Rights Reserved.
*******************************************************************************************/
#ifndef _USB_HOST_API_H_
#define _USB_HOST_API_H_

#include "typedef.h"
#include "uart.h"

/*USB模块打印*/

//#define USB_HOST_PUTS
#ifdef USB_HOST_PUTS
#include "uart.h"
#define husb_puts           puts
#define husb_printf         printf
#define husb_put_u8hex      put_u8hex
#define husb_printf_buf     printf_buf
#define husb_put_u32hex0    put_u32hex
#else
#define husb_puts(...)
#define husb_printf(...)
#define husb_put_u8hex(...)
#define husb_printf_buf(...)
#define husb_put_u32hex0(...)
#endif


/*for usb slave mass*/
typedef union __U32_U8 {
    u32 dWord;
    u8  aByte[4];
} U32_U8_;

typedef struct _USB_HOST_SCSI_CBW {
    u32 dCBWSignature;          //[3:0]
    u32 dCBWTag;                //[7:4]
    U32_U8_  uCBWDataTransferLength; //[11:8]
    u8  bmCBWFlags;             //[12]
    u8  bCBWLUN;                //[13]
    u8  bCBWLength;             //[14]
    //COMMAND_BLOCK_FORMAT  CBWCB;              //[30:15]CBW
    u8  operationCode;
    u8  LUN;            //<Logical Unit Number
    u8  LBA[4];           //<Logical Block Address[7:31]
    u8  Reserved;
    u8  LengthH;         //<Transfer or Parameter List or Allocation Length
    u8  LengthL;
    u8  XLength;        //<
    u8  Null[6];
} USB_HOST_SCSI_CBW;

typedef struct _USB_HOST_SCSI_CSW {
    u32 dCSWSignature;          //[3:0]
    u32 dCSWTag;                //[7:4]
    U32_U8_  uCSWDataResidue;        //[11:8]
    u8  bCSWStatus;             //[12]
} USB_HOST_SCSI_CSW;





/*枚举失败列表*/
enum {
    USB_SUCC = 0x0,
    USB_ERR_NOT_MOUNTED,            ///<设备未枚举
    USB_ERR_OVER_CAPACITY,          ///<读容量超出范围
    USB_ERR_UNKNOW_CLASS,                ///<设备非MassStorage类
    USB_ERR_UNIT_NOT_READY,         ///<设别逻辑单元未准备
    USB_ERR_LUN,
    USB_ERR_BULK_RPT30,
    USB_ERR_BULK_RECEIVE,

    USB_ERR_READ = 0x10,            ///<读出错

    USB_ERR_WRITE = 0x20,           ///<写出错
    USB_ERR_WRITE_SECTOR_SIZE,


    USB_ERR_TIMEOUT = 0x30,         ///<传输超时
    USB_ERR_TIMEOUT_CONTROL,        ///<传输超时 控制传输阶段
    USB_ERR_TIMEOUT_BULK_SEND,      ///<传输超时 批量传输阶段
    USB_ERR_TIMEOUT_BULK_RECEIVE,   ///<传输超时 批量传输阶段

    USB_ERR_OFFLINE = 0x40,         ///<设备不在线
    USB_ERR_OFFLINE_CONTROL,
    USB_ERR_OFFLINE_BULK_SEND,
    USB_ERR_OFFLINE_BULK_RECEIVE,

    USB_ERR_STALL = 0x50,           ///<传输无效
    USB_ERR_STALL_CONTROL,          ///<传输无效 控制传输阶段
    USB_ERR_STALL_BULK_SEND,        ///<传输无效 批量传输阶段
    USB_ERR_STALL_BULK_RECEIVE,

    USB_ERR3 = 0x60,                ///<发送无效
    USB_ERR3_CONTROL,               ///<发送无效 控制传输阶段
};


/*****************************
        Typedef
*****************************/
typedef struct _USB_DETECT {
    volatile u8  bDevOnline;    //<设备在线标志	[0]:USB Host [1]:USB Slave
    u8  bOnlineCnt;             //<USB 在线计数器
    u8  bLastData;              //<USB 控制器已检测的值
    u8  bCurData;               //<USB 控制器当前检测的值
} sUSB_DETECT;

typedef struct _USB_HOST_VAR {
    /*-----设备公共属性-------*/
    u32 dwDevCapacity;          //<设备容量
    u32 dwDevCurLBA;            //<当前设备操作逻辑地址
    u16 wDevTimeOutMax;		    //<设备操作超时上限
    volatile u16 wDevTimeOut;   //<设备操作超时计数
    u8  bDevOperation;          //<设备操作状态
    u8  bDevStatus;			    //<设备状态
    volatile u8 bDevBusy;	    //<设备繁忙标记
    u8  bDevError;              //<设备操作错误

    /*-----设备特有属性-------*/
    u8  bDevSectorPerLBA;       //<设备逻辑地址Sector大小 Logical block sector size (SectorPerLBA * 512)
    u8  bDevCurOffset;          //<设备操作逻辑地址偏移 Logical Block Sector offset
    u8  bDevTransFail;          //<Bulk-Only 传输出错标志
    u8  bClass;                 //<[0]:MSD [1]:Audio [2]:HID

    /* ------为兼容以前接口，不在此更改结构体------ */
    // u16 idVender;				//厂商ID
    // u16 idProduct;				//产品ID
} sUSB_HOST_VAR;

typedef struct _USB_MSD_CLASS_INFO {
    u16 wMaxPacketSize;
    u8  bInterfaceNumber;
    u8  bDevAddr;		        //<设备地址
    u8  bDevEpIn;               //<Bulk Only 传输输入端
    u8  bDevEpOut;              //<Bulk Only 传输输出端
} sUSB_MSD_CLASS_INFO;

typedef struct _USB_CONTROL_TRANSFER {
    u8 bTokens;		    //<令牌包类型
    u8 bDatalen;	    //<数据阶段所需传输的字节数
    u8 bMaxPktSize;     //<设备接收最大数据包长度
    u8 bEpNum;          //<所使用的端点数（不包括端点0）

    u8 *pBuf;           //<接受EP0 FIFO buffer
} sUSB_CONTROL_TRANSFER;

typedef struct _USBH_REQUEST_BYTE {	//<USB 事务
    u8 bmRequestType;	            //<[7]:传输方向 [6:5]:类型 [4:0]:接收方
    u8 bRequset;
    u8 wValueL;
    u8 wValueH;
    u8 wIndexL;
    u8 wIndexH;
    u8 wLengthL;
    u8 wLengthH;
} sUSBH_REQUEST_BYTE;

typedef union _USB_CONTROL {
    sUSBH_REQUEST_BYTE SETUP_pkt; //
    u8  aStdReq[8];             //<Standard Request FIFO
} sUSB_CONTROL;

typedef struct _USB_BULK_ONLY_TRANSFER {
    //u8 *pBuf;        //<Data 缓冲buffer
    u8 bMaxLUN;         //<
    u8 bCBWLUN;         //<
} sUSB_BULK_ONLY_TRANSFER;

/*****************************
        Typedef Mix
*****************************/
typedef struct _USB_VAR {
    sUSB_DETECT             detect;
    sUSB_HOST_VAR           host_var;
    sUSB_MSD_CLASS_INFO     msd_class_info;
    sUSB_CONTROL            control_setup;
    sUSB_CONTROL_TRANSFER   control_trans;
    sUSB_BULK_ONLY_TRANSFER bulk_only_trans;
} sUSB_VAR;

typedef union _USB_BULK_ONLY { //Bulk-Only Transport protocol
    /*端点0*/
    sUSBH_REQUEST_BYTE SETUP_PKT;
    u8 USB_EP0_FIFO[0x40];
    /*端点1*/
    USB_HOST_SCSI_CBW CBW;           //<CBW use SCSI Command Set
    USB_HOST_SCSI_CSW CSW;           //<CSW use SCSI Command Set
    u8  Ep1_FIFO[0x40];         //<Endpoint1 Buffer
} sUSB_BULK_ONLY;

// typedef struct __HUSB_REG_VAR
// {
// dev_io_t husb_io;
// OS_MUTEX husb_rwmutex;
// }HUSB_REG_VAR;









/*****************************
        Function Declare
*****************************/
/*----------------------------------------------------------------------------*/
/**@brief   USB HOST 模块参数读取与设置函数(eg.DEV_GET_BLOCK_NUM/DEV_GET_STATUS/DEV_GET_BLOCK_SIZE)
   @param	*pram:暂无
   @param	cmd:参数设置
   @return	读取参数返回
   @note	s32 usb_ioctl_api(void *parm , u32 cmd)
*/
/*----------------------------------------------------------------------------*/
s32 usb_ioctl_api(void *parm, u32 cmd);


/*----------------------------------------------------------------------------*/
/**@brief   USB HOST power设置函数
   @param	mode：DEV_POWER_STANDBY有效
   @return	0：读成功；etc：读失败，详细见错误列表
   @note	s32 usb_power_api(u32 mode)
*/
/*----------------------------------------------------------------------------*/
s32 usb_power_api(u32 mode);


/*----------------------------------------------------------------------------*/
/**@brief   USB Host 挂载(函数实现有判断保护)
   @param   *pram:暂时不用设置
   @return  0：挂载成功；etc：挂载失败,详细可见错误列表
   @note	s32 usb_host_mount_api(void *pram)
*/
/*----------------------------------------------------------------------------*/
s32 usb_host_mount_api(void *pram);


/*----------------------------------------------------------------------------*/
/**@brief   USB 主机卸载，不能执行读写操作，I/O恢复为普通I/O模式
   @param   void
   @return  0：挂载成功；etc：挂载失败,详细可见错误列表
   @note	s32 usb_host_unmount_api(void)
*/
/*----------------------------------------------------------------------------*/
s32 usb_host_unmount_api(void);


/*----------------------------------------------------------------------------*/
/**@brief   USB Host 读接口，支持多LBA Sector容量大于512B设备的读操作(函数实现有判断保护)
   @param   buf：数据缓冲区；
   @param   lba：物理地址
   @param	len：多少个扇区(sector)
   @return  0：读成功；etc：读失败，详细见错误列表
   @note	s32 usb_host_read_api(u8 *buf, u32 lba, u32 len)
*/
/*----------------------------------------------------------------------------*/
s32 usb_host_read_api(u8 *buf, u32 lba, u32 len);


/*----------------------------------------------------------------------------*/
/**@brief   USB Host 写接口，仅支持多LBA Sector容量为512B设备的写操作(函数实现有判断保护)
   @param   buf：数据缓冲区；
   @param   lba：物理地址
   @param	len：多少个扇区(sector)
   @return  0：读成功；etc：读失败，详细见错误列表
   @note	s32 usb_host_write_api(u8 *buf, u32 lba, u32 len)
*/
/*----------------------------------------------------------------------------*/
s32 usb_host_write_api(u8 *buf, u32 lba, u32 len);


/*----------------------------------------------------------------------------*/
/**@brief   USB Host 在线检测函数
   @param   无
   @return  3种状态：DEV_HOLD/DEV_ONLINE/DEV_OFFLINE
   @note	s32 usb_host_detect_api(void)
*/
/*----------------------------------------------------------------------------*/
s32 usb_host_detect_api(void);


/*----------------------------------------------------------------------------*/
/**@brief   USB Host API变量传入函数
   @param   *var_p：usb模块运行过程中使用的变量
   @param   *parm：暂无参数
   @return  var_p指针
   @note	s32 usb_var_api(void *var_p, void *parm)
*/
/*----------------------------------------------------------------------------*/
s32 usb_var_api(void *var_p, void *parm);


/*USB在线状态*/
typedef enum __USB_ONLINE_STATUS {
    NULL_USB = 0,
    USB_HOST_ON = 1,
    USB_HOST_OFF = 2,
    USB_SLAVE_ON = 3,
    USB_SLAVE_OFF = 4,
} USB_ONLINE_STATUS;


/*普通定义*/
#define USB_CHECK_CNT   20   //USB 在线检测滤波次数
#define USB_SECTOR_SIZE 512  //U盘扇区容量大小


s32 usb_host_mount(u8 retry, u16 timeout, u8 reset_delay, u8 test_delay, void *dma_buf);

USB_ONLINE_STATUS usb_online(u32 cnt);
void usb_online_status_save(USB_ONLINE_STATUS cur_status);
USB_ONLINE_STATUS usb_host_status_set(void);
u8 usb_is_online(void);
u32 usb_online_check(u32 cnt);
u32 usb_mult_online_check(u32 cnt);
u32 usb_get_capacity(void);
void usb_suspend(void);
s32 usb_host_unmount(void);
u8 usb_host_read(void *buf, u32 lba, u8 *dma_buf);
u8 usb_host_write(void *buf, u32 lba, u8 *dma_buf);
u8 usbh_msd_enum(void *dma_buf, u8 delay);
void  usb_host_status(u32 status);
void set_usb_io(void);
u8 usb_read_remain(void);
s32 usb_get_sector_unit(void);



void usb_host_g_var_init(u32 *buf);
u32 usb_host_g_var_need_buf(void);
void usb_set_read_page_size(u32 size);

void set_usb_io_suspend(void);
void set_usb_io_resume(void);

#endif /*_USB_HOST_API_H_*/
