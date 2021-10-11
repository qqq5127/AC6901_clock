#include "typedef.h"
#include "uart.h"
#include "irq_api.h"
#include "common.h"
#include "hw_cpu.h"
#include "audio/dac_api.h"
#include "audio/dac.h"
#include "string.h"
#include "timer.h"
#include "clock_api.h"
#include "src.h"
#include "sdk_cfg.h"
#include "task_spdif.h"

#if SPDIF_EN
#define PRINTF_DEBUG 0
static u8 error_flag = 0;
#define CHANNEL_NUMBER    2
#define SPDIF_DAT_DMA_LEN 96
#define SPDIF_INFO_LEN    6

#define INFO_VALIDITY_BIT       BIT(0)
#define INFO_USER_DATA_BIT      BIT(1)
#define INFO_CHANNEL_STA_BIT    BIT(2)
#define INFO_PARITY_BIT         BIT(3)
#define SS_EN       1
#define INT_EN      1
#define SS_IOS      2
#define SE_CNT      0xff
#define DAT_DMA_EN  1
#define DAT_DMA_MD  0    //1:16bit   0:24bit
#define INF_DMA_EN  1
#define SE_DECT_EN  1
#define CSBR_CNT    24

static s32 infor_buf[2][SPDIF_INFO_LEN];
#if DAT_DMA_MD
static s16 spdif_data_buf[2][SPDIF_DAT_DMA_LEN * CHANNEL_NUMBER] ;
#else
static s32 spdif_data_buf[2][SPDIF_DAT_DMA_LEN * CHANNEL_NUMBER] ;
#endif

#define SS_START    (JL_SPDIF->SS_CON |= BIT(1))
#define SS_EN_ISR   (JL_SPDIF->SS_CON |= BIT(2))
#define D_PND_CLR   (JL_SPDIF->SS_CON |= BIT(3))
#define D_PND       (JL_SPDIF->SS_CON &  BIT(4))
#define I_PND_CLR   (JL_SPDIF->SS_CON |= BIT(5))
#define I_PND       (JL_SPDIF->SS_CON &  BIT(6))
#define ERR_CLR     (JL_SPDIF->SS_CON |= BIT(7))
#define CSB_PND_CLR (JL_SPDIF->SS_DMA_CON |= BIT(13))
#define CSB_PND     (JL_SPDIF->SS_DMA_CON & BIT(14))
#define USING_BUF_INDEX  (JL_SPDIF->SS_CON & BIT(12))
#define ERROR_VALUE ((JL_SPDIF->SS_CON & (BIT(8) | BIT(9) | BIT(10) | BIT(11)))>>8)
static void spdif_channel_status_data_deal()
{
    u32 sample_rate = 0;
    u32 max_sample_bits = 0;
    put_u32hex(JL_SPDIF->SS_CSB0);
    put_u32hex(JL_SPDIF->SS_CSB1);
    put_u32hex(JL_SPDIF->SS_CSB2);
    put_u32hex(JL_SPDIF->SS_CSB3);
    put_u32hex(JL_SPDIF->SS_CSB4);
    put_u32hex(JL_SPDIF->SS_CSB5);
    if (JL_SPDIF->SS_CSB0 & BIT(0)) {
        //	prefessional applications
        puts("prefessional app\n");
        if (JL_SPDIF->SS_CSB0 & BIT(1)) {
            puts("NOT LINEAR PCM\n");
        }
        sample_rate = JL_SPDIF->SS_CSB0 & (BIT(6) | BIT(7));
        sample_rate = sample_rate >> 6;
        if (sample_rate == 0) {
            puts("default 48khz\n");
        } else if (sample_rate == 1) {
            puts("48 kHz\n");
        } else if (sample_rate == 2) {
            puts("44.1 kHz\n");
        } else if (sample_rate == 3) {
            puts("32 kHz\n");
        }
        max_sample_bits = JL_SPDIF->SS_CSB0 & (BIT(16) | BIT(17) | BIT(18));
        max_sample_bits = max_sample_bits >> 16;
        if (max_sample_bits == 0 || max_sample_bits == 2) {
            max_sample_bits = JL_SPDIF->SS_CSB0 & (BIT(19) | BIT(20) | BIT(21));
            max_sample_bits = max_sample_bits >> 19;
            if (max_sample_bits == 4) { //max 16 bits
                puts("set max 16 bits\n");
                JL_SPDIF->SS_DMA_CON |= BIT(1);
            }
        }
    } else {
        //	consumer applications
        u32 category_code = 0;
        puts("consumer app\n");
        category_code = JL_SPDIF->SS_CSB0 & 0x0000ff00;
        sample_rate   = JL_SPDIF->SS_CSB0 & 0xff000000;
        category_code = category_code >> 8;
        printf("category_code %x", category_code);
        if (JL_SPDIF->SS_CSB1 & BIT(0)) { //MAX_SAMPLE_BITS 24 BITS

        } else {
            max_sample_bits = JL_SPDIF->SS_CSB1 & (BIT(1) | BIT(2) | BIT(3));
            max_sample_bits = max_sample_bits >> 1;
            if (max_sample_bits == 4) { //max 16 bits
                puts("set max 16 bits\n");
                JL_SPDIF->SS_DMA_CON |= BIT(1);
            }
        }
    }
}
#define SAMPLE_RATE_TABLE_SIZE 14
u8 old_samplerate_index = 7;
u16 spdif_sample_rate_table[SAMPLE_RATE_TABLE_SIZE] = {
    80,
    110,
    160,
    221,
    240,
    320,
    441,
    480,
    504,
    640,
    882,
    960,
    1764,
    1920
};
static u32 cal_samplerate()
{
    int different;
    u32 now_tick_counter = 0;
    u32 tick_result = 0 ;
    u8  search_index = 0, i = 0;

    now_tick_counter = JL_TICK->CNT;
    JL_TICK->CNT   = 0;

    tick_result = ((SPDIF_DAT_DMA_LEN  * (clock_get_sys_freq() / 100)) / now_tick_counter);
    for (i = 0; i < SAMPLE_RATE_TABLE_SIZE; i++) {
        if ((tick_result > spdif_sample_rate_table[i] - 20) && tick_result < spdif_sample_rate_table[i] + 20)	{
            search_index = i;
        }
    }
    if ((search_index == 0) || i > SAMPLE_RATE_TABLE_SIZE) {
        //printf("R %d  ", tick_result);
    }
    if ((search_index != 0) && (old_samplerate_index != search_index)) {
        //printf("SET %d %d ", spdif_sample_rate_table[search_index], tick_result);
        spdif_src_set_input_samplerate(spdif_sample_rate_table[search_index]);
        old_samplerate_index = search_index;
    }
    return tick_result;

}
#define SYNCWORD1 0xF872
#define SYNCWORD2 0x4E1F
#define BURST_HEADER_SIZE 0x8
enum  IEC61937DataType {
    IEC61937_AC3                = 0x01,          ///< AC-3 data
    IEC61937_MPEG1_LAYER1       = 0x04,          ///< MPEG-1 layer 1
    IEC61937_MPEG1_LAYER23      = 0x05,          ///< MPEG-1 layer 2 or 3 data or MPEG-2 without extension
    IEC61937_MPEG2_EXT          = 0x06,          ///< MPEG-2 data with extension
    IEC61937_MPEG2_AAC          = 0x07,          ///< MPEG-2 AAC ADTS
    IEC61937_MPEG2_LAYER1_LSF   = 0x08,          ///< MPEG-2, layer-1 low sampling frequency
    IEC61937_MPEG2_LAYER2_LSF   = 0x09,          ///< MPEG-2, layer-2 low sampling frequency
    IEC61937_MPEG2_LAYER3_LSF   = 0x0A,          ///< MPEG-2, layer-3 low sampling frequency
    IEC61937_DTS1               = 0x0B,          ///< DTS type I   (512 samples)
    IEC61937_DTS2               = 0x0C,          ///< DTS type II  (1024 samples)
    IEC61937_DTS3               = 0x0D,          ///< DTS type III (2048 samples)
    IEC61937_ATRAC              = 0x0E,          ///< ATRAC data
    IEC61937_ATRAC3             = 0x0F,          ///< ATRAC3 data
    IEC61937_ATRACX             = 0x10,          ///< ATRAC3+ data
    IEC61937_DTSHD              = 0x11,          ///< DTS HD data
    IEC61937_WMAPRO             = 0x12,          ///< WMA 9 Professional data
    IEC61937_MPEG2_AAC_LSF_2048 = 0x13,          ///< MPEG-2 AAC ADTS half-rate low sampling frequency
    IEC61937_MPEG2_AAC_LSF_4096 = 0x13 | 0x20,   ///< MPEG-2 AAC ADTS quarter-rate low sampling frequency
    IEC61937_EAC3               = 0x15,
};
static int length_code = 0;
static  u16 decoder_type = 0;
u8  find_sync_word = 0;
int spdif_non_linear_packet(u16 *pkt, u16 pkt_len)
{
    enum IEC61937DataType data_type;
    u16 search_index = 0;
    int data_cp_begin_idx = 0;
    int data_cp_len = 0;
    u16 *data = pkt;
    u16 *data_ptr = NULL;
    int zero_cnt = 0;
    uint32_t state = 0;
    int pkt_size_bits, offset, skip_len;
    if (length_code > 0) {
        if (length_code >= SPDIF_DAT_DMA_LEN * 2) {
            data_cp_begin_idx = 0;
            data_cp_len       = SPDIF_DAT_DMA_LEN * 2;
            length_code       -= data_cp_len;
        } else {
            data_cp_begin_idx = 0;
            data_cp_len       = length_code;
            length_code       = 0;
        }
        spdif_data_write(&data[data_cp_begin_idx], data_cp_len * 2);
        if (length_code != 0) {
            return 0;
        } else {
            search_index = data_cp_len;
        }
    }
    if (find_sync_word != 0) {
        //puts("last packet have SYNCWORD1\n");
        printf("[last:%d,%x]", search_index, data[search_index]);
        if (data[search_index] == SYNCWORD2) {
            putchar('3');
        } else {
            putchar('M');
            find_sync_word = 0;
            return -1;
        }
    } else {
        while (search_index < pkt_len) {
            if (data[search_index] == SYNCWORD1) {
                find_sync_word = 1;
            }
            if (++search_index >= pkt_len) {
                return -1;
            }
            if (find_sync_word) {
                if ((data[search_index] == SYNCWORD2) && (data[search_index - 1] == SYNCWORD1)) {
                    break;
                } else {
                    find_sync_word = 0;
                }
            }
        }
    }
    find_sync_word = 0;
    data_type = data[++search_index];
    decoder_type = data_type & 0xff;
    pkt_size_bits = data[++search_index];
    length_code   = (pkt_size_bits >> 3) >> 1;  /*16 bit*/
    if (pkt_size_bits % 16) {
        printf("Packet not ending at a 16-bit boundary %d-%d\n", pkt_size_bits, search_index);
        for (int i = 0; i < pkt_len; i++) {
            printf("%x ", data[i]);
        }
    }
    ++search_index;
    data_cp_len       = pkt_len - search_index;
    skip_len = 2048 - (length_code << 1) - BURST_HEADER_SIZE;
    //printf("{%d,%d}",length_code, skip_len);
    length_code       -= data_cp_len;
    if (data_cp_len) {
        spdif_data_write(&data[search_index], data_cp_len * 2);
        data_cp_len = 0;
    }
    return 0;
}
static u8  validity_bit_flag = 0;
//u32 sdpif
static void spdif_isr_fun()
{
    s32 *ptr = NULL;
    u8  *info_ptr = NULL;
    s16 data_ptr[SPDIF_DAT_DMA_LEN * CHANNEL_NUMBER ];
    u16 *u_data_ptr = (u16 *)data_ptr;
    s16 temp1 = 0;
    s16 temp2 = 0;
    s16 temp3 = 0;
    int index = 0;
    u16 write_len = 0 ;
    //deal pendig or  flags in SS_CON
    clear_wdt();
    if (I_PND) { //information PENDING
        I_PND_CLR;	//CLS information PENDING
        index = !(USING_BUF_INDEX >> 13) ;
        info_ptr = (u8 *)&infor_buf[index][0];
        for (int i = 0; i < SPDIF_INFO_LEN; i++) {
            if (!(info_ptr[0] & INFO_VALIDITY_BIT)) {
                break;
            }
            validity_bit_flag = 1;
        }
    }
    if (ERROR_VALUE) { //some error flag
        //__asm__ volatile("trigger");
        ERR_CLR;	//CLS error flag
        if (ERROR_VALUE & (BIT(8) | BIT(9))) {
            //putchar('.');
            memset(&spdif_data_buf[0][0], 0x0, sizeof(spdif_data_buf) >> 1);
            memset(&spdif_data_buf[1][0], 0x0, sizeof(spdif_data_buf) >> 1);
        }
        // printf("ss isr error %x\n", ERROR_VALUE);
        if (JL_SPDIF->SS_CON & BIT(11)) {
            error_flag = 1;
            spdif_close();
            if (get_spdif_none_linear_sta() == NONE_LINEAR_STATUS_WORKING) {
                spdif_none_linear_stop(NULL);
                return;
            }
            puts("close\n");
        }
    }
    if (D_PND) { //DATA PENDING
        D_PND_CLR;  	//CLS DATA PENDING
        if (validity_bit_flag) {
            validity_bit_flag = 0;
            index = !(USING_BUF_INDEX >> 12) ;
            ptr = (s32 *)&spdif_data_buf[index][0];
            for (int i = 0; i < SPDIF_DAT_DMA_LEN * 2; i += 2) {
                u_data_ptr[i] = (u16)(ptr[i] >> 8);
                u_data_ptr[i + 1] = (u16)(ptr[i + 1] >> 8);
            }
            if (get_spdif_none_linear_sta() == NONE_LINEAR_STATUS_STOP) {
                spdif_none_linear_init(0);
            }
            if (get_spdif_none_linear_sta() == NONE_LINEAR_STATUS_WORKING) {
                spdif_non_linear_packet(u_data_ptr, SPDIF_DAT_DMA_LEN * 2);
            }
            return;
        } else {
            if (get_spdif_none_linear_sta() == NONE_LINEAR_STATUS_WORKING) {
                spdif_none_linear_stop(NULL);
                spdif_src_init();
                return;
            }
        }
        cal_samplerate();
        index = !(USING_BUF_INDEX >> 12) ;
#if PRINTF_DEBUG
        for (int i = 0 ; i < SPDIF_DAT_DMA_LEN * 2 - 6 ; i += 2) {
            {
                temp1 = spdif_data_buf[index][i + 4];
                temp2 = spdif_data_buf[index][i + 2];
                temp3 = spdif_data_buf[index][i];
                if (temp2 > temp1 && temp2 > temp3) {
                    if (temp2 < 22000) {
                        putchar('1');
                        __asm__ volatile("trigger");
                    }
                } else if (temp2 < temp1 && temp2 < temp3) {
                    if (temp2 > -22000) {
                        __asm__ volatile("trigger");
                        putchar('2');
                    }
                }
            }
        }
#else
#if DAT_DMA_MD   //16 bit
        //using buffer 1 , please deal buffer0 data
        write_len = src_run((u8 *)&spdif_data_buf[index][0], (u16)SPDIF_DAT_DMA_LEN * sizeof(s16) * CHANNEL_NUMBER); //two channel
        //  write_len = dac_write(&spdif_data_buf[index][0], (u32)SPDIF_DAT_DMA_LEN * sizeof(s16) * CHANNEL_NUMBER); //two channel
#else        //24 bit
        //using buffer 1 , please deal buffer0 data
        ptr = (s32 *)&spdif_data_buf[index][0];
        for (int i = 0; i < SPDIF_DAT_DMA_LEN * 2; i += 2) {
            data_ptr[i] = (s16)(ptr[i] >> 8);
            data_ptr[i + 1] = (s16)(ptr[i + 1] >> 8);
        }
        write_len =	src_run((u8 *)data_ptr, (u16)SPDIF_DAT_DMA_LEN * sizeof(s16) * CHANNEL_NUMBER);
#endif //DAT_DMA_MD
#endif
        memset(&spdif_data_buf[index][0], 0x0, sizeof(spdif_data_buf) >> 1);
        if (write_len != SPDIF_DAT_DMA_LEN * 2 * 2) {
            putchar('l');
        }
    }
    //deal pendig or  flags in  SS_DMA_CON
    if (CSB_PND) { //CHANNEL STATUS enough pending
        CSB_PND_CLR;//cls channel status pending
        // spdif_channel_status_data_deal();
        puts("CSB_PND\n");

    }
}
IRQ_REGISTER(IRQ_SPDIF_IDX, spdif_isr_fun);

static void tick_timer_isr_fun()
{
    JL_TICK->CON |= BIT(6);
}
IRQ_REGISTER(IRQ_TICK_TMR_IDX, tick_timer_isr_fun);
static void tick_timer_init()
{
    IRQ_REQUEST(IRQ_TICK_TMR_IDX, tick_timer_isr_fun);
    JL_TICK->CNT   = 0;
    JL_TICK->PRD   = 0xfffffffe;
    JL_TICK->CON  = BIT(0);
    puts("---tick_timer_init\n");

}
void spdif_open()
{
    //-------------------------------------------------------
    //  port initial
    //-------------------------------------------------------
    JL_PORTA->PD |= BIT(5);
    JL_PORTA->DIR |= BIT(5);
    //-------------------------------------------------------
    //  LEN & ADR config
    //-------------------------------------------------------
    JL_SPDIF->SS_DMA_LEN = SPDIF_DAT_DMA_LEN << 0 | SPDIF_INFO_LEN << 16;
    JL_SPDIF->SS_DAT_ADR = (volatile unsigned short)spdif_data_buf;
    JL_SPDIF->SS_INF_ADR = (volatile unsigned short)infor_buf;

    //-------------------------------------------------------
    //  CON config
    //-------------------------------------------------------
    IRQ_REQUEST(IRQ_SPDIF_IDX, spdif_isr_fun);
    JL_SPDIF->SS_CON |= SS_EN << 0 | INT_EN << 2 | SS_IOS << 14 | SE_CNT << 16;
    delay(10);

    //-------------------------------------------------------
    //  DMA_CON config
    //-------------------------------------------------------
    JL_SPDIF->SS_DMA_CON |= SE_DECT_EN << 3 | CSBR_CNT << 8;
    spdif_src_init();
    tick_timer_init();
    JL_SPDIF->SS_DMA_CON |= DAT_DMA_EN << 0 | DAT_DMA_MD << 1 | INF_DMA_EN << 2;
    SS_START;

    //dac_mute(0, 0);
    printf("ssdma_con %x\n", JL_SPDIF->SS_DMA_CON);
    printf("ss_con %x\n", JL_SPDIF->SS_CON);
    puts("++++spdif int ++++++\n");
}

void spdif_close()
{
    JL_SPDIF->SS_CON = 0;
    JL_SPDIF->SS_DMA_CON = 0;
    error_flag = 0;
}
static void spdif_error_detect()
{
    if (error_flag) {
        puts("reset\n");
        error_flag = 0;
        spdif_close();
        delay(40);
        spdif_open();
    }
}
LOOP_DETECT_REGISTER(spdif_error_handler) = {
    .time = 2,
    .fun  = spdif_error_detect,
};
#endif
