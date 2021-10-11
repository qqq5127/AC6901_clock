#include "key_drv/key_drv_ad.h"
#include "key_drv_io.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".key_drv_io_bss")
#pragma data_seg(	".key_drv_io_data")
#pragma const_seg(	".key_drv_io_const")
#pragma code_seg(	".key_drv_io_code")
#endif


#if KEY_IO_EN

#if EXTERN_R_UP
#define R_UP       EXTERN_R_UP//220     //22K
#else
#define R_UP       100    //内部上拉为10K
#endif

#define ADC10_33   (0x3ffL)
#define ADC10_30   (0x3ffL*2200/(2200 + R_UP))     //220K
#define ADC10_27   (0x3ffL*1000/(1000 + R_UP))     //100K
#define ADC10_23   (0x3ffL*510 /(510  + R_UP))     //51K
#define ADC10_20   (0x3ffL*330 /(330  + R_UP))     //33K
#define ADC10_17   (0x3ffL*240 /(240  + R_UP))     //24K
#define ADC10_13   (0x3ffL*150 /(150  + R_UP))     //15K
#define ADC10_10   (0x3ffL*91  /(91   + R_UP))     //9.1K
#define ADC10_07   (0x3ffL*62  /(62   + R_UP))     //6.2K
#define ADC10_04   (0x3ffL*30  /(30   + R_UP))     ///3K
#define ADC10_00   (0)

#define AD_NOKEY        ((ADC10_33 + ADC10_30)/2)
#define ADKEY1_0		((ADC10_30 + ADC10_27)/2)
#define ADKEY1_1		((ADC10_27 + ADC10_23)/2)
#define ADKEY1_2		((ADC10_23 + ADC10_20)/2)
#define ADKEY1_3		((ADC10_20 + ADC10_17)/2)
#define ADKEY1_4		((ADC10_17 + ADC10_13)/2)
#define ADKEY1_5		((ADC10_13 + ADC10_10)/2)
#define ADKEY1_6		((ADC10_10 + ADC10_07)/2)
#define ADKEY1_7		((ADC10_07 + ADC10_04)/2)
#define ADKEY1_8		((ADC10_04 + ADC10_00)/2)

const u16 ad_key2_table[] = {
    ADKEY1_0, ADKEY1_1, ADKEY1_2, ADKEY1_3, ADKEY1_4,
    ADKEY1_5, ADKEY1_6, ADKEY1_7, ADKEY1_8
};

/*----------------------------------------------------------------------------*/
/**@brief   按键去抖函数，输出稳定键值
   @param   key：键值
   @return  稳定按键
   @note    u8 key_filter(u8 key)
*/
/*----------------------------------------------------------------------------*/
static u8 key_filter(u8 key)
{
    static u8 used_key = NO_KEY;
    static u8 old_key;
    static u8 key_counter;

    if (old_key != key) {
        key_counter = 0;
        old_key = key;
    } else {
        key_counter++;
        if (key_counter == KEY_BASE_CNT) {
            used_key = key;
        }
    }
    return used_key;
}

/*----------------------------------------------------------------------------*/
/**@brief   io按键初始化
   @param   void
   @param   void
   @return  void
   @note    void io_key_init(void)
*/
/*----------------------------------------------------------------------------*/
void io_key_init(void)
{
#if 0
#if 0
    PORTR_PU(AD_KEY2_IO_PRX, 0);
    PORTR_PD(AD_KEY2_IO_PRX, 0);
    PORTR_DIR(AD_KEY2_IO_PRX, 1);
    PORTR_DIE(AD_KEY2_IO_PRX, 1);
#if (AD_KEY2_IO_PRX == PORTR1)
    PORTR1_ADCEN_CTL(1);
#elif (AD_KEY2_IO_PRX == PORTR2)
    PORTR2_ADCEN_CTL(1);
#endif
#else
#if EXTERN_R_UP
    AD_KEY2_IO_PX->PU  &= ~BIT(AD_KEY2_IO_PAX);
#else
    AD_KEY2_IO_PX->PU  |= BIT(AD_KEY2_IO_PAX);    //使用内部上拉要打开上拉
#endif
    AD_KEY2_IO_PX->PD  &= ~BIT(AD_KEY2_IO_PAX);
    AD_KEY2_IO_PX->DIR |=  BIT(AD_KEY2_IO_PAX);
    AD_KEY2_IO_PX->DIE &= ~BIT(AD_KEY2_IO_PAX);
#endif
#else
    KEY_INIT();
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief   获取IO按键电平值
   @param   void
   @param   void
   @return  key_num:io按键号
   @note    u8 get_iokey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 get_iokey_value(void)
{
#if 0
    u8 key_number;
    u32 key_value;
    static u8 aux_in_cnt = 0;
    static u8 aux_out_cnt = 0;
    key_value = adc_value[R_AD_CH_KEY2];

    {

        if (key_value > AD_NOKEY) {
			key_filter(NO_KEY);
            return NO_KEY;
        }
        for (key_number = 0; key_number < sizeof(ad_key2_table) / sizeof(ad_key2_table[0]); key_number++) {
            if (key_value > ad_key2_table[key_number]) {
                break;
            }
        }
    }

    /* log_printf("key_value:%d, vdd_ad_val:%d, key_num:0x%x\n", key_value, key_value * 33 / 0x3ff, key_number); */

    return key_filter(9 - key_number);
#else
    //key_puts("get_iokey_value\n");
    u8 key_num = NO_KEY;

    if (IS_KEY0_DOWN()) {
        key_puts(" KEY0 ");
        key_num = 0;
    } else if (IS_KEY1_DOWN()) {
        key_puts(" KEY1 ");
        key_num = 1;
    } else if (IS_KEY2_DOWN()) {
        key_puts(" KEY2 ");
        key_num = 2;
    } else if (IS_KEY3_DOWN()) {
        key_puts(" KEY3 ");
        key_num = 3;
    }

    return key_filter(key_num);
#endif
}

const key_interface_t key_io_info = {
    .key_type = KEY_TYPE_IO,
    .key_init = io_key_init,
    .key_get_value = get_iokey_value,
};

u8 check_key_prev(void)
{
#if 0
    u32 key_value;
    key_value = adc_value[R_AD_CH_KEY2];
    if ((key_value > ADKEY1_7)&&(key_value < ADKEY1_6))
    {
        return 1;
    }
#endif
    return 0;
}

#endif/*KEY_IO_EN*/
