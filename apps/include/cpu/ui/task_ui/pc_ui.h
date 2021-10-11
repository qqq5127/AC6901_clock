#ifndef _PC_UI_H_
#define _PC_UI_H_

#include "typedef.h"
#include "ui/ui_api.h"

#if UI_ENABLE
void ui_open_pc(void *buf, u32 len);
void ui_close_pc(void);
#else
#define ui_open_pc(...)
#define ui_close_pc(...)
#endif


#endif/*_PC_UI_H_*/
