#include "key_drv_ir.h"
#include "irq_api.h"
#include "clock_interface.h"
#include "common.h"

#include "uart.h"

#if KEY_IR_EN

IR_CODE  ir_code;       ///<红外遥控信息
u16 timer1_pad;

static const u16 timer_div[] = {
    /*0000*/    1,
    /*0001*/    4,
    /*0010*/    16,
    /*0011*/    64,
    /*0100*/    2,
    /*0101*/    8,
    /*0110*/    32,
    /*0111*/    128,
    /*1000*/    256,
    /*1001*/    4 * 256,
    /*1010*/    16 * 256,
    /*1011*/    64 * 256,
    /*1100*/    2 * 256,
    /*1101*/    8 * 256,
    /*1110*/    32 * 256,
    /*1111*/    128 * 256,
};


#define IRTabFF00_NUMBER		21
const u8 IRTabFF00[IRTabFF00_NUMBER] = {
#if 0
    NKEY_00, NKEY_01, NKEY_02, NKEY_03, NKEY_04, NKEY_05, NKEY_06, IR_06, IR_15, IR_08, NKEY_0A, NKEY_0B, IR_12, IR_11, NKEY_0E, NKEY_0F,
    NKEY_10, NKEY_11, NKEY_12, NKEY_13, NKEY_14, IR_07, IR_09, NKEY_17, IR_13, IR_10, NKEY_1A, NKEY_1B, IR_16, NKEY_1D, NKEY_1E, NKEY_1F,
    NKEY_20, NKEY_21, NKEY_22, NKEY_23, NKEY_24, NKEY_25, NKEY_26, NKEY_27, NKEY_28, NKEY_29, NKEY_2A, NKEY_2B, NKEY_2C, NKEY_2D, NKEY_2E, NKEY_2F,
    NKEY_30, NKEY_31, NKEY_32, NKEY_33, NKEY_34, NKEY_35, NKEY_36, NKEY_37, NKEY_38, NKEY_39, NKEY_3A, NKEY_3B, NKEY_3C, NKEY_3D, NKEY_3E, NKEY_3F,
    IR_04, NKEY_41, IR_18, IR_05, IR_03, IR_00, IR_01, IR_02, NKEY_48, NKEY_49, IR_20, NKEY_4B, NKEY_4C, NKEY_4D, NKEY_4E, NKEY_4F,
    NKEY_50, NKEY_51, IR_19, NKEY_53, NKEY_54, NKEY_55, NKEY_56, NKEY_57, NKEY_58, NKEY_59, IR_17, NKEY_5B, NKEY_5C, NKEY_5D, IR_14, NKEY_5F,
#else
	0x45,	0x46,	0x47,
	0x44,	0x40,	0x43,
	0x07,	0x15,	0x09,
	0x16,	0x19,	0x0d,
	0x0c,	0x18,	0x5e,
	0x08,	0x1c,	0x5a,
	0x42,	0x52,	0x4a
#endif
};

#define IRTab7F80_NUMBER		21
const u8 IRTab7F80[IRTab7F80_NUMBER] = {
	0x12,	0x1A,	0x1E,
	0x01,	0x02,	0x03,
	0x04,	0x05,	0x06,
	0x07,	0x08,	0x09,
	0x0A,	0x1B,	0x1F,
	0x0C,	0x0D,	0x0E,
	0x00,	0x0F,	0x19
};


/*----------------------------------------------------------------------------*/
/**@brief   ad按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ad_key0_init(void)
*/
/*----------------------------------------------------------------------------*/
static void ir_timeout(void)
{
    ir_code.boverflow++;
    if (ir_code.boverflow > 56) { //56*2ms ~= 112ms
        ir_code.bState = 0;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   time1红外中断服务函数
   @param   void
   @param   void
   @return  void
   @note    void timer1_ir_isr(void)
*/
/*----------------------------------------------------------------------------*/
void timer1_isr_fun(void)
{
    u16 bCap1;
    u8 cap = 0;

    JL_TIMER1->CON |= BIT(14);

    bCap1 = JL_TIMER1->PRD;
    JL_TIMER1->CNT = 0;
    cap = bCap1 / timer1_pad;

    if (cap <= 1) {
        ir_code.wData >>= 1;
        ir_code.bState++;
        ir_code.boverflow = 0;
    } else if (cap == 2) {
        ir_code.wData >>= 1;
        ir_code.bState++;
        ir_code.wData |= 0x8000;
        ir_code.boverflow = 0;
    }
    /*13ms-Sync*/
    /*
    else if ((cap == 13) || (cap < 8) || (cap > 110))
    {
        ir_code.bState = 0;
    }
    else
    {
        ir_code.boverflow = 0;
    }
    */
    else if ((cap == 13) && (ir_code.boverflow < 8)) {
        ir_code.bState = 0;
    } else if ((cap < 8) && (ir_code.boverflow < 5)) {
        ir_code.bState = 0;
    } else if ((cap > 110) && (ir_code.boverflow > 53)) {
        ir_code.bState = 0;
    } else if ((cap > 20) && (ir_code.boverflow > 53)) { //溢出情况下 （12M 48M）
        ir_code.bState = 0;
    } else {
        ir_code.boverflow = 0;
    }
    if (ir_code.bState == 16) {
        ir_code.wUserCode = ir_code.wData;
    }
    if (ir_code.bState == 32) {
        /* printf("[0x%X]",ir_code.wData); */
    }
}
IRQ_REGISTER(IRQ_TIME1_IDX, timer1_isr_fun);

/*----------------------------------------------------------------------------*/
/**@brief   ir按键初始化
   @param   void
   @param   void
   @return  void
   @note    void set_ir_clk(void)

   ((cnt - 1)* 分频数)/lsb_clk = 1ms
*/
/*----------------------------------------------------------------------------*/
#define MAX_TIME_CNT 0x07ff //分频准确范围，更具实际情况调整
#define MIN_TIME_CNT 0x0030
void set_ir_clk(void)
{
    u32 clk;
    u32 prd_cnt;
    u8 index;

    clk = OSC_Hz;//clock_get_lsb_freq();

    clk /= 1000;
    clk *= 1; //1ms for cnt

    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = (clk + timer_div[index]) / timer_div[index];
        if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT) {
            break;
        }
    }

    timer1_pad = prd_cnt;
    JL_TIMER1->CON = ((index << 4) | BIT(3) | BIT(1) | BIT(0));

    //log_printf("-------------\ntimmer2 prd_cnt = %x\n\n\n\n\n",prd_cnt);
    //log_printf("timmer2 index = %x\n\n\n\n\n",index);
}

static void irflt_io_set(u8 value)
{

    JL_IOMAP->CON2 &= ~(BIT(8) | BIT(9) | BIT(10) | BIT(11) | BIT(12) | BIT(13));
    if ((value <= 0x0f) && (value >= 0)) {
        JL_PORTA->DIR |= BIT(value);
    } else if ((value <= 0x1f) && (value >= 0x10)) {
        JL_PORTB->DIR |= BIT(value - 0x10);
    } else if ((value <= 0x2f) && (value >= 0x20)) {
        JL_PORTC->DIR |= BIT(value - 0x20);
    } else if ((value <= 37) && (value >= 30)) {
        JL_PORTD->DIR |= BIT(value - 0x30);
    } else if (value == 0x3d) {
        JL_USB->CON0 = BIT(0);  //dp
        JL_USB->IO_CON0 |= BIT(2);  //dp
        JL_USB->IO_CON0 &= ~BIT(4);  //dp
    } else if (value == 0x3e) {
        JL_USB->CON0 = BIT(0);  //dp
        JL_USB->IO_CON0 |= BIT(3);   //dm
        JL_USB->IO_CON0 &= ~BIT(7);  //dp
    } else {
        puts("err");
    }

    JL_IOMAP->CON2 |= (value << 8);
}


/*----------------------------------------------------------------------------*/
/**@brief   ir按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ir_key_init(void)
*/
/*----------------------------------------------------------------------------*/
void ir_key_init(void)
{
    //timer1
    key_puts("ir key init\n");

    IRQ_REQUEST(IRQ_TIME1_IDX, timer1_isr_fun) ; //timer0_isr

#if 1		//PORT->IRFLT->TIMER
    JL_IR->RFLT_CON = 0;
    JL_IR->RFLT_CON |= BIT(7);		//256 div
    JL_IR->RFLT_CON |= BIT(3);		//osc
    JL_IR->RFLT_CON |= BIT(0);		//irflt enable

    JL_IOMAP->CON0 &= ~(BIT(10) | BIT(9) | BIT(8));
    JL_IOMAP->CON0 |= (BIT(10) | BIT(8));                 //IRFLT output to timer1

    irflt_io_set(IR_IO);
#else		//PORT->TIMER
    JL_IOMAP->CON0 &= ~(BIT(10) | BIT(9) | BIT(8));
    JL_IR->RFLT_CON = 0;
    JL_PORTC->DIR |= BIT(2);	//cap1 (PORTC2) for timer1
    /* JL_PORTA->DIR |= BIT(12);	//cap2 (PORTA12) for timer2 */
    /* JL_PORTA->DIR |= BIT(2);	//cap3 (PORTA2) for timer3 */
#endif

    set_ir_clk();
}

/*----------------------------------------------------------------------------*/
/**@brief   获取ir按键值
   @param   void
   @param   void
   @return  void
   @note    void get_irkey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 get_irkey_value(void)
{
    u8 tkey = 0xff;
	u8 i;
    if (ir_code.bState != 32) {
        return tkey;
    }

    /* printf("(0x%X_%d)",(u8)ir_code.wUserCode,(u8)ir_code.wData); */
    if ((((u8 *)&ir_code.wData)[0] ^ ((u8 *)&ir_code.wData)[1]) == 0xff) {
        if (ir_code.wUserCode == 0xFF00)
        {
            /* printf("<%d>",(u8)ir_code.wData); */
		#if 0
            tkey = IRTabFF00[(u8)ir_code.wData];
		#else
			for (i=0;i<IRTabFF00_NUMBER;i++)
			{
				if (IRTabFF00[i] == ((u8)ir_code.wData))
				{
					tkey = i;
					break;
				}
			}
		#endif
        }
	#if USE_MULTI_REMOTE
		else if (ir_code.wUserCode == 0x7F80)
		{
			for (i=0;i<IRTab7F80_NUMBER;i++)
			{
				if (IRTab7F80[i] == ((u8)ir_code.wData))
				{
					tkey = i + IRTabFF00_NUMBER;
					break;
				}
			}
		}
	#endif
    } else {
        ir_code.bState = 0;
    }

    /* if(tkey != NO_KEY){ */
    /* log_printf("tch %x", tkey); */
    /* } */

    return tkey;
}

const key_interface_t key_ir_info = {
    .key_type = KEY_TYPE_IR,
    .key_init = ir_key_init,
    .key_get_value = get_irkey_value,
};

LOOP_DETECT_REGISTER(ir_timeout_detect) = {
    .time = 1,
    .fun  = ir_timeout,
};

#endif/*KEY_IR_EN*/
