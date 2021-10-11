#include "sdk_cfg.h"
#include "ui/ui_api.h"
#include "fm_ui.h"
//#include "fm/fm_radio.h"
//#include "echo/echo_api.h"

#if UI_ENABLE
extern void *fm_reverb;

#if (LED_7_EN || LED_1888_EN)
void ui_open_fm_led(void *buf, u32 len)
{
    puts("ui_open_aux_led\n");

	ui_mode_wait_flag = 0;
    SET_UI_MAIN(MENU_FM_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();
    //SET_UI_BUF_ADR(buf);
    //SET_UI_BUF_LEN(len);
    // UI_menu(MENU_FM_MAIN);
}
#endif

#if LCD_128X64_EN
void ui_open_fm_lcd(void *buf, u32 len)
{
    puts("ui_open_fm_lcd\n");

    SET_UI_MAIN(MENU_FM_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();
}
#endif

void ui_open_fm(void *buf, u32 len)
{
#if (LED_7_EN || LED_1888_EN)
    if (UI_var.ui_type == UI_LED) {
        ui_open_fm_led(buf, len);
    }
#endif

#if LCD_128X64_EN
    if (UI_var.ui_type == UI_LCD_128X64) {
        ui_open_fm_lcd(buf, len);
    }
#endif
#if (USE_DISPLAY_WAIT == 0)
	ui_mode_wait_flag = 0;
#endif
}

void ui_close_fm(void)
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

#endif
