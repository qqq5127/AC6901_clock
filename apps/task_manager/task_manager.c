#include "task_manager.h"
#include "strings.h"
#include "sdk_cfg.h"
#include "task_idle.h"
#include "task_bt.h"
#include "task_music.h"
#include "task_fm.h"
#include "task_pc.h"
#include "task_linein.h"
#include "task_rtc.h"
#include "task_rec.h"
#include "task_echo.h"
#include "task_spdif.h"
#include "common.h"
#include "ui_api.h"
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif


//#define TASK_MG_DEBUG_ENABLE
#ifdef TASK_MG_DEBUG_ENABLE
#define task_mg_printf	log_printf
#else
#define task_mg_printf(...)
#endif

extern void vm_check_api(u8 level);
extern u16 auto_sleep_time_cnt;
extern bool is_dac_mute(void);

#if TASK_MANGER_ENABLE

typedef struct __TASK_MG {
    void 				*hdl;
    void 				*param;
    TASK_ID_TYPE		 cur_id;
    TASK_ID_TYPE		 last_id;
    TASK_ID_TYPE		 target_id;
    u8					 flag;
} TASK_MG;


static TASK_MG task_mg_t = {
    .hdl 		= NULL,
    .cur_id 	= TASK_ID_UNACTIVE,
    .last_id 	= TASK_ID_UNACTIVE,
    .target_id	= TASK_ID_UNACTIVE,
    .flag		= 0,
};



static const TASK_APP *task_app_list[TASK_ID_MAX] = {
    [TASK_ID_BT]		= &task_bt_info,
#if BT_HID_INDEPENDENT_MODE
    [TASK_ID_BT_HID]	= &task_bt_hid_info,
#endif
#if (BT_TWS_LINEIN==0)//开启linein_tws时，linein在蓝牙模式进行linein播放
	#if AUX_EN
    [TASK_ID_LINEIN]  	= &task_linein_info,
    #endif
#endif
    [TASK_ID_MUSIC] 	= &task_music_info,
#if USB_PC_EN
    [TASK_ID_PC]    	= &task_pc_info,
#endif
#if FM_RADIO_EN
    [TASK_ID_FM]    	= &task_fm_info,
#endif
#if RTC_CLK_EN
    [TASK_ID_RTC]       = &task_rtc_info,
    [TASK_ID_ALARM]     = &task_alarm_info,
#endif
#if MIC_REC_EN
    [TASK_ID_REC]       = &task_rec_info,
#endif
#if TASK_ECHO_EN
    [TASK_ID_ECHO]       = &task_echo_info,
#endif
#if SPDIF_EN
    [TASK_ID_SPDIF]     = &task_spdif_info,
#endif
    [TASK_ID_IDLE]  	= &task_idle_info,
};



tbool task_switch(TASK_ID_TYPE target, void *param)
{
    TASK_APP *cur_task = NULL;
    int err = 0;
    u8 i = 0;
    u8 j = 0;
    u8 find = 0;
    u8 task_max = sizeof(task_app_list) / sizeof(task_app_list[0]) - 1 - 2;

    task_mg_printf("task_max = %d !!, fun = %s, line = %d\n", task_max,  __func__, __LINE__);

    if (sys_global_value.mask_task_switch_flag) {
        return false;
    }

    if (target > TASK_ID_MAX) {
        switch (target) {
        case TASK_ID_TYPE_NEXT:
            task_mg_printf("task_next !!\n");
            if (task_mg_t.target_id != TASK_ID_UNACTIVE) {
                i = task_mg_t.target_id;
            } else if (task_mg_t.cur_id != TASK_ID_UNACTIVE) {

                i = task_mg_t.cur_id;
            } else {
                task_mg_printf("no next task err!!, fun = %s, line = %d\n", __func__, __LINE__);
                return false;
            }

            i++;
            if (i >= task_max) {
                i = 0;
            }
            break;
        case TASK_ID_TYPE_PREV:
            task_mg_printf("task_prev !!\n");

            if (task_mg_t.target_id != TASK_ID_UNACTIVE) {
                i = task_mg_t.target_id;
            } else if (task_mg_t.cur_id != TASK_ID_UNACTIVE) {

                i = task_mg_t.cur_id;
            } else {
                task_mg_printf("no prev task err!!, fun = %s, line = %d\n", __func__, __LINE__);
                return false;
            }

            if (i) {
                i--;
            } else {
                i = task_max - 1;
            }
            break;
        case TASK_ID_TYPE_LAST:
            task_mg_printf("task_last !!\n");
            if (task_mg_t.last_id != TASK_ID_UNACTIVE) {
                i = task_mg_t.last_id;
            } else {
                task_mg_printf("no last task err!!, fun = %s, line = %d\n", __func__, __LINE__);
                return false;
            }
            break;

        default:
            break;
        }
    } else {
        i = target;
    }

    ///skip deal
    find = 0;
    j = 0;//统计skip次数，如果超过总数，默认调到模式0(如果有蓝牙建议选择蓝牙作为默认模式)，并且该模式不再做skip检查
    do {
        if (!task_app_list[i]) {
            i++;
            continue;
        }
        if (task_app_list[i]->skip_check) {
            if (task_app_list[i]->skip_check(&param) == false) {
                task_mg_printf("skip to next!!, fun = %s, line = %d\n", __func__, __LINE__);
                find = 0;
                j++;
                if (j >= task_max) { //超过总数，默认调到模式0
                    task_mg_printf("all skip, switch to fisrt app!!, fun = %s, line = %d\n", __func__, __LINE__);
                    i = 0;
                    break;
                }
                i++;
                if (i >= task_max) {
                    i = 0;
                }
            } else {
                find = 1;
            }
        } else {
            find = 1;
        }
    } while (find == 0);


    task_mg_t.target_id = i;
    task_mg_t.param = param;
    task_mg_t.flag = 1;

    return true;
}

extern u32 overlay_task_begin;
extern u32 overlay_task_size;
extern void dac_mute(u8 mute_flag, u8 fade_en);
void task_manager(void)
{
    TASK_APP *cur_task = NULL;
    TASK_ID_TYPE target;
    while (1) {
        task_mg_t.flag = 0;
        target = task_mg_t.target_id;
        if (target < TASK_ID_MAX) {
            if (task_mg_t.cur_id != TASK_ID_UNACTIVE) {
                cur_task = (TASK_APP *)task_app_list[task_mg_t.cur_id];
                if (cur_task->exit) {
                    cur_task->exit(&(task_mg_t.hdl));
                    if (task_mg_t.flag) {
                        //提出模式的时候有模式切换动作
                        task_mg_t.flag = 0;
                        target = task_mg_t.target_id;
                        task_mg_printf(" task switch next!!, fun = %s, line = %d\n", __func__, __LINE__);
                    }
                }
                task_mg_t.last_id = task_mg_t.cur_id;
            }

            vm_check_api(1);

            //cur task init
            cur_task = (TASK_APP *)task_app_list[target];
            /* task_mg_t.priv = priv; */
				
			AMP_UNMUTE_CTL();
			auto_sleep_time_cnt = 0;
			vol_maxmin_play_flag = 0;
			if (is_dac_mute())
			{
				dac_mute(0,1);
			}
			mute_flag = 0;
			rtc_display_cnt = RTC_DISPLAY_BACK_CNT+4;
            alarm_up_flag = 0;
            rtc_alm_coordinate_init();
            if (cur_task->init) {
                memset((u8 *)&overlay_task_begin, 0x0, (u32)&overlay_task_size);
                task_mg_t.hdl = cur_task->init(task_mg_t.param);
                if (task_mg_t.flag) {
                    //初始化的时候有模式切换动作
                    task_mg_t.flag = 0;
                    task_mg_t.cur_id = target;
                    task_mg_printf(" task switch next!!, fun = %s, line = %d\n", __func__, __LINE__);
                    continue;
                }
            }

            key_msg_table_reg(cur_task->key);
            task_mg_t.cur_id = target;
            task_mg_t.target_id = TASK_ID_UNACTIVE;

            if (cur_task != NULL &&  cur_task->task) {
                //creat task
                task_mg_printf(" task switch ok!!, fun = %s, line = %d\n", __func__, __LINE__);
                cur_task->task(task_mg_t.hdl);
            } else {
                task_mg_printf(" task ptr is null !!, fun = %s, line = %d\n", __func__, __LINE__);
            }
        } else {
            task_mg_printf("warnning!! task id err !!, fun = %s, line = %d\n", __func__, __LINE__);
        }
    }
}


TASK_ID_TYPE task_get_cur(void)
{
    return 	task_mg_t.cur_id;
}

TASK_ID_TYPE task_get_last(void)
{
    return 	task_mg_t.last_id;
}

#endif
