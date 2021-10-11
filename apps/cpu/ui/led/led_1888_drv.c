#include "ui/ui_api.h"

#if LED_1888_EN

#include "ui/ui_common.h"
#include "ui/led/led_1888_drv.h"
#include "task_fm.h"
#include "music_ui.h"
#include "timer.h"
#include "key_drv/key.h"
#include "file_operate/file_op.h"
#include "rtc_setting.h"
#include "common/common.h"
#include "fmtx_api.h"
#include "bluetooth/avctp_user.h"
#include "led.h"
#include "power.h"
#include "audio/tone.h"

#if REC_EN
#include "encode/encode.h"
#endif

extern u8 get_sys_halfsec(void);
led1888_VAR led1888_var;

const  u8 LED_NUMBER[10] = {
    /*0*/
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F),
    /*1*/
    (u8)(LED_B | LED_C),
    /*2*/
    (u8)(LED_A | LED_B | LED_D | LED_E | LED_G),
    /*3*/
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_G),
    /*4*/
    (u8)(LED_B | LED_C | LED_F | LED_G),
    /*5*/
    (u8)(LED_A | LED_C | LED_D | LED_F | LED_G),
    /*6*/
    (u8)(LED_A | LED_C | LED_D | LED_E | LED_F | LED_G),
    /*7*/
    (u8)(LED_A | LED_B | LED_C),
    /*8*/
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G),
    /*9*/
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_F | LED_G),
};

const  u8 LED_LARGE_LETTER[26] = {
    0x77, 0x40, 0x39, 0x3f, 0x79, ///<ABCDE
    0x71, 0x40, 0x76, 0x06, 0x40, ///<FGHIJ
    0x40, 0x38, 0x40, 0x37, 0x3f, ///<KLMNO
    0x73, 0x40, 0x50, 0x6d, 0x78, ///<PQRST
    0x3e, 0x3e, 0x40, 0x76, 0x40, ///<UVWXY
    0x40///<Z
};

const  u8 LED_SMALL_LETTER[26] = {
    0x77, 0x7c, 0x58, 0x5e, 0x79, ///<abcde
    0x71, 0x40, 0x40, 0x06, 0x40, ///<fghij
    0x40, 0x38, 0x40, 0x54, 0x5c, ///<klmno
    0x73, 0x67, 0x50, 0x40, 0x78, ///<pqrst
    0x3e, 0x3e, 0x40, 0x40, 0x40, ///<uvwxy
    0x40///<z
};


#define NIL  7
#if 0
//实捷6脚屏188.8
const u8 led_1888[36][2] = {
    {NIL, NIL}, //1A
    {2, 3}, //1B
    {2, 4}, //1C
    {NIL, NIL}, //1D
    {NIL, NIL}, //1E
    {NIL, NIL}, //1F
    {NIL, NIL}, //1G

    {0, 1}, //2A
    {0, 2}, //2B
    {0, 3}, //2C
    {0, 4}, //2D
    {0, 5}, //2E
    {1, 2}, //2F
    {1, 3}, //2G

    {1, 0}, //3A
    {2, 0}, //3B
    {3, 0}, //3C
    {4, 0}, //3D
    {5, 0}, //3E
    {1, 4}, //3F
    {1, 5}, //3G

    {2, 1}, //4A
    {3, 1}, //4B
    {4, 1}, //4C
    {5, 1}, //4D
    {3, 2}, //4E
    {4, 2}, //4F
    {5, 2}, //4G

    {NIL, NIL}, //LED_PLAY
    {NIL, NIL}, //LED_PAUSE
    {NIL, NIL}, //LED_USB
    {NIL, NIL}, //SD
    {NIL, NIL}, //:
    {NIL, NIL}, //MHz
    {2, 5}, //.
    {NIL, NIL}, //MP3
};
#endif
const u8 led_1888[36][2] = {
    {NIL, NIL}, //1A
    {3, 2}, //1B
    {4, 2}, //1C
    {NIL, NIL}, //1D
    {NIL, NIL}, //1E
    {NIL, NIL}, //1F
    {NIL, NIL}, //1G

    {1, 0}, //2A
    {2, 0}, //2B
    {3, 0}, //2C
    {4, 0}, //2D
    {5, 0}, //2E
    {2, 1}, //2F
    {3, 1}, //2G

    {0, 1}, //3A
    {0, 2}, //3B
    {0, 3}, //3C
    {0, 4}, //3D
    {0, 5}, //3E
    {4, 1}, //3F
    {5, 1}, //3G

    {1, 2}, //4A
    {1, 3}, //4B
    {1, 4}, //4C
    {1, 5}, //4D
    {2, 3}, //4E
    {2, 4}, //4F
    {2, 5}, //4G

    {NIL, NIL}, //LED_PLAY
    {NIL, NIL}, //LED_PAUSE
    {NIL, NIL}, //LED_USB
    {NIL, NIL}, //SD
    {NIL, NIL}, //:
    {NIL, NIL}, //MHz
    {5, 2}, //.
    {NIL, NIL}, //MP3
};

//按位于 查表
const u8 bit_table[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
};

const u8 playmodestr[][5] = {
    " ALL",
    " dev",
    " ONE",
    " rAn",
    " Fol",
};

const u8 menu_string[][5] = {
    " HI ",
    " Lod",
    " bt ",
    " PC ",
    " UP ",
    " dn ",
    " AUX",
    " AL "
};
const u8 other_string[][5] = {
    " Eq",
    " V",
    " P",
    " NOP",
    " rec",
};

void led1888_flash(void)
{
    led1888_var.bFlashChar = 0x1F;
}

/*----------------------------------------------------------------------------*/
/**@brief   Music模式 设备显示
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_dev(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_dev(void *dev)
{
    {
        if ((dev == sd0) || (dev == sd1)) {
            LED_STATUS |= LED_SD;
        } else if (dev == usb) {
            LED_STATUS |= LED_USB;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   led1888_drv 状态位缓存清除函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_clear_icon(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_clear_icon(void)
{
    led1888_var.bFlashChar = 0;
    led1888_var.bFlashIcon = 0;
    memset(led1888_var.bShowBuff, 0x00, 5);
    /* led1888_var.bShowBuff[4] = 0; */
}



/*----------------------------------------------------------------------------*/
/**@brief   led1888_drv 显示坐标设置
   @param   x：显示横坐标
   @return  void
   @author  Change.tsai
   @note    void led1888_setX(u8 X)
*/
/*----------------------------------------------------------------------------*/
void led1888_setX(u8 X)
{
    led1888_var.bCoordinateX = X;
}

/*----------------------------------------------------------------------------*/
/**@brief   LED 清屏函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_null(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_null(void)
{
    led1888_clear_icon();
    led1888_var.bShowBuff[0] = 0;
    led1888_var.bShowBuff[1] = 0;
    led1888_var.bShowBuff[2] = 0;
    led1888_var.bShowBuff[3] = 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   led1888_drv 扫描函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_init(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_init(void)
{
    led1888_clear();
}

/*----------------------------------------------------------------------------*/
/**@brief   led1888_drv 单个字符显示函数
   @param   chardata：显示字符
   @return  void
   @author  Change.tsai
   @note    void led1888_show_char(u8 chardata)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_char(u8 chardata)
{
    //必须保证传入的参数符合范围，程序不作判断
    //if ((chardata < ' ') || (chardata > '~') || (led1888_var.bCoordinateX > 4))
    //{
    //    return;
    //}
    if ((chardata >= '0') && (chardata <= '9')) {
        led1888_var.bShowBuff[led1888_var.bCoordinateX++] = LED_NUMBER[chardata - '0'];
    } else if ((chardata >= 'a') && (chardata <= 'z')) {
        led1888_var.bShowBuff[led1888_var.bCoordinateX++] = LED_SMALL_LETTER[chardata - 'a'];
    } else if ((chardata >= 'A') && (chardata <= 'Z')) {
        led1888_var.bShowBuff[led1888_var.bCoordinateX++] = LED_LARGE_LETTER[chardata - 'A'];
    } else if (chardata == ':') {
        LED_STATUS |= LED_2POINT;
    } else if (chardata == ' ') {
        led1888_var.bShowBuff[led1888_var.bCoordinateX++] = 0;
    } else { //if (chardata == '-')     //不可显示
        led1888_var.bShowBuff[led1888_var.bCoordinateX++] = BIT(6);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   led1888_drv 字符串显示函数
   @param   *str：字符串的指针   offset：显示偏移量
   @return  void
   @author  Change.tsai
   @note    void led1888_show_string(u8 *str)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_string(u8 *str)
{
    while ('\0' != *str) {
        led1888_show_char(*str++);
    }
}

/*----------------------------------------------------------------------------*/
/** @brief:
    @param:
    @return:
    @author:
    @note:
*/
/*----------------------------------------------------------------------------*/
void led1888_show_string_menu(u8 menu)
{
    if (menu >= (sizeof(menu_string) / 5)) {
        printf("*strid(%d) is over!\n", menu);
    } else {
        led1888_show_string((u8 *)menu_string[menu]);
    }
}

void led1888_show_power(void)
{
    u16  temp;

    temp = get_power_external_value();

    itoa4(temp);
    if (temp <= 99) {
        bcd_number[0] = ' ';
        bcd_number[1] = ' ';
        led1888_show_string((u8 *)bcd_number);
        LED_STATUS |= LED_DOT;
    } else if (temp <= 365) {
        bcd_number[0] = ' ';
        led1888_show_string((u8 *)bcd_number);
        LED_STATUS |= LED_DOT;
    }
#if 0
    if (get_low_power_external_flag()) {
        if (get_sys_halfsec()) {
            return;
        }
        sin_tone_play(250);
    }
#endif
}

void led1888_show_linin_main(u8 menu)
{
    //led1888_show_string_menu(menu);
    /* led1888_clear_icon(); */

    itoa4(fmtx_get_freq());

    if (fmtx_get_freq() <= 999) {
        bcd_number[0] = ' ';
    }

    led1888_show_string((u8 *)bcd_number);
    LED_STATUS |= LED_DOT;

#if REC_EN

    RECORD_OP_API *rec_var_p;
    REC_CTL *rec_ctl_var;

    rec_var_p = *(RECORD_OP_API **)UI_var.ui_buf_adr;
    if ((UI_var.ui_buf_adr) && (rec_var_p)) {
        rec_ctl_var = rec_var_p->rec_ctl;

        LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
        LED_STATUS &= ~(LED_SD | LED_USB);

        if ((menu == MENU_AUX_MAIN) || (menu == MENU_BT_MAIN)) {
            if ((rec_ctl_var) && (ENC_STOP != rec_ctl_var->enable)) {
                if (ENC_STAR == rec_ctl_var->enable) {
                    LED_STATUS |= LED_PLAY;
                } else if (ENC_PAUS == rec_ctl_var->enable) {
                    LED_STATUS |= LED_PAUSE;
                }

                led1888_show_dev(rec_ctl_var->curr_device);
            }
        }
    }
#endif
}
/*----------------------------------------------------------------------------*/
/**@brief   蓝牙 模式主界面
   @param   void
   @return  void
   @author
   @note    void led1888_show_bt_main(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_bt_main(u8 menu)
{
    if (get_call_status() != BT_CALL_HANGUP) {
        led1888_show_string((u8 *)" CAL");
    } else {

        if (get_bt_connect_status() == BT_STATUS_CONNECTING || \
            get_bt_connect_status() == BT_STATUS_PLAYING_MUSIC) {

            /* led1888_clear_icon(); */
            itoa4(fmtx_get_freq());

            if (fmtx_get_freq() <= 999) {
                bcd_number[0] = ' ';
            }

            led1888_show_string((u8 *)bcd_number);
            LED_STATUS |= LED_DOT;

        } else {
            led1888_show_string((u8 *)" bt ");
            /* led1888_var.bFlashChar = 0x0F; */
            /* led1888_var.bFlashIcon = LED_DOT; */
        }

    }

}
/*----------------------------------------------------------------------------*/
/**@brief   Music 播放文件号显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_filenumber(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_filenumber(void)
{
    MUSIC_DIS_VAR *music_var;

    music_var = (MUSIC_DIS_VAR *)UI_var.ui_buf_adr;

    if (music_var) {
        MUSIC_PLAYER *music_obj;
        music_obj = music_var->mapi;
        u32 file_num;

        file_num = music_player_get_file_number(music_obj);
        itoa4(file_num);
        bcd_number[0] = ' ';
        led1888_show_string((u8 *)bcd_number);
    }
}


/*----------------------------------------------------------------------------*/
/**@brief   红外输入文件号显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_IR_number(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_IR_number(s32 arg)
{
    u16 ir_num;
    ir_num = (u16)(arg & 0xffff);
    /*IR File Number info*/
    itoa4(ir_num);
    if (ir_num <= 999) {
        bcd_number[0] = ' ';
    }
    led1888_show_string((u8 *)bcd_number);
}

/*----------------------------------------------------------------------------*/
/**@brief   Music模式 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_music_main(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_music_main(void)
{
    //led1888_show_string((u8*)"MUSI");
    u32 play_time;
    MUSIC_DIS_VAR *music_var;

    music_var = (MUSIC_DIS_VAR *)UI_var.ui_buf_adr;

    if (music_var) {
        if (music_var->curr_statu == MUSIC_DECODER_ST_PLAY) {
            itoa4(fmtx_get_freq());

            if (fmtx_get_freq() <= 999) {
                bcd_number[0] = ' ';
            }
            led1888_show_string((u8 *)bcd_number);
            LED_STATUS |= LED_DOT;
        } else if (music_var->curr_statu == MUSIC_PLAYRR_ST_PAUSE) {
            led1888_show_string((u8 *)" PAU");
        } else {
            led1888_show_string((u8 *)" Lod");
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   EQ显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_eq(s32 arg)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_eq(s32 arg)
{
    u8 eq_cnt;
    eq_cnt = (u8)arg;
    led1888_show_string((u8 *)other_string[0]);
    led1888_show_char(eq_cnt % 10 + '0');
}

/*----------------------------------------------------------------------------*/
/**@brief   循环模式显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_playmode(s32 arg)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_playmode(s32 arg)
{
    u8 pm_cnt;
    pm_cnt = (u8)arg;
    led1888_show_string((u8 *)&playmodestr[pm_cnt - REPEAT_ALL][0]);
}

/*----------------------------------------------------------------------------*/
/**@brief   音量显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_volume(s32 vol)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_volume(s32 vol)
{
    u8 tmp_vol;

    tmp_vol = (u8)vol;
    led1888_show_string((u8 *)other_string[1]);
    itoa2(tmp_vol);
    led1888_show_string((u8 *)bcd_number);
}

/*----------------------------------------------------------------------------*/
/**@brief   FM 模式主界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_fmtx_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_fm_main(void)
{
    /*FM - Frequency*/
#if 0
    FM_MODE_VAR *fm_var;

    if (!UI_var.ui_buf_adr) {
        return;
    }

    fm_var = *(FM_MODE_VAR **)UI_var.ui_buf_adr;

    if (fm_var) {
        itoa4(fm_var->wFreq);

        if (fm_var->wFreq <= 999) {
            bcd_number[0] = ' ';
        }

        led7_show_string((u8 *)bcd_number);
        LED_STATUS |= LED_MHZ;

#if REC_EN
        RECORD_OP_API *rec_var_p;
        REC_CTL *rec_ctl_var;

        if ((fm_var->fm_rec_op) && (*(RECORD_OP_API **)(fm_var->fm_rec_op))) {
            rec_var_p = *(RECORD_OP_API **)fm_var->fm_rec_op;
            rec_ctl_var = rec_var_p->rec_ctl;

            if ((rec_ctl_var) && (ENC_STOP != rec_ctl_var->enable)) {
                LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
                LED_STATUS &= ~(LED_SD | LED_USB);

                /* led7_show_dev(); */
                led7_show_dev(rec_ctl_var->curr_device);
                if (ENC_STAR == rec_ctl_var->enable) {
                    LED_STATUS |= LED_PLAY;
                } else if (ENC_PAUS == rec_ctl_var->enable) {
                    LED_STATUS |= LED_PAUSE;
                }
            }

        }
#endif
    }

#endif // 0
}

void led1888_show_fmtx_main(void)
{
    itoa4(fmtx_get_freq());

    if (fmtx_get_freq() <= 999) {
        bcd_number[0] = ' ';
    }

    led1888_show_string((u8 *)bcd_number);
    LED_STATUS |= LED_DOT;

    //调频模式时闪烁频率
    if (fmtx_get_whether_to_flash_freq()) {
        //puts("\n******set_fre_flag == 1*****\n");
        led1888_var.bFlashChar = BIT(0) | BIT(1) | BIT(2) | BIT(3);
        led1888_var.bFlashIcon = LED_DOT;
    } else {
        //puts("\n******set_fre_flag == 0*****\n");
        led1888_var.bFlashChar = 0;
        led1888_var.bFlashIcon = 0;
    }

}

/*----------------------------------------------------------------------------*/
/**@brief   FM 模式主界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_fm_station(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_fm_station(void)
{
#if 0
    FM_MODE_VAR *fm_var;

    if (!UI_var.ui_buf_adr) {
        return;
    }

    fm_var = *(FM_MODE_VAR **)UI_var.ui_buf_adr;

    if (fm_var) {
        led1888_show_string((u8 *)other_string[2]);
        itoa2(fm_var->wFreChannel);
        led1888_show_string((u8 *)bcd_number);
    }
#endif
}

#if RTC_CLK_EN
/*----------------------------------------------------------------------------*/
/**@brief   RTC 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_RTC_main(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_RTC_main(void)
{
    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time;

    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var) {
        ui_time = rtc_var->calendar_set.curr_rtc_time;
        if (rtc_var->rtc_set_mode == RTC_SET_MODE) {
            u8 coordinate =  rtc_var->calendar_set.coordinate;
            if (coordinate == RTC_YEAR_SETTING) {
                itoa4(ui_time->dYear);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(0) | BIT(1) | BIT(2) | BIT(3);
            } else if (coordinate == RTC_MONTH_SETTING) {
                itoa2(ui_time->bMonth);
                led1888_show_string((u8 *)bcd_number);
                itoa2(ui_time->bDay);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(0) | BIT(1);
            } else if (coordinate == RTC_DAT_SETTING) {
                itoa2(ui_time->bMonth);
                led1888_show_string((u8 *)bcd_number);
                itoa2(ui_time->bDay);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(2) | BIT(3);
            } else if (coordinate == RTC_HOUR_SETTING) {
                itoa2(ui_time->bHour);
                led1888_show_string((u8 *)bcd_number);
                itoa2(ui_time->bMin);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(0) | BIT(1);
                LED_STATUS |= LED_2POINT;
            } else if (coordinate == RTC_MIN_SETTING) {
                itoa2(ui_time->bHour);
                led1888_show_string((u8 *)bcd_number);
                itoa2(ui_time->bMin);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(2) | BIT(3);
                LED_STATUS |= LED_2POINT;
            }
        } else {
            itoa2(ui_time->bHour);
            led1888_show_string((u8 *)bcd_number);
            itoa2(ui_time->bMin);
            led1888_show_string((u8 *)bcd_number);
            led1888_var.bFlashChar = 0;
            led1888_var.bFlashIcon |= LED_2POINT;
            LED_STATUS |= LED_2POINT;
        }
    }

}

/*----------------------------------------------------------------------------*/
/**@brief   Alarm 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_show_alarm(void)
*/
/*----------------------------------------------------------------------------*/
#if RTC_ALM_EN
void led1888_show_alarm(void)
{
    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time;
    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var) {
        ui_time = rtc_var->alarm_set.curr_alm_time;
        if (rtc_var->rtc_set_mode == ALM_SET_MODE) {
            u8 coordinate =  rtc_var->alarm_set.coordinate;
            if (coordinate == RTC_YEAR_SETTING) {
                itoa4(ui_time->dYear);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(0) | BIT(1) | BIT(2) | BIT(3);
            } else if (coordinate == RTC_MONTH_SETTING) {
                itoa2(ui_time->bMonth);
                led1888_show_string((u8 *)bcd_number);
                itoa2(ui_time->bDay);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(0) | BIT(1);
            } else if (coordinate == RTC_DAT_SETTING) {
                itoa2(ui_time->bMonth);
                led1888_show_string((u8 *)bcd_number);
                itoa2(ui_time->bDay);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(2) | BIT(3);
            } else if (coordinate == RTC_HOUR_SETTING) {
                itoa2(ui_time->bHour);
                led1888_show_string((u8 *)bcd_number);
                itoa2(ui_time->bMin);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(0) | BIT(1);
                LED_STATUS |= LED_2POINT;
            } else if (coordinate == RTC_MIN_SETTING) {
                itoa2(ui_time->bHour);
                led1888_show_string((u8 *)bcd_number);
                itoa2(ui_time->bMin);
                led1888_show_string((u8 *)bcd_number);
                led1888_var.bFlashChar = BIT(2) | BIT(3);
                LED_STATUS |= LED_2POINT;
            }
        } else {
            LED_STATUS |= LED_2POINT;
        }


        /*Alarm info - Switch On/Off*/
        if (rtc_var->alarm_set.alarm_sw) {
            LED_STATUS |= LED_PLAY;
        } else {
            LED_STATUS |= LED_PAUSE;
        }
    }

}
#endif
#endif

#if REC_EN
/*----------------------------------------------------------------------------*/
/**@brief   REC 显示界面
   @param   void
   @return  void
   @note    void led1888_show_rec_start(void)
*/
/*----------------------------------------------------------------------------*/
static void led1888_show_rec_start(REC_CTL *rec_ctl_var)
{
    u32 rec_time;

//    LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
//    if(rec_ctl_var)
    {
        rec_time = rec_ctl_var->file_info.enc_time_cnt;

        itoa2(rec_time / 60);
        led1888_show_string((u8 *)bcd_number);

        itoa2(rec_time % 60);
        led1888_show_string((u8 *)bcd_number);

        /* led1888_show_dev(); */
        led1888_show_dev(rec_ctl_var->curr_device);

        LED_STATUS |= LED_2POINT; //| LED_PLAY;

        if (ENC_STAR == rec_ctl_var->enable) {
            LED_STATUS |= LED_PLAY;
        } else if (ENC_PAUS == rec_ctl_var->enable) {
            LED_STATUS |= LED_PAUSE;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   REC 显示界面
   @param   void
   @return  void
   @note    void led1888_show_rec_main(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_show_rec_main(void)
{
    RECORD_OP_API *rec_var_p;
    REC_CTL *rec_ctl_var;

    LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
    LED_STATUS &= ~LED_2POINT; //| LED_PLAY;
    LED_STATUS &= ~(LED_SD | LED_USB);

    if (UI_var.ui_buf_adr) {
        rec_var_p = *(RECORD_OP_API **)UI_var.ui_buf_adr;
        if (rec_var_p) {
            rec_ctl_var = rec_var_p->rec_ctl;
            if ((rec_ctl_var) && (ENC_STOP != rec_ctl_var->enable)) {
                led1888_show_rec_start(rec_ctl_var);
                return;
            }
        }
    }

    led1888_show_string((u8 *)other_string[4]);
}
#endif

extern u8 get_sys_halfsec(void);
void led1888_driver(void)
{
    u8 k, i, j;
    u8 temp;
    k = 0;

    led1888_var.bShowBuff1[0] = 0;
    led1888_var.bShowBuff1[1] = 0;
    led1888_var.bShowBuff1[2] = 0;
    led1888_var.bShowBuff1[3] = 0;
    led1888_var.bShowBuff1[4] = 0;
    led1888_var.bShowBuff1[5] = 0;
    led1888_var.bShowBuff1[6] = 0;


    for (i = 0; i < 5; i++) {
        temp = led1888_var.bShowBuff[i];
        if (get_sys_halfsec()) {
            if ((led1888_var.bFlashIcon) && (i == 4)) {
                temp = LED_STATUS & (~led1888_var.bFlashIcon);
            } else if (led1888_var.bFlashChar & BIT(i)) {
                temp = 0x0;
            }
        }

        for (j = 0; j < 7; j++) {
            if (temp & bit_table[j]) {
                led1888_var.bShowBuff1[led_1888[k][0]] |= bit_table[led_1888[k][1]];
            }
            k++;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   LED清屏函数
   @param   x：显示横坐标
   @return  void
   @author  Change.tsai
   @note    void led1888_clear(void)
*/
/*----------------------------------------------------------------------------*/
void led1888_clear(void)
{

    LEDN_S0_PORT->OUT &= ~(BIT(LEDN_S0_BIT));
    LEDN_S0_PORT->DIR |= (BIT(LEDN_S0_BIT));
    LEDN_S0_PORT->PU  &= ~(BIT(LEDN_S0_BIT));
    LEDN_S0_PORT->PD  &= ~(BIT(LEDN_S0_BIT));
    LEDN_S0_PORT->HD  |= (BIT(LEDN_S0_BIT));

    LEDN_S1_PORT->OUT &= ~(BIT(LEDN_S1_BIT));
    LEDN_S1_PORT->DIR |= (BIT(LEDN_S1_BIT));
    LEDN_S1_PORT->PU  &= ~(BIT(LEDN_S1_BIT));
    LEDN_S1_PORT->PD  &= ~(BIT(LEDN_S1_BIT));
    LEDN_S1_PORT->HD  |= (BIT(LEDN_S1_BIT));

    LEDN_S2_PORT->OUT &= ~(BIT(LEDN_S2_BIT));
    LEDN_S2_PORT->DIR |= (BIT(LEDN_S2_BIT));
    LEDN_S2_PORT->PU  &= ~(BIT(LEDN_S2_BIT));
    LEDN_S2_PORT->PD  &= ~(BIT(LEDN_S2_BIT));
    LEDN_S2_PORT->HD  |= (BIT(LEDN_S2_BIT));

    LEDN_S3_PORT->OUT &= ~(BIT(LEDN_S3_BIT));
    LEDN_S3_PORT->DIR |= (BIT(LEDN_S3_BIT));
    LEDN_S3_PORT->PU  &= ~(BIT(LEDN_S3_BIT));
    LEDN_S3_PORT->PD  &= ~(BIT(LEDN_S3_BIT));
    LEDN_S3_PORT->HD  |= (BIT(LEDN_S3_BIT));

    LEDN_S4_PORT->OUT &= ~(BIT(LEDN_S4_BIT));
    LEDN_S4_PORT->DIR |= (BIT(LEDN_S4_BIT));
    LEDN_S4_PORT->PU  &= ~(BIT(LEDN_S4_BIT));
    LEDN_S4_PORT->PD  &= ~(BIT(LEDN_S4_BIT));
    LEDN_S4_PORT->HD  |= (BIT(LEDN_S4_BIT));

    LEDN_S5_PORT->OUT &= ~(BIT(LEDN_S5_BIT));
    LEDN_S5_PORT->DIR |= (BIT(LEDN_S5_BIT));
    LEDN_S5_PORT->PU  &= ~(BIT(LEDN_S5_BIT));
    LEDN_S5_PORT->PD  &= ~(BIT(LEDN_S5_BIT));
    LEDN_S5_PORT->HD  |= (BIT(LEDN_S5_BIT));

}

/*----------------------------------------------------------------------------*/
/**@brief   LED扫描函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led1888_scan(void *param)
*/
/*----------------------------------------------------------------------------*/
void led1888_scan(void *param)
{
    static u8 cnt = 0;
    u16 seg;

    led1888_driver();

    seg = led1888_var.bShowBuff1[cnt];

    led1888_clear();

    switch (cnt) {
    case 0:
        LEDN_S0_PORT->OUT |=  BIT(LEDN_S0_BIT);
        LEDN_S0_PORT->DIR &= ~BIT(LEDN_S0_BIT);
        break;

    case 1:
        LEDN_S1_PORT->OUT |=  BIT(LEDN_S1_BIT);
        LEDN_S1_PORT->DIR &= ~BIT(LEDN_S1_BIT);
        break;

    case 2:
        LEDN_S2_PORT->OUT |=  BIT(LEDN_S2_BIT);
        LEDN_S2_PORT->DIR &= ~BIT(LEDN_S2_BIT);
        break;

    case 3:
        LEDN_S3_PORT->OUT |=  BIT(LEDN_S3_BIT);
        LEDN_S3_PORT->DIR &= ~BIT(LEDN_S3_BIT);
        break;

    case 4:
        LEDN_S4_PORT->OUT |=  BIT(LEDN_S4_BIT);
        LEDN_S4_PORT->DIR &= ~BIT(LEDN_S4_BIT);
        break;

    case 5:
        LEDN_S5_PORT->OUT |=  BIT(LEDN_S5_BIT);
        LEDN_S5_PORT->DIR &= ~BIT(LEDN_S5_BIT);
        break;

    default :
        break;
    }

    if (seg & BIT(0)) {
        LEDN_S0_PORT->OUT &= ~BIT(LEDN_S0_BIT);
        LEDN_S0_PORT->DIR &= ~BIT(LEDN_S0_BIT);
    }

    if (seg & BIT(1)) {
        LEDN_S1_PORT->OUT &= ~BIT(LEDN_S1_BIT);
        LEDN_S1_PORT->DIR &= ~BIT(LEDN_S1_BIT);
    }

    if (seg & BIT(2)) {
        LEDN_S2_PORT->OUT &= ~BIT(LEDN_S2_BIT);
        LEDN_S2_PORT->DIR &= ~BIT(LEDN_S2_BIT);
    }

    if (seg & BIT(3)) {
        LEDN_S3_PORT->OUT &= ~BIT(LEDN_S3_BIT);
        LEDN_S3_PORT->DIR &= ~BIT(LEDN_S3_BIT);
    }

    if (seg & BIT(4)) {
        LEDN_S4_PORT->OUT &= ~BIT(LEDN_S4_BIT);
        LEDN_S4_PORT->DIR &= ~BIT(LEDN_S4_BIT);
    }

    if (seg & BIT(5)) {
        LEDN_S5_PORT->OUT &= ~BIT(LEDN_S5_BIT);
        LEDN_S5_PORT->DIR &= ~BIT(LEDN_S5_BIT);
    }

    cnt = (cnt >= 5) ? 0 : cnt + 1;
}

LOOP_UI_REGISTER(led1888_scan_loop) = {
    .time = 1,
    .fun  = led1888_scan,
};

#endif

