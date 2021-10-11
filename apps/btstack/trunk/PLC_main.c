#include "PLC_main.h"
#include "aec/sync_out.h"
#include "aec/PLC.h"
#include "audio/audio.h"
#include "uart.h"
#include "sdk_cfg.h"

#define USE_AEC_REPAIR 		1

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".aec_app_bss")
#pragma data_seg(	".aec_app_data")
#pragma const_seg(	".aec_app_const")
#pragma code_seg(	".aec_app_code")
#endif
/*
***********************************************************************************
*					Audio Repair Module
*
*Description: This module is defined for repair loss packets
*
*Argument(s):
*
*Returns	:
*
*Note(s)	:
***********************************************************************************
*/
#if USE_AEC_REPAIR
static u8 repair_buf[1344] AT(.bt_aec) ALIGNED(4);
static u32 repair_cnt = 0;
void audio_repair_init()
{
    s8 err = 0;
    u32 buf_size;

    repair_cnt = 0;
    buf_size = PLC_query();	/*size = 0x540(1344)*/
    log_printf("repair_buf:0x%x\n", buf_size);
    memset(repair_buf, 0, sizeof(repair_buf));

    err = PLC_init(repair_buf);
    if (err != 0) {
        puts("PLC init err\n");
    }
    //puts("PLC init OK\n");
}

#define REPAIR_LEN_MAX	30
void audio_repair_run(s16 *inbuf, s16 *output, u16 point, u8 repair_flag)
{
    u16 repair_len, tmp_len;
    s16 *p_in, *p_out;

    p_in    = inbuf;
    p_out   = output;
    tmp_len = point;

#if 1
    if (repair_flag) {
        repair_cnt++;
        log_printf("[E%d]", repair_cnt);
    } else {
        repair_cnt = 0;
    }
    //printf("[%d]",point);
#endif

    while (tmp_len) {
        repair_len = (tmp_len > REPAIR_LEN_MAX) ? REPAIR_LEN_MAX : tmp_len;
        tmp_len = tmp_len - repair_len;
        PLC_run(p_in, p_out, repair_len, repair_flag);
        p_in  += repair_len;
        p_out += repair_len;
    }
}

void audio_repair_exit()
{
    PLC_exit();
    //puts("audio_repair_exit OK\n");
}

#else
void audio_repair_init() {};
void audio_repair_run(s16 *inbuf, s16 *output, u16 point, u8 repair_flag) {};
void audio_repair_exit() {};
#endif
