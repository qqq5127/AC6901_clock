#include "sdk_cfg.h"
#include "msg.h"
#include "uart.h"
#include "audio/dac_api.h"
#include "task_common.h"
#include "dev_pc.h"
#include "task_pc.h"
#include "wdt.h"
#include "usb_slave_api.h"
#include "task_pc_key.h"
#include "updata.h"
#include "warning_tone.h"
#include "dev_manage.h"
#include "music_player.h"
#include "pc_ui.h"
#include "led.h"

#if USB_PC_EN

#define PC_TASK_DEBUG_ENABLE
#ifdef PC_TASK_DEBUG_ENABLE
#define pc_task_printf log_printf
#else
#define pc_task_printf(...)
#endif

u8 usb_spk_vol_value;
u8 pc_module_start_flag;

extern u8 usb_slave_is_online(void);
void pc_mutex_init(void *priv)
{
    dac_set_samplerate(48000, 0);     //DAC采样率设置为48K
    pc_speak_out_sw(1);
}

void pc_mutex_stop(void *priv)
{
    pc_speak_out_sw(0);
}


static tbool task_pc_skip_check(void **priv)
{
    printf("usb_device_status = 0x%x\n", usb_slave_is_online());

    if (usb_slave_is_online()) {
        return true;
    } else {
        return false;
    }
}

static void *task_pc_init(void *priv)
{
    pc_task_printf("task_pc_init !!\n");
#if WARNING_TASK_USBDEV
    tone_play(TONE_PC_MODE, 0);
#else
    task_post_msg(NULL, 1, SYS_EVENT_PLAY_SEL_END);
#endif
    led_idle();//led_fre_set(C_BLED_SLOW_MODE);
    pc_module_start_flag = 0;
    dev_power_on_all();
    return 0;
}


static void task_pc_exit(void **hdl)
{
    pc_task_printf("task_pc_e2xit\n");
    if (pc_module_start_flag) {
        app_usb_slave_close(); //关闭usb slave模块
        mutex_resource_release("pc");
    }
    task_clear_all_message();
    ui_close_pc();
    dev_power_off_all();
}


static void task_pc_deal(void *hdl)
{
    int msg;
    int msg_error = MSG_NO_ERROR;
    u32 pc_mute_status = 0;
    tbool ret = true;
    u8 sync_pc_vol_flag = 0;


    printf("****************PC TSAK*********************\n");
    while (1) {

        clear_wdt();

        msg_error = task_get_msg(0, 1, &msg);

        if (NO_MSG == msg) {
            //读卡器流程
            if (0 != app_usb_slave_online_status()) {
                app_usb_slave_card_reader(MSG_PC_UPDATA); //读卡器流程
            }
        }


        switch (msg) {
        case MSG_SD0_ONLINE:
            dev_online_mount(sd0);
            break;
        case MSG_SD1_ONLINE:
            dev_online_mount(sd1);
            break;
        case MSG_PC_OFFLINE:
            puts("MSG_PC_OFFLINE\n");
            if (task_switch(TASK_ID_TYPE_NEXT, NULL) == true) {
                music_tone_stop();
                task_common_msg_deal(NULL, NO_MSG);
                return;
            }
            puts("WARNING:task_switch ERR\n");
            break;

        case SYS_EVENT_PLAY_SEL_END:
            pc_task_printf("task pc init\n");
            if (pc_module_start_flag == 0) {
                if (0 ==  app_usb_slave_init()) {
                    pc_task_printf("init pc fail\n");
                    task_switch(TASK_ID_TYPE_NEXT, NULL);
                    return;
                }

                mutex_resource_apply("pc", 3, pc_mutex_init, pc_mutex_stop, 0);
                ui_open_pc(NULL, 0);
                pc_module_start_flag = 1;
                sync_pc_vol_flag = 6;
            }
            break;

        /*************** for AUDIO ******************/
        case MSG_PC_SPK_MUTE:
            pc_task_printf("MSG_PC_SPK_MUTE\n");
            pc_mute_status = 1;
            pc_dac_mute(1, 1);
            break;

        case MSG_PC_SPK_UNMUTE:
            pc_task_printf("MSG_PC_SPK_UNMUTE\n");
            pc_mute_status = 0;
            pc_dac_mute(0, 1);

        case MSG_PC_SPK_VOL:
            pc_task_printf("MSG_PC_SPK_VOL\n");
            usb_spk_vol_value = app_pc_set_speaker_vol(pc_mute_status);
            if (sync_pc_vol_flag != 1) {
                sync_pc_vol_flag = 1;
            }
            break;

        /*************** for HID KEY ******************/
        case MSG_PC_MUTE:
            pc_task_printf("p_m\n");
            app_usb_slave_hid(USB_AUDIO_MUTE);
            break;

        case MSG_PC_VOL_DOWN:
            pc_task_printf("vd\n");
            app_usb_slave_hid(USB_AUDIO_VOLDOWN);
            break;

        case MSG_PC_VOL_UP:
            pc_task_printf("vu\n");
            app_usb_slave_hid(USB_AUDIO_VOLUP);
            break;

        case MSG_PC_PP:
            pc_task_printf("pp\n");
            app_usb_slave_hid(USB_AUDIO_PP);
            break;

        case MSG_PC_PLAY_NEXT:
            pc_task_printf("ne\n");
            app_usb_slave_hid(USB_AUDIO_NEXTFILE);
            break;

        case MSG_PC_PLAY_PREV:
            pc_task_printf("pr\n");
            app_usb_slave_hid(USB_AUDIO_PREFILE);
            break;

        case MSG_PC_UPDATA:
            updata_mode_api(PC_UPDATA);
            break;

        case MSG_HALF_SECOND:
            if (sync_pc_vol_flag) {
                if (sync_pc_vol_flag > 2) {
                    sync_pc_vol_flag--;
                } else if (sync_pc_vol_flag == 2) {
                    app_usb_slave_hid(USB_AUDIO_VOLUP);
                    sync_pc_vol_flag = 4;
                }
            }
            break;

        case MSG_INPUT_NUMBER_END:
        case MSG_INPUT_TIMEOUT:
            get_input_number(NULL);
            break;

        default:
            if (task_common_msg_deal(hdl, msg) == false) {
                music_tone_stop();
                task_common_msg_deal(NULL, NO_MSG);
                sync_pc_vol_flag = 0;
                return;
            }
            break;
        }
    }
}


const TASK_APP task_pc_info = {
    .skip_check  = task_pc_skip_check,
    .init        = task_pc_init,
    .exit        = task_pc_exit,
    .task        = task_pc_deal,
    .key         = &task_pc_key,
};
#endif


