#ifndef __KEY_DRV_IO_H__
#define __KEY_DRV_IO_H__

#include "sdk_cfg.h"
#include "key.h"
#include "sys_detect.h"
#include "rtc/rtc_api.h"
#include "common.h"

#define IO_KEY_ONLY_ONE
//set usb_io to key_io
#ifdef IO_KEY_ONLY_ONE
#define IS_KEY0_DOWN()    (!PORTR_IN(PORTR2))        //PP PR2
#define IS_KEY1_DOWN()    0
#define IS_KEY2_DOWN()    0
#define IS_KEY3_DOWN()    0
#define KEY_INIT()        do{PORTR_PU(PORTR2,1);PORTR_PD(PORTR2,0);PORTR_DIR(PORTR2,1);}while(0)
#else
#define IS_KEY0_DOWN()    (!PORTR_IN(PORTR2))        //PP PR2
#define IS_KEY1_DOWN()    (!(JL_PORTB->IN & BIT(2)))
#define IS_KEY2_DOWN()    (!(JL_PORTB->IN & BIT(3)))
#define IS_KEY3_DOWN()    (!(JL_PORTB->IN & BIT(4)))

#define KEY_INIT()        do{PORTR_PU(PORTR2,1);PORTR_PD(PORTR2,0);PORTR_DIR(PORTR2,1);\
						 	 JL_PORTB->DIR |= (BIT(3) | BIT(4) | BIT(2));\
						     JL_PORTB->PU  |= (BIT(3) | BIT(4) | BIT(2));\
							 JL_PORTB->PD  &=~(BIT(3) | BIT(4) | BIT(2));\
					    	}while(0)
#endif


extern const key_interface_t key_io_info;

#endif/*__KEY_DRV_IO_H__*/
