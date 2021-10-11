#ifndef _PC_UI_H_
#define _PC_UI_H_

#include "typedef.h"
#include "ui/ui_api.h"

#if UI_ENABLE
void ui_open_rec(void *buf, u32 len);
void ui_close_rec(void);
#else
#define ui_open_rec(...)
#define ui_close_rec(...)
#endif


#endif/*_PC_UI_H_*/
