#include "task_echo.h"
#include "task_echo_key.h"
#include "common/msg.h"
#include "audio/dac_api.h"
#include "key_drv/key.h"
#include "clock_api.h"
#include "common/common.h"
#include "task_common.h"
#include "clock.h"
#include "audio/audio.h"
#include "warning_tone.h"
#include "music_player.h"
#include "ui_api.h"
#include "led.h"
#include "echo_api.h"
#include "echo_ui.h"

#if TASK_ECHO_EN

static void *task_echo_init(void *priv)
{
    printf("task_echo_init !!\n");
    tone_play(TONE_ECHO_MODE, 0);
    led_idle();//led_fre_set(C_BLED_SLOW_MODE);


    return NULL;
}

static void task_echo_exit(void **priv)
{
    task_clear_all_message();

    echo_stop();
    ui_close_echo();

    printf("task_echo_exit...\n");
}

static void task_echo_deal(void *p)
{
    int msg = NO_MSG;
    int error = MSG_NO_ERROR;
    led_idle();//led_fre_set(C_BLED_SLOW_MODE);

    u8 echo_start_flag = 0;


    printf("\n***********************ECHO TASK********************\n");

    while (1) {
        error = task_get_msg(0, 1, &msg);
        if (NO_MSG == msg) {
            continue;
        }

        if (task_common_msg_deal(p, msg) == false) {
            music_tone_stop();
            task_common_msg_deal(NULL, NO_MSG);
            return ;
        }

        /* if (echo_msg_deal(p, msg) == true) { */
        /*     continue; */
        /* } */

        if ((msg != MSG_HALF_SECOND) && (msg != MSG_ONE_SECOND) && (msg != MSG_FM_SCAN_ALL)) {
            printf("\nfm_msg=   0x%x\n", msg);
        }

        switch (msg) {
        case SYS_EVENT_PLAY_SEL_END:

            if (echo_start_flag == 0) {
                echo_start_flag = 1;
                ui_open_echo(NULL, 0);
            }

            printf("ECHO SYS_EVENT_PLAY_TONE_END\n");
            dac_set_samplerate(SR16000, 0);
            /* msg = MSG_ECHO_START; */
            /* echo_msg_deal(p, msg); */

            break;

        case MSG_HALF_SECOND:
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


const TASK_APP task_echo_info = {
    .skip_check = NULL,//
    .init 		= task_echo_init,
    .exit 		= task_echo_exit,
    .task 		= task_echo_deal,
    .key 		= &task_echo_key,
};

#endif


