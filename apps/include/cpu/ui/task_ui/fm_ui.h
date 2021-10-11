#ifndef _FM_UI_H_
#define _FM_UI_H_
#include "ui/ui_api.h"

#if UI_ENABLE
void ui_open_fm(void *buf, u32 len);
void ui_close_fm(void);
#else
#define ui_open_fm(...)
#define ui_close_fm(...)
#endif

#endif/*_FM_UI_H_*/
