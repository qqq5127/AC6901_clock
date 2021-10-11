#include "sdk_cfg.h"
#include "ui/ui_api.h"
#include "bt_ui.h"
#include "audio/audio.h"

#if  UI_ENABLE

#if (LED_7_EN || LED_1888_EN)
void ui_open_bt_led(void *buf, u32 len)
{
    puts("ui_open_bt_led\n");

	ui_mode_wait_flag = 0;
    SET_UI_MAIN(MENU_BT_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();
}
#endif


#if LCD_128X64_EN
void ui_open_bt_lcd(void *buf, u32 len)
{
    puts("ui_open_bt_lcd\n");

    SET_UI_MAIN(MENU_BT_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();
}
#endif

void ui_open_bt(BT_DIS_VAR *buf, u32 len)
{
    if (buf == NULL) {
        return ;
    }
    memset(buf, 0x00, len);
    ui_bt_update_var((BT_DIS_VAR *)buf);

#if (LED_7_EN || LED_1888_EN)
    if (UI_var.ui_type == UI_LED) {
        ui_open_bt_led(buf, len);
    }
#endif

#if LCD_128X64_EN
    if (UI_var.ui_type == UI_LCD_128X64) {
        ui_open_bt_lcd(buf, len);
    }
#endif
#if (USE_DISPLAY_WAIT == 0)
	ui_mode_wait_flag = 0;
#endif
}

void ui_close_bt(void)
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

extern u8 a2dp_get_status(void);
void ui_bt_update_var(BT_DIS_VAR *bt_dis_var)
{
    if (bt_dis_var == NULL) {
        return;
    }

    bt_dis_var->ui_bt_connect_sta = get_bt_connect_status();
    bt_dis_var->ui_bt_a2dp_sta = (u8)a2dp_get_status();
    bt_dis_var->bt_eq_mode = sound.eq_mode;

#if (USE_DISPLAY_WAIT == 0)
	if (ui_mode_wait_flag == 0)
#endif
    	UI_REFRESH(MENU_REFRESH);
}

#endif
