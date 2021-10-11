
#ifndef _WDT_H
#define _WDT_H

#define WD_1MS			0x00
#define WD_2MS			0x01
#define WD_4MS			0x02
#define WD_8MS			0x03
#define WD_16MS			0x04
#define WD_32MS			0x05
#define WD_64MS			0x06
#define WD_128MS		0x07
#define WD_256MS		0x08
#define WD_512MS		0x09
#define WD_1S			0x0A
#define WD_2S			0x0B
#define WD_4S			0x0C
#define WD_8S			0x0D
#define WD_16S			0x0E
#define WD_32S			0x0F

void open_wdt(u8 cfg);

void close_wdt(void);

void clear_wdt(void);

#endif
