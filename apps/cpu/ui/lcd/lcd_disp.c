#include "sdk_cfg.h"
#include "common/common.h"
#include "ui/lcd/lcd_drv_api.h"
#include "ui/lcd/lcd_spi.h"
#include "ui/lcd/lcd_disp.h"
#include "ui/lcd/lcd_drv_interface.h"
#include "uicon/ui_con.h"
#include "uicon/ui.h"
#include "ui/ui_api.h"
#include "music_ui.h"
#include "bt_ui.h"
#include "ui/ui_common.h"
#include "fm_ui.h"
#include "timer.h"
#include "key_drv/key.h"
#include "file_operate/file_op.h"
#include "dac.h"
#include "rtc_setting.h"
#include "fmtx_api.h"
#include "eq.h"
#include "task_manager.h"
#include "power.h"
#include "task_fm.h"
#include "lyrics_api.h"
#include "encode/encode.h"
#include "rec_api.h"

#define MUSIC_UI_FILENAME_TEXTSIZE  14
#define MUSIC_UI_FOLDER_TEXTSIZE    14


#if  LCD_128X64_EN

u16 music_ui_filename_pt;
u16 music_ui_foldername_pt;

static const _lcd_area_size_t time_set_backgroup[] = {
    //left,top,right,bottom

    //DVcTime1_5
    {19, 3, 55, 4},
    {64, 3, 82, 4},
    {91, 3, 109, 4},

    //DVcTime2_5
    {41, 5, 59, 6},
    {68, 5, 86, 6},

    //DVcRzImg1_5
    {90, 5, 106, 6},
};

static const GUI_POINT  del_pic_point = {0, 32};
static const GUI_POINT  pause_pic_point = {16, 32};
static const GUI_POINT  vol_pic_point = {0, 0};
static const GUI_POINT  mute_pic_point = {112, 0};
//static const GUI_POINT  echo_pic_point = {80,0};
static const GUI_POINT  eq_pic_point = {110, 0};
static const GUI_POINT  playmode_pic_point = {96, 0};
static const GUI_POINT  rec_opt_pic_point = {32, 48};

static const u8 play_mode_pic[] = {DVcRzImg1_6, DVcRzImg15_6, DVcRzImg3_6, DVcRzImg4_6, DVcRzImg2_6};
static const u8 eq_mode_pic[] = {DVcRzImg5_6, DVcRzImg6_6, DVcRzImg7_6, DVcRzImg8_6, DVcRzImg9_6, DVcRzImg10_6};
static const u8 rec_opt_pic[] = {DVcRzImg4_8, DVcRzImg3_8, DVcRzImg2_8, DVcRzImg4_8};
static const u8 mode_pic[] = {DVcRzImg4_3, DVcRzImg5_3, DVcRzImg6_3}; //SD/U/bt

///------------common-api--------------
void lcd_setX(u8 X)
{

}

int unicode_len(u8 *str)
{
    u8 *src = str;
    while ((*str != 0) || (*(str + 1) != 0)) {
        str += 2;
    }
    return (int)(str - src);
}

void lcd_set_cur_menu(u8 menu)
{
    UI_var.bMenuReturnCnt = 0;
    UI_var.bCurMenu = menu;
}

u16 lcd_disp_text2(const char *str, u8 dvtxt_num, u8 flags)
{
    DTEXT disp_txt;
    u16 show_bytes;

    disp_txt.buf = (u8 *)str;

    disp_txt.flags = flags;

    if ((flags & ANSI) != 0) {
        disp_txt.len = strlen((const char *)disp_txt.buf);
    } else {
        disp_txt.len = unicode_len((u8 *)disp_txt.buf);
        //printf("\ndisp_unicode:%d\n",disp_txt.len);
        //printf_buf(disp_txt.buf,disp_txt.len);
    }

//    puts("disp_buf:\n");
//    printf_buf(str,disp_txt.len);

    show_bytes = ui_text(dvtxt_num, &disp_txt);

    return show_bytes;
}


void lcd_disp_text(const char *str, u8 dvtxt_num)
{

    lcd_disp_text2(str, dvtxt_num, ANSI);

    /*
        DTEXT disp_txt;

        disp_txt.buf = (u8 *)str;

        disp_txt.flags = ANSI;
        disp_txt.len = strlen((const char *)disp_txt.buf);
        ui_text(dvtxt_num, &disp_txt);
    */
}

void lcd_disp_string_menu(u8 menu)
{
    const char lcd_string_Chinese[][17] = {
        "     您好!   ",
        "    加载中... ",
    };

    const char dev_err_string_Chinese[][17] = {
        "     设备     ",
        "   不存在!!!  ",
    };

#if RTC_ALM_EN
    if (menu == MENU_ALM_UP) {
        lcd_alarm_ring();
        return;
    }
#endif

    if (menu == MENU_DEV_ERR) {
        lcd_disp_text(dev_err_string_Chinese[0], DVcTxt2_1);
        lcd_disp_text(dev_err_string_Chinese[1], DVcTxt3_1);
        return;
    }

    lcd_disp_text(lcd_string_Chinese[menu], DVcTxt1_2);
}

void lcd_null_dis(void)
{

}

int lcd_check_sys_vol(void)//静音显示
{
#if 1
    if ((*(UI_var.sys_vol) == 0) || (is_dac_mute())) {
        ui_pic(DVcRzImg1_3, (void *)&mute_pic_point);
        return 1;
    } else {
        return 0;
    }
#endif
}

void  lcd_check_echo_sw(void)//混响显示
{
#if 0
    if (lcd_check_sys_vol()) {
        return;
    }

#if ECHO_EN
    if (*(UI_var.sys_vol) != 0) {
        if ((UI_var.echo_pt != NULL) && (UI_var.echo_pt[0] != 0)) {
            ui_pic(DVcRzImg4_3, (void *)&vol_pic_point);
        }
    }
#endif
#endif
}

void lcd_disp_freq(u16 freq, u8 font_size) //1号字体为16*32大字体，2号字体为8*16字体
{
    char lcd_freq_str[9] = {"     MHz"};
    u32 freq_temp;

    if (font_size == 2) {
        itoa4(freq);
        if (freq <= 999) {
            lcd_freq_str[0] = bcd_number[1];
            lcd_freq_str[1] = bcd_number[2];
            lcd_freq_str[2] = '.';
            lcd_freq_str[3] = bcd_number[3];
        } else {
            lcd_freq_str[0] = bcd_number[0];
            lcd_freq_str[1] = bcd_number[1];
            lcd_freq_str[2] = bcd_number[2];
            lcd_freq_str[3] = '.';
            lcd_freq_str[4] = bcd_number[3];
        }
        lcd_disp_text(lcd_freq_str, DVcTxt1_3);
        ui_pic(DVcRzImg7_3, 0);
    } else if (font_size == 1) {
        ui_pic(DVcRzImg1_4, 0);

        if (freq <= 999) {
            freq_temp = freq % 1000 / 10;
        } else {
            freq_temp = freq / 10;
        }
        ui_number(DVcNumber1_4, (u32)freq_temp, -1, -1, 1);

        freq_temp = freq % 10;
        ui_number(DVcNumber2_4, (u32)freq_temp, -1, -1, 1);
    }
}

void lcd_disp_rec_error(void)
{
#if REC_EN
    const char lcd_string[] = {"  REC error!!!  "};
    lcd_clear();
    lcd_disp_text(lcd_string, DVcTxt1_2);
    lcd_set_cur_menu(MENU_RECODE_ERR);
#endif
}

void  lcd_disp_rec_opt_state(RECORD_OP_API *rec_api)
{
#if REC_EN
    u32 rec_sta = 0;
    rec_sta = rec_get_enc_sta(rec_api);
    ui_pic(DVcRzImg1_8, 0);
    ui_pic(rec_opt_pic[rec_sta], (GUI_POINT *)&rec_opt_pic_point);
#endif
}

///------------main-display--------------
void lcd_disp_welcome(void)
{
    const char lcd_string[] = {"     HELLO      "};
    lcd_disp_text(lcd_string, DVcTxt1_2);
}

void lcd_disp_IR_number(s32 input_number)
{
    u8 i, input_cnt;

    input_number &= 0xffff;
    //input_cnt = (input_number>>24);
    //printf("ir-input: %d,%d\n",input_number,input_cnt);

    ui_number(DVcNumber1_2, input_number, -1, -1, 0);
}

void lcd_disp_vol(s32 vol)
{
    ui_pic(DVcRzImg5_8, 0);
    ui_number(DVcNumber1_8, vol, -1, -1, 0);
}

void lcd_disp_power(void)
{
    u32 voltage;
    ui_pic(DVcRzImg1_5, 0);
    voltage = get_power_external_value() / 10;
    ui_number(DVcNumber1_5, (u32)voltage, -1, -1, 1);
    voltage = get_power_external_value() % 10;
    ui_number(DVcNumber2_5, (u32)voltage, -1, -1, 1);
}


void lcd_disp_dev(void *dev, u8 dvtxt_num)
{
    char lcd_dev_str[][4] = {
        "SD0",
        "SD1",
        "USB",
        "NON",
    };
    if (dev == sd0) {
        lcd_disp_text(lcd_dev_str[0], dvtxt_num);
    } else if (dev == sd1) {
        lcd_disp_text(lcd_dev_str[1], dvtxt_num);
    } else if (dev == usb) {
        lcd_disp_text(lcd_dev_str[2], dvtxt_num);
    } else {
        lcd_disp_text(lcd_dev_str[3], dvtxt_num);
    }


}

void lcd_disp_eq(s32 eq_mode)
{
    char eq_str[][17] = {
        "     NORMAL",
        "      ROCK",
        "       POP",
        "     CLASSIC",
        "      JAZZ",
        "     COUNTRY",
    };

    lcd_disp_text((char *)eq_str[eq_mode], DVcTxt3_1);
}

void lcd_disp_playmode(s32 play_mode)
{
    char playmode_str[][17] = {
        "       ALL",
        "    ONE LGDEV",
        "       ONE",
        "      RANDOM",
        "      FOLDER",
    };

    lcd_disp_text((char *)playmode_str[play_mode - REPEAT_ALL], DVcTxt3_1);
}

void lcd_disp_filenumber(void)
{
    MUSIC_DIS_VAR *music_var;

    music_var = (MUSIC_DIS_VAR *)UI_var.ui_buf_adr;

    if (music_var) {
        ui_number(DVcNumber1_2, music_var->ui_curr_file, -1, -1, 0);
    }
}

///-------------app-display---------------

//返回显示字符偏移位置
int text_scroll_deal(u16 disp_len, u16 text_size, int pt)
{
    if ((disp_len <= text_size) || (disp_len < pt)) {
        return 0;
    }

    disp_len = disp_len - pt;

    if (text_size >= disp_len) {
        return 0;
    }

    return pt + 1;

}

int disp_unicode_len(u8 *str)
{
    int char_cnt = 0;
    u8 *src = str;
    while ((*str != 0) || (*(str + 1) != 0)) {
        if (*(str + 1) == 0) {
            char_cnt++;
        }
        str += 2;
    }
    return (int)(str - src) - char_cnt;
}

///------------music-display--------------
void lcd_music_main(void)
{
    char lcd_file_num[] = {" 0000/0000"};
    char lcd_playtime_str[] = {"00:00"};
    u8 flags;
    u32 tmp;
    u8 *point;

    const char lcd_string_Chinese[] = {
        "    加载中... ",
    };


    MUSIC_DIS_VAR *music_var;

    music_var = (MUSIC_DIS_VAR *)UI_var.ui_buf_adr;

    if (music_var) {
        if (music_var->ui_total_file == 0) {
            lcd_disp_text(lcd_string_Chinese, DVcTxt1_2);
            return;
        }


#if FMTX_EN
        lcd_disp_freq(fmtx_get_freq(), 2);
#endif

        if ((music_var->ui_curr_device == sd0) || (music_var->ui_curr_device == sd1)) {
            ui_pic(mode_pic[0], NULL);
        } else if (music_var->ui_curr_device == usb) {
            ui_pic(mode_pic[1], NULL);
        }
        ui_pic(eq_mode_pic[music_var->eq_mode], (GUI_POINT *)&eq_pic_point);
        ui_pic(play_mode_pic[music_var->play_mode - REPEAT_ALL], (GUI_POINT *)&playmode_pic_point);

        if (music_var->ui_file_info->update_flag != 0) {
            //start play music,init parm
            music_ui_filename_pt = 0;
            music_ui_foldername_pt = 0;
            music_var->ui_file_info->update_flag = 0;
            //puts("file_ui_update\n");
        }
//显示文件名
        point = (u8 *)music_var->ui_file_info->file_name.lfn;
        tmp = music_var->ui_file_info->file_name.lfn_cnt;
        if (tmp > 0) {
            flags = UNICODE_LE;

            if (UI_var.param_menu == MENU_REFRESH) {
                /* if (1) { */
                //半秒才刷
                if (disp_unicode_len(point) > MUSIC_UI_FILENAME_TEXTSIZE) {
                    music_ui_filename_pt = text_scroll_deal(tmp, MUSIC_UI_FILENAME_TEXTSIZE, music_ui_filename_pt);
                }
            } else {
                /* music_ui_filename_pt = 0; */
            }

        } else {
            flags = ANSI;
        }
        ui_pic(DVcRzImg3_3, 0); //folder
        lcd_disp_text2((const char *)point + (music_ui_filename_pt & 0xfffe), DVcTxt2_3, flags); //文件名

//FF
        if ((music_var->opt_state & MUSIC_OPT_BIT_FF) != 0) {
            ui_pic(DVcRzImg2_7, (GUI_POINT *)&pause_pic_point);
            return;
        }

//FR
        if ((music_var->opt_state & MUSIC_OPT_BIT_FR) != 0) {
            ui_pic(DVcRzImg1_7, (GUI_POINT *)&pause_pic_point);
            return;
        }

//del file
        if ((music_var->opt_state & MUSIC_OPT_BIT_DEL) != 0) {
            music_var->opt_state &= ~MUSIC_OPT_BIT_DEL;
            ui_pic(DVcRzImg4_7, (GUI_POINT *)&del_pic_point);
            return;
        }

//LRC
#if LRC_LYRICS_EN
        if ((MUSIC_DECODER_ST_PLAY == music_var->curr_statu)) {
            if (music_var->lrc_flag == true) {
                if (lrc_show_api(music_var->play_time, 0)) {
                    return;
                }
            }
        }
#endif

        if ((MUSIC_PLAYRR_ST_PAUSE == music_var->curr_statu)) {
            ui_pic(DVcRzImg3_7, (GUI_POINT *)&pause_pic_point);
            return;
        }

        itoa2_api(music_var->play_time / 60, (u8 *)&lcd_playtime_str[0]);
        itoa2_api(music_var->play_time % 60, (u8 *)&lcd_playtime_str[3]);
#if  POWER_EXTERN_DETECT_EN
        lcd_disp_text(lcd_playtime_str, DVcTxt1_5); //文件
#else
        lcd_disp_text(lcd_playtime_str, DVcTxt3_3); //文件
#endif

        itoa4_api(music_var->ui_curr_file, (u8 *)&lcd_file_num[1]);
        itoa4_api(music_var->ui_total_file, (u8 *)&lcd_file_num[6]);
#if POWER_EXTERN_DETECT_EN
        lcd_disp_text(lcd_file_num, DVcTxt2_5); //目录
#else
        lcd_disp_text(lcd_file_num, DVcTxt4_3); //目录
#endif

#if POWER_EXTERN_DETECT_EN
        lcd_disp_power();
#endif
#if 1
        //显示目录文件夹
        point = (u8 *)music_var->ui_file_info->dir_name.lfn;
        tmp = music_var->ui_file_info->dir_name.lfn_cnt;
        if (tmp > 0) {
            flags = UNICODE_LE;
            if (UI_var.param_menu == MENU_REFRESH) {
                /* if (1) { */
                //半秒才刷
                if (disp_unicode_len(point) > MUSIC_UI_FOLDER_TEXTSIZE) {
                    music_ui_foldername_pt = text_scroll_deal(tmp, MUSIC_UI_FOLDER_TEXTSIZE, music_ui_foldername_pt);
                }
            } else {
                /* music_ui_foldername_pt = 0; */
            }

        } else {
            flags = ANSI;
        }

        ui_pic(DVcRzImg2_3, 0); //folder
        lcd_disp_text2((const char *)point + (music_ui_foldername_pt & 0xfffe), DVcTxt5_3, flags); //文件夹
#endif

    }
}


///------------bt-display--------------
void lcd_bt_main(u8 menu)
{
    BT_DIS_VAR *bt_var;
    GUI_POINT main_pic_point;
    char *lcd_string;

    bt_var = (BT_DIS_VAR *)UI_var.ui_buf_adr;

    const char lcd_string_Chinese[][20] = {
        "   蓝牙未连接",
        "   蓝牙已连接",
        "    蓝牙音乐",
        "    蓝牙通话",
    };

    if (bt_var) {

        ui_pic(mode_pic[2], NULL);
        ui_pic(eq_mode_pic[bt_var->bt_eq_mode], (GUI_POINT *)&eq_pic_point);


#if FMTX_EN
        lcd_disp_freq(fmtx_get_freq(), 2);
#endif

        if ((BT_STATUS_CONNECTING == bt_var->ui_bt_connect_sta) ||
            (BT_STATUS_TAKEING_PHONE == bt_var->ui_bt_connect_sta) ||
            (BT_STATUS_PLAYING_MUSIC == bt_var->ui_bt_connect_sta)) {
#if  POWER_EXTERN_DETECT_EN
            ui_pic(DVcRzImg2_2, 0);
#else
            main_pic_point.x = (128 - 60) / 2;
            main_pic_point.y = 32;
            ui_pic(DVcRzImg2_2, &main_pic_point);
#endif
        } else {
#if  POWER_EXTERN_DETECT_EN
            ui_pic(DVcRzImg1_2, 0);
#else
            main_pic_point.x = (128 - 42) / 2;
            main_pic_point.y = 32;
            ui_pic(DVcRzImg1_2, &main_pic_point);
#endif
        }

        if (BT_STATUS_CONNECTING == bt_var->ui_bt_connect_sta) {
            lcd_string = (char *)lcd_string_Chinese[1];
        } else if (BT_STATUS_PLAYING_MUSIC == bt_var->ui_bt_connect_sta) {
            lcd_string = (char *)lcd_string_Chinese[2];
        } else if (BT_STATUS_TAKEING_PHONE == bt_var->ui_bt_connect_sta) {
            lcd_string = (char *)lcd_string_Chinese[3];
        } else {
            lcd_string = (char *)lcd_string_Chinese[0];
        }

        lcd_disp_text(lcd_string, DVcTxt2_1);

#if POWER_EXTERN_DETECT_EN
        lcd_disp_power();
#endif

    }
}

///------------pc-display--------------
void lcd_pc_main(u8 menu)
{
#if USB_PC_EN
    const char lcd_string[] = {" PC "};
    lcd_disp_text(lcd_string, DVcTxt1_1);
    ui_pic(DVcRzImg1_1, 0);
#endif
}

void lcd_echo_main(u8 menu)
{
#if ECHO_EN
    const char lcd_string[] = {"ECHO"};
    lcd_disp_text(lcd_string, DVcTxt1_1);
#endif
}

///------------fm-display--------------
extern u8 get_sys_halfsec(void);
void lcd_fmtx_main(void)
{
    if ((fmtx_get_whether_to_flash_freq() == 1) && get_sys_halfsec()) {

    } else {
        lcd_disp_freq(fmtx_get_freq(), 1);
    }
}

void lcd_fm_main(void)
{
#if FM_RADIO_EN
    const char lcd_string[] = {"      收音"};
    char lcd_freq_str[] = {"        87.5MHz "};

    lcd_disp_text(lcd_string, DVcTxt1_1);
    ui_pic(DVcRzImg7_3, 0);

    FM_MODE_VAR *fm_var;

    fm_var = *(FM_MODE_VAR **)UI_var.ui_buf_adr;

    if (fm_var) {

        if (fm_var->wFreChannel > 0) {
            itoa2((u8)fm_var->wFreChannel);
            lcd_freq_str[3] = 'C';
            lcd_freq_str[4] = bcd_number[0];
            lcd_freq_str[5] = bcd_number[1];
        }

        itoa4(fm_var->wFreq);

        if (fm_var->wFreq <= 999) {
            bcd_number[0] = ' ';
        }
        lcd_freq_str[7] = bcd_number[0];
        lcd_freq_str[8] = bcd_number[1];
        lcd_freq_str[9] = bcd_number[2];
        lcd_freq_str[11] = bcd_number[3];

        lcd_disp_text(lcd_freq_str, DVcTxt3_1);

        if (fm_mode_var && ((fm_mode_var->scan_mode >= FM_SCAN_BUSY) || (fm_mode_var->fm_mute))) { ///FM正在搜台，只响应部分按键 //MUTE
            /* printf("no_show_echo %d %d\n",fm_mode_var->scan_mode, fm_mode_var->fm_mute); */
        } else {
            /* printf("show_echo\n"); */
            /* lcd_check_echo_sw(); */
        }
    }
#endif
}

void lcd_fm_channel(void)
{

}

///------------rtc-display--------------
void lcd_rtc_main(void)
{
#if RTC_CLK_EN
    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time_var;

    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var == 0) {
        return;
    }

    if (rtc_var->rtc_set_mode == RTC_SET_MODE) {
        lcd_rtc_set();
        return;
    } else {
        if (rtc_var->rtc_set_mode == ALM_SET_MODE) {
            //超时后界面也要显示闹钟设置，让上层来决定什么时候更新显示
            lcd_alarm_set();
            return;
        }
    }

    const char lcd_string[] = {"时钟"};
    lcd_disp_text(lcd_string, DVcTxt1_1);

    if (rtc_var) {
        ui_time_var = rtc_var->calendar_set.curr_rtc_time;
        ui_time(DVcTime2_12, (TIME *)ui_time_var, -1, -1, 0);
        ui_time(DVcTime1_12, (TIME *)ui_time_var, -1, -1, 0);
    }
#endif
}

///------------rec-display--------------
void lcd_rec_main(void)
{
#if REC_EN
    RECORD_OP_API *rec_var_p;
    ENC_CTL *enc_ctl_var;
    u32 rec_time;

//    TIME time_tmp;
    char lcd_time_str[] = {"     00:00:00  "};
    const char lcd_string[] = {"录音"};
    char rec_nodevice_str[] = {"   no device!!!"};
    const char lcd_sta_str[][17] = {
        "    ENC_STOP    ",
        "    ENC_START   ",
        "    ENC_PAUSE   ",
    };

    lcd_disp_text(lcd_string, DVcTxt1_1);

    if (UI_var.ui_buf_adr) {
        rec_var_p = *(RECORD_OP_API **)UI_var.ui_buf_adr;
        if (rec_var_p) {
            enc_ctl_var = rec_var_p->enc_ctl;
            if (enc_ctl_var) {
                lcd_disp_text((const char *)(lcd_sta_str[enc_ctl_var->enable]), DVcTxt2_1);

                rec_time = enc_ctl_var->file_info.enc_time_cnt;

                if (enc_ctl_var->enable != ENC_STOP) {
                    lcd_disp_dev(rec_get_cur_dev(rec_var_p), DVcTxt3_1);
                    /* printf("rec_time = %d\n",rec_time); */
                    itoa2_api(rec_time / 3600, (u8 *)&lcd_time_str[5]);
                    itoa2_api(rec_time % 3600 / 60, (u8 *)&lcd_time_str[8]);
                    itoa2_api(rec_time % 60, (u8 *)&lcd_time_str[11]);
                    /* printf("\n%s\n",lcd_time_str); */
                    lcd_disp_text((const char *)(lcd_time_str), DVcTxt3_1);//TIME
                }
                //return;
            }
        } else {
            lcd_disp_text((const char *)(lcd_sta_str[ENC_STOP]), DVcTxt2_1);//STOP
            if (dev_get_phydev_total(MUSIC_DEV_TYPE, DEV_ONLINE) == 0) {
                lcd_disp_text(rec_nodevice_str, DVcTxt3_1);
                return;
            }
        }

        lcd_disp_rec_opt_state(rec_var_p);
    }

#endif

}

///------------aux-display--------------
void lcd_aux_main(u8 menu)
{
    const char lcd_string_Chinese[] = {"      AUX"};
    /* GUI_POINT main_pic_point; */
    RECORD_OP_API *rec_var_p;

    rec_var_p = *(RECORD_OP_API **)UI_var.ui_buf_adr;

#if FMTX_EN
    lcd_disp_freq(fmtx_get_freq(), 2);
#else
    lcd_disp_text(lcd_string_Chinese, DVcTxt1_1);
#endif

#if POWER_EXTERN_DETECT_EN
    ui_pic(DVcRzImg2_5, 0);
    lcd_disp_power();
#else
    /* main_pic_point.x = (128 - 80) / 2; */
    /* main_pic_point.y = 32; */
    /* ui_pic(DVcRzImg2_5, &main_pic_point); */
    ui_pic(DVcRzImg2_1, 0);
#endif

    if (rec_var_p) {
        lcd_disp_rec_opt_state(rec_var_p);
    }
}


///------------rtc,alarm set-display--------------
void lcd_rtc_set(void)
{
#if RTC_CLK_EN
    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time_var;
    _lcd_area_size_t *area_pt;
    const char lcd_string[] = {"时钟设置"};

//    puts("lcd_rtc_set start\n");
    lcd_disp_text(lcd_string, DVcTxt1_1);
    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var) {
        ui_time_var = rtc_var->calendar_set.curr_rtc_time;
        ui_time(DVcTime2_13, (TIME *)ui_time_var, -1, -1, 0);
        ui_time(DVcTime1_13, (TIME *)ui_time_var, -1, -1, 0);

        if (rtc_var->calendar_set.coordinate <= COORDINATE_MAX) {
            area_pt = (_lcd_area_size_t *)&time_set_backgroup[rtc_var->calendar_set.coordinate];
            //        printf("coordinate,%d\n",rtc_var->calendar_set.coordinate);
            lcd_TurnPixelReverse_Rect(area_pt->left, area_pt->top, area_pt->right, area_pt->bottom);
        }
    }
//    puts("lcd_rtc_set end\n");
#endif

}

///------------rtc,alarm set-display--------------
void lcd_alarm_set(void)
{

#if RTC_ALM_EN

    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time_var;
    const char lcd_string[] = {"闹钟设置"};

//    puts("lcd_alarm_set start\n");

    lcd_disp_text(lcd_string, DVcTxt1_1);
    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var) {
        ui_time_var = rtc_var->alarm_set.curr_alm_time;
        ui_time(DVcTime2_13, (TIME *)ui_time_var, -1, -1, 0);
        ui_time(DVcTime1_13, (TIME *)ui_time_var, -1, -1, 0);


        if (rtc_var->alarm_set.alarm_sw != 0) {
            ui_pic(DVcRzImg1_13, 0);
        }

        if (rtc_var->alarm_set.coordinate <= COORDINATE_MAX) {
            _lcd_area_size_t *area_pt = (void *)&time_set_backgroup[rtc_var->alarm_set.coordinate];
            //        printf("coordinate,%d\n",rtc_var->alarm_set.coordinate);
            lcd_TurnPixelReverse_Rect(area_pt->left, area_pt->top, area_pt->right, area_pt->bottom);
        }
    }
//    puts("lcd_alarm_set end\n");
#endif

}


///------------rtc,alarm set-display--------------
void lcd_alarm_ring(void)
{
#if RTC_ALM_EN
    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time_var;
    const char lcd_string[] = {"闹钟响铃"};
    const GUI_POINT  ring_pic_point = {21, 40};

    //puts("lcd_alarm ring start\n");

    lcd_disp_text(lcd_string, DVcTxt1_1);
    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var) {
        ui_time_var = rtc_var->alarm_set.curr_alm_time;
        //printf("ui_alarm:%d-%d-%d,%d:%d:%d\n",ui_time_var->dYear,ui_time_var->bMonth,ui_time_var->bDay,\
        //ui_time_var->bHour,ui_time_var->bMin,ui_time_var->bSec);

        ui_time(DVcTime1_13, (TIME *)ui_time_var, -1, -1, 0);
        ui_pic(DVcRzImg1_13, 0);
        ui_pic(DVcRzImg1_13, (GUI_POINT *)&ring_pic_point);
        lcd_check_sys_vol();
    }
#endif
}


#endif

