#include "key_drv/key_drv_ad.h"
#include "key_drv/key.h"
#include "clock_api.h"
#include "adc_api.h"
#include "timer.h"
#include "sys_detect.h"
#include "rtc/rtc_api.h"
#include "clock.h"
#include "power_manage_api.h"


#if KEY_AD_VDDIO_EN

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

#if AUX_DET_MULTI_AD_KEY

#define AUX_DET_R            2200                                         //外部AUX DET电阻220k
//#define ADC_AUX             (0x3ffL*AUX_DET_R / (AUX_DET_R+R_UP))         //AUX插入时电压
//#define ADC_AUX_IN          ((ADC10_27 + ADC_AUX)/2)                      //AUX插入相当于220K按键按下

//   R并 = R1R2/(R1+R2)
#define Rmul(R1,R2)         ((R1*R2)/(R1+R2))
#define ADC_AUX_220			(0x3ffL*Rmul(AUX_DET_R,2200)/(Rmul(AUX_DET_R,2200) + R_UP))
#define ADC_AUX_100			(0x3ffL*Rmul(AUX_DET_R,1000)/(Rmul(AUX_DET_R,1000) + R_UP))
#define ADC_AUX_51			(0x3ffL*Rmul(AUX_DET_R, 510)/(Rmul(AUX_DET_R, 510) + R_UP))
#define ADC_AUX_33			(0x3ffL*Rmul(AUX_DET_R, 330)/(Rmul(AUX_DET_R, 330) + R_UP))
#define ADC_AUX_24			(0x3ffL*Rmul(AUX_DET_R, 240)/(Rmul(AUX_DET_R, 240) + R_UP))
#define ADC_AUX_15			(0x3ffL*Rmul(AUX_DET_R, 150)/(Rmul(AUX_DET_R, 150) + R_UP))
#define ADC_AUX_10			(0x3ffL*Rmul(AUX_DET_R, 100)/(Rmul(AUX_DET_R, 100) + R_UP))
#define ADC_AUX_62			(0x3ffL*Rmul(AUX_DET_R,  62)/(Rmul(AUX_DET_R,  62) + R_UP))
#define ADC_AUX_3			(0x3ffL*Rmul(AUX_DET_R,  30)/(Rmul(AUX_DET_R,  30) + R_UP))
#define ADC_AUX_0			(0)

#define ADKEY2_0			((ADC_AUX_220 + ADC_AUX_100)/2)
#define ADKEY2_1			((ADC_AUX_100 + ADC_AUX_51) /2)
#define ADKEY2_2			((ADC_AUX_51  + ADC_AUX_33) /2)
#define ADKEY2_3			((ADC_AUX_33  + ADC_AUX_24) /2)
#define ADKEY2_4			((ADC_AUX_24  + ADC_AUX_15) /2)
#define ADKEY2_5			((ADC_AUX_15  + ADC_AUX_10) /2)
#define ADKEY2_6			((ADC_AUX_10  + ADC_AUX_62) /2)
#define ADKEY2_7			((ADC_AUX_62  + ADC_AUX_3)  /2)
#define ADKEY2_8			((ADC_AUX_3   + ADC_AUX_0)  /2)

const u16 ad_key_aux_table[] = {
    ADKEY2_0, ADKEY2_1, ADKEY2_2, ADKEY2_3, ADKEY2_4,
    ADKEY2_5, ADKEY2_6, ADKEY2_7, ADKEY2_8
};
#endif

const u16 ad_key_table[] = {
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
/**@brief   ad按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ad_key0_init(void)
*/
/*----------------------------------------------------------------------------*/
void ad_key_init(void)
{
    s32 ret;
    key_puts("ad key init\n");
#if 0
#if IC_R_UP
    PORTR_PU(AD_KEY_IO_PRX, 1);
#else
    PORTR_PU(AD_KEY_IO_PRX, 0);
#endif
    PORTR_PD(AD_KEY_IO_PRX, 0);
    PORTR_DIR(AD_KEY_IO_PRX, 1);
    PORTR_DIE(AD_KEY_IO_PRX, 1);

#if (AD_KEY_IO_PRX == PORTR1)
    PORTR1_ADCEN_CTL(1);
#elif (AD_KEY_IO_PRX == PORTR2)
    PORTR2_ADCEN_CTL(1);
#endif
#else
#if 1
#if EXTERN_R_UP
    AD_KEY_IO_PX->PU  &= ~BIT(AD_KEY_IO_PAX);
#else
    AD_KEY_IO_PX->PU  |= BIT(AD_KEY_IO_PAX);    //使用内部上拉要打开上拉
#endif
    AD_KEY_IO_PX->PD  &= ~BIT(AD_KEY_IO_PAX);
    AD_KEY_IO_PX->DIR |=  BIT(AD_KEY_IO_PAX);
    AD_KEY_IO_PX->DIE &= ~BIT(AD_KEY_IO_PAX);
#else
    //使用DM做ADKEY
#if EXTERN_R_UP
    USB_DM_PU(0);
#else
    USB_DM_PU(1);
#endif
    USB_DM_PD(0);
    USB_DM_DIR(1);
    USB_DM_DIE(0);
#endif
#endif
    /* adc_init_api(ad_table[0], LSB_CLK, SYS_LVD_EN); */
}
#if ADKEY_SD_MULT_EN
void adkey_sd_mult_set_sd_io()
{
    AD_KEY_IO_PX->PU  |= BIT(AD_KEY_IO_PAX);
    AD_KEY_IO_PX->PD  &= ~BIT(AD_KEY_IO_PAX);
    AD_KEY_IO_PX->DIR &= ~BIT(AD_KEY_IO_PAX);
    AD_KEY_IO_PX->OUT |= BIT(AD_KEY_IO_PAX);
    AD_KEY_IO_PX->DIE |= BIT(AD_KEY_IO_PAX);
    AD_KEY_IO_PX->HD  |= BIT(AD_KEY_IO_PAX);
}
#endif
/*----------------------------------------------------------------------------*/
/**@brief   获取ad按键值
   @param   void
   @param   void
   @return  key_number
   @note    tu8 get_adkey_value(void)
*/
/*----------------------------------------------------------------------------*/
extern u8 check_key_prev(void);
u8 get_adkey_value(void)
{
    u8 key_number;
    u32 key_value;
    static u8 aux_in_cnt = 0;
    static u8 aux_out_cnt = 0;
    key_value = adc_value[R_AD_CH_KEY];

#if AUX_DET_MULTI_AD_KEY
    extern void set_aux_sta(u8 sta);
    extern u8   get_aux_sta(void);
    if (key_value > AD_NOKEY) {
        set_aux_sta(1);
        return NO_KEY;
    } else if (key_value > ADC_AUX_IN) {
        set_aux_sta(0);        //低电平,为AUX插入
        return NO_KEY;
    }

    if (get_aux_sta()) {
        for (key_number = 0; key_number < sizeof(ad_key_aux_table) / sizeof(ad_key_aux_table[0]); key_number++) {
            if (key_value > ad_key_aux_table[key_number]) {
                break;
            }
        }
    } else
#endif //ENDIF AUX_DET_MULTI_AD_KEY
    {

        if (key_value > AD_NOKEY) {
			key_filter(NO_KEY);
            return NO_KEY;
        }
        for (key_number = 0; key_number < sizeof(ad_key_table) / sizeof(ad_key_table[0]); key_number++) {
            if (key_value > ad_key_table[key_number]) {
                break;
            }
        }
    }

    /* log_printf("key_value:%d, vdd_ad_val:%d, key_num:0x%x\n", key_value, key_value * 33 / 0x3ff, key_number); */

    //if ((key_number == 7) && (check_key_prev()))
    {
    //    return key_filter(8);
    }
    return key_filter(9 - key_number);
}



const key_interface_t key_ad_info = {
    .key_type = KEY_TYPE_AD,
    .key_init = ad_key_init,
    .key_get_value = get_adkey_value,
};
#endif
