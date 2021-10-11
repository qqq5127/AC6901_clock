#ifndef _AUDIO_LINK_H_
#define _AUDIO_LINK_H_

#include "typedef.h"

//PC1(MCLK) PA9(SCLK) PA10(LRCLK) PA11(CHL0) PA12(CHL1) PA7(CHL2) PA8(CHL3)
#define ALNK_MCLKA		BIT(1)
#define ALNK_SCLKA		BIT(9)
#define ALNK_LRCKA		BIT(10)
#define ALNK_DAT0A		BIT(11)
#define ALNK_DAT1A		BIT(12)
#define ALNK_DAT2A		BIT(7)
#define ALNK_DAT3A		BIT(8)

//PB6(MCLK) PB0(SCLK) PB1(LRCLK) PB2(CHL0) PB3(CHL1) PB4(CHL2) PB5(CHL3)
#define ALNK_MCLKB		BIT(6)
#define ALNK_SCLKB		BIT(0)
#define ALNK_LRCKB		BIT(1)
#define ALNK_DAT0B		BIT(2)
#define ALNK_DAT1B		BIT(3)
#define ALNK_DAT2B		BIT(4)
#define ALNK_DAT3B		BIT(5)

//ch_num
#define ALINK_CH0    	0
#define ALINK_CH1    	4
#define ALINK_CH2    	8
#define ALINK_CH3    	12

//ch_io
enum {
    ALINK_IOMAP_PA	= 0u,
    ALINK_IOMAP_PB 		,
};
//ch_dir
enum {
    ALINK_DIR_TX	= 0u,
    ALINK_DIR_RX		,
};
//ch_bitwide
enum {
    ALINK_LEN_16BIT = 0u,
    ALINK_LEN_24BIT		,
};
//ch_mode
enum {
    ALINK_MD_NONE	= 0u,
    ALINK_MD_IIS		,
    ALINK_MD_IIS_LALIGN	,
    ALINK_MD_IIS_RALIGN	,
    ALINK_MD_DSP0		,
    ALINK_MD_DSP1		,
};

typedef struct _ALINK_PARM {
    u8 ch_io;
    u8 ch_num;
    u8 ch_dir;
    u8 ch_mode;
    u8 ch_bitwide;
    u8 ch_moe;
    u8 ch_soe;
    u8 dma_len;
    u32 ch_sr;
    void (*isr_cb)(void *buf, u16 len);
} ALINK_PARM;

s32 alink_init(ALINK_PARM *parm);
s32 alink_exit(void);
s32 alink_sr_set(u16 sr);

#endif
