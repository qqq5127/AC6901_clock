#include "audio/pdm_link.h"
#include "irq_api.h"
#include "uart.h"
#include "audio/src.h"
#include "audio/dac_api.h"
#include <string.h>
#include "sdk_cfg.h"
#include "clock_api.h"
#include "aec/aec.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".dac_app_bss")
#pragma data_seg(	".dac_app_data")
#pragma const_seg(	".dac_app_const")
#pragma code_seg(	".dac_app_code")
#endif

PLNK_DRV *plnk_ops;

#define PLINK_DIV   	  	60
#define PLINK_CHL   	  	2
#define PLINK_LEN         	64
#define PLINK_CLK2SR_DIV  	50
/*
	sclk = lsb/PLINK_DIV
	dat_sr = sclk/PLINK_CLK2SR_DIV
	e.g.,
	lsb = 24M
	sclk = 24000000/60 = 400000
	dat_sr = 400000/50=8000
*/

s16 plink_buf[PLINK_CHL][2 * PLINK_LEN] sec(.dac_buf_sec) __attribute__((aligned(4)));

static void plink_isr_deal(s16 *buf, u8 buf_flag, u16 len)
{
    u8 cnt;
    s16 *ch0;
    s16 *ch1;
    s16 tmp_buf[PLINK_LEN * 2];

    ch0 = (s16 *)buf;
    ch0 += buf_flag * len;
    //ch1
    ch1 = (s16 *)buf + len * 2;
    ch1 += buf_flag * len;


    if (aec_interface.fill_adc_ref_buf) {
        aec_interface.fill_adc_ref_buf(ch0, len * 2);
    }

#if 0/*PLNK_2_DAC DEBUG*/
    putchar('o');
    for (cnt = 0; cnt < PLINK_LEN; cnt++) {
        tmp_buf[cnt * 2] 		= ch0[cnt];
        tmp_buf[cnt * 2 + 1] 	= ch1[cnt];
    }

    if (is_dac_write_able(len * 2 * 2) != 0) {
        dac_write(tmp_buf, len * 2 * 2);
    }
#endif
}


s32 pdm_link_init(PLNK_DRV *ops)
{
    plnk_ops = ops;
    if (plnk_ops->init) {
        return plnk_ops->init(plink_buf, PLINK_LEN, PLINK_DIV, plink_isr_deal);
    }
    return -1;
}

void pdm_link_enable(void)
{
    if (plnk_ops->on) {
        plnk_ops->on();
    }
}

void pdm_link_disable(void)
{
    if (plnk_ops->off) {
        plnk_ops->off();
    }
}

void pdm_link_demo(u8 en)
{
    if (en) {
        JL_PORTA->DIR |= BIT(0) | BIT(2);    //DAT0 DAT1
        JL_PORTA->DIR &= ~BIT(1);            //SCLK
        pdm_link_init(&plnk_drv_ops);
        pdm_link_enable();
    } else {
        pdm_link_disable();
    }

    //extern void close_wdt(void);
    //close_wdt();
    //while (1);
}
