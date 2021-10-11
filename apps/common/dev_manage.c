#include "dev_manage.h"
#include "sdk_cfg.h"
#include "common/common.h"

#include "string.h"
#include "msg.h"
#include "mbr.h"
#include "fs.h"

#include "dev_cache.h"
#include "dev_usb.h"
#include "dev_sd.h"

#include "errno-base.h"
#include "uart.h"
#include "board_init.h"
#include "irq_api.h"


DEV_HANDLE cache;
DEV_HANDLE usb;
DEV_HANDLE sd0;
DEV_HANDLE sd1;

#if 1
#define dev_dbg  log_printf
#define mbr_deg  log_printf
#else
#define dev_dbg(...)
#define mbr_deg(...)
#endif


static volatile u8 dev_start = 0;//device detect after all device regist
LOOP_DETECT_REGISTER(device_detect_loop) = {
    .time = 5,
    .fun  = dev_detect_task,
};

static u8 get_powerof2(u8 n)
{
    u8 i = 0;
    while ((n >>= 1) != 0) {
        i++;
    }
    return i;
}

static volatile u8 is_sd_usb = 0;//BIT(0):USB，BIT(1):SD
void io_2_sd(void)
{
    is_sd_usb = BIT(1);
}

void io_2_usb(void)
{
    is_sd_usb = BIT(0);
}

void io_clean(void)
{
    is_sd_usb = 0;
}

u8 get_io_status(void)
{
    return is_sd_usb;
}


void suspend_sd_io(void)
{
#if USB_SD0_MULT_EN	//sd0a
    JL_PORTB->DIR |= (BIT(3) | BIT(4));
    JL_PORTB->PU &= ~(BIT(3) | BIT(4));
    JL_PORTB->PD &= ~(BIT(3) | BIT(4));
#else				//sd1b
    JL_PORTC->DIR |= (BIT(3) | BIT(4));
    JL_PORTC->PU &= ~(BIT(3) | BIT(4));
    JL_PORTC->PD &= ~(BIT(3) | BIT(4));
#endif
}



/*----------------------------------------------------------------------------*/
/** @brief: physical device mbr info scan
    @param: mbr_inf: mbr_info return
    @param: hdev : device's handle
    @param: read_fun: device read api
    @param: write_fun: device write api
    @return: none_zero:err code. zero:get info succ
    @note:
*/
/*----------------------------------------------------------------------------*/
int mbr_scan_parition(MBR_DRIVE_INFO *mbr_inf, DEV_HANDLE hdev, void *read_fun, void *write_fun)
{
    MRESULT res;
    MBR_FS *mbr;
    u8 i;

    mbr_deg("func = %s\n", __FUNCTION__);

#if MBR_MALLOC
    mbr = malloc(sizeof(*mbr));
    ASSERT(mbr);
#else
    MBR_FS mbr_val;
    mbr = &mbr_val;
#endif
    memset(mbr, 0, sizeof(*mbr));

    mbr->win.mbr = mbr;
    mbr->win.sector = 0xffffffff;
    mbr->disk_read = read_fun;  //需要补齐代码
    mbr->disk_write = write_fun; //需要补齐代码
    mbr->hdev = hdev;

    mbr->inf = mbr_inf;
    mbr->inf->drive_cnt = 0;
    mbr->inf->drive_max = MBR_DRV_MAX;

    u32 parm;
    if (DEV_ERR_NONE != dev_io_ctrl(hdev, DEV_GET_BLOCK_SIZE, &parm)) {
        log_printf("DEV_GET_BLOCK_SIZE\r");
#if MBR_MALLOC
        free(mbr);
#endif
        return MBR_FAIL;
    }
    log_printf("DEV_GET_BLOCK_SIZE = %d\r", parm);
    parm /= 512;
    mbr->sector_512size = get_powerof2(parm);

    res = mbr_scan(mbr, 0, (u8)255);//,查找第一个盘

#if MBR_MALLOC
    free(mbr);
#endif

    if (MBR_FAIL == res) {
        mbr_deg("no mbr\n");
        mbr_inf->drive_cnt = 1;
        mbr_inf->drive_boot_sect[0] = 0;
    } else {
        for (i = 0; i < mbr_inf->drive_cnt; i++) {
            log_printf("%d	addr:0x%x\n", i, mbr_inf->drive_boot_sect[i]);
        }
    }

    return MBR_OK;
}

/*----------------------------------------------------------------------------*/
/** @brief: physical device partition info scan
    @param: hdev : device's handle
    @return: number of partition
    @note:
*/
/*----------------------------------------------------------------------------*/
u32 dev_scan_partition(DEV_HANDLE dev)
{
    MBR_DRIVE_INFO mbr_inf;
    s32 err;
    u32 i;

    mbr_inf.drive_cnt = 0;

    if (dev == cache) {
        mbr_inf.drive_cnt = 1;
        mbr_inf.drive_boot_sect[0] = 0;
    } else {
#if ((USB_DISK_EN == 1) || (SDMMC0_EN == 1) || (SDMMC1_EN == 1))
        err = mbr_scan_parition(&mbr_inf, dev, read_api, write_api);
        if (MBR_OK != err) {
            puts("mbr_scan_parition err\n");
            return 0;
        }
#endif
    }

    if (mbr_inf.drive_cnt) {
        puts("dev_dpt_info_init\n");
        dev_dpt_info_init(dev, mbr_inf.drive_cnt, mbr_inf.drive_boot_sect);
        /* put_msg_filo("MAIN_TASK", 2, MSG_DEV_ONLINE, dev); */
    }

    return mbr_inf.drive_cnt;
}

/*----------------------------------------------------------------------------*/
/** @brief: device on_off line info
    @param: hdev : device's handle
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void dev_handle_to_msg(DEV_HANDLE dev, u8 on_off)
{
    if (dev == usb) {
        if (on_off) {
            if (get_event_status(EVENT_HUSB_OFFLINE) == false) {
                task_post_event(NULL, 1,  EVENT_HUSB_ONLINE);
            }
        } else {
            task_post_event(NULL, 1, EVENT_HUSB_OFFLINE);
        }
    } else if (dev == sd0) {
        if (on_off) {
            if (get_event_status(EVENT_SD0_OFFLINE) == false) {
                task_post_event(NULL, 1, EVENT_SD0_ONLINE);
            }
        } else {
            task_post_event(NULL, 1, EVENT_SD0_OFFLINE);
        }
    } else if (dev == sd1) {
        if (on_off) {
            if (get_event_status(EVENT_SD1_OFFLINE) == false) {
                task_post_event(NULL, 1, EVENT_SD1_ONLINE);
            }
        } else {
            task_post_event(NULL, 1, EVENT_SD1_OFFLINE);
        }
    } else {
    }
}

s32 dev_offline_unmount(DEV_HANDLE dev)
{
    s32 ret;

    ret = dev_unmount(dev);
    if (ret) {
        puts("dev_unmount err\n");
        return false;
    }
    dev_dpt_info_del(dev);

    return true;
}

void dev_mult_sel_deal(DEV_HANDLE dev)
{
#if (USB_SD1_MULT_EN == 1)
    if (dev == sd1) {
        dev_power_ctrl(usb, DEV_POWER_OFF);
        io_2_sd();
        dev_power_ctrl(sd1, DEV_POWER_ON);
    } else if (dev == usb) {
        dev_power_ctrl(sd1, DEV_POWER_OFF);
        io_2_usb();
        dev_power_ctrl(usb, DEV_POWER_ON);
    }
#endif
#if (USB_SD0_MULT_EN == 1)
    if (dev == sd0) {
        dev_power_ctrl(usb, DEV_POWER_OFF);
        io_2_sd();
        dev_power_ctrl(sd0, DEV_POWER_ON);
    } else if (dev == usb) {
        dev_power_ctrl(sd0, DEV_POWER_OFF);
        io_2_usb();
        dev_power_ctrl(usb, DEV_POWER_ON);
    }
#endif
}


#if ((USB_SD0_MULT_EN == 1)||(USB_SD1_MULT_EN == 1))
static u8 dev_no_detect_flag;
#include "music_decoder.h"
#include "rec_api.h"

s32 dev_online_mount(DEV_HANDLE dev)
{
    s32 ret = false;

    u32 online_status;
    u8 io2_status;

#if USB_SD0_MULT_EN
    DEV_HANDLE sd_dev = sd0;
#endif
#if USB_SD1_MULT_EN
    DEV_HANDLE sd_dev = sd1;
#endif

    dev_no_detect_flag = 1;
    io2_status = get_io_status();
    if (io2_status) {
        music_decoder_set_loop_en(0); //关掉解码器循环，防止读设备导致冲突
        rec_set_loop_en(0);  //关掉录音编码器循环，防止写设备导致冲突
    }

    if (dev == sd_dev) {
        dev_power_ctrl(usb, DEV_POWER_OFF);  //关闭USB
        io_2_sd();
    } else if (dev == usb) {
        dev_power_ctrl(sd_dev, DEV_POWER_OFF);  //关闭SD1
        io_2_usb();
    }

    if (!dev_mount(dev, NULL)) {
        if (dev_scan_partition(dev)) {
            ret = true;
        } else {
            printf("dev_scan_partition err\n");
        }
    } else {
        printf("dev_mount err : %d\n", ret);
    }


    if (io2_status == BIT(1)) {
        dev_mult_sel_deal(sd_dev);
    } else if (io2_status == BIT(0)) {
        dev_mult_sel_deal(usb);
    } else {
        io_clean();
    }

    music_decoder_set_loop_en(1);
    rec_set_loop_en(1);
    dev_no_detect_flag = 0;
    return ret;
}

void dev_detect_deal(DEV_HANDLE dev)
{
    u32 parm = DEV_HOLD;
    int ret;

#if USB_SD0_MULT_EN
    DEV_HANDLE sd_dev = sd0;
#endif
#if USB_SD1_MULT_EN
    DEV_HANDLE sd_dev = sd1;
#endif

    if (dev_no_detect_flag) {
        return;
    }

    if (dev == usb) {
        if (get_io_status() == IO2SD) {
            //        if (!get_sd1_dev_busy_status_api()) {
            u32 DevBusy;
            if (DEV_ERR_NONE != dev_io_ctrl(sd_dev, SDMMC_GET_BUSY, &DevBusy)) {
                return;
            }
            if (!DevBusy) {
                //  dev_power_ctrl(sd_dev, DEV_POWER_OFF);
                suspend_sd_io();
                ret = dev_detect(dev, &parm);
                dev_power_ctrl(sd_dev, DEV_POWER_ON);

                if (DEV_ERR_NONE != ret) {
                    return;
                }
            } else {
                return;
            }

        } else {
            suspend_sd_io();
            ret = dev_detect(dev, &parm);
        }
    } else {
        ret = dev_detect(dev, &parm);
    }

    if (DEV_ONLINE == parm) {
        log_printf("---------------dev_online name = %s------------------\n", dev_get_name_by_handle(dev));
        log_printf("dev_msg = %x\n", MSG_DEV_ONLINE | (u32)dev);
        dev_handle_to_msg(dev, DEV_ONLINE);
    } else if (DEV_OFFLINE == parm) {
        log_printf("---------------dev_offline name = %s------------------\n", dev_get_name_by_handle(dev));
        dev_handle_to_msg(dev, DEV_OFFLINE);
    }
}

#else
s32 dev_online_mount(DEV_HANDLE dev)
{
    s32 ret;

    ret = dev_mount(dev, NULL);
    if (ret) {
        printf("dev_mount err : %d\n", ret);
        return false;
    }

    ret = dev_scan_partition(dev);
    printf("dev_scan_partition = %d\n", ret);
    if (0 == ret) {
        return false;
    }

    return true;
}
/*----------------------------------------------------------------------------*/
/** @brief: dev_detect_deal
    @param: hdev : device's handle
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
static void dev_detect_deal(DEV_HANDLE dev)
{
    u32 parm;
    int ret;

    if (DEV_ERR_NONE == dev_detect(dev, &parm)) {
        if (DEV_ONLINE == parm) {
            log_printf("---------------dev_online name = %s------------------\n", dev_get_name_by_handle(dev));

            log_printf("dev_msg = %x\n", MSG_DEV_ONLINE | (u32)dev);
            dev_handle_to_msg(dev, DEV_ONLINE);
        } else if (DEV_OFFLINE == parm) {
            log_printf("---------------dev_offline name = %s------------------\n", dev_get_name_by_handle(dev));

            dev_handle_to_msg(dev, DEV_OFFLINE);
        }
    }

}

#endif
/*----------------------------------------------------------------------------*/
/** @brief: dev_detect_task
    @param: hdev : device's handle
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void dev_detect_task(void *priv)
{
    DEV_HANDLE dev;
    s32 ret;

    if (dev_start == 0) {
        return ;
    }

    dev = dev_get_fisrt(DEV_ALL, 0);
    if (dev == NULL) {
        goto __EXIT;
    }

    dev_detect_deal(dev);

    while (1) {
        dev = dev_get_next(dev, DEV_ALL, 0);
        if (dev == NULL) {
            goto __EXIT;
        }

        dev_detect_deal(dev);
    }

__EXIT:
    /* _thread_resume_delay(th, 1);//run after 1*10ms */
    return;
}

/*----------------------------------------------------------------------------*/
/** @brief: device manage init
    @param: null
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
#define MAX_DEV_REG				4		//注册的设备最大数
static u8 device_manage_need_buf[512] ALIGNED(4);

void dev_manage_init(void)
{
    int err;
    u32 parm;

    parm = dev_need_buf(MAX_DEV_REG);//will regis 4 device
    ASSERT(parm <= sizeof(device_manage_need_buf));

    err = dev_mg_init(MAX_DEV_REG, device_manage_need_buf);
    ASSERT(0 == err);

    /**
     * @brief mango driver - cache
     */
    DEVICE_REG(cache, NULL);
    parm = FLASH_BASE_ADDR;
    cache = dev_open("cache", &parm);
    ASSERT(cache);

#if USB_DISK_EN
    /**
     * @brief mango driver - usb
     */
    parm = 4096;
#if ((USB_SD0_MULT_EN == 1)||(USB_SD1_MULT_EN == 1))
    DEVICE_REG(usbmult, NULL);
    usb = dev_open("usbmult", &parm);
#else
    DEVICE_REG(usb, NULL);
    usb = dev_open("usb", &parm);
#endif
    ASSERT(usb);
    //挂载参数配置，部分U盘 mount不成功可能与参数延时时间有关
    HUSB_MOUNT_PARM mount_parm;
    mount_parm.mount_retry = 3;
    mount_parm.mount_timeout = 500;
    mount_parm.reset_delay = 2;
    mount_parm.test_delay = 100;
    usb_set_monut_parm(&mount_parm);
#endif

#if SDMMC0_EN
    /**
     * @brief mango driver - sd0
     */
    sSD_API_SET sd0_api_set;
    memset(&sd0_api_set, 0x0, sizeof(sSD_API_SET));
    //SD0_A	(DAT_CMD_CLK) : CHIP:PA5_PA6_PA7	FPGA:PO5_PO6_PO7
    //SD0_B	(DAT_CMD_CLK) : CHIP:PB3_PB4_PB5	FPGA:PP3_PP4_PP5
    sd0_api_set.controller_io = SD0_IO_B;        	//SD0_IO_A：SD0控制器B出口，SD0_IO_B：SD1控制器B出口

#ifdef FPGA
    sd0_api_set.online_check_way = SD_CMD_CHECK; 	//CMD检测方式：SD_CMD_CHECK，CLK检测方式：SD_CLK_CHECK，IO检测方式：SD_IO_CHECK
#else
    sd0_api_set.online_check_way = SD_CLK_CHECK; 	//CMD检测方式：SD_CMD_CHECK，CLK检测方式：SD_CLK_CHECK，IO检测方式：SD_IO_CHECK
#endif

    sd0_api_set.max_data_baud = 5;               	//数据传输波特率(0为最快速度)
    sd0_api_set.hc_mode = SD_1WIRE_MODE;         	//1线模式：SD_1WIRE_MODE，4线模式：SD_4WIRE_MODE，高速模式：SD_HI_SPEED

#if USB_SD0_MULT_EN
    ASSERT(sd0_api_set.online_check_way != SD_CMD_CHECK);  //复用不支持CMD检测
#endif

    DEVICE_REG(sd0, NULL);               			///<注册sd0_usb_复用到系统
    sd0 = dev_open("sd0", &sd0_api_set);
    ASSERT(sd0);
#endif

#if SDMMC1_EN
    /**
     * @brief mango driver - sd0
     */
    sSD_API_SET sd1_api_set;
    memset(&sd1_api_set, 0x0, sizeof(sSD_API_SET));
    //SD1_A	(DAT_CMD_CLK) : CHIP:PC3_PC4_PC5	FPGA:PQ3_PQ4_PQ5
    //SD1_B	(DAT_CMD_CLK) : CHIP:PB0_PB1_PB2	FPGA:PP0_PP1_PP2
    sd1_api_set.controller_io = SD1_IO_A;        	//SD0_IO_A：SD0控制器B出口，SD0_IO_B：SD1控制器B出口

#ifdef FPGA
    sd1_api_set.online_check_way = SD_CMD_CHECK; 	//CMD检测方式：SD_CMD_CHECK，CLK检测方式：SD_CLK_CHECK，IO检测方式：SD_IO_CHECK
#else
    sd1_api_set.online_check_way = SD_CLK_CHECK; 	//CMD检测方式：SD_CMD_CHECK，CLK检测方式：SD_CLK_CHECK，IO检测方式：SD_IO_CHECK
#endif

    sd1_api_set.max_data_baud = 5;               	//数据传输波特率(0为最快速度)
    sd1_api_set.hc_mode = SD_1WIRE_MODE;         	//1线模式：SD_1WIRE_MODE，4线模式：SD_4WIRE_MODE，高速模式：SD_HI_SPEED

#if USB_SD1_MULT_EN
    ASSERT(sd1_api_set.online_check_way != SD_CMD_CHECK);  //复用不支持CMD检测
#endif

    DEVICE_REG(sd1, NULL);               			///<注册sd0_usb_复用到系统
    sd1 = dev_open("sd1", &sd1_api_set);
    ASSERT(sd1);
#endif

    log_printf("cache 0x%x\n", cache);
    log_printf("usb 0x%x\n", usb);
    log_printf("sd0 0x%x\n", sd0);
    log_printf("sd1 0x%x\n", sd1);

    /**
     * @brief dev_manage_init
     */
    printf("wait cache_online\n");
    DEV_HANDLE dev = cache;		//wait flash online
    while (1) {
        if (DEV_ERR_NONE == dev_detect(dev, &parm)) {
            if (DEV_ONLINE == parm) {
                printf("cache_online & sys start\n");
                err = dev_mount(dev, NULL);
                ASSERT(err == 0);
                /* void cache_test(void); */
                /* cache_test(); */
                /* while(1); */
                dev_start = 1;		//all device regist
                return;
            }
        }
    }

#if 0		//debug
    extern void dev_read_test(DEV_HANDLE hd);
    dev_read_test(sd0);

    /* void vfs_example(void); */
    /* vfs_example(); */
#endif

}

/*----------------------------------------------------------------------------*/
/** @brief: dev_all_refurbish_part
    @param: null
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void dev_all_refurbish_part(void)
{
    DEV_HANDLE dev;
    s32 ret;

    dev = dev_get_fisrt(DEV_ALL, 0);
    if (dev == NULL) {
        goto __EXIT;
    }

    dev_refurbish_part(dev);
    while (1) {
        dev = dev_get_next(dev, DEV_ALL, 0);
        if (dev == NULL) {
            goto __EXIT;
        }
        dev_refurbish_part(dev);
    }
__EXIT:
    return;
}

#if DEV_POWER_OFF_EN
void dev_power_off_all(void)
{
    DEV_HANDLE dev;
    s32 ret;
    u32 dev_status_tmp;

    dev = dev_get_fisrt(DEV_ALL, 0);
    if (dev == NULL) {
        return;
    }

    while (1) {
        if (DEV_ERR_NONE == dev_get_online_status(dev, &dev_status_tmp)) {
            if (dev_status_tmp) { //设备在线
                dev_power_ctrl(dev, DEV_POWER_OFF);
            }
        }
        dev = dev_get_next(dev, DEV_ALL, 0);
        if (dev == NULL) {
            return;
        }
    }
}

void dev_power_on_all(void)
{
    DEV_HANDLE dev;
    s32 ret;
    u32 dev_status_tmp;

    dev = dev_get_fisrt(DEV_ALL, 0);
    if (dev == NULL) {
        return;
    }

    while (1) {
        if (DEV_ERR_NONE == dev_get_online_status(dev, &dev_status_tmp)) {
            if (dev_status_tmp) { //设备在线
                dev_power_ctrl(dev, DEV_POWER_ON);
            }
        }
        dev = dev_get_next(dev, DEV_ALL, 0);
        if (dev == NULL) {
            return;
        }
    }
}

void dev_power_off_spec_dev(DEV_HANDLE dev)
{
    u32 dev_status_tmp;

    if (DEV_ERR_NONE == dev_get_online_status(dev, &dev_status_tmp)) {
        if (dev_status_tmp) { //设备在线
            dev_power_ctrl(dev, DEV_POWER_OFF);
        }
    }
}

void dev_power_on_spec_dev(DEV_HANDLE dev)
{
    u32 dev_status_tmp;

    if (DEV_ERR_NONE == dev_get_online_status(dev, &dev_status_tmp)) {
        if (dev_status_tmp) { //设备在线
            dev_power_ctrl(dev, DEV_POWER_ON);
        }
    }
}


#else
void dev_power_off_all(void)
{
}
void dev_power_on_all(void)
{
}
void dev_power_off_spec_dev(DEV_HANDLE dev)
{
}
void dev_power_on_spec_dev(DEV_HANDLE dev)
{
}
#endif


#if ADKEY_SD_MULT_EN
bool adkey_sd_mult_sd_suspend()
{
    u32 DevBusy;
    DEV_HANDLE dev;
    u32 dev_status_tmp;

#if (ADKEY_SD_MULT_EN == 1)
    dev = sd0;
#else if (ADKEY_SD_MULT_EN == 2)
    dev = sd1;
#endif

    if (DEV_ERR_NONE == dev_get_online_status(dev, &dev_status_tmp)) {
        if (dev_status_tmp) { //设备在线

            if (DEV_ERR_NONE != dev_io_ctrl(dev, SDMMC_GET_BUSY, &DevBusy)) {
                return false;
            }

            if (!DevBusy) {
                dev_power_ctrl(dev, DEV_POWER_OFF);
            } else { //忙碌
                return false;
            }

        }
    }
    return true;
}

void adkey_sd_mult_sd_resume()
{
    DEV_HANDLE dev;
    u32 dev_status_tmp;

#if (ADKEY_SD_MULT_EN == 1)
    dev = sd0;
#else if (ADKEY_SD_MULT_EN == 2)
    dev = sd1;
#endif

    if (DEV_ERR_NONE == dev_get_online_status(dev, &dev_status_tmp)) {
        if (dev_status_tmp) { //设备在线
            dev_power_ctrl(dev, DEV_POWER_ON);
        }
    }
}
#endif
/*############################################################################*/
/*
								test_function
*/
/*############################################################################*/

/*----------------------------------------------------------------------------*/
/** @brief: test_function
    @param: null
    @return: null
    @note:
*/
/*----------------------------------------------------------------------------*/
void dev_read_test(DEV_HANDLE hd)
{
    while (1) {
        u32 parm;
        u32 ret;
        DEV_HANDLE dev = hd;
        dev_detect_task(NULL);
        u8 buf[512];

        if (dev_get_online_status(dev, (void *)&parm) == DEV_ERR_NONE) {
            if (parm == DEV_ONLINE) {
                puts("dev_online\n");
                ret = dev_mount(dev, NULL);
                ASSERT(!ret);

                ret = dev_read(dev, buf, 0, 1);
                ASSERT(ret);

                //printf_buf(buf, 512);

                ret = dev_unmount(dev);
                ASSERT(!ret);

                while (1);
            }
        }
    }
}

void vfs_example(void)
{
    u32 parm;
    DEV_HANDLE dev = cache;
    /* DEV_HANDLE dev = sd1; */
    u32 ret;

    while (1) {
        /* clear_wdt(); */
        ret = dev_detect(dev, (void *)&parm);
        if (dev_get_online_status(dev, (void *)&parm) == DEV_ERR_NONE) {
            if (parm == DEV_ONLINE) {
                puts("dev_online\n");

                ret = dev_mount(dev, NULL);
                ASSERT(!ret);

                //vfs_test(dev, read_api, write_api);

                ret = dev_unmount(dev);
                ASSERT(!ret);

                puts("test_ok\n");
                while (1);
            }
        }
    }
}

