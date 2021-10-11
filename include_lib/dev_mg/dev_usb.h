#ifndef __DEV_USB_H__
#define __DEV_USB_H__

#include "typedef.h"

#include "dev_mg_api.h"


/*****************************
        Function Declare
*****************************/
const struct DEV_IO *dev_reg_usb(void *parm);
const struct DEV_IO *dev_reg_usbmult(void *parm);

typedef struct  _HUSB_MOUNT_PARM {
    u16 mount_timeout;  //挂载超时时间
    u8  mount_retry;    //挂载失败重复次数
    u8 reset_delay;     //复位延时，10ms为单位
    u8 test_delay;      //Test Unit Ready 命令等待时间
} HUSB_MOUNT_PARM;

void usb_set_monut_parm(HUSB_MOUNT_PARM *parm);

#endif
