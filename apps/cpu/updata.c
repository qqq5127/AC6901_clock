#include "updata.h"
#include <stdarg.h>
#include <string.h>
#include "irq_api.h"
#include "crc_api.h"
#include "dev_manage.h"
#include "sd_host_api.h"
#include "led.h"
#include "audio/dac_api.h"
#include "audio/tone.h"
#include "audio/dac.h"
#include "audio/audio.h"
#include "uart.h"
#include "common.h"
#include "task_manager.h"

extern void reset_bt_bredrexm_addr(void);
extern void enter_sys_soft_poweroff();
extern void echo_stop(void);

#define UPDATE_LED_REMIND
#define UPDATE_VOICE_REMIND

#define DEVICE_UPDATE_KEY_ERR  BIT(30)
#define DEVICE_FIRST_START     BIT(31)
//升级文件路径必须是短文件名（8+3）结构，仅支持２层目录
/* const char updata_file_name[] = "/UPDATA/JL_692X.BFU"; */
const char updata_file_name[] = "/UPDATA.BFU";
u32 g_updata_flag = 0;
u16 updata_result_get(u32 first_start_flag)
{
    u16 ret = UPDATA_NON;

    UPDATA_PARM *p = UPDATA_FLAG_ADDR;
    u16 crc_cal;
    crc_cal = crc16(((u8 *)p) + 2, sizeof(UPDATA_PARM) - 2);	//2 : crc_val
    if (crc_cal && crc_cal == p->parm_crc) {
        ret =  p->parm_result;
    }
    memset(p, 0x00, sizeof(UPDATA_PARM));


    g_updata_flag = ret;
    if ((first_start_flag & DEVICE_FIRST_START)) {
        g_updata_flag |= DEVICE_FIRST_START;

    }
    printf("UPDATA_RESULT : 0x%x\n", ret);
    return ret;
}
bool device_is_first_start()
{
    printf("g_updata_flag=0x%x\n", g_updata_flag);
    if ((g_updata_flag & DEVICE_FIRST_START) || (g_updata_flag & DEVICE_UPDATE_KEY_ERR)) {
        puts("\n=================device_is_first_start=========================\n");
        return true;
    }
    return false;
}
void led_update_start(void)
{
#ifdef UPDATE_LED_REMIND
    puts("led_update_start\n");
    R_LED_OFF();
    B_LED_OFF();
#endif
}
void led_update_finish(void)
{
#ifdef UPDATE_LED_REMIND
    puts("led_update_finish\n");
    R_LED_ON();
    B_LED_ON();
#endif
}
extern void set_lowpower_keep_32K_osc_flag(u8 flag);
void update_result_deal()
{
    u8 key_voice_cnt = 0;
    u16 result = 0;
    result = (g_updata_flag & 0xffff);
    printf("<--------update_result_deal=0x%x--------->\n", result);
    if (result == UPDATA_NON) {
        return;
    }
#ifdef UPDATE_VOICE_REMIND
	AMP_UNMUTE();
    set_sys_vol(30, 30, FADE_ON);
	delay_2ms(250);
#endif
#ifdef UPDATE_LED_REMIND
    if (result == UPDATA_SUCC) {
        led_update_finish();
    }
#endif
    while (1) {
        clear_wdt();
        key_voice_cnt++;
#ifdef UPDATE_VOICE_REMIND
        if (result == UPDATA_SUCC) {
            puts("<<<<<<UPDATA_SUCC");
            sin_tone_play(500);
            delay_2ms(500);
            puts(">>>>>>>>>>>\n");
        } else {
            printf("!!!!!!!!!!!!!!!updata waring !!!!!!!!!!!=0x%x\n", result);
            sin_tone_play(1000);
            delay_2ms(500);
        }
#endif
        if (key_voice_cnt > 2) {
            key_voice_cnt = 0;
            delay_2ms(500);
            puts("enter_sys_soft_poweroff\n");
		#if RTC_CLK_EN
            set_lowpower_keep_32K_osc_flag(1);
		#endif
		#if SOFT_POWER_ON_OFF_INSIDE
            enter_sys_soft_poweroff();
		#elif SOFT_POWER_ON_OFF
			while(1)
			{
	            SOFT_POWER_CTL_OFF();
				clear_wdt();
			}
		#else
			JL_POWER->CON |= BIT(4);	//软件复位
		#endif
        }
    }

}
void updata_parm_set(UPDATA_TYPE up_type, void *priv, u32 len)
{
    UPDATA_PARM *p = UPDATA_FLAG_ADDR;

#ifdef UPDATE_LED_REMIND
    led_update_start();
#endif
    p->parm_type = (u16)up_type;
    p->parm_result = (u16)UPDATA_READY;
    memcpy(p->file_patch, updata_file_name, strlen(updata_file_name));
    if (priv) {
        memcpy(p->parm_priv, priv, len);
    } else {
        memset(p->parm_priv, 0x00, sizeof(p->parm_priv));
    }
    p->parm_crc = crc16(((u8 *)p) + 2, sizeof(UPDATA_PARM) - 2);	//2 : crc_val

    printf("UPDATA_PARM_ADDR = 0x%x\n", p);
    printf_buf((void *)p, sizeof(UPDATA_PARM));
}

//reset
void updata_enter_reset(UPDATA_TYPE up_type)
{
    //reser
    JL_POWER->CON |= BIT(4);
}

//j to maskrom
void updata_enter_jump(UPDATA_TYPE up_type)
{
    asm volatile("	movh sp, 0");
    asm volatile("	movl sp, 2560");
    asm volatile("	movh ssp, 0");
    asm volatile("	movl ssp,1536");
    asm volatile("	movs r0, 0");
    asm volatile("	mov r1, r0");
    asm volatile("	mov r2, r0");
    asm volatile("	mov r3, r0");
    asm volatile("	mov r4, r0");
    asm volatile("	mov r5, r0");
    asm volatile("	mov r6, r0");
    asm volatile("	mov r7, r0");
    asm volatile("	mov r8, r0");
    asm volatile("	mov r9, r0");
    asm volatile("	mov r10, r0");
    asm volatile("	mov r11, r0");
    asm volatile("	mov r12, r0");
    asm volatile("	mov r13, r0");
    asm volatile("	mov r14, r0");
    asm volatile("	mov psr, r0");
    asm volatile("	mov rets, r0");
    asm volatile("	mov reti, r0");
    asm volatile("	mov maccl, r0");
    asm volatile("	mov macch, r0");
    asm volatile(" movh reti, 2");
    asm volatile(" movl reti, 270");
    asm volatile(" rti");
}

void updata_mode_api(UPDATA_TYPE up_type, ...)
{
    int dev;
    void *parm = NULL;
#if ECHO_EN
	echo_stop();
#endif

    //step 1: disable irq
    irq_global_disable();
    irq_clear_all_ie();

    //step 2: prepare parm

    va_list argptr;
    va_start(argptr, up_type);
    /* dev = va_arg(argptr, int); */
    va_end(argptr);

    switch (up_type) {
    case SD0_UPDATA:
        dev_io_ctrl(sd0, SDMMC_GET_VAR_ADDR, &parm);
        updata_parm_set(up_type, (void *)parm, sizeof(sSD_NOT_CLEAR));
        break;

    case SD1_UPDATA:
        dev_io_ctrl(sd1, SDMMC_GET_VAR_ADDR, &parm);
        updata_parm_set(up_type, (void *)parm, sizeof(sSD_NOT_CLEAR));
        break;

    case USB_UPDATA:
        updata_parm_set(up_type, NULL, 0);
        break;

    case PC_UPDATA:
        updata_parm_set(up_type, NULL, 0);
        break;

    case UART_UPDATA:
        uart_updata_io_ctrl(&parm);
        //printf("up_io = %x\nup_baud = %d\nup_timeout = %dms\n", ((UPDATA_UART *)parm)->control_io, ((UPDATA_UART *)parm)->control_baud, ((UPDATA_UART *)parm)->control_timeout * 10);
        updata_parm_set(up_type, (void *)parm, sizeof(UPDATA_UART));
        break;

    case BT_UPDATA:
        updata_parm_set(up_type, NULL, 0);
        reset_bt_bredrexm_addr();
        updata_enter_jump(up_type);
        break;

    default:
        break;
    }


    //step 3: enter updata
    //updata_enter_reset(up_type);
    updata_enter_jump(up_type);
}

void device_updata(void *dev)
{
    if (dev == usb) {
        updata_mode_api(USB_UPDATA);
    } else if (dev == sd0) {
        updata_mode_api(SD0_UPDATA);
    } else if (dev == sd1) {
        updata_mode_api(SD1_UPDATA);
    } else {
        puts("device_updata_err\n");
    }
}
