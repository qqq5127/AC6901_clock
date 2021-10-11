#include "key_drv/key_drv_ad.h"
#include "key_drv/key.h"
#include "clock_api.h"
#include "adc_api.h"
#include "timer.h"
/* #include "sys_detect.h" */
#include "rtc/rtc_api.h"
#include "clock.h"
#include "power_manage_api.h"


#if KEY_AD_RTCVDD_EN


#define FULL_ADC   0x3ffL

#if EXTERN_R_UP
#define R_UP       EXTERN_R_UP       //外部上拉
#define IC_R_UP    0        //用芯片内部上拉10K，用外部上拉时设置为0
#else
#define IC_R_UP    100        //用芯片内部上拉10K，用外部上拉时设置为0
#endif

#define ADC10_33(x)   (x)
#define ADC10_30(x)   (x*2200L/(2200 + R_UP))     //220K
#define ADC10_27(x)   (x*1000L/(1000 + R_UP))     //100K
#define ADC10_23(x)   (x*510L /(510  + R_UP))     //51K
#define ADC10_20(x)   (x*330L /(330  + R_UP))     //33K
#define ADC10_17(x)   (x*240L /(240  + R_UP))     //24K
#define ADC10_13(x)   (x*150L /(150  + R_UP))     //15K
#define ADC10_10(x)   (x*91L  /(91   + R_UP))     //9.1K
#define ADC10_07(x)   (x*62L  /(62   + R_UP))     //6.2K
#define ADC10_04(x)   (x*30L  /(30   + R_UP))     //3K
#define ADC10_00(x)   (0)

#define AD_NOKEY(x)     ((ADC10_33(x) + ADC10_30(x))/2)
#define ADKEY1_0(x)		((ADC10_30(x) + ADC10_27(x))/2)
#define ADKEY1_1(x)		((ADC10_27(x) + ADC10_23(x))/2)
#define ADKEY1_2(x)		((ADC10_23(x) + ADC10_20(x))/2)
#define ADKEY1_3(x)		((ADC10_20(x) + ADC10_17(x))/2)
#define ADKEY1_4(x)		((ADC10_17(x) + ADC10_13(x))/2)
#define ADKEY1_5(x)		((ADC10_13(x) + ADC10_10(x))/2)
#define ADKEY1_6(x)		((ADC10_10(x) + ADC10_07(x))/2)
#define ADKEY1_7(x)		((ADC10_07(x) + ADC10_04(x))/2)
#define ADKEY1_8(x)		((ADC10_04(x) + ADC10_00(x))/2)



#define IC_ADC10_33(x)   (x)
#define IC_ADC10_20(x)   (x*470L/(470 + IC_R_UP))     //47K
#define IC_ADC10_10(x)   (x*200L/(200 + IC_R_UP))     //20K
#define IC_ADC10_00(x)   (0)


#define IC_AD_NOKEY(x)     ((IC_ADC10_33(x) + IC_ADC10_20(x))/2)
#define IC_ADKEY1_0(x)     ((IC_ADC10_20(x) + IC_ADC10_10(x))/2)
#define IC_ADKEY1_1(x)	   ((IC_ADC10_10(x) + IC_ADC10_00(x))/2)

u16 ad_rtcvdd_key_table[] = {
#if IC_R_UP
    IC_ADKEY1_0(FULL_ADC), IC_ADKEY1_1(FULL_ADC)
#else
    ADKEY1_0(FULL_ADC), ADKEY1_1(FULL_ADC), ADKEY1_2(FULL_ADC), ADKEY1_3(FULL_ADC), ADKEY1_4(FULL_ADC),
    ADKEY1_5(FULL_ADC), ADKEY1_6(FULL_ADC), ADKEY1_7(FULL_ADC), ADKEY1_8(FULL_ADC)
#endif
};

volatile u8 adkey_lock_cnt = 0;
static u8 rtcvdd_cnt = 10;
static u8 rtcvdd_full_cnt = 0xff;
u16 rtcvdd_full_value = FULL_ADC;
u16 max_value = 0;
u16 min_value = 0xffff;
u32 total_value = 0;
static u8 check_rtcvdd_cnt = 0;

static void set_rtcvdd_table(u16 adc_rtcvdd)
{
    u16 rtcvdd;
    rtcvdd = adc_rtcvdd;
#if IC_R_UP
    ad_rtcvdd_key_table[0] = IC_ADKEY1_0(rtcvdd);
    ad_rtcvdd_key_table[1] = IC_ADKEY1_1(rtcvdd);
#else
    ad_rtcvdd_key_table[0] = ADKEY1_0(rtcvdd);
    ad_rtcvdd_key_table[1] = ADKEY1_1(rtcvdd);
    ad_rtcvdd_key_table[2] = ADKEY1_2(rtcvdd);
    ad_rtcvdd_key_table[3] = ADKEY1_3(rtcvdd);
    ad_rtcvdd_key_table[4] = ADKEY1_4(rtcvdd);
    ad_rtcvdd_key_table[5] = ADKEY1_5(rtcvdd);
    ad_rtcvdd_key_table[6] = ADKEY1_6(rtcvdd);
    ad_rtcvdd_key_table[7] = ADKEY1_7(rtcvdd);
    ad_rtcvdd_key_table[8] = ADKEY1_8(rtcvdd);
#endif
}
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


static void SET_ADKEY_LOCK_CNT(u8 cnt)
{
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();

    adkey_lock_cnt = cnt;

    OS_EXIT_CRITICAL();
}

static u8 GET_ADKEY_LOCK_CNT(void)
{
    u8 val;
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();

    val = adkey_lock_cnt;

    OS_EXIT_CRITICAL();
    return val;
}

static void POST_ADKEY_LOCK_CNT(void)
{
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();

    adkey_lock_cnt --;

    OS_EXIT_CRITICAL();
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

    /* adc_init_api(ad_table[0], LSB_CLK, SYS_LVD_EN); */
}

void ad_rtcvdd_key_filter(u8 cnt)
{
    SET_ADKEY_LOCK_CNT(cnt);
}


/*把cnt个值里的最大值和最小值去掉，求剩余cnt-2个数的平均值*/
static u16 rtcvdd_full_vaule_update(u16 value)
{
    u16 full_value = FULL_ADC;
    if (rtcvdd_full_cnt == 0xff) {
        rtcvdd_full_cnt = 0;
        SET_ADKEY_LOCK_CNT(20);
        return value;   //first time
    } else {
        rtcvdd_full_cnt ++;
        if (value > max_value) {
            max_value = value;
        }
        if (value < min_value) {
            min_value = value;
        }
        total_value += value;

        /* printf("%d %x %x %x %x\n",rtcvdd_full_cnt , value , max_value , min_value , total_value); */
        if (rtcvdd_full_cnt > 10 - 1) { //算10个数
            full_value = (total_value - max_value - min_value) / (rtcvdd_full_cnt - 2);
            /* printf("-----%x %x %x\n",full_value,AD_NOKEY(full_value),ADKEY1_0(full_value)); */
            rtcvdd_full_cnt = 0;
            max_value = 0;
            min_value = 0xffff;
            total_value = 0;
        } else {
            return rtcvdd_full_value;
        }
    }
    return full_value;
}

/*检测到RTCVDD 比 VDDIO 高的时候自动把RTCVDD降一档*/
static u8 rtcvdd_auto_match_vddio_lev(u32 rtcvdd_value)
{
    u8 rtcvdd_lev = 0;
    if (rtcvdd_value >= FULL_ADC) { //trim rtcvdd < vddio
        if (rtcvdd_cnt > 10) {
            rtcvdd_cnt = 0;
            rtcvdd_lev = get_rtcvdd_level();
            rtcvdd_lev++;
            if (rtcvdd_lev < 8) {
                log_printf("trim rtcvdd - %d\n", rtcvdd_lev);
                set_rtcvdd_level(rtcvdd_lev);
                SET_ADKEY_LOCK_CNT(20);
                return 1;
            }
        } else {
            rtcvdd_cnt ++;
        }
    } else {
        rtcvdd_cnt = 0;
        rtcvdd_full_value = rtcvdd_full_vaule_update(rtcvdd_value);
    }
    return 0;
}



/*----------------------------------------------------------------------------*/
/**@brief   获取ad按键值
   @param   void
   @param   void
   @return  key_number
   @note    tu8 get_adkey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 get_adkey_value(void)
{
    u8 key_number;
    u32 key_value;
    u16 rtcvdd_value;

    rtcvdd_value = 2 * adc_value[R_AD_CH_RTCVDD];

    /* printf("rtcvdd_value:%x\n",rtcvdd_value); */

    if (rtcvdd_auto_match_vddio_lev(rtcvdd_value)) {
        return NO_KEY;
    }

    key_value = adc_value[R_AD_CH_KEY];

    if (GET_ADKEY_LOCK_CNT()) {
        POST_ADKEY_LOCK_CNT();
        return NO_KEY;
    }

#if IC_R_UP
    if (key_value > IC_AD_NOKEY(rtcvdd_full_value)) {
        return NO_KEY;
    }
#else
    if (key_value > AD_NOKEY(rtcvdd_full_value)) {
        return NO_KEY;
    }
#endif

    set_rtcvdd_table(rtcvdd_full_value);

    for (key_number = 0; key_number < sizeof(ad_rtcvdd_key_table) / sizeof(ad_rtcvdd_key_table[0]); key_number++) {
        if (key_value > ad_rtcvdd_key_table[key_number]) {
            break;
        }
    }

    //log_printf("adkey_value:%x  rtcvd_value:%x   key_num:0x%x\n", key_value, rtcvdd_value, key_number);

    return key_filter(9 - key_number);
}

const key_interface_t key_ad_info = {
    .key_type = KEY_TYPE_AD,
    .key_init = ad_key_init,
    .key_get_value = get_adkey_value,
};
#endif
