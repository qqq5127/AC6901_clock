#include "sdk_cfg.h"
#include "common/common.h"
#include "common/flash_api.h"
#include "board_init.h"
#include "fmtx_api.h"
#include "ui_api.h"

#if FMTX_EN

#define FMTX_DEBUG_ENABLE
#ifdef FMTX_DEBUG_ENABLE
#define fmtx_printf  printf
#else
#define fmtx_printf(...)
#endif

#if QN8027
#include "qn8027.h"
extern FMTX_API qn8027_api;
#endif

#if QN8007
#include "qn8007.h"
extern FMTX_API qn8007_api;
#endif

volatile u8 fmtx_freq_set_time_cnt = 0;
FMTX_API *fmtx_api = NULL;

FMTX_API *fmtx_api_arr[] = {
#if QN8027
    &qn8027_api,
#endif
#if QN8007
    &qn8007_api,
#endif
};

FMTX_VAR fmtx_var = {
    .max_freq = FREQ_MAX,
    .min_freq = FREQ_MIN,
    .freq = FREQ_MIN,
    .state = FREQ_TRANSMIT,
    .freq_set_total_time = UI_FREQ_RETURN, //进入调频模式的总时间
};

void fmtx_dev_init(void)
{
#if (FMTX_CHIP_CLK_SOURCE == CLK_BY_CPU_IO)

    ///*********IO口通过output channel输出时钟************/
    FMTX_CLK_PORT->DIR &= ~FMTX_CLK_BIT;
    FMTX_CLK_PORT->PU |= FMTX_CLK_BIT;
    FMTX_CLK_PORT->PD |= FMTX_CLK_BIT;

#if (FMTX_CHIP_OSC_SELECT == OSC_24M)
    //使用OUT_CH2输出btosc(蓝牙晶振需使用24M)
    FMTX_CLK_PORT->OUT |= FMTX_CLK_BIT;
    FMTX_CLK_PORT->DIE &= ~FMTX_CLK_BIT;
    JL_IOMAP->CON3 &= ~(BIT(24) | BIT(25) | BIT(26) | BIT(27));
    JL_IOMAP->CON3 |= (BIT(24) | BIT(26));
#else
    //使用OUT_CH2输出PLL12M
    FMTX_CLK_PORT->OUT |= FMTX_CLK_BIT;
    FMTX_CLK_PORT->DIE &= ~FMTX_CLK_BIT;
    JL_IOMAP->CON3 &= ~(BIT(24) | BIT(25) | BIT(26) | BIT(27));
    JL_IOMAP->CON3 |= (BIT(25) | BIT(26));
    /****************************************/
#endif

#endif

#if (FMTX_CHIP_SELECT_BY_RES == 1)

#else
    fmtx_api = fmtx_api_arr[0];
#endif
}

void fmtx_var_init(void)
{
    u16 fmtx_fre = 0;

    fmtx_freq_set_time_cnt = 0;

    vm_read(VM_FM_TRANSMIT_FREQ, &fmtx_fre, VM_FM_TRANSMIT_FREQ_LEN);

    if ((fmtx_fre < fmtx_var.min_freq) || (fmtx_fre > fmtx_var.max_freq)) {
        fmtx_fre = fmtx_var.min_freq;
    }
    fmtx_var.freq = fmtx_fre;

    fmtx_printf("\n\n*****FM_Transmitter_VAR*****\n");
    fmtx_printf("fmtx_var.freq         = %d\n", fmtx_var.freq);
    fmtx_printf("fmtx_var.max_freq     = %d\n", fmtx_var.max_freq);
    fmtx_printf("fmtx_var.min_freq     = %d\n", fmtx_var.min_freq);
    fmtx_printf("fmtx_var.freq_set_total_time  = %d\n", fmtx_var.freq_set_total_time);
}

void fmtx_init(void)
{
    fmtx_dev_init();
    fmtx_var_init();

    if (fmtx_api && fmtx_api->init) {
        fmtx_api->init(fmtx_var.freq);
    }

}
void fmtx_setpower(u8 power, u16 fre)
{
    if (fmtx_api && fmtx_api->set_power) {
        fmtx_api->set_power(power, fre);
    }

}
void fmtx_setfre(FREQ_SEL_MODE mode, u16 fre)
{
    u16 fmtx_fre = 0;
    if (fmtx_api == NULL) {
        return;
    }

    if (fmtx_var.state == FREQ_SETTING) {
        fmtx_freq_set_time_cnt = fmtx_var.freq_set_total_time + 2;
    }

    switch (mode) {
    case FREQ_NEXT:
        fmtx_var.freq++;
        if (fmtx_var.freq > fmtx_var.max_freq) {
            fmtx_var.freq = fmtx_var.min_freq;
        }
        if (fmtx_api->set_freq) {
            fmtx_api->set_freq(fmtx_var.freq);
        }
        fmtx_printf("fre:%d\n", fmtx_var.freq);
        vm_write(VM_FM_TRANSMIT_FREQ, &fmtx_var.freq, VM_FM_TRANSMIT_FREQ_LEN);
        break;
    case FREQ_PREV:
        fmtx_var.freq--;
        if (fmtx_var.freq < fmtx_var.min_freq) {
            fmtx_var.freq = fmtx_var.max_freq;
        }
        if (fmtx_api->set_freq) {
            fmtx_api->set_freq(fmtx_var.freq);
        }
        fmtx_printf("fre:%d\n", fmtx_var.freq);
        vm_write(VM_FM_TRANSMIT_FREQ, &fmtx_var.freq, VM_FM_TRANSMIT_FREQ_LEN);
        break;
    case FREQ_SEL:      ///<指定频点
        if (fre < fmtx_var.min_freq || fre > fmtx_var.max_freq) {
            fmtx_printf("---fre is invalid,set fre fail---\n");
        } else {
            fmtx_var.freq = fre;
            if (fmtx_api->set_freq) {
                fmtx_api->set_freq(fmtx_var.freq);
            }
            fmtx_printf("fre:%d\n", fmtx_var.freq);
            vm_write(VM_FM_TRANSMIT_FREQ, &fmtx_var.freq, VM_FM_TRANSMIT_FREQ_LEN);
        }
        break;
    default:
        break;
    }
    /* vm_read(VM_FM_TRANSMIT_FREQ, &fmtx_fre, VM_FM_TRANSMIT_FREQ_LEN); */
    /* fmtx_printf("\nfmtx_var.freq         = %d\n", fmtx_fre); */
}

void fmtx_start(void)
{
    if (fmtx_api && fmtx_api->start) {
        fmtx_api->start();
    }

}

void fmtx_stop(void)
{
    if (fmtx_api && fmtx_api->stop) {
        fmtx_api->stop();
    }

}

FREQ_STATE fmtx_get_state(void)
{
    return fmtx_var.state;
}

void fmtx_set_state(FREQ_STATE state)
{
    fmtx_var.state = state;
    switch (state) {
    case FREQ_TRANSMIT: ///< 正常模式
        fmtx_freq_set_time_cnt = 0;
        break;
    case FREQ_SETTING:   ///频点设置模式
        fmtx_freq_set_time_cnt = fmtx_var.freq_set_total_time;
        break;
    default:
        break;
    }
}

u16 fmtx_get_freq(void)
{
    return fmtx_var.freq;
}

u8 fmtx_get_whether_to_flash_freq(void)
{
    if (fmtx_var.state == FREQ_SETTING) {
        if (fmtx_freq_set_time_cnt > fmtx_var.freq_set_total_time) {
            return 0;	//调频时刚设置完频点那一刻，屏幕不闪烁
        } else {
            return 1;
        }
    } else {
        return 0;
    }

}

void fmtx_freq_set_timeout_check(void)
{
    if (fmtx_var.state == FREQ_SETTING) {
        if (fmtx_freq_set_time_cnt) {
            fmtx_freq_set_time_cnt--;
        } else {
            fmtx_set_state(FREQ_TRANSMIT);
        }
    } else {
        fmtx_freq_set_time_cnt = 0;
    }
}

LOOP_DETECT_REGISTER(fm_transmitter_freq_set_timeout_detect) = {
    .time = 500,
    .fun  = fmtx_freq_set_timeout_check,
};

no_sequence_initcall(fmtx_init);

#else
void fmtx_init(void)
{

}
void fmtx_setfre(FREQ_SEL_MODE mode, u16 fre)
{

}
void fmtx_setpower(u8 power, u16 fre)
{

}
void fmtx_start(void)
{

}
void fmtx_stop(void)
{

}

u16 fmtx_get_freq(void)
{
    return 0;
}
FREQ_STATE fmtx_get_state(void)
{
    return 0;
}
void fmtx_set_state(FREQ_STATE state)
{

}
u8 fmtx_get_whether_to_flash_freq(void)
{
    return 0;
}

#endif

