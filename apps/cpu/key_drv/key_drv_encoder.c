#include "common/common.h"
#include "board_init.h"
#include "msg.h"
#include "key.h"
#include "key_drv_encoder.h"
#include "task_manager.h"

#if KEY_ENCODER_EN

static void encoder_key_init(void)
{
    ENCODER_INIT();
}

no_sequence_initcall(encoder_key_init);

/**********************************************************
扫描编码器子函数
在编码器引脚A为低电平期间：
编码器引脚B从0到1为正转，编码器引脚B从1到0为反转。
**********************************************************/
static void encoder_scan(void)
{
    static u8 Aold = 0, Bold = 0; //定义了两个变量用来储蓄上一次调用此方法是编码开关两引脚的电平
    static u8 st1 = 0, st2 = 0; //定义了一个变量用来储蓄以前是否出现了两个引脚都为高电平的状态

    ENCODER_INIT();

    if ((A_ENCODER_IN && B_ENCODER_IN)) {
        st1 = 1;
    }

    else if (((!A_ENCODER_IN) && (!B_ENCODER_IN))) {
        st2 = 1;
    }

    if (st1) {
        if (((!A_ENCODER_IN) && (!B_ENCODER_IN))) {
            if (Bold) {   //为高说明编码开关在向加大的方向转
                st1 = 0;
                if (mode_type == TASK_ID_BT)
                {
                    task_post_msg(NULL, 1, MSG_BT_NEXT_FILE);
                }
                else if (mode_type == TASK_ID_MUSIC)
                {
                    task_post_msg(NULL, 1, MSG_MUSIC_NEXT_FILE);
                }
                else if (mode_type == TASK_ID_FM)
                {
                    if (preset_station_flag)
                    {
                        task_post_msg(NULL, 1, MSG_FM_PRESET_UP);
                    }
                    else
                    {
                        task_post_msg(NULL, 1, MSG_FM_NEXT_STEP);
                    }
                }
                else if (mode_type == TASK_ID_RTC)
                {
                    task_post_msg(NULL, 1, MSG_RTC_PLUS);
                }
            }
            if (Aold) {   //为高说明编码开关在向减小的方向转
                st1 = 0;
                if (mode_type == TASK_ID_BT)
                {
                    task_post_msg(NULL, 1, MSG_BT_PREV_FILE);
                }
                else if (mode_type == TASK_ID_MUSIC)
                {
                    task_post_msg(NULL, 1, MSG_MUSIC_PREV_FILE);
                }
                else if (mode_type == TASK_ID_FM)
                {
                    if (preset_station_flag)
                    {
                        task_post_msg(NULL, 1, MSG_FM_PRESET_DOWN);
                    }
                    else
                    {
                        task_post_msg(NULL, 1, MSG_FM_PREV_STEP);
                    }
                }
                else if (mode_type == TASK_ID_RTC)
                {
                    task_post_msg(NULL, 1, MSG_RTC_MINUS);
                }
            }
        }
    }

    if (st2) {
        if (((A_ENCODER_IN) && (B_ENCODER_IN))) {
            if (!Bold) {   //为高说明编码开关在向加大的方向转
                st2 = 0;
                //task_post_msg(NULL, 1, MSG_VOL_UP);
            }
            if (!Aold) {   //为高说明编码开关在向减小的方向转
                st2 = 0;
                //task_post_msg(NULL, 1, MSG_VOL_DOWN);
            }
        }
    }

    if (A_ENCODER_IN) {
        Aold = 1;
    } else {
        Aold = 0;
    }

    if (B_ENCODER_IN) {
        Bold = 1;
    } else {
        Bold = 0;
    }
//
    static u8 Aold_2 = 0, Bold_2 = 0; //定义了两个变量用来储蓄上一次调用此方法是编码开关两引脚的电平
    static u8 st1_2 = 0, st2_2 = 0; //定义了一个变量用来储蓄以前是否出现了两个引脚都为高电平的状态

    ENCODER_INIT_2();

    if ((A_ENCODER_IN_2 && B_ENCODER_IN_2)) {
        st1_2 = 1;
    }

    else if (((!A_ENCODER_IN_2) && (!B_ENCODER_IN_2))) {
        st2_2 = 1;
    }

    if (st1_2) {
        if (((!A_ENCODER_IN_2) && (!B_ENCODER_IN_2))) {
            if (Bold_2) {   //为高说明编码开关在向加大的方向转
                st1_2 = 0;
                task_post_msg(NULL, 1, MSG_VOL_UP_SHORT);
            }
            if (Aold_2) {   //为高说明编码开关在向减小的方向转
                st1_2 = 0;
                task_post_msg(NULL, 1, MSG_VOL_DOWN_SHORT);
            }
        }
    }

    if (st2_2) {
        if (((A_ENCODER_IN_2) && (B_ENCODER_IN_2))) {
            if (!Bold_2) {   //为高说明编码开关在向加大的方向转
                st2_2 = 0;
                //task_post_msg(NULL, 1, MSG_VOL_UP);
            }
            if (!Aold_2) {   //为高说明编码开关在向减小的方向转
                st2_2 = 0;
                //task_post_msg(NULL, 1, MSG_VOL_DOWN);
            }
        }
    }

    if (A_ENCODER_IN_2) {
        Aold_2 = 1;
    } else {
        Aold_2 = 0;
    }

    if (B_ENCODER_IN_2) {
        Bold_2 = 1;
    } else {
        Bold_2 = 0;
    }
}

LOOP_DETECT_REGISTER(encoder_scan_loop) = {
    .time = 1,
    .fun  = encoder_scan,
};

#endif

