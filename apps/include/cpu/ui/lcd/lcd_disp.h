#ifndef _LCD_DISP_H_
#define _LCD_DISP_H_

#include "sdk_cfg.h"
#if LCD_128X64_EN
typedef struct {
    u8 left;
    u8 top;
    u8 right;
    u8 bottom;
} _lcd_area_size_t;

typedef struct _LCD_VAR {
    u8  bMaxPage;
    u8  bMaxColumn;
    volatile u8  bBusy;
    u8  bCurPage;       //<当前页
    u8  bUpdateLength;  //<更新长度
    u8  bCoordinateX;
    u8  bCoordinateY;
    u8  bFontSize;     //<字符大小
} LCD_VAR;

typedef union _LCD_BUFF {
    //<LCD 显示buffer
    u8 LCDBuff32X128[4][128];
    u8 LCDBuff64X128[8][128];
} LCD_BUFF;


extern LCD_VAR lcd_var;
extern LCD_BUFF lcd_buff;
extern u32 filenameCnt;

#define LANGUAGE_ENGLISH      0
#define LANGUAGE_CHINESE      1


#define LCD_START_LUMN          0
#define LCD_END_LUMN            lcd_var.bMaxColumn


#define LCD_CONTRAST_12832			0x44                  //contrast
#define LCD_CONTRAST_12864			0x34//0x20                  //contrast

/*UI - Music 界面定位*/
#define DEVICE_COLUMN 1
#define PLAYTIME_COLUMN (DEVICE_COLUMN+16+2)

#define EQ_COLUMN       (DEVICE_COLUMN+16+2+29)
#define FILENUM_COLUMN  (DEVICE_COLUMN+16+2+29+21)


/*UI - RTC 界面定位*/
#define RTC_YEAR	   	(LCD_END_LUMN-10*8)/2+0
#define RTC_MONTH	    (LCD_END_LUMN-10*8)/2+40
#define RTC_DAY 		(LCD_END_LUMN-10*8)/2+64
#define RTC_HOUR		(LCD_END_LUMN-10*8)/2+0
#define RTC_MIN		    (LCD_END_LUMN-10*8)/2+24
#define RTC_SEC		    (LCD_END_LUMN-10*8)/2+48

#define ALM_SWITCH      (LCD_END_LUMN-8*8)/2+48
#define ALM_HOUR	    (LCD_END_LUMN-8*8)/2+0
#define ALM_MIN		    (LCD_END_LUMN-8*8)/2+24

#define ALM_MONDAY      (LCD_END_LUMN-7*10)/2+0
#define ALM_TUESDAY	    (LCD_END_LUMN-7*10)/2+10
#define ALM_WEDNESDAY	(LCD_END_LUMN-7*10)/2+20
#define ALM_THRUSDAY	(LCD_END_LUMN-7*10)/2+30
#define ALM_FRIDAY  	(LCD_END_LUMN-7*10)/2+40
#define ALM_SATURDAY  	(LCD_END_LUMN-7*10)/2+50
#define ALM_SUNDAY  	(LCD_END_LUMN-7*10)/2+60

#define FRE_4_COLUMN    ((LCD_END_LUMN - 8*8)/2)
#define FRE_3_COLUMN    ((LCD_END_LUMN - 7*8)/2)
#define FM_CHANNL_COLUMN    1



///------------common-api--------------
void lcd_setX(u8 X);
void lcd_disp_text(const char *str, u8 dvtxt_num);
u16 lcd_disp_text2(const char *str, u8 dvtxt_num, u8 flags);
void lcd_disp_string_menu(u8 menu);
void lcd_null_dis(void);
void lcd_disp_common(void);
void init_lcd_bmp_add_table(void);
void lcd_disp_power(void);

///------------main-display--------------
void lcd_disp_welcome(void);
void lcd_disp_IR_number(s32 input_number);
void lcd_disp_vol(s32 vol);
void lcd_clear(void);


///-------------app-display---------------


///------------music-display--------------
void lcd_music_main(void);
void lcd_disp_filenumber(void);
void lcd_disp_playmode(s32 arg);
void lcd_disp_music_time(void);
void lcd_disp_filename(void);
void lcd_disp_eq(s32 arg);

///------------bt-display--------------
void lcd_bt_main(u8 menu);

///------------pc-display--------------
void lcd_pc_main(u8 menu);

///------------echo-display--------------
void lcd_echo_main(u8 menu);

///------------fm-display--------------
void lcd_fm_main(void);
void lcd_fm_channel(void);
void lcd_fmtx_main(void);

///------------rtc-display--------------
void lcd_rtc_main(void);
// void lcd_disp_RTC_main(void);

///------------rec-display--------------
void lcd_rec_main(void);

///------------aux-display--------------
void lcd_aux_main(u8 menu);

extern void lcd_rtc_set(void);
extern void lcd_alarm_set(void);
extern void lcd_alarm_ring(void);



#endif/*LCD_128X64_EN*/
#endif/*_LCD_DISP_H_*/
