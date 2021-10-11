/*********************************************************************************************
    *   Filename        : cpu.h

    *   Description     :

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2016-07-12 10:56

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _CPU_H_
#define _CPU_H_



#ifndef __ASSEMBLY__
#include "asm_type.h"
#include "hw_cpu.h"
#endif
/*
 *      -------------------------User Config---------------------
 */
#if 0
#define     IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}
#define     IO_DEBUG_TOGGLE(i,x)  {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT ^= BIT(x);}
#else
#define     IO_DEBUG_0(i,x)
#define     IO_DEBUG_1(i,x)
#define     IO_DEBUG_TOGGLE(i,x)
#endif

#define     DEBUG_PC_LIMIT(start, end)    \
	JL_DEBUG->DSP_PC_LIMH = (volatile unsigned long)end;  \
	JL_DEBUG->DSP_PC_LIML = (volatile unsigned long)start

/*
*********************************************************************************************************
*                                              Defines
*********************************************************************************************************
*/
#define     CLR_WDT()	(JL_SYSTEM->WDT_CON |= BIT(6))
#define     trig        __asm__ volatile ("trigger")
#define     LITTLE_ENDIAN

#define MAX_SDRAM_SIZE          (80*1024 - 0x400 - 30*1024)

#define  CPU_SR_ALLOC()

#ifndef __ASSEMBLY__
extern void CPU_INT_DIS();
extern void CPU_INT_EN();
#endif

#define OS_ENTER_CRITICAL()  CPU_INT_DIS();           /* 关中断*/
#define OS_EXIT_CRITICAL()   CPU_INT_EN();          /* 开中断*/

#define MULU(Rm,Rn) __builtin_pi32_umul64(Rm,Rn);
#define MUL(Rm,Rn)  __builtin_pi32_smul64(Rm,Rn);
#define MLA(Rm,Rn)  __builtin_pi32_smla64(Rm,Rn);
#define MLA0(Rm,Rn) MUL(Rm,Rn);
#define MLS(Rm,Rn)  __builtin_pi32_smls64(Rm,Rn);
#define MLS0(Rm,Rn) MUL(-Rm,Rn);
#define MRSIS(Rm,Rn) (Rm = __builtin_pi32_sreadmacc(Rn));//mac read right shift imm&sat
#define MRSRS(Rm,Rn) (Rm = __builtin_pi32_sreadmacc(Rn)); //mac read right shift reg&sat
#define MACCLR() __asm__ volatile("clrmacc":::"maccl","macch");
#define MACSET(h,l) __asm__ volatile ("mov maccl, %0; mov macch, %1"::"r" (l), "r" (h));
#define MACRL(l) __asm__ volatile ("mov %0, maccl":"=r" (l));
#define MACRH(l) __asm__ volatile ("mov %0, macch":"=r" (l));
#define MULSIS(Ro,Rm,Rn,Rp) MUL(Rm, Rn); MRSIS(Ro, Rp);
#define MULSRS(Ro,Rm,Rn,Rp) MUL(Rm, Rn); MRSRS(Ro, Rp);

#if 0
//debug
#define ASSERT(a,...)   \
        do { \
            if(!(a)){ \
                printf("%s %d", __FILE__, __LINE__); \
                printf("ASSERT-FAILD: "#a"\n"__VA_ARGS__); \
                while(1); \
            } \
        }while(0);
#define SOFT_RESET()    \
		do { \
			printf("soft reset by %s %d", __FILE__, __LINE__); \
			JL_POWER->CON |= BIT(4); \
		}while(0);
#else
/*assert error reset*/
#define ASSERT(a,...)   \
        do { \
            if(!(a)){ \
     			JL_POWER->CON |= BIT(4);\
            } \
        }while(0);

#define SOFT_RESET()    \
		do { \
			JL_POWER->CON |= BIT(4); \
		}while(0);
#endif
#endif

