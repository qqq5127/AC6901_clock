#ifndef	_IIC_HW_H_
#define _IIC_HW_H_

#include "hw_cpu.h"
#include "typedef.h"

/**
 * @brief SFR
 */
#define	IIC_PORTA		0		//CLK:USBDP,	DAT:USBDM
#define	IIC_PORTB		1		//CLK:PC4,		DAT:PC5
#define	IIC_PORTC		2		//CLK:PC1,		DAT:PC2
#define	IIC_PORTD		3		//CLK:PA5,		DAT:PA6

#define IIC_PORT_SEL(x)     SFR(JL_IOMAP->CON1,18,2,x)

#define IIC_DATA_OUT()      SFR(JL_IIC->CON0,3,1,0)
#define IIC_DATA_IN()       SFR(JL_IIC->CON0,3,1,1)

#define IIC_WR_NACK_ACK(x)	SFR(JL_IIC->CON0,6,1,x)		//0:ack	1:noack

#define M_SET_N_RSTART()    SFR(JL_IIC->CON0,5,1,0)
#define M_SET_RSTART()      SFR(JL_IIC->CON0,5,1,1)

#define M_SET_N_END()       SFR(JL_IIC->CON0,4,1,0)
#define M_SET_END()         SFR(JL_IIC->CON0,4,1,1)

#define CFG_DONE()          SFR(JL_IIC->CON0,2,1,1);



/**
 * @brief function
 */
void iic_hw_init(void);

#endif
