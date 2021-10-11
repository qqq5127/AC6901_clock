#include "sdk_cfg.h"
#include "usb_slave_api.h"
#include "dev_manage.h"
#include "card_reader_io.h"
#include "sdmmc/sd_host_api.h"
//#include "sdmmc/sdmmc_libs_interface.h"

#if USB_PC_EN
static sUSB_DEV_IO CARD_READER_IO SEC(.pc_mem);

extern void recover_cardreader_popup(u8 dev_type);
extern void set_cardreader_popup(u8 dev_type);
extern u8 get_sdmmc_popup_status(u8 dev_type);
extern u8 get_sdmmc_dev_status(u8 dev_type);
extern u8 sd0_rw_wait_idle(u8 rw_ctl, u8 retry);
extern u8 sd1_rw_wait_idle(u8 rw_ctl, u8 retry);
extern s32 sd1_write_go(u8 *buf, u32 lba);
extern s32 sd0_write_go(u8 *buf, u32 lba);
extern s32 sd1_read_go(u8 *buf, u32 lba);
extern s32 sd0_read_go(u8 *buf, u32 lba);
//***********USB MASSTORAGE**************/
#if SDMMC0_EN
///for sd0接口
static u8 usb_sd0_init(void)
{
    return !dev_mount(sd0, NULL);
}

static u32 usb_read_sd0(void *buf, u32 lba)
{
//    return dev_read(sd0, buf, lba, 1);
    return sd0_read_go(buf, lba);
}

static u32 usb_write_sd0(void *buf, u32 lba)
{
    //return dev_write(sd0, buf, lba, 1);
    return sd0_write_go(buf, lba);
}

static u32 usb_get_sd0_capacity(void)
{
    u32 capacity;

    dev_io_ctrl(sd0, DEV_GET_BLOCK_NUM, &capacity);
    return capacity;
}

static void usb_sd0_set_cardreader_popup(void)
{
    set_cardreader_popup(SD_CONTROLLER_0); //设置弹出标志
}

static void usb_sd0_recover_cardreader_popup(void)
{
    recover_cardreader_popup(SD_CONTROLLER_1); //恢复弹出标志
}

static u8 usb_sd0_get_popup_status(void)
{
    return get_sdmmc_popup_status(SD_CONTROLLER_1);
}

static u8 usb_sd0_get_dev_status(void)
{
    return get_sdmmc_dev_status(SD_CONTROLLER_0);
}

static u8 usb_sd0_rw_wait_idle(u8 rw_ctl, u8 retry)
{
    return sd0_rw_wait_idle(rw_ctl, retry);
}

#endif
//***********USB MASSTORAGE**************/
#if SDMMC1_EN
///for sd1接口
static u8 usb_sd1_init(void)
{
    return !dev_mount(sd1, NULL);
}

static u32 usb_read_sd1(void *buf, u32 lba)
{
    //return dev_read(sd1, buf, lba, 1);
    return sd1_read_go(buf, lba);
}

static u32 usb_write_sd1(void *buf, u32 lba)
{
    //return dev_write(sd1, buf, lba, 1);
    return sd1_write_go(buf, lba);
}

static u32 usb_get_sd1_capacity(void)
{
    u32 capacity;

    dev_io_ctrl(sd1, DEV_GET_BLOCK_NUM, &capacity);
    return capacity;
}

static void usb_sd1_set_cardreader_popup(void)
{
    set_cardreader_popup(SD_CONTROLLER_1); //设置弹出标志
}

static void usb_sd1_recover_cardreader_popup(void)
{
    recover_cardreader_popup(SD_CONTROLLER_1); //恢复出标志
}

static u8 usb_sd1_get_popup_status(void)
{
    return get_sdmmc_popup_status(SD_CONTROLLER_1);
}

static u8 usb_sd1_get_dev_status(void)
{
    return get_sdmmc_dev_status(SD_CONTROLLER_1);
}

static u8 usb_sd1_rw_wait_idle(u8 rw_ctl, u8 retry)
{
    return sd1_rw_wait_idle(rw_ctl, retry);
}
#endif
/*----------------------------------------------------------------------------*/
/**@brief   获取读卡器设备操作IO
   @param   dev_type:设备类型
   @return  sUSB_DEV_IO ：根据类型获取到的设备IO
   @note
*/
/*-----------------------------------------------------------------------------*/
sUSB_DEV_IO *get_card_read_io(DEV_TYPE dev_type)
{

    memset(&CARD_READER_IO, 0x00, sizeof(sUSB_DEV_IO));
    if (dev_type == DEV_SDCRAD_0) {
#if SDMMC0_EN
        CARD_READER_IO.usb_get_dev_capacity     = usb_get_sd0_capacity;
        CARD_READER_IO.usb_write_dev            = usb_write_sd0;
        CARD_READER_IO.usb_read_dev             = usb_read_sd0;
        CARD_READER_IO.usb_dev_init             = usb_sd0_init;
        CARD_READER_IO.usb_wait_sd_end          = usb_sd0_rw_wait_idle;
        CARD_READER_IO.set_cardreader_popup     = usb_sd0_set_cardreader_popup;
        CARD_READER_IO.recover_cardreader_popup = usb_sd0_recover_cardreader_popup;
        CARD_READER_IO.get_sdmmc_popup_status   = usb_sd0_get_popup_status;
        CARD_READER_IO.get_sdmmc_dev_status     = usb_sd0_get_dev_status;
        CARD_READER_IO.bAttr                    = SD_CONTROLLER_0;
        return &CARD_READER_IO;
#else
        return NULL;
#endif
    } else if (dev_type == DEV_SDCRAD_1) {
#if SDMMC1_EN
        CARD_READER_IO.usb_get_dev_capacity     = usb_get_sd1_capacity;
        CARD_READER_IO.usb_write_dev            = usb_write_sd1;
        CARD_READER_IO.usb_read_dev             = usb_read_sd1;
        CARD_READER_IO.usb_dev_init             = usb_sd1_init;
        CARD_READER_IO.usb_wait_sd_end          = usb_sd1_rw_wait_idle;
        CARD_READER_IO.set_cardreader_popup     = usb_sd1_set_cardreader_popup;
        CARD_READER_IO.recover_cardreader_popup = usb_sd1_recover_cardreader_popup;
        CARD_READER_IO.get_sdmmc_popup_status   = usb_sd1_get_popup_status;
        CARD_READER_IO.get_sdmmc_dev_status     = usb_sd1_get_dev_status;
        CARD_READER_IO.bAttr                    = SD_CONTROLLER_1;
        return &CARD_READER_IO;
#else
        return NULL;
#endif
    }

    return NULL;
}

#endif
