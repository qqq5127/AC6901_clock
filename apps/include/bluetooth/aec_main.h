#ifndef __AEC_MAIN_H__
#define __AEC_MAIN_H__

#include "typedef.h"
#include "aec/aec.h"
#include "aec/SPD.h"
#include "aec/PLC.h"

void aec_task_main();

s32 hook_sco_conn(void *priv);
s32 hook_sco_disconn(void *priv);
void hook_sco_rx(s16 *data, u16 point, u8 sco_flags);
void hook_sco_tx(s16 *data, u16 point);

#endif
