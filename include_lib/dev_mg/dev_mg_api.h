#ifndef __DEV_MG_API_H__
#define __DEV_MG_API_H__

#include "ioctl.h"
#include "common/list.h"


//设备错误代码
typedef enum __dev_err {
    DEV_ERR_NONE = 0,

    DEV_ERR_INIT_ALREADY,
    DEV_ERR_INIT_NOTYET,

    DEV_ERR_MEM_INIT,
    DEV_ERR_MEM_FULL,

    DEV_ERR_NOT_MOUNT,
    DEV_ERR_OVER_CAPACITY,

    DEV_ERR_UNKNOW_CLASS,

    DEV_ERR_NOT_READY,//设备已经在线，但是没初始化完成
    DEV_ERR_LUN,

    DEV_ERR_TIMEOUT,
    DEV_ERR_CMD_TIMEOUT,
    DEV_ERR_READ_TIMEOUT,//0x08
    DEV_ERR_WRITE_TIMEOUT,

    DEV_ERR_OFFLINE,//0x0a

    DEV_ERR_CRC,
    DEV_ERR_CMD_CRC,
    DEV_ERR_READ_CRC,
    DEV_ERR_WRITE_CRC,

    DEV_ERR_CONTROL_STALL,
    DEV_ERR_RXSTALL,//0x10
    DEV_ERR_TXSTALL,
    DEV_ERR_CONTROL,

    DEV_ERR_NOT_STORAGE,
    DEV_ERR_INVALID_PATH,
    DEV_ERR_INVALID_DATA,
    DEV_ERR_OUTOFMEMORY,
    DEV_ERR_HANDLE_FREE,
    DEV_ERR_INVALID_HANDLE,//24
    DEV_ERR_INVALID_BUF,
    DEV_ERR_INUSE,
    DEV_ERR_NO_READ,
    DEV_ERR_NO_WRITE,
    DEV_ERR_NO_IOCTL,
    DEV_ERR_NO_POWER,
    DEV_ERR_NOT_EXIST,
    DEV_ERR_UNKNOW,
    DEV_ERR_FULL,
} DEV_ERR;

//设备类型
typedef enum _dev_type {
    DEV_NOR_FLASH = 	BIT(0),

    DEV_SDCRAD_0 = 		BIT(1),
    DEV_SDCRAD_1 = 		BIT(2),

    DEV_UDISK_F0 = 		BIT(3),

    DEV_USB_SLAVE = 	BIT(4),

    DEV_STORAGE = 		BIT(5),
    DEV_LOGIC_DISK = 	BIT(6),

    DEV_ALL = 			0xFFFFFFFF
} DEV_TYPE;


#if USED_OS
struct dev_mutex {
    OS_MUTEX *write_mutex;
    OS_MUTEX *read_mutex;
};
#endif

typedef struct DEV_IO {
    const char name[8];							//设备名字
    s32(*mount)(void *parm);					//设备挂载，成功返回0，错误返回错误码
    s32(*unmount)(void);						//设备取消挂载，成功返回0，错误返回错误码
    s32(*read)(u8 *buf, u32 addr, u32 len);		//设备读，成功返回读取长度，错误返回错误码
    s32(*write)(u8 *buf, u32 addr, u32 len);	//设备写，成功返回写长度，错误返回错误码
    s32(*ioctrl)(void *parm, u32 cmd);			//设备控制
    s32(*power)(u32 mod);						//设备电源管理
    s32(*detect)(void);							//设备状态检测
    s32(*var_init)(void *var_p, void *parm);

#if USED_OS
    struct dev_mutex *mutex;
#endif

    DEV_TYPE device_type;
} DEV_IO_T;

//设备状态：
typedef enum dev_sta {
    DEV_OFFLINE 			= 0,	//设备从在线切换到离线。
    DEV_ONLINE 				= 1,	//设备从离线切换到在线。
    DEV_HOLD				= 2,	//其他值表示设备状态未改变。
    DEV_POWER_ON			= 3,	//开机
    DEV_POWER_OFF			= 4,	//关机
    DEV_POWER_STANDBY		= 5,  	//待机
    DEV_POWER_WAKEUP		= 6,  	//唤醒
    DEV_STA_ALL				= 0xFFFFFFFF
} DEV_STA_T;

//头字符信息
#define DEV_GENERAL_MAGIC		0xe0

//每个设备必须支持的命令
#define DEV_GET_STATUS			_IOR(DEV_GENERAL_MAGIC,0xe0,u32)    //获取设备状态，在线、离线、power_on、power_off、standby、……

//下面两个命令当设备不支持是返回 -ENOTTY
#define DEV_GET_BLOCK_SIZE      _IOR(DEV_GENERAL_MAGIC,0xe1,u32)    //获取存储设备的块大小
#define DEV_GET_BLOCK_NUM       _IOR(DEV_GENERAL_MAGIC,0xe2,u32)    //获取存储设备的块总数

#define DEV_SET_REMAIN          _IOW(DEV_GENERAL_MAGIC,0xe3,u32)    //读取存储设备剩余sector



typedef struct __dev_list_t *DEV_HANDLE;



/*******************************

   		FUNCTION

*******************************/
//base_function
char *dev_get_name_by_handle(DEV_HANDLE dev_it);
DEV_HANDLE dev_get_handle_by_name(const char *name);

//init/close_function		usage:manage_init->reg_dev->detect->mount->io_ctrl_function->unmount
u32 dev_need_buf(u8 dev_max_num);
s32 dev_mg_init(u32 dev_max_num, void *need_buf);
s32 dev_register(const DEV_IO_T *dev_io);
DEV_HANDLE dev_open(const char *name, void *parm);
s32 dev_mount(DEV_HANDLE dev_it, void *parm);
s32 dev_unmount(DEV_HANDLE hdev);


//io_ctrl_function
s32 dev_io_ctrl(DEV_HANDLE hdev, u32 cmd, void *parm);
s32 dev_power_ctrl(DEV_HANDLE dev_it, u32 parm);
s32 dev_read(DEV_HANDLE hdev, u8 *buf, u32 addr, u32 len);
s32 dev_write(DEV_HANDLE hdev, u8 *buf, u32 addr, u32 len);
s32 dev_get_phydev_total(DEV_TYPE type, DEV_STA_T status);
s32 dev_detect(DEV_HANDLE hdev, u32 *parm);
s32 dev_get_online_status(DEV_HANDLE hdev, DEV_STA_T *parm);//0:offline 1:online

//device list circles
DEV_HANDLE dev_get_fisrt(DEV_TYPE type, DEV_STA_T status);
DEV_HANDLE dev_get_last(DEV_TYPE type, DEV_STA_T status);
DEV_HANDLE dev_get_next(DEV_HANDLE curr_hdev, DEV_TYPE type, DEV_STA_T status);
DEV_HANDLE dev_get_prev(DEV_HANDLE curr_hdev, DEV_TYPE type, DEV_STA_T status);

s32 dev_dpt_info_init(DEV_HANDLE hdev, u32 total, u32 *addr_arr);
s32 dev_dpt_info_del(DEV_HANDLE hdev);
s32 dev_part_get_base_addr(DEV_HANDLE hdev, u32 index, u32 *addr_p);
s32 dev_get_specific_part_total(DEV_HANDLE hdev, u32 *total_p);
s32 dev_get_all_dev_part_total(DEV_TYPE type, u32 *total_p);

s32 dev_rm_specific_part(DEV_HANDLE hdev, u32 index);
s32 dev_refurbish_part(DEV_HANDLE hdev);

//dev_mem_info
void dev_mem_info(void);

s32 get_dev_mg_stats(void);

#define DEVICE_REG(dev,parm) dev_register(dev_reg_##dev((parm)))

#endif
