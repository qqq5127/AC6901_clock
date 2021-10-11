#ifndef _PC_UI_H_
#define _PC_UI_H_

#include "typedef.h"
#include "ui/ui_api.h"

#if UI_ENABLE
void ui_open_echo(void *buf, u32 len);
void ui_close_echo(void);
#else
#define ui_open_echo(...)
#define ui_close_echo(...)
#endif


#endif/*_PC_UI_H_*/
