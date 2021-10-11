#ifndef __FONT_OUT_H__
#define __FONT_OUT_H__

#include "typedef.h"
#include "string.h"

typedef void (*FONT_HOOK)(u8 *pixbuf, u16 width, u16 height, u16 x, u16 y);

//字号大小
typedef enum {
    SMALL_FONT,
    MIDDLE_FONT,
    BIG_FONT,
} FONT_SIZE;

typedef struct _FONT_IO {
    tbool(*get_file_addr_bypath)(u8 *path, u32 *addr);
    tbool(*flash_read)(u8 _xdata *buf, u32 addr, u32 length);
} FONT_IO;

u8 font_get_language_mode(void);
bool font_init(u8 language, FONT_IO *);
u16  font_textout(u8 *buf, u16 len, u16 x, u16 y, u8 fontsize);
u16  font_textoutw(u8 *buf, u16 len, u16 x, u16 y, bool isBigEndian, u8 fontsize);
u16  font_utf16toansi(u16 utf);
tbool font_get_file_addr_bypath(u8 *path, u32 *addr);

void font_flash_read(u8 _xdata *buf, u32 addr, u32 length);

u8 font_get_ansi_width(u8 *str, u16 offset);
u8 font_get_unicode_width(u8 *str, u16 offset, bool bigendian);
void font_set_output_callback(FONT_HOOK callback);
void font_output(u8 *pixbuf, u16 width, u16 height, u16 x, u16 y);

#endif
