#ifndef __DEV_MANAGE__
#define __DEV_MANAGE__

#include "typedef.h"

#include "dev_mg/dev_mg_api.h"

#define IO2USB  BIT(0)
#define IO2SD  BIT(1)

void dev_manage_init(void);
void dev_detect_task(void *priv);
u32 dev_scan_partition(DEV_HANDLE dev);
void dev_detect_fun(void);
void dev_all_refurbish_part(void);
s32 dev_online_mount(DEV_HANDLE dev);
s32 dev_offline_unmount(DEV_HANDLE dev);
void dev_mult_sel_deal(DEV_HANDLE dev);

void io_2_sd(void);
void io_2_usb(void);
void io_clean(void);


extern DEV_HANDLE cache;
extern DEV_HANDLE usb;
extern DEV_HANDLE sd0;
extern DEV_HANDLE sd1;

void dev_power_off_all(void);
void dev_power_on_all(void);
void dev_power_off_spec_dev(DEV_HANDLE dev);
void dev_power_on_spec_dev(DEV_HANDLE dev);

#endif

