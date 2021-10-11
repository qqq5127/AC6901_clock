#include "sdk_cfg.h"
#include "audio/dac_api.h"
#include "audio/eq.h"
#include "audio/eq_tab.h"
#include "crc_api.h"
#include "dev_manage.h"
#include "fs.h"
#include "ui/ui_api.h"
#include "crc_api.h"
#include "uart_param.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".eq_app_bss")
#pragma data_seg(	".eq_app_data")
#pragma const_seg(	".eq_app_const")
#pragma code_seg(	".eq_app_code")
#endif

static const struct eq_driver *eq_ops = NULL;
EQ_ARG eq_arg;

/*
 *****************************************************************************
 *
 *						HARDWARE EQ API
 *
 *
 *
 *
 *****************************************************************************
 */
typedef struct {
    u32 type;             ///<EQ TYPE
    u32 seg_num;          ///<EQ segments
    int freq_gain[7][10]; ///<EQ gain for each segments
    int global_gain[7];   ///<EQ sum gain
    int filt_0[50];       ///<EQ filter 0
    int filt_1[50];       ///<EQfilter 1
    int filt_2[50];       ///<EQfilter 2
    int filt_3[50];       ///<EQfilter 3
    int filt_4[50];       ///<EQfilter 4
    int filt_5[50];       ///<EQfilter 5
    int filt_6[50];       ///<EQfilter 6
    int filt_7[50];       ///<EQfilter 7
    int filt_8[50];       ///<EQfilter 8
    int crc16;
} EQ_CFG;
EQ_CFG eq_cfg;

s32 eq_cfg_read(void)
{
    char *path = NULL;
    s32 ret = 0;
    DEV_HANDLE dev = cache;
#if (AUDIO_EFFECT & AUDIO_EFFECT_HW_EQ)
    path = "/eq_cfg_hw.bin";
#endif
#if (AUDIO_EFFECT & AUDIO_EFFECT_SW_EQ)
    path = "/eq_cfg_sw.bin";
#endif

    if (path == NULL) {
        return -1;
    }

    log_printf("read:%s\n", path);
    ret = fs_open_file_bypath(dev, &eq_cfg, sizeof(EQ_CFG), path);
    if (ret == false) {
        puts("open eq cfg file error!\n");
        return -1;
    }
    puts("eq_cfg_read OK\n");
    return 0;
}

static s32 eq_cfg_check(void)
{
    //put_buf((u8 *)&eq_cfg, sizeof(EQ_CFG));
    if ((eq_cfg.crc16 == crc16((u8 *)&eq_cfg, sizeof(EQ_CFG) - 4)) && (eq_cfg.crc16 != 0)) {
        puts("EQ cfg crc ok\n");
        return 0;
    } else {
        puts("EQ cfg crc err\n");
        return -1;
    }
}
/*
 *****************************************************************************
 *
 *
 *
 *
 *
 *
 *****************************************************************************
 */

static u8 hw_eq_buffer[704] sec(.dac_buf_sec) __attribute__((aligned(4)));
static HARD_EQ_PARAM hw_eq_param;

const int eq_seg_gain[7][10] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 		///<Normal
    { 0, 8, 8, 4, 0, 0, 0, 0, 2, 2}, 		///<Rock
    {-2, 0, 2, 4, -2, -2, 0, 0, 4, 4}, 		///<Pop
    { 4, 2, 0, -3, -6, -6, -3, 0, 3, 5}, 	///<Classic
    { 0, 0, 0, 4, 4, 4, 0, 2, 3, 4}, 		///<Jazz
    {-2, 0, 0, 2, 2, 0, 0, 0, 4, 4}, 		///<Country
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   		///<user_defined
};

const int eq_global_gain[6] = {
    0, -6, -3, -6, -6, -3
};

static const int eq_filt_44100[] = {
    2085775, -1037283, 5647, 0, -1,
    2051103, -1003829, 22374, 0, -1,
    1905750, -876682, 85947, 0, -1,
    1294532, -488959, 279808, 0, -1,
    -204953, 733664, 891120, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};
//22.05
static const int eq_filt_22050[] = {
    2074352, -1026110, 11233, 0, -1,
    2004391, -960912, 43832, 0, -1,
    1705578, -728669, 159954, 0, -1,
    476389, -91739, 478419, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};
//11.025
static const int eq_filt_11025[] = {
    2051368, -1004115, 22230, 0, -1,
    1909022, -879987, 84294, 0, -1,
    1286901, -479895, 284341, 0, -1,
    -204625, 734221, 891398, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};

//48k
static const int eq_filt_48000[] = {
    2086701, -1038196, 5190, 0, -1,
    2054870, -1007394, 20591, 0, -1,
    1921645, -889651, 79463, 0, -1,
    1363328, -525660, 261458, 0, -1,
    -239791, 568886, 808731, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};
//24.00
static const int eq_filt_24000[] = {
    2076211, -1027918, 10329, 0, -1,
    2012026, -967768, 40404, 0, -1,
    1738642, -751399, 148589, 0, -1,
    600306, -152036, 448270, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};
//12.00
static const int eq_filt_12000[] = {
    2055116, -1007658, 20459, 0, -1,
    1924692, -892724, 77926, 0, -1,
    1355916, -517100, 265738, 0, -1,
    -239623, 569330, 808953, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};

//32k
static const int eq_filt_32000[] = {
    2081461, -1033044, 7766, 0, -1,
    2033518, -987394, 30591, 0, -1,
    1831021, -818317, 115130, 0, -1,
    974346, -329358, 359609, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};

static const int eq_filt_16000[] = {
    2065683, -1017739, 15419, 0, -1,
    1968630, -929579, 59498, 0, -1,
    1549409, -628492, 210042, 0, -1,
    0, 187252, 617914, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};

static const int eq_filt_8000[] = {
    2033866, -987783, 30397, 0, -1,
    1835253, -822632, 112972, 0, -1,
    966373, -318082, 365247, 0, -1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};

void eq_isr(s16 *outbuf, u16 npoint)
{
}

void eq_init(void)
{
    int need_buf;
    s32 ret = 0;
    if (eq_ops == &eq_drv) {
        puts("HW_EQ initialized\n");
        return;
    }
    eq_ops = &eq_drv;
    need_buf = eq_ops->need_buf(EQ_CHANNEL_NUM, EQ_SECTION_NUM);
    //log_printf("hard_eq_buf:%d\n", need_buf);

    ret = eq_cfg_check();
    if (ret == 0) {
        //puts("HW_EQ CFG\n");
        //log_printf("EQ_TYPE:%d\n", eq_cfg.type);
        //log_printf("EQ_SEG:%d\n", eq_cfg.seg_num);
        hw_eq_param.filt.filt_44100 	= (int *)eq_cfg.filt_0;
        hw_eq_param.filt.filt_22050 	= (int *)eq_cfg.filt_1;
        hw_eq_param.filt.filt_11025 	= (int *)eq_cfg.filt_2;
        hw_eq_param.filt.filt_48000 	= (int *)eq_cfg.filt_3;
        hw_eq_param.filt.filt_24000 	= (int *)eq_cfg.filt_4;
        hw_eq_param.filt.filt_12000 	= (int *)eq_cfg.filt_5;
        hw_eq_param.filt.filt_32000 	= (int *)eq_cfg.filt_6;
        hw_eq_param.filt.filt_16000 	= (int *)eq_cfg.filt_7;
        hw_eq_param.filt.filt_8000  	= (int *)eq_cfg.filt_8;
        hw_eq_param.filt.freq_gain	  	= (int (*)[10])eq_cfg.freq_gain;
        hw_eq_param.filt.global_gain	= (int *)eq_cfg.global_gain;
        //set eq mode
        eq_arg.mode = eq_cfg.type;

    } else {
        //puts("HW_EQ Default\n");
        hw_eq_param.filt.filt_44100 	= (int *)eq_filt_44100;
        hw_eq_param.filt.filt_22050 	= (int *)eq_filt_22050;
        hw_eq_param.filt.filt_11025 	= (int *)eq_filt_11025;
        hw_eq_param.filt.filt_48000 	= (int *)eq_filt_48000;
        hw_eq_param.filt.filt_24000 	= (int *)eq_filt_24000;
        hw_eq_param.filt.filt_12000 	= (int *)eq_filt_12000;
        hw_eq_param.filt.filt_32000 	= (int *)eq_filt_32000;
        hw_eq_param.filt.filt_16000 	= (int *)eq_filt_16000;
        hw_eq_param.filt.filt_8000  	= (int *)eq_filt_8000;
        hw_eq_param.filt.freq_gain	  	= (int (*)[10])eq_seg_gain;
        hw_eq_param.filt.global_gain	= (int *)eq_global_gain;
        //set eq mode
        eq_arg.mode = EQ_NORMAL;
    }

    hw_eq_param.ie = 0;
    hw_eq_param.isr_cb = eq_isr;

    eq_arg.pEQ = &hw_eq_param;
    eq_ops->init(hw_eq_buffer, eq_arg.pEQ);
    eq_enable();
    puts("eq_init ok\n");
}

void hw_eq_cfg_update(void)
{
    int need_buf;
    s32 ret = 0;
    eq_ops = &eq_drv;
    puts("hw_eq_update\n");
    log_printf("eq_type:%d\n", eq_cfg.type);
    log_printf("seg_num:%d\n", eq_cfg.seg_num);
    hw_eq_param.filt.filt_44100 	= (int *)eq_cfg.filt_0;
    hw_eq_param.filt.filt_22050 	= (int *)eq_cfg.filt_1;
    hw_eq_param.filt.filt_11025 	= (int *)eq_cfg.filt_2;
    hw_eq_param.filt.filt_48000 	= (int *)eq_cfg.filt_3;
    hw_eq_param.filt.filt_24000 	= (int *)eq_cfg.filt_4;
    hw_eq_param.filt.filt_12000 	= (int *)eq_cfg.filt_5;
    hw_eq_param.filt.filt_32000 	= (int *)eq_cfg.filt_6;
    hw_eq_param.filt.filt_16000 	= (int *)eq_cfg.filt_7;
    hw_eq_param.filt.filt_8000  	= (int *)eq_cfg.filt_8;
    hw_eq_param.filt.freq_gain	  	= (int (*)[10])eq_cfg.freq_gain;
    hw_eq_param.filt.global_gain	= (int *)eq_cfg.global_gain;
    //set eq mode
    eq_arg.mode = eq_cfg.type;

    hw_eq_param.ie = 0;
    hw_eq_param.isr_cb = eq_isr;
    eq_arg.pEQ = &hw_eq_param;
    eq_ops->init(hw_eq_buffer, eq_arg.pEQ);
    eq_enable();
}

#if EQ_UART_DEBUG
#define EQ_PACKET_MAX_IDX  4      //4+1 = 5 packet.
#define EQ_PACKET_LEN      512
extern void sw_eq_cfg_update(void);
extern void hw_eq_cfg_update(void);
u8 update_eq_info(void *new_eq_info, u32 size, u8 packet_idx)
{
    EQ_CFG *eq_cfg_buf = &eq_cfg;
    if (0 == packet_idx) {
        memcpy((u8 *)eq_cfg_buf, new_eq_info, size);
    } else {
        memcpy(((u8 *)eq_cfg_buf) + (EQ_PACKET_LEN - 4) * packet_idx, new_eq_info, size);
    }

    if (EQ_PACKET_MAX_IDX == packet_idx) {
        if (eq_cfg_buf->crc16 == crc16(eq_cfg_buf, sizeof(EQ_CFG) - 4)) {
            puts("recv new EQ, crc ok\n");
#if(AUDIO_EFFECT &  AUDIO_EFFECT_HW_EQ)
            hw_eq_cfg_update();
#elif(AUDIO_EFFECT & AUDIO_EFFECT_SW_EQ)
            sw_eq_cfg_update();
#endif
            return 1;
        } else {
            puts("recv new EQ, crc Err!!!\n");
        }
    }

    return 0;
}

void eq_uart_debug_write(char a);
void eq_uart_debug_isr_callback(u8 uto_buf, void *p, u8 isr_flag)
{
    u8 packet_idx = 0;
    u8 *rx_buf = p;
    u8 res = 0;
    if ((UART_ISR_TYPE_DATA_COME == isr_flag) && ('E' == rx_buf[0] && 'Q' == rx_buf[1])) {
        packet_idx = rx_buf[2];
        log_printf("[%d]", packet_idx);

        if (EQ_PACKET_MAX_IDX == packet_idx) {
            res = update_eq_info(rx_buf + 4, 2140 - 4 * EQ_PACKET_LEN - 4, packet_idx);
        } else if (packet_idx < EQ_PACKET_MAX_IDX) {
            res = update_eq_info(rx_buf + 4, EQ_PACKET_LEN - 4, packet_idx);
        }

        if (EQ_PACKET_MAX_IDX - 1 == packet_idx) { //packet 3
            JL_UART1->RXCNT = 2140 - 4 * EQ_PACKET_LEN;
            JL_UART1->RXSADR = (u32)rx_uart1_buf;
            JL_UART1->RXEADR = (u32)(rx_uart1_buf + ut_dma_wr_cnt[1]);
        }

        if (res || (EQ_PACKET_MAX_IDX == packet_idx)) { //all packet recv ok
            if (res) {
                puts("send ok\n");
                eq_uart_debug_write('O');
                eq_uart_debug_write('K');
            } else {
                puts("send err\n");
                eq_uart_debug_write('E');
                eq_uart_debug_write('R');

            }
            JL_UART1->RXCNT = (u32)ut_dma_wr_cnt[1];
            JL_UART1->RXSADR = (u32)rx_uart1_buf;
            JL_UART1->RXEADR = (u32)(rx_uart1_buf + ut_dma_wr_cnt[1]);
        }
    }

    if (UART_ISR_TYPE_TIMEOUT == isr_flag) {
        puts("eq uart timeout!!!\n");
        JL_UART1->RXCNT = (u32)ut_dma_wr_cnt[1];
        JL_UART1->RXSADR = (u32)rx_uart1_buf;
        JL_UART1->RXEADR = (u32)(rx_uart1_buf + ut_dma_wr_cnt[1]);
    }
    JL_UART1->OTCNT = 20000 * 1000;
}

#endif


/*
 *****************************************************************************
 *
 *						SOFTWARE EQ API
 *
 *
 *
 *
 *****************************************************************************
 */

typedef struct {
    AUDIO_STREAM input;
    AUDIO_STREAM *output;
} SW_EQ;
SW_EQ sw_eq;

static u8 soft_eq_buffer[4060] sec(.dac_buf_sec) __attribute__((aligned(4)));
SOFT_EQ_PARAM sw_eq_param;

static int CurModify_Segment = 0;
static const int *CurModify_GlobalGain = NULL;
static const int (*CurModify_Tab)[5] = NULL;

#include "sw_eq_table_normal.h"
#include "sw_eq_table_classic.h"
#include "sw_eq_table_country.h"
#include "sw_eq_table_jazz.h"
#include "sw_eq_table_pop.h"
#include "sw_eq_table_rock.h"
static const int (*cur_sw_eq_table)[9][5] = NULL;
static const int (*sw_eq_table_all[6])[9][5] = {
    sw_eq_table_normal, sw_eq_table_rock, sw_eq_table_pop, sw_eq_table_classic, sw_eq_table_jazz, sw_eq_table_country,
};

//Change All One EqTable Coeff.
void soft_eq_table_modify(const int (*coeff)[9][5])
{
    int seg, filt, i;
    int *filt_tab;
    EQ_CFG *p_eq_cfg = &eq_cfg;
    for (seg = 0; seg < 10; seg++) {
        filt_tab = p_eq_cfg->filt_0 + seg * 5;
        for (filt = 0; filt < 9; filt++, filt_tab += 50) { //one seg
            memcpy(filt_tab, (int *)coeff[seg][filt], sizeof(int) * 5);
        }
    }
}

void soft_eq_set(int eq_mode)
{
    if (eq_ops) {
        eq_arg.mode = eq_mode;
        if (eq_mode >= sizeof(sw_eq_table_all) / sizeof(int *)) {
            eq_mode = 0;
        }
        //printf("soft_eq_set eq_mode = %d,total = %d\n",eq_mode,sizeof(sw_eq_table_all)/sizeof(int*));
        cur_sw_eq_table = sw_eq_table_all[eq_mode];
    }
}


void soft_eq_GlobalGain_Modify(int TabIdx)
{
    CurModify_GlobalGain = GlobalGainTab + TabIdx;
}

void soft_eq_SegmentCoeff_Modify(int seg_idx, int tab_idx)
{
    switch (seg_idx) {
    case 0:
        CurModify_Segment = 0;
        CurModify_Tab = Seg0Coeff[tab_idx];
        break;
    default:
        break;
    }
}

static void soft_eq_SegmentCoefficientModify(const int (*Coeff)[5], int seg_idx)
{
    int *pSegCoeff = (int *)eq_cfg.filt_0 + seg_idx * 5;
    int Step = sizeof(eq_cfg.filt_0) / sizeof(int);
    log_printf("CurModify_Segment:%d\n", CurModify_Segment);
    for (int i = 0; i < 9; i++) {
        memcpy(pSegCoeff, (int *)Coeff[i], sizeof(int) * 5);
        /* for(int j=0;j<5;j++) */
        /* log_printf("%d,",Coeff[i][j]); */
        /* log_printf("\n"); */
        pSegCoeff += Step;
    }
}

static void soft_eq_callback(void *priv)
{
    if (CurModify_Tab != NULL) {
        soft_eq_SegmentCoefficientModify(CurModify_Tab, CurModify_Segment);
        CurModify_Tab = NULL;
        CurModify_Segment = 0;
    }

    if (CurModify_GlobalGain != NULL) {
        soft_eq_drv.ioctl(SET_EQ_GLOBALGAIN, *CurModify_GlobalGain, NULL);
        CurModify_GlobalGain = NULL;
    }

    if (cur_sw_eq_table != NULL) {
        soft_eq_table_modify(cur_sw_eq_table);
        cur_sw_eq_table = NULL;
    }
}


static void soft_eq_init(void)
{
    u32 need_buf;
    s32 ret = 0;
    if (eq_ops == &soft_eq_drv) {
        puts("SW_EQ initialized\n");
        return;
    }
    ret = eq_cfg_check();
    if (ret != 0) {
        eq_ops = NULL;
        puts("\n\nsoft_eq_init failed\n\n");
        return;
    }

    cur_sw_eq_table = NULL;
    eq_ops = &soft_eq_drv;
    need_buf = eq_ops->need_buf(EQ_CHANNEL_NUM, SOFT_EQ_SECTION_NUM);
    log_printf("soft_eq_buf:%d\n", need_buf);
    log_printf("soft_eq_seg:%d\n", eq_cfg.seg_num);

    sw_eq_param.filt.filt_44100 	= (int *)eq_cfg.filt_0;
    sw_eq_param.filt.filt_22050 	= (int *)eq_cfg.filt_1;
    sw_eq_param.filt.filt_11025		= (int *)eq_cfg.filt_2;
    sw_eq_param.filt.filt_48000		= (int *)eq_cfg.filt_3;
    sw_eq_param.filt.filt_24000 	= (int *)eq_cfg.filt_4;
    sw_eq_param.filt.filt_12000 	= (int *)eq_cfg.filt_5;
    sw_eq_param.filt.filt_32000 	= (int *)eq_cfg.filt_6;
    sw_eq_param.filt.filt_16000 	= (int *)eq_cfg.filt_7;
    sw_eq_param.filt.filt_8000  	= (int *)eq_cfg.filt_8;
    sw_eq_param.filt.freq_gain	  	= (int (*)[10])eq_cfg.freq_gain;
    sw_eq_param.filt.TargetLvl	  	= eq_cfg.global_gain[0];
    sw_eq_param.filt.ReleaseTime	= eq_cfg.global_gain[1];
    sw_eq_param.filt.AttackTime	  	= eq_cfg.global_gain[2];
    sw_eq_param.filt.GlobalGain	  	= eq_cfg.global_gain[3];
    sw_eq_param.filt.LimiterTog	  	= eq_cfg.global_gain[4];
    sw_eq_param.filt.seg_num        = eq_cfg.seg_num;
    sw_eq_param.callback = soft_eq_callback;

    eq_arg.mode = EQ_NORMAL;
    eq_arg.pEQ = &sw_eq_param;
    eq_ops->init(soft_eq_buffer, eq_arg.pEQ);
    puts("eq_init ok\n");
}

void sw_eq_cfg_update(void)
{
    u32 need_buf;
    s32 ret = 0;
    printf("\nsw_eq_cfg_update\n");
    eq_ops = &soft_eq_drv;
    printf("soft_eq_seg:%d\n", eq_cfg.seg_num);

    sw_eq_param.filt.filt_44100     = (int *)eq_cfg.filt_0;
    sw_eq_param.filt.filt_22050     = (int *)eq_cfg.filt_1;
    sw_eq_param.filt.filt_11025     = (int *)eq_cfg.filt_2;
    sw_eq_param.filt.filt_48000     = (int *)eq_cfg.filt_3;
    sw_eq_param.filt.filt_24000     = (int *)eq_cfg.filt_4;
    sw_eq_param.filt.filt_12000     = (int *)eq_cfg.filt_5;
    sw_eq_param.filt.filt_32000     = (int *)eq_cfg.filt_6;
    sw_eq_param.filt.filt_16000     = (int *)eq_cfg.filt_7;
    sw_eq_param.filt.filt_8000      = (int *)eq_cfg.filt_8;
    sw_eq_param.filt.freq_gain      = (int (*)[10])eq_cfg.freq_gain;
    sw_eq_param.filt.TargetLvl      = eq_cfg.global_gain[0];
    sw_eq_param.filt.ReleaseTime    = eq_cfg.global_gain[1];
    sw_eq_param.filt.AttackTime     = eq_cfg.global_gain[2];
    sw_eq_param.filt.GlobalGain     = eq_cfg.global_gain[3];
    sw_eq_param.filt.LimiterTog     = eq_cfg.global_gain[4];
    sw_eq_param.filt.seg_num = eq_cfg.seg_num;
    sw_eq_param.callback = soft_eq_callback;

    eq_arg.mode = EQ_NORMAL;
    eq_arg.pEQ = &sw_eq_param;
    eq_ops->init(soft_eq_buffer, eq_arg.pEQ);
}


static u32 soft_eq_run(void *priv, void *buf, u32 len)
{
    if (sw_eq.output->free_len(sw_eq.output->priv) < len) {
        return 0;
    }
    if (eq_ops) {
        eq_ops->run(buf, buf, len / 2);
    }
    return sw_eq.output->output(sw_eq.output->priv, buf, len);
}

static void soft_eq_clear(void *priv)
{
    if (sw_eq.output) {
        sw_eq.output->clear(sw_eq.output->priv);
    }
}

static tbool soft_eq_check(void *priv)
{
    if (sw_eq.output) {
        return sw_eq.output->check(sw_eq.output->priv);
    } else {
        return false;
    }
}

static void soft_eq_samplerate(void *priv, u16 sr, u8 wait)
{
    if (sw_eq.output->set_sr) {
        sw_eq.output->set_sr(sw_eq.output->priv, sr, wait);
    }
}

static u32 soft_eq_free_len(void *priv)
{
    if (sw_eq.output->free_len) {
        return sw_eq.output->free_len(sw_eq.output->priv);
    }
    return 0;
}

static u32 soft_eq_data_len(void *priv)
{
    if (sw_eq.output->data_len) {
        return sw_eq.output->data_len(sw_eq.output->priv);
    }
    return 0;
}

AUDIO_STREAM *audio_eq_input(AUDIO_STREAM *output)
{
    if (output == NULL) {
        return NULL;
    }
    soft_eq_init();

    sw_eq.output 		= output;
    sw_eq.input.priv  	= (void *)NULL;
    sw_eq.input.output 	= soft_eq_run;
    sw_eq.input.clear	= soft_eq_clear;
    sw_eq.input.check 	= soft_eq_check;
    sw_eq.input.free_len = soft_eq_free_len;
    sw_eq.input.data_len = soft_eq_data_len;
    sw_eq.input.set_sr 	= soft_eq_samplerate;

    return &(sw_eq.input);
}

/*
 *****************************************************************************
 *
 *							EQ COMMON API
 *
 *
 *
 *
 *****************************************************************************
 */
void eq_enable(void)
{
    if (eq_ops) {
        eq_ops->on(NULL);
        eq_ops->ioctl(SET_EQ_MODE, eq_arg.mode, NULL);
        puts("eq enable\n");
    }
}

void eq_disable(void)
{
    if (eq_ops) {
        eq_ops->off(NULL);
        puts("eq_disable\n");
    }
}

/**
  * bypass_en = 0:eq enable
  * bypass_en = 1:eq disable
  */
void eq_bypass_en(u8 en)
{
    if (eq_ops) {
        eq_ops->ioctl(SET_EQ_BYPASS, en, NULL);
    }
}

AT_AUDIO
void eq_run(short *in, short *out, int npoint)
{
    if (eq_ops) {
        eq_ops->run(in, out, npoint);
    } else {
        memcpy((u8 *)out, (u8 *)in, npoint * 4);
    }
}

void eq_samplerate(u16 sr)
{
    if (eq_ops) {
        eq_ops->ioctl(SET_EQ_SR, sr, NULL);
    }
}

void eq_mode_set(u8 mode)
{
    if (eq_ops) {
        eq_arg.mode = mode;
#if(AUDIO_EFFECT &  AUDIO_EFFECT_HW_EQ)
        eq_ops->ioctl(SET_EQ_MODE, mode, NULL);
#elif(AUDIO_EFFECT &  AUDIO_EFFECT_SW_EQ)
        soft_eq_set(mode);
#endif
    }
}

void eq_mode_switch(u8 eq_mode)
{
    eq_mode_set(eq_mode);
    UI_menu(MENU_SET_EQ, eq_mode, 2);
}

void eq_mode_switch_bt_sync(u8 eq_mode)
{
    eq_mode_set(eq_mode);
    //UI_menu(MENU_SET_EQ, eq_mode, 2);
}