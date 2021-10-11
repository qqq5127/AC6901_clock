#include "rotate_dec.h"
#include "common.h"
#include "sdk_cfg.h"
#include "board_init.h"

static inline void rotate_dec_set_clk(void)
{
    RDEC_SPD(12);//Tsr = (2^RDEC_SPD)/Flsb, (Tsr建议：0.5ms~2ms)
}

static void __rotate_dec_init(u8 mode)
{
    rotate_dec_set_clk();

    JL_PORTA->DIR |= (BIT(1) | BIT(2));
    JL_PORTA->PU |= (BIT(1) | BIT(2));
    JL_PORTA->PD &= ~(BIT(1) | BIT(2));

    RDEC_POL(mode);
    RDEC_EN(1);
    RDEC_CPND();
    log_printf("JL_RDEC->CON = x%x\n", JL_RDEC->CON);

}
static void rotate_dec_init()
{
    __rotate_dec_init(RDEC_POL_PU);
}
//no_sequence_initcall(rotate_dec_init);

static void rotate_dec_get_dat(void)
{
    s8 ret = 0;
    if (RDEC_PND()) {
        ret = JL_RDEC->DAT;
        RDEC_CPND();
        log_printf("dat = %d\n", ret);
    }
}
//timer detect register
/*
LOOP_DETECT_REGISTER(rotate_detect) = {
    .time = 1,
    .fun  = rotate_dec_get_dat,
};
*/


