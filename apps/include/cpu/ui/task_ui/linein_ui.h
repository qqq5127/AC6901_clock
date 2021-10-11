#ifndef _LINEIN_UI_H_
#define _LINEIN_UI_H_

#include "ui/ui_api.h"
#if UI_ENABLE
void ui_open_aux(void *buf, u32 len);
void ui_close_aux(void);
#else
#define ui_open_aux(...)
#define ui_close_aux(...)
#endif
#endif/*_LINEIN_UI_H_*/
