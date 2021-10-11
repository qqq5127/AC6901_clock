#ifndef __FONT_AREA_CTL_H__
#define __FONT_AREA_CTL_H__

#include "typedef.h"

typedef struct _COORDINATE {
    u16 x;
    u16 y;
} COORDINATE;


void screen_set_size(u16 width, u16 heigth);
u16  screen_get_width(void);
u16  screen_get_height(void);
void font_set_range(u16 x, u16 y, u16 width, u16 height);
u16 font_get_width(void);
u16 font_get_height(void);

extern u16 g_text_width;
extern u16 g_text_heigth;

u16 font_get_posX(void);

#endif
