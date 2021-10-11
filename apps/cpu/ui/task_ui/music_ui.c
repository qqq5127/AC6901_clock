#include "sdk_cfg.h"
#include "music_ui.h"
#include "ui/ui_api.h"
#include "dev_manage.h"
#include "audio/audio.h"
#include "lyrics_api.h"
#include "irq_api.h"
#include "fat_io.h"

#if UI_ENABLE

#if LCD_128X64_EN
char g_lfn_buf[512 * 2] __attribute__((aligned(4)));
#endif

#if LRC_LYRICS_EN

SET_INTERRUPT
void ui_lcd_soft_irq_loop()
{
    MUSIC_DIS_VAR *music_ui_api;

    irq_common_handler(IRQ_UI_LCD_IDX);

    music_ui_api = (MUSIC_DIS_VAR *)UI_var.ui_buf_adr;

    if (music_ui_api) {
        music_ui_api->lrc_flag = lrc_get_api(music_ui_api->play_time, 0);
    }

}

static void ui_lcd_soft_irq_resume()
{
    irq_set_pending(IRQ_UI_LCD_IDX);
}
#endif //LRC_LYRICS_EN

#if (LED_7_EN || LED_1888_EN)
void ui_open_music_led(void *buf, u32 len)
{
    puts("ui_open_music_led\n");
    SET_UI_MAIN(MENU_MUSIC_MAIN);
    SET_UI_BUF(buf, len);
#if USE_DISPLAY_WAIT
    UI_DIS_MAIN();
#endif
}
#endif

#if LCD_128X64_EN
void ui_open_music_lcd(void *buf, u32 len)
{
    puts("ui_open_music_lcd\n");

    SET_UI_MAIN(MENU_MUSIC_MAIN);
    SET_UI_BUF(buf, len);
    UI_DIS_MAIN();
}
#endif

void ui_open_music(MUSIC_DIS_VAR *music_ui_api, u16 len)
{
    if (music_ui_api == NULL) {
        return ;
    }
    memset(music_ui_api, 0x00, len);
    ui_music_update_var(music_ui_api);

#if (LED_7_EN || LED_1888_EN)
    if (UI_var.ui_type == UI_LED) {
        ui_open_music_led(music_ui_api, len);
    }
#endif

#if LCD_128X64_EN
    if (UI_var.ui_type == UI_LCD_128X64) {
        ui_open_music_lcd(music_ui_api, len);
        memset(g_lfn_buf, 0, sizeof(g_lfn_buf));
        fat_set_lfn_buf(g_lfn_buf);
#if LRC_LYRICS_EN
        irq_handler_register(IRQ_UI_LCD_IDX, ui_lcd_soft_irq_loop, irq_index_to_prio(IRQ_UI_LCD_IDX));
#endif
    }
#endif
//#if (USE_DISPLAY_WAIT == 0)
//	ui_mode_wait_flag = 0;
//#endif


}

void ui_close_music(MUSIC_DIS_VAR *music_ui_api)
{
    if (music_ui_api == NULL) {
        return;
    }
#if (USE_DISPLAY_WAIT == 0)
	ui_mode_wait_flag = 1;
#endif
    UI_CLEAR_MSG_BUF();
#if USE_DISPLAY_WAIT
    SET_UI_MAIN(MENU_WAIT);
    SET_UI_BUF(NULL, 0);
    UI_DIS_MAIN();
#if LRC_LYRICS_EN
    irq_handler_unregister(IRQ_UI_LCD_IDX);
#endif
#endif
}

extern u8 music_play_get_rpt_mode(void);
void ui_music_update_var(MUSIC_DIS_VAR *music_ui_api)
{
    static u32 curr_file = 0;
    static void *curr_device = NULL;

    MUSIC_PLAYER *obj;
    if (music_ui_api == NULL) {
        return;
    }

    if (music_ui_api->mapi == NULL) {
        music_ui_api->curr_statu = MUSIC_DECODER_ST_STOP;
        return;
    }
    obj = music_ui_api->mapi;

    if (music_ui_api->ui_total_file == 0) {
        curr_device = NULL;
        curr_file = 0;
    }

    music_ui_api->play_mode		 = music_play_get_rpt_mode();
    music_ui_api->eq_mode        = sound.eq_mode;
    music_ui_api->ui_curr_device = music_player_get_cur_dev(obj);
    music_ui_api->play_time      = music_player_get_cur_time(obj);
    music_ui_api->ui_curr_file   = music_player_get_file_number(obj);
    music_curr_file = music_ui_api->ui_curr_file;
    music_ui_api->curr_statu     = music_player_get_status(obj);
    music_ui_api->ui_total_file  = music_player_get_total_file(obj);
    music_total_file = music_ui_api->ui_total_file;

#if LCD_128X64_EN
    music_ui_api->ui_file_info   = music_player_get_file_info();
#if LRC_LYRICS_EN
    ui_lcd_soft_irq_resume();
#endif//LRC_LYRICS_EN
#endif//LCD_128X64_EN

    if (((curr_file != music_ui_api->ui_curr_file) || (curr_device != music_ui_api->ui_curr_device)) && (music_ui_api->ui_total_file != 0)) {
        curr_file = music_ui_api->ui_curr_file;
        curr_device = music_ui_api->ui_curr_device;
#if (USE_DISPLAY_WAIT == 0)
	ui_mode_wait_flag = 0;
#endif
        UI_menu(MENU_FILENUM, 0, 3);
    }
	if (music_ui_api->ui_total_file > 0)
    	UI_REFRESH(MENU_REFRESH);
}
#endif/*UI_ENABLE*/
