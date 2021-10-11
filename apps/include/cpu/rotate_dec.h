#ifndef __ROTATE_DEC_H__
#define __ROTATE_DEC_H__

#include "typedef.h"

#define RDEC_POL_PU			1
#define RDEC_POL_PD			0

#define RDEC_EN(x)			SFR(JL_RDEC->CON, 0, 1, x)
#define RDEC_POL(x)			SFR(JL_RDEC->CON, 1, 1, x)
#define RDEC_SPD(x)			SFR(JL_RDEC->CON, 2, 4, x)
#define RDEC_CPND()			(JL_RDEC->CON |= BIT(6))
#define RDEC_PND()			(JL_RDEC->CON & BIT(7))



#endif
