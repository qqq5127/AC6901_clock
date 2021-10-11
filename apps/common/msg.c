#include "msg.h"
#include "common.h"
#include "string.h"
#include "wdt.h"
#include "stdarg.h"
#include "circular_buf.h"
#include "uart.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif

static const u16 evnet2msg[MAX_EVENT] = {
    MSG_PC_ONLINE,
    MSG_PC_OFFLINE,
    MSG_HUSB_ONLINE,
    MSG_HUSB_OFFLINE,
    MSG_SD0_ONLINE,
    MSG_SD0_OFFLINE,
    MSG_SD1_ONLINE,
    MSG_SD1_OFFLINE,
    MSG_SUSB_ONLINE,
    MSG_SUSB_OFFLINE,
    MSG_AUTOMUTE_ON,
    MSG_AUTOMUTE_OFF,
    MSG_SD0_MOUNT_SUCC,
    MSG_SD1_MOUNT_SUCC,
    MSG_USB_MOUNT_SUCC,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    NO_MSG,
    MSG_ACTIVE_TASK,
    MSG_ONE_SECOND,
    MSG_DEV_DETECT,
    MSG_HALF_SECOND,
};

static cbuffer_t msg_cbuf;
static u32 event_buf_L = 0;
static u32 event_buf_H = 0;
static u32 msg_pool[MAX_POOL];

bool get_event_status(u32 event)
{
    bool ret = false;
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    if (event < 32) {
        if (event_buf_L & BIT(event)) {
            ret = true;
        }
    } else {
        if (event_buf_H & BIT(event - 32)) {
            ret = true;
        }
    }
    OS_EXIT_CRITICAL();
    return ret;
}

static void clear_one_event(u32 event)
{
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    if (event < 32) {
        event_buf_L &= ~BIT(event);
    } else {
        event_buf_H &= ~BIT(event - 32);
    }
    OS_EXIT_CRITICAL();
}

static u32 get_event(void)
{
    u32 i;

    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    if (event_buf_L == 0 && event_buf_H == 0) {
        OS_EXIT_CRITICAL();
        return NO_EVENT;    //no event
    }
    for (i = 0; i < MAX_EVENT; i++) {
        if (i < 32) {
            if (event_buf_L & BIT(i)) {
                OS_EXIT_CRITICAL();
                return i;    //get a event
            }
        } else {
            if (event_buf_H & BIT(i - 32)) {
                OS_EXIT_CRITICAL();
                return i;    //get a event
            }
        }
    }
    OS_EXIT_CRITICAL();

    return NO_EVENT;
}

int task_get_msg(u16 timeout, int len, int *msg)
{
    u16 msg_value = 0x0fff;
    u8  param_len = 0;
    int i = 0;
    int param;
    u32 event, event_to_msg;
    //debug
    for (i = 0; i < len; i++) {
        msg[i] = 0x0fff;
    }
    //get_msg
    clear_wdt();
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    event = get_event();
    if (event != NO_EVENT) {
        clear_one_event(event);
        event_to_msg = evnet2msg[event];
        msg[0] = event_to_msg;
        //printf("event_mag %d\n ", event_to_msg);
        OS_EXIT_CRITICAL();
        return MSG_NO_ERROR;
    }
    if (2 != cbuf_read(&msg_cbuf, (void *)&msg_value, 2)) {
        /* memset(msg, NO_MSG, len); */
        OS_EXIT_CRITICAL();

        /*get no msg,cpu enter idle*/
        __builtin_pi32_idle();

        return MSG_NO_ERROR;
    }
    msg[0] = msg_value & 0x0fff;
    param_len = msg_value >> 12;
    for (i = 0 ; i < param_len; i++) {
        cbuf_read(&msg_cbuf, (void *)&param, 4);
        if (i + 1 < len) {
            msg[i + 1] = param;
        }
    }
    if (i >= len) {
        puts("msg_buf_not_enc\n");
        OS_EXIT_CRITICAL();
        return MSG_BUF_NOT_ENOUGH;
    }
    OS_EXIT_CRITICAL();
    return MSG_NO_ERROR;
}
int task_post_event(char *name, int argc, ...)
{
    int  param_len = 0;
    int param = 0;
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    va_list argptr;
    va_start(argptr, argc);
    if (argc != 1) {
        OS_EXIT_CRITICAL();
        va_end(argptr);
        return MSG_EVENT_PARAM_ERROR ;
    }
    //printf("param_len %d \n",argc );
    //for(i=0;i<argc;++i)
    {
        param = va_arg(argptr, int);
        if (param >= 32) {
            event_buf_H |= BIT(param - 32);
        } else {
            event_buf_L |= BIT(param);
        }
        //printf("param  %d \n",param );
    }
    va_end(argptr);
    OS_EXIT_CRITICAL();
    return MSG_NO_ERROR;
}
int task_post_msg(char *name, int argc, ...)
{
    u16 msg_value = 0x0fff;
    int i = 0;
    int  param_len = 0;
    int param = 0;
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    va_list argptr;
    va_start(argptr, argc);
    for (i = 0; i < argc; ++i) {
        if (i == 0) {
            msg_value = 0x0fff & va_arg(argptr, int);
            if (msg_value == MSG_CLEAN_ALL_MSG) {
                cbuf_clear(&msg_cbuf);
                va_end(argptr);
                OS_EXIT_CRITICAL();
                return MSG_NO_ERROR;
            }
            param_len = argc - 1;
            msg_value = (param_len << 12) | msg_value;
            cbuf_write(&msg_cbuf, (void *)&msg_value, 2);

        } else {
            param = va_arg(argptr, int);
            cbuf_write(&msg_cbuf, (void *)&param, 4);
        }
    }
    va_end(argptr);
    OS_EXIT_CRITICAL();
    return MSG_NO_ERROR;
}
void task_clear_all_message(void)
{
    cbuf_clear(&msg_cbuf);
}

void task_message_init()
{
    cbuf_init(&msg_cbuf, msg_pool, sizeof(msg_pool));
    cbuf_clear(&msg_cbuf);
}
static void half_second_msg()
{
    static u8 half_sec_cnt = 0;
    task_post_event(NULL, 1,  EVENT_HALF_SECOND);
    if (++half_sec_cnt == 2) {
        half_sec_cnt = 0;
        task_post_event(NULL, 1, EVENT_ONE_SECOND);
    }
}
void active_task_schedule_msg()
{
    task_post_event(NULL, 1,  EVENT_ACTIVE_TASK);
}

LOOP_DETECT_REGISTER(half_second_det) = {
    .time = 250,
    .fun  = half_second_msg,
};
