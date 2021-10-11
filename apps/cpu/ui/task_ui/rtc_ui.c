/*
*********************************************************************************************************
*                                             BC51
*
*                                             CODE
*
*                          (c) Copyright 2015-2016, ZHUHAI JIELI
*                                           All Rights Reserved
*
* File : *
* By   : jamin.li
* DATE : 11/11/2015 build this file
*********************************************************************************************************
*/

#include "rtc_setting.h"
#include "ui_api.h"

#if UI_ENABLE

#if (LED_7_EN || LED_1888_EN)
void ui_open_rtc_led(void *buf, u32 len)
{
	ui_mode_wait_flag = 0;
    SET_UI_MAIN(MENU_RTC_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();
}
#endif

#if LCD_128X64_EN
void ui_open_rtc_lcd(void *buf, u32 len)
{

    SET_UI_MAIN(MENU_RTC_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();

}
#endif

void ui_open_rtc(void *buf, u32 len)
{

#if (LED_7_EN || LED_1888_EN)
    if (UI_var.ui_type == UI_LED) {
        ui_open_rtc_led(buf, len);
    }
#endif

#if LCD_128X64_EN
    if (UI_var.ui_type == UI_LCD_128X64) {
        ui_open_rtc_lcd(buf, len);
    }
#endif
#if (USE_DISPLAY_WAIT == 0)
	ui_mode_wait_flag = 0;
#endif


}

void ui_close_rtc(void)
{
#if (USE_DISPLAY_WAIT == 0)
	ui_mode_wait_flag = 1;
#endif
    UI_CLEAR_MSG_BUF();
#if USE_DISPLAY_WAIT
    SET_UI_MAIN(MENU_WAIT);
    SET_UI_BUF(NULL, 0);
    UI_DIS_MAIN();
#endif
}

#endif/*UI_ENABLE*/
