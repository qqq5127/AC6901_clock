#ifndef _PDM_LINK_H
#define _PDM_LINK_H

#include "typedef.h"
#include "audio/plink_drv.h"

s32 pdm_link_init(PLNK_DRV *ops);
void pdm_link_enable(void);
void pdm_link_disable(void);

void pdm_link_demo(u8 en);

#endif  //_PDM_LINK_H
