#ifndef __JL_HW_CPU__
#define __JL_HW_CPU__

#include "sfr.h"

//===============================================================================//
//
//      sfr define
//
//===============================================================================//


//............. 0x0000 - 0x00ff............ for cpu
typedef struct {
    __RW __u32 CON;
} JL_DSP_TypeDef;

#define JL_DSP_BASE               (hs_base + map_adr(0x00, 0x00))

#define JL_DSP                    ((JL_DSP_TypeDef			*)JL_DSP_BASE)

typedef struct {
    __RO __u32 ILAT1;
    __WO __u32 ILAT1_SET;
    __WO __u32 ILAT1_CLR;
    __RO __u32 ILAT0;
    __WO __u32 ILAT0_SET;
    __WO __u32 ILAT0_CLR;
    __RW __u32 IPCON0;
    __RW __u32 IPCON1;
    __RW __u32 IPCON2;
    __RW __u32 IPCON3;
} JL_NVIC_TypeDef;

#define JL_NVIC_BASE               (hs_base + map_adr(0x00, 0x01))

#define JL_NVIC                    ((JL_NVIC_TypeDef			*)JL_NVIC_BASE)

//
typedef struct {
    __RW __u32 CON;
    __RW __u32 CNT;
    __RW __u32 PRD;
} JL_TICK_TypeDef;

#define JL_TICK_BASE               (hs_base + map_adr(0x00, 0x10))

#define JL_TICK                    ((JL_TICK_TypeDef			*)JL_TICK_BASE)


//............. 0x0100 - 0x01ff............ for debug
typedef struct {
    __RW __u32 DSP_BF_CON;
    __RW __u32 WR_EN;
    __RO __u32 MSG;
    __WO __u32 MSG_CLR;
    __RW __u32 DSP_PC_LIMH;
    __RW __u32 DSP_PC_LIML;
    __RW __u32 DSP_EX_LIMH;
    __RW __u32 DSP_EX_LIML;
    __RW __u32 PRP_EX_LIMH;
    __RW __u32 PRP_EX_LIML;
    __RO __u32 PRP_MMU_MSG;
    __RO __u32 LSB_MMU_MSG_CH;
    __RO __u32 PRP_WR_LIMIT_MSG;
    __RO __u32 LSB_WR_LIMIT_CH;
} JL_DEBUG_TypeDef;

#define JL_DEBUG_BASE               (hs_base + map_adr(0x01, 0x00))

#define JL_DEBUG                    ((JL_DEBUG_TypeDef			*)JL_DEBUG_BASE)

//............. 0x0200 - 0x02ff............ for sfc
typedef struct {
    __RW __u32 CON;
    __WO __u32 BAUD;
    __WO __u32 CODE;
    __WO __u32 BASE_ADR;
} JL_SFC_TypeDef;

#define JL_SFC_BASE               (hs_base + map_adr(0x02, 0x00))

#define JL_SFC                    ((JL_SFC_TypeDef			*)JL_SFC_BASE)

//............. 0x0300 - 0x03ff............ for enc
//
typedef struct {
    __RW __u32 CON;
    __WO __u32 KEY;
    __WO __u32 ADR;
    __RW __u32 UNENC_ADRH;
    __RW __u32 UNENC_ADRL;
} JL_ENC_TypeDef;

#define JL_ENC_BASE               (hs_base + map_adr(0x03, 0x00))

#define JL_ENC                    ((JL_ENC_TypeDef			*)JL_ENC_BASE)


//............. 0x0400 - 0x04ff............ for others

//............. 0x0500 - 0x05ff............ for aes
//
typedef struct {
    __RW __u32 CON;
    __RW __u32 DATIN;
    __WO __u32 KEY;
    __RW __u32 ENCRES0;
    __RW __u32 ENCRES1;
    __RW __u32 ENCRES2;
    __RW __u32 ENCRES3;
    __RW __u32 DECRES0;
    __RW __u32 DECRES1;
    __RW __u32 DECRES2;
    __RW __u32 DECRES3;
} JL_AES_TypeDef;

#define JL_AES_BASE               (hs_base + map_adr(0x05, 0x00))

#define JL_AES                    ((JL_AES_TypeDef			*)JL_AES_BASE)

//............. 0x0600 - 0x06ff............ for fft

typedef struct {
    __RW __u32 CON;
    __RW __u32 ADRI;
    __RW __u32 ADRO;
    __RW __u32 ADRW;
} JL_FFT_TypeDef;

#define JL_FFT_BASE               (hs_base + map_adr(0x06, 0x00))

#define JL_FFT                    ((JL_FFT_TypeDef			*)JL_FFT_BASE)

//............. 0x0700 - 0x07ff............ for eq

typedef struct {
    __RW __u32 CON;
    __RW __u32 LEN;
    __RW __u32 ADRI;
    __RW __u32 ADRO;
    __RW __u32 CADR;
} JL_EQ_TypeDef;

#define JL_EQ_BASE               (hs_base + map_adr(0x07, 0x00))

#define JL_EQ                    ((JL_EQ_TypeDef			*)JL_EQ_BASE)

//............. 0x0800 - 0x08ff............ for src
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 IDAT_ADR;
    __RW __u32 IDAT_LEN;
    __RW __u32 ODAT_ADR;
    __RW __u32 ODAT_LEN;
    __RW __u32 FLTB_ADR;
} JL_SRC_TypeDef;

#define JL_SRC_BASE               (hs_base + map_adr(0x08, 0x00))

#define JL_SRC                    ((JL_SRC_TypeDef			*)JL_SRC_BASE)

//............. 0x0800 - 0x08ff............ for fm
/*
typedef struct {
    __RW __u32 FM_CON;
    __RW __u32 FM_BASE;
    __RW __u32 FMADC_CON;
    __RW __u32 FMADC_CON1;
    __RW __u32 FMHF_CON0;
    __RW __u32 FMHF_CON1;
    __RW __u32 RESERVE;
    __RW __u32 FMHF_CRAM;
    __RW __u32 FMHF_CRAM2;
    __RW __u32 FMHF_DRAM;
    __RW __u32 FMLF_CON;
    __RW __u32 FMLF_RES;
    __RW __u32 FM_DEVLOCK;
    __RW __u32 FM_SEEKCNT;
    __RW __u32 FM_IRSSI;
    __RW __u32 FM_IRSSI_CS0;
    __RW __u32 FM_IRSSI_CS1;
    __RW __u32 FM_IRSSI_CS2;
    __RW __u32 FM_IRSSI_CS3;
    __RW __u32 FM_CNR;
    __RW __u32 FM_CNR_CS0;
    __RW __u32 FM_CNR_CS1;
    __RW __u32 FM_CNR_CS2;
    __RW __u32 FM_CNR_CS3;
} JL_FMRX_TypeDef;

#define JL_FMRX_BASE               (hs_base + map_adr(0x09, 0x00))

#define JL_FMRX                    ((JL_FMRX_TypeDef			*)JL_FMRX_BASE)
*/
//===============================================================================//
//
//      low speed sfr address define
//
//===============================================================================//

//............. 0x0000 - 0x00ff............ for syscfg
typedef struct {
    __WO __u32 CHIP_ID;
    __RW __u32 MODE_CON;
    __u32 RESERVED[0x1f - 0x1 - 1];
    __RW __u32 RST_SRC;
    __RW __u32 RESRVED0;
    __RW __u32 LDO_CON0;
    __RW __u32 LVD_CON;
    __RW __u32 WDT_CON;
    __RW __u32 OSA_CON;
    __RW __u32 EFUSE_CON;
    __RW __u32 RESRVED1;
    __RW __u32 LDO_CON1;
} JL_SYSTEM_TypeDef;

#define JL_SYSTEM_BASE               (ls_base + map_adr(0x00, 0x00))

#define JL_SYSTEM                    ((JL_SYSTEM_TypeDef			*)JL_SYSTEM_BASE)


typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
} JL_WAKEUP_TypeDef;

#define JL_WAKEUP_BASE               (ls_base + map_adr(0x00, 0x02))

#define JL_WAKEUP                    ((JL_WAKEUP_TypeDef			*)JL_WAKEUP_BASE)

//
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
} JL_IOMAP_TypeDef;

#define JL_IOMAP_BASE               (ls_base + map_adr(0x00, 0x06))

#define JL_IOMAP                    ((JL_IOMAP_TypeDef			*)JL_IOMAP_BASE)


typedef struct {
    __RW __u32 CON;
} JL_POWER_TypeDef;

#define JL_POWER_BASE               (ls_base + map_adr(0x00, 0x10))

#define JL_POWER                    ((JL_POWER_TypeDef			*)JL_POWER_BASE)

typedef struct {
    __RW __u32 SYS_DIV;
    __RW __u32 CLK_CON0;
    __RW __u32 CLK_CON1;
    __RW __u32 CLK_CON2;
    __u32 RESERVED[0x20 - 0x14 - 1];
    __RW __u32 PLL_CON;
    __u32 RESERVED1[0x28 - 0x20 - 1];
    __RW __u32 PLL_INTF;
    __RW __u32 PLL_DMAX;
    __RW __u32 PLL_DMIN;
    __RW __u32 PLL_DSTP;
    __RW __u32 PLL_CON1;
} JL_CLOCK_TypeDef;

#define JL_CLOCK_BASE               (ls_base + map_adr(0x00, 0x11))

#define JL_CLOCK                    ((JL_CLOCK_TypeDef			*)JL_CLOCK_BASE)


//............. 0x0100 - 0x01ff............ for port

typedef struct {
    __RW __u32 OUT;
    __RO __u32 IN;
    __RW __u32 DIR;
    __RW __u32 DIE;
    __RW __u32 PU;
    __RW __u32 PD;
    __RW __u32 HD;
} JL_PORT_TypeDef;

typedef struct {
    __RW __u32 OUT;
    __RO __u32 IN;
    __RW __u32 DIR;
    __RW __u32 DIE;
    __RW __u32 PU;
    __RW __u32 PD;
    __RW __u32 HD;
    __RW __u32 HD1;
} JL_PORT_TypeDef_HD;

#define JL_PORTA_BASE               (ls_base + map_adr(0x01, 0x00))
#define JL_PORTA                    ((JL_PORT_TypeDef			*)JL_PORTA_BASE)

#define JL_PORTB_BASE               (ls_base + map_adr(0x01, 0x08))
#define JL_PORTB                    ((JL_PORT_TypeDef_HD		*)JL_PORTB_BASE)

#define JL_PORTC_BASE               (ls_base + map_adr(0x01, 0x10))
#define JL_PORTC                    ((JL_PORT_TypeDef			*)JL_PORTC_BASE)

#define JL_PORTD_BASE               (ls_base + map_adr(0x01, 0x18))
#define JL_PORTD                    ((JL_PORT_TypeDef_HD		*)JL_PORTD_BASE)

//............. 0x0200 - 0x02ff............ for timer

typedef struct {
    __RW __u32 CON;
    __RW __u32 CNT;
    __RW __u32 PRD;
    __RW __u32 PWM;
} JL_TIMER_TypeDef;

#define JL_TIMER0_BASE               (ls_base + map_adr(0x02, 0x00))
#define JL_TIMER0                    ((JL_TIMER_TypeDef			*)JL_TIMER0_BASE)

#define JL_TIMER1_BASE               (ls_base + map_adr(0x02, 0x04))
#define JL_TIMER1                    ((JL_TIMER_TypeDef			*)JL_TIMER1_BASE)

#define JL_TIMER2_BASE               (ls_base + map_adr(0x02, 0x08))
#define JL_TIMER2                    ((JL_TIMER_TypeDef			*)JL_TIMER2_BASE)

#define JL_TIMER3_BASE               (ls_base + map_adr(0x02, 0x0c))
#define JL_TIMER3                    ((JL_TIMER_TypeDef			*)JL_TIMER3_BASE)

//............. 0x0300 - 0x03ff............ for uart


typedef struct {
    __RW __u16 CON0;
    __RW __u16 CON1;
    __WO __u16 BAUD;
    __RW __u8  BUF;
    __RW __u32 OTCNT;
    __RW __u32 TXADR;
    __WO __u16 TXCNT_HRXCNT;
    //__RO __u16 HRXCNT;
    __RW __u32 RXSADR;
    __RW __u32 RXEADR;
    __RW __u32 RXCNT;
} JL_UART_TypeDef;

#define JL_UART0_BASE               (ls_base + map_adr(0x03, 0x00))
#define JL_UART0                    ((JL_UART_TypeDef			*)JL_UART0_BASE)

#define JL_UART1_BASE               (ls_base + map_adr(0x03, 0x0a))
#define JL_UART1                    ((JL_UART_TypeDef			*)JL_UART1_BASE)


typedef struct {
    __RW __u16 CON;
    __RW __u8 BUF;
    __WO __u16 BAUD;
} JL_UART2_TypeDef;

#define JL_UART2_BASE               (ls_base + map_adr(0x03, 0x14))
#define JL_UART2                    ((JL_UART2_TypeDef			*)JL_UART2_BASE)



typedef struct {
    __RW __u32 CON;
    __WO __u32 BAUD;
    __RW __u32 BUF;
    __WO __u32 ADR;
    __WO __u32 CNT;
} JL_SPI_TypeDef;

#define JL_SPI0_BASE                  (ls_base + map_adr(0x04, 0x00))
#define JL_SPI0                       ((JL_SPI_TypeDef			*)JL_SPI0_BASE)

#define JL_SPI1_BASE                  (ls_base + map_adr(0x04, 0x05))
#define JL_SPI1                       ((JL_SPI_TypeDef			*)JL_SPI1_BASE)

#define JL_SPI2_BASE                  (ls_base + map_adr(0x04, 0x0a))
#define JL_SPI2                       ((JL_SPI_TypeDef			*)JL_SPI2_BASE)


//............. 0x0500 - 0x05ff............ for pap
typedef struct {
    __RW __u32 CON;
    __WO __u32 DAT0;
    __WO __u32 DAT1;
    __RO __u32 RESERVED[1];
    __WO __u32 ADR;
    __WO __u32 CNT;
} JL_PAP_TypeDef;

#define JL_PAP_BASE                  (ls_base + map_adr(0x05, 0x00))
#define JL_PAP                       ((JL_PAP_TypeDef			*)JL_PAP_BASE)


//............. 0x0600 - 0x06ff............ for sd
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __WO __u32 CPTR;
    __WO __u32 DPTR;
} JL_SD_TypeDef;

#define JL_SD0_BASE                  (ls_base + map_adr(0x06, 0x00))
#define JL_SD0                       ((JL_SD_TypeDef			*)JL_SD0_BASE)

#define JL_SD1_BASE                  (ls_base + map_adr(0x06, 0x05))
#define JL_SD1                       ((JL_SD_TypeDef			*)JL_SD1_BASE)

//............. 0x0700 - 0x07ff............ for iic
typedef struct {
    __RW __u16 CON0;
    __RW __u8 BUF;
    __WO __u8 BAUD;
    __RW __u16 CON1;
} JL_IIC_TypeDef;

#define JL_IIC_BASE                  (ls_base + map_adr(0x07, 0x00))
#define JL_IIC                       ((JL_IIC_TypeDef			*)JL_IIC_BASE)

//............. 0x0800 - 0x08ff............ for lcd

typedef struct {
    __RW __u32 CON0;
    __RO __u32 RESERVED[1];
    __RW __u32 SEG_IOEN0;
    __RW __u32 SEG_IOEN1;
} JL_LCDC_TypeDef;

#define JL_LCDC_BASE                  (ls_base + map_adr(0x08, 0x00))
#define JL_LCDC                       ((JL_LCDC_TypeDef			*)JL_LCDC_BASE)

//............. 0x0900 - 0x09ff............ for others
// #define PWM4_CON                      (*(__WO __u8  *)(ls_base + map_adr(0x09, 0x00)))
#if 0
typedef struct {
    __WO __u32 CON;
} JL_PWM4_TypeDef;

#define JL_PWM4_BASE                  (ls_base + map_adr(0x09, 0x00))
#define JL_PWM4                       ((JL_PWM4_TypeDef			*)JL_PWM4_BASE)
#endif

// #define RFLT_CON                      (*(__RW __u8  *)(ls_base + map_adr(0x09, 0x01)))

typedef struct {
    __RW __u32 RFLT_CON;
} JL_IR_TypeDef;

#define JL_IR_BASE                  (ls_base + map_adr(0x09, 0x01))
#define JL_IR                       ((JL_IR_TypeDef			*)JL_IR_BASE)


typedef struct {
    __RW __u32 CON;
    __RW __u32 DAT;
    __RW __u32 RESERVED[2];
    __RW __u32 LCON;
    __RW __u32 LNUM;
} JL_IRTC_TypeDef;

#define JL_IRTC_BASE                (ls_base + map_adr(0x09, 0x02))
#define JL_IRTC                     ((JL_IRTC_TypeDef			*)JL_IRTC_BASE)


typedef struct {
    __RW __u8 CON;
    __RW __u8 DAT;
} JL_RDEC_TypeDef;

#define JL_RDEC_BASE                (ls_base + map_adr(0x09, 0x04))
#define JL_RDEC                     ((JL_RDEC_TypeDef			*)JL_RDEC_BASE)




//............. 0x0a00 - 0x0aff............ for dac

typedef struct {
    __WO __u16 DAC_LEN;
    __RW __u16 DAC_CON;
    __WO __u32 DAC_ADR;
    __WO __u8  DAC_TRML;
    __WO __u8  DAC_TRMR;
    __RW __u16 DAC_CON1;

    __u32 RESERVED0[0x8 - 0x5 - 1];
    __RW __u16 LADC_CON;
    __u32 RESERVED1[0xb - 0x8 - 1];
    __WO __u32 LADC_ADR;
    __WO __u16 LADC_LEN;
    __u32 RESERVED2[0x10 - 0xc - 1];
    __RW __u32 DAA_CON0;
    __RW __u32 DAA_CON1;
    __RW __u32 DAA_CON2;
    __RW __u32 DAA_CON3;
    __u32 RESERVED3[0x20 - 0x13 - 1];
    __RW __u32 ADA_CON0;
    __RW __u32 ADA_CON1;
} JL_AUDIO_TypeDef;

#define JL_AUDIO_BASE                  (ls_base + map_adr(0x0a, 0x00))
#define JL_AUDIO                       ((JL_AUDIO_TypeDef			*)JL_AUDIO_BASE)

//............. 0x0b00 - 0x0bff............ for alnk
typedef struct {
    __RW __u16 CON0;
    __RW __u16 CON1;
    __RW __u8  CON2;
    __RW __u8  CON3;
    __WO __u32 ADR0;
    __WO __u32 ADR1;
    __WO __u32 ADR2;
    __WO __u32 ADR3;
    __WO __u16 LEN;
} JL_ALINK_TypeDef;

#define JL_ALINK_BASE                  (ls_base + map_adr(0x0b, 0x00))
#define JL_ALINK                       ((JL_ALINK_TypeDef			*)JL_ALINK_BASE)


typedef struct {
    __RW __u16 CON;
    __RW __u8  SMR;
    __RW __u32 ADR;
    __RW __u32 LEN;
} JL_PLNK_TypeDef;

#define JL_PLNK_BASE                  (ls_base + map_adr(0x0b, 0x09))
#define JL_PLNK                       ((JL_PLNK_TypeDef			*)JL_PLNK_BASE)


//............. 0x0c00 - 0x0cff............ for nfc
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 BUF0;
    __RW __u32 BUF1;
    __WO __u32 BUF2;
    __WO __u32 BUF3;
} JL_NFC_TypeDef;

#define JL_NFC_BASE                  (ls_base + map_adr(0x0c, 0x01))
#define JL_NFC                       ((JL_NFC_TypeDef			*)JL_NFC_BASE)

//............. 0x0d00 - 0x0dff............ for usb
typedef struct {
    __RW __u32 IO_CON0;
    __RW __u32 CON0;
    __RW __u32 CON1;
    __WO __u32 EP0_CNT;
    __WO __u32 EP1_CNT;
    __WO __u32 EP2_CNT;
    __WO __u32 EP3_CNT;
    __WO __u32 EP0_ADR;
    __WO __u32 EP1_TADR;
    __WO __u32 EP1_RADR;
    __WO __u32 EP2_TADR;
    __WO __u32 EP2_RADR;
    __WO __u32 EP3_TADR;
    __WO __u32 EP3_RADR;
    __RW __u32 IO_CON1;
} JL_USB_TypeDef;

#define JL_USB_BASE                  (ls_base + map_adr(0x0d, 0x00))
#define JL_USB                       ((JL_USB_TypeDef			*)JL_USB_BASE)

//............. 0x0e00 - 0x0eff............ for crc
typedef struct {
    __WO __u32 FIFO;
    __RW __u32 REG;
} JL_CRC_TypeDef;

#define JL_CRC_BASE                  (ls_base + map_adr(0x0e, 0x00))
#define JL_CRC                       ((JL_CRC_TypeDef			*)JL_CRC_BASE)

//............. 0x0f00 - 0x0fff............ for rand64
typedef struct {
    __RO __u32 R64L;
    __RO __u32 R64H;
} JL_RAND_TypeDef;

#define JL_RAND_BASE                  (ls_base + map_adr(0x0f, 0x00))
#define JL_RAND                       ((JL_RAND_TypeDef			*)JL_RAND_BASE)

//............. 0x1000 - 0x10ff............ for adc
typedef struct {
    __RW __u32 CON;
    __RO __u32 RES;
} JL_ADC_TypeDef;

#define JL_ADC_BASE                  (ls_base + map_adr(0x10, 0x00))
#define JL_ADC                       ((JL_ADC_TypeDef			*)JL_ADC_BASE)

//............. 0x1100 - 0x11ff............ for pulse counter
typedef struct {
    __RW __u32 CON;
    __RW __u32 TVL;
} JL_PLL_COUNTER_TypeDef;

#define JL_PLL_COUNTER_BASE                  (ls_base + map_adr(0x11, 0x00))
#define JL_PLL_COUNTER                       ((JL_PLL_COUNTER_TypeDef			*)JL_PLL_COUNTER_BASE)
//............. 0x1200 - 0x12ff............ for power down
typedef struct {
    __RW __u32 CON;
    __RW __u32 DAT;
    __RW __u32 P33_CON;
    __RW __u32 P33_DAT;
} JL_POWER_DOWN_TypeDef;

#define JL_POWER_DOWN_BASE                  (ls_base + map_adr(0x12, 0x00))
#define JL_POWER_DOWN                       ((JL_POWER_DOWN_TypeDef			*)JL_POWER_DOWN_BASE)


//............. 0x1300 - 0x13ff............ for ctmu
typedef struct {
    __RW __u16 CON0;
    __RW __u32 CON1;
    __RO __u16 RES;
} JL_CTM_TypeDef;

#define JL_CTM_BASE                  		(ls_base + map_adr(0x13, 0x00))
#define JL_CTM                       		((JL_CTM_TypeDef			*)JL_CTM_BASE)


typedef struct {
    __RW __u32 CH0_CON0;
    __RW __u32 CH0_CON1;
    __RW __u32 CH0_CMP;
    __RW __u32 CH1_CON0;
    __RW __u32 CH1_CON1;
    __RW __u32 CH1_CMP;
    __RW __u32 CH2_CON0;
    __RW __u32 CH2_CON1;
    __RW __u32 CH2_CMP;
    __RW __u32 TMR0_CON;
    __RW __u32 TMR0_CNT;
    __RW __u32 TMR0_PR;
    __RW __u32 FPIN_CON;
} JL_MCPWM_TypeDef;

#define JL_MCPWM_BASE               (ls_base + map_adr(0x13, 0x03))
#define JL_PWM                      ((JL_MCPWM_TypeDef *)JL_MCPWM_BASE)

#define JL_WL_SFR_ADR                (hs_base + map_adr(0x04, 0x00))
#define JL_WL_SFR_NUM                (0x6 + 1)

#define JL_WLA_SFR_ADR                (ls_base + map_adr(0x1c, 0x00))
#define JL_WLA_SFR_NUM                (0x26 + 1)

//............. 0x1400 - 0x14ff............
typedef struct {
    __RW __u32 SS_CON;
    __RW __u32 SS_DMA_CON;
    __RW __u32 SS_DMA_LEN;
    __WO __u32 SS_DAT_ADR;
    __WO __u32 SS_INF_ADR;
    __RO __u32 SS_CSB0;
    __RO __u32 SS_CSB1;
    __RO __u32 SS_CSB2;
    __RO __u32 SS_CSB3;
    __RO __u32 SS_CSB4;
    __RO __u32 SS_CSB5;
} JL_SPDIF_Typedef;

#define JL_SPDIF_BASE               (ls_base + map_adr(0x14, 0x00))
#define JL_SPDIF                    ((JL_SPDIF_Typedef				 *)JL_SPDIF_BASE)
/*
#define SS_CON                        (*(__RW __u32 *)(ls_base + map_adr(0x14, 0x00)))
#define SS_DMA_CON                    (*(__RW __u32 *)(ls_base + map_adr(0x14, 0x01)))
#define SS_DMA_LEN                    (*(__RW __u32 *)(ls_base + map_adr(0x14, 0x02)))
#define SS_DAT_ADR                    (*(__WO __u32 *)(ls_base + map_adr(0x14, 0x03)))
#define SS_INF_ADR                    (*(__WO __u32 *)(ls_base + map_adr(0x14, 0x04)))
#define SS_CSB0                       (*(__RO __u32 *)(ls_base + map_adr(0x14, 0x05)))
#define SS_CSB1                       (*(__RO __u32 *)(ls_base + map_adr(0x14, 0x06)))
#define SS_CSB2                       (*(__RO __u32 *)(ls_base + map_adr(0x14, 0x07)))
#define SS_CSB3                       (*(__RO __u32 *)(ls_base + map_adr(0x14, 0x08)))
#define SS_CSB4                       (*(__RO __u32 *)(ls_base + map_adr(0x14, 0x09)))
#define SS_CSB5                       (*(__RO __u32 *)(ls_base + map_adr(0x14, 0x0a)))
*/
//............. 0x1c00 - 0x1fff............ for anl ctl
//
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 CON4;
    __RW __u32 CON5;
    __RW __u32 CON6;
    __RW __u32 CON7;
    __RW __u32 CON8;
} JL_FMA_TypeDef;

#define JL_FMA_BASE               (ls_base + map_adr(0x1d, 0x00))

#define JL_FMA                    ((JL_FMA_TypeDef			*)JL_FMA_BASE)


#endif

