#include "msg.h"
#include "uart.h"
#include "audio/audio.h"
#include "music_player.h"
#include "task_common.h"
#include "warning_tone.h"
#include "rec_api.h"
#include "task_rec.h"
#include "task_rec_key.h"
#include "rec_api.h"
#include "string.h"
#include "wdt.h"
#include "audio/dac_api.h"
#include "ui/ui_api.h"
#include "fat_io.h"
#include "rec_ui.h"
#include "rec_play.h"


#define REC_TASK_DEBUG_ENABLE
#ifdef REC_TASK_DEBUG_ENABLE
#define rec_task_printf log_printf
#else
#define rec_task_printf(...)
#endif

#if MIC_REC_EN

RECORD_OP_API *rec_mic_api = NULL;

void record_mutex_init(void *priv)
{
}

void record_mutex_stop(void *priv)
{
    rec_exit(&rec_mic_api);
}

static void *task_rec_init(void *priv)
{
    rec_task_printf("task rec init !!\n");
    fat_init();
    tone_play(TONE_REC_MODE, 0);
    return NULL;
}

static void task_rec_exit(void **hdl)
{
    task_clear_all_message();
    rec_exit(&rec_mic_api);
    mutex_resource_release("record");
    mutex_resource_release("record_play");
    fat_del();
    ui_close_rec();
    rec_task_printf("task_rec_exit !!\n");
}


static void task_rec_deal(void *p)
{
    int msg;
    int msg_error = MSG_NO_ERROR;
    tbool ret = true;

    printf("****************REC TSAK*********************\n");

    while (1) {

        clear_wdt();

        msg_error = task_get_msg(0, 1, &msg);


        if (task_common_msg_deal(NULL, msg) == false) {
            music_tone_stop();
            task_common_msg_deal(NULL, NO_MSG);
            return;
        }
        if (msg == MSG_REC_START) {
            if (FALSE == is_cur_resource("record")) { //当前资源不属record所有，不允许录音，防止资源冲突
                continue;
            }
        }
        rec_msg_deal_api(&rec_mic_api, msg); //record 流程

        if (NO_MSG == msg) {
            continue;
        }

        switch (msg) {
        case SYS_EVENT_PLAY_SEL_END: //提示音结束
            rec_task_printf("RECORD_SYS_EVENT_PLAY_SEL_END\n");

        case MSG_REC_INIT:
            rec_task_printf("MSG_REC_INIT\n");
            dac_channel_on(DAC_DIGITAL_CH, FADE_ON);
            ui_open_rec(&rec_mic_api, sizeof(RECORD_OP_API **));
            dac_set_samplerate(48000, 0);//
            mutex_resource_apply("record", 3, record_mutex_init, record_mutex_stop, rec_mic_api);
            break;

#if LCD_SUPPORT_MENU
        case MSG_ENTER_MENULIST:
            UI_menu_arg(MENU_LIST_DISPLAY, UI_MENU_LIST_ITEM);
            break;
#endif

        case MSG_HALF_SECOND: {
            u32 tmp_rec_time = rec_get_enc_time(rec_mic_api);
            if (tmp_rec_time) {
                rec_task_printf("rec time %d:%d\n", tmp_rec_time / 60, tmp_rec_time % 60);
            }
            UI_REFRESH(MENU_REFRESH);
        }
        break;

        case MSG_INPUT_NUMBER_END:
        case MSG_INPUT_TIMEOUT:
            get_input_number(NULL);
            break;

        default:
            break;
        }
    }
}

const TASK_APP task_rec_info = {
    .skip_check = NULL,
    .init 		= task_rec_init,
    .exit 		= task_rec_exit,
    .task 		= task_rec_deal,
    .key 		= &task_rec_key,
};
#endif
