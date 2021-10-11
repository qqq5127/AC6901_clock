#ifndef __DEV_SD_H__
#define __DEV_SD_H__

#include "typedef.h"

#include "sdmmc/sd_host_api.h"
#include "dev_mg_api.h"



/*****************************
        Function Declare
*****************************/
/*----------------------------------------------------------------------------*/
/**@brief   SD/MMC power设置函数
   @param	mode：DEV_POWER_STANDBY有效
   @return	0：读成功；etc：读失败，详细见错误列表
   @note	s32 sd0_power_api(u32 mode)
*/
/*----------------------------------------------------------------------------*/
s32 sd0_power_api(u32 mode);
s32 sd1_power_api(u32 mode);


/*----------------------------------------------------------------------------*/
/**@brief    SD Host 卸载，关闭控制器，I/O恢复为普通I/O
   @param    void
   @return   0：读成功；etc：读失败，详细见错误列表
   @note	 s32 sd0_unmount_api(void)
*/
/*----------------------------------------------------------------------------*/
s32 sd0_unmount_api(void);
s32 sd1_unmount_api(void);


/*---------------------------------------------------------------------------*/
/**@brief	SD 读接口，等待读操作完成
   @param	buf：读数据缓冲区；
   @param	lba：物理地址
   @param	len：多少个扇区(sector)
   @return  0：读成功；etc：读失败，详细见错误列表
   @note    s32 sd0_read_api(u8 *buf, u32 lba, u32 len);
*/
/*---------------------------------------------------------------------------*/
s32 sd0_read_api(u8 *buf, u32 lba, u32 len);
s32 sd1_read_api(u8 *buf, u32 lba, u32 len);


/*----------------------------------------------------------------------------*/
/**@brief	SD 写接口，等待写操作完成
   @param	buf：读数据缓冲区；
   @param	lba：物理地址
   @param	len：多少个扇区(sector)
   @return  0：写成功；etc：写失败，详细见错误列表
   @note	s32 sd0_write_api(u8 *buf, u32 lba, u32 len)
*/
/*-----------------------------------------------------------------------------*/
s32 sd0_write_api(u8 *buf, u32 lba, u32 len);
s32 sd1_write_api(u8 *buf, u32 lba, u32 len);


/*----------------------------------------------------------------------------*/
/**@brief   SD Host 在线检测函数
   @param   无
   @return  3种状态：DEV_HOLD /DEV_ONLINE/DEV_OFFLINE
   @note	s32 sd0_det_api(void)
*/
/*----------------------------------------------------------------------------*/
s32 sd0_det_api(void);
s32 sd1_det_api(void);


/*----------------------------------------------------------------------------*/
/**@brief   SD Host API变量传入函数
   @param   *var_p：sd模块运行过程中使用的变量
   @param   *parm：对应controller_io/online_check_way/max_data_baud数据
   @return  var_p指针
   @note	s32 sd0_var_api(void *var_p, void *parm)
*/
/*----------------------------------------------------------------------------*/
s32 sd0_var_api(void *var_p, void *parm);
s32 sd1_var_api(void *var_p, void *parm);



/*****************************
        Function Declare
*****************************/
const struct DEV_IO *dev_reg_sd0(void *parm);
const struct DEV_IO *dev_reg_sd1(void *parm);

#endif
