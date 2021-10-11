#include "sdk_cfg.h"
#include "linein_ui.h"
#include "ui/ui_api.h"
#if UI_ENABLE

//extern REVERB_API_STRUCT * aux_reverb;

#if (LED_7_EN || LED_1888_EN)
void ui_open_aux_led(void *buf, u32 len)
{
    puts("ui_open_aux_led\n");

    SET_UI_MAIN(MENU_AUX_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();
}
#endif

#if LCD_128X64_EN
void ui_open_aux_lcd(void *buf, u32 len)
{
    puts("ui_open_aux_lcd\n");

    SET_UI_MAIN(MENU_AUX_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();
}
#endif

void ui_open_aux(void *buf, u32 len)
{
#if (LED_7_EN || LED_1888_EN)
    if (UI_var.ui_type == UI_LED) {
        ui_open_aux_led(buf, len);
    }
#endif

#if LCD_128X64_EN
    if (UI_var.ui_type == UI_LCD_128X64) {
        ui_open_aux_lcd(buf, len);
    }
#endif
#if (USE_DISPLAY_WAIT == 0)
	ui_mode_wait_flag = 0;
#endif
}

void ui_close_aux(void)
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
#endif/*LINEIN_TASK*/
