/*--------------------------------------------------------------------------*/
/**@file    LED.c
   @brief   LED 模块驱动接口函数
   @details
   @author  bingquan Cai
   @date    2012-8-30
   @note    AC319N
*/
/*----------------------------------------------------------------------------*/
#include "ui/ui_api.h"

#if (LED_7_EN == 1)
#include "ui/ui_common.h"
#include "ui/led/led7_drv.h"
#include "task_fm.h"
#include "music_ui.h"
#include "timer.h"
#include "key_drv/key.h"
#include "file_operate/file_op.h"
#include "rtc_setting.h"
#include "common.h"
#include "power.h"
#include "bluetooth/avctp_user.h"

#if REC_EN
//#include "encode/encode.h"
#include "rec_api.h"
#endif
#define LCD_NUM_MAX    2999

LED7_VAR led7_var;
extern u8 get_sys_halfsec(void);

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
    0x71, 0x40, 0x40, 0x40, 0x40, ///<fghij
    0x40, 0x38, 0x40, 0x54, 0x5c, ///<klmno
    0x73, 0x67, 0x50, 0x40, 0x78, ///<pqrst
    0x3e, 0x3e, 0x40, 0x40, 0x40, ///<uvwxy
    0x40///<z
};

const u8 wday_icon[7] =
{
	ICON_WDAY_6,ICON_WDAY_7,ICON_WDAY_1,ICON_WDAY_2,ICON_WDAY_3,ICON_WDAY_4,ICON_WDAY_5
};

#if THEME_LEDSEG_7PIN
/* 通过测试 7断的真值表
   0    1    2     3     4     5     6
0  X    1A   1B    1E    SD   播放   X
1  1F   X    2A    2B    2E   2D     X
2  1G   2F   X     :     3B   ||     MP3
3  1C   2G   3F    X     3C   4E     X
4  1D   2C   3G    3A    X    4C     4G
5  3D   U    3E    4D    4F   X      4B
6  X    X    MHz    X    X    4A     X
*/
// 7断数码管转换表
const u8 led_7[35][2] = {
    {0, 1}, ///1A
    {0, 2}, ///1B
    {3, 0}, ///1C
    {4, 0}, ///1D
    {0, 3}, ///1E
    {1, 0}, ///1F
    {2, 0}, ///1G

    {1, 2}, ///2A
    {1, 3}, ///2B
    {4, 1}, ///2C
    {1, 5}, ///2D
    {1, 4}, ///2E
    {2, 1}, ///2F
    {3, 1}, ///2G

    {4, 3}, ///3A
    {2, 4}, ///3B
    {3, 4}, ///3C
    {5, 0}, ///3D
    {5, 2}, ///3E
    {3, 2}, ///3F
    {4, 2}, ///3G

    {6, 5}, ///4A
    {5, 6}, ///4B
    {4, 5}, ///4C
    {5, 3}, ///4D
    {3, 5}, ///4E
    {5, 4}, ///4F
    {4, 6}, ///4G

    {0, 5}, //LED_PLAY
    {2, 5}, //LED_PAUSE
    {5, 1}, //LED_USB
    {0, 4}, //LED_SD
    {2, 3}, //LED_2POINT
    {6, 2}, //LED_MHZ
    {2, 6}, //LED_MP3
};
#else
/***********新7段数码管真值表**************/
/* 0    1     2     3     4     5     6
0  X    2A    2B    2C    2D    2E    2F
1  1A   X     2G    :     .     MHz   MP3
2  1B   X     X     4A    4B    4C    4D
3  1C   PLAY  3A    X     4E    4F    4G
4  1D   PAUSE 3B    3E    X     X     X
5  1E   USB   3C    3F    X     X     X
6  1F   SD    3D    3G    X     X     X
*/
const u8 led_7[36][2] = { ///< 新7断数码管转换表
    {1, 0}, ///1A
    {2, 0}, ///1B
    {3, 0}, ///1C
    {4, 0}, ///1D
    {5, 0}, ///1E
    {6, 0}, ///1F
    {2, 1}, ///1G

    {0, 1}, ///2A
    {0, 2}, ///2B
    {0, 3}, ///2C
    {0, 4}, ///2D
    {0, 5}, ///2E
    {0, 6}, ///2F
    {1, 2}, ///2G

    {3, 2}, ///3A
    {4, 2}, ///3B
    {5, 2}, ///3C
    {6, 2}, ///3D
    {4, 3}, ///3E
    {5, 3}, ///3F
    {6, 3}, ///3G

    {2, 3}, ///4A
    {2, 4}, ///4B
    {2, 5}, ///4C
    {2, 6}, ///4D
    {3, 4}, ///4E
    {3, 5}, ///4F
    {3, 6}, ///4G

    {3, 1}, //LED_PLAY
    {4, 1}, //LED_PAUSE
    {5, 1}, //LED_USB
    {6, 1}, //SD
    {1, 3}, //:
    {1, 5}, //MHz
    {1, 4}, //.
    {1, 6}, //MP3
};
#endif
//按位于 查表
const u8 bit_table[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
};

const u8 playmodestr[][5] = {
    " ALL",
    " dev",
    " ONE",
    " rAn",
    "Fold",
    " Nor",
};

const u8 menu_string[][5] = {
    "HLLo",
    " Lod",
    " bt ",
    " PC ",
    " UP ",
    " dN ",
    "LINE",
    "-AL-",
    "derr",
    "ECHO",
    " OFF",
    " ---",
    " Sd ",
    " USb",
    "    "
};
const u8 other_string[][5] = {
    " Eq",
    " V",
    "CH",
    " NOP",
    " rec",
};
const u8 auto_time_string[][5] = {
    "  5 ",
    " 15 ",
    " 30 ",
    " 45 ",
    " 60 ",
    " 90 ",
    " 120",
    " 0FF",
};

enum {
    MUSIC_DECODER_PAUSE	= 0,
    MUSIC_DECODER_STOP,
};
const u8 spec_string[][5] = {
    " PAU",///"PAUS",
    " STO",
};

/*----------------------------------------------------------------------------*/
/**@brief   Music模式 设备显示
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_dev(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_dev(void *dev)
{
    if (dev) {
        if ((dev == sd0) || (dev == sd1)) {
            //LED_STATUS |= LED_SD;
        } else if (dev == usb) {
            led7_var.bShowBuff[4] |= LED_USB;//LED_STATUS |= LED_USB;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 状态位缓存清除函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_clear_icon(void)
*/
/*----------------------------------------------------------------------------*/
void led7_clear_icon(void)
{
    led7_var.bFlashChar = 0;
    led7_var.bFlashIcon = 0;
    memset(led7_var.bShowBuff, 0x00, 13);
    // led7_var.bShowBuff[4] = 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 显示坐标设置
   @param   x：显示横坐标
   @return  void
   @author  Change.tsai
   @note    void led7_setX(u8 X)
*/
/*----------------------------------------------------------------------------*/
void led7_setX(u8 X)
{
    led7_var.bCoordinateX = X;
}

/*----------------------------------------------------------------------------*/
/**@brief   LED 清屏函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_null(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_null(void)
{
    led7_clear_icon();
    led7_var.bShowBuff[0] = 0;
    led7_var.bShowBuff[1] = 0;
    led7_var.bShowBuff[2] = 0;
    led7_var.bShowBuff[3] = 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 扫描函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_init(void)
*/
/*----------------------------------------------------------------------------*/
void led7_init(void)
{
    led7_clear();
//   s32 ret;
//	ret = timer_reg_isr_fun(timer0_hl,1,led7_scan,NULL);
//	if(ret != TIMER_NO_ERR)
//	{
//		printf("led7_scan err = %x\n",ret);
//	}
}
//LOOP_UI_REGISTER(led7_scan_loop) = {
//    .time = 1,
//    .fun  = led7_scan,
//};
/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 单个字符显示函数
   @param   chardata：显示字符
   @return  void
   @author  Change.tsai
   @note    void led7_show_char(u8 chardata)
*/
/*----------------------------------------------------------------------------*/
void led7_show_char(u8 chardata)
{
    //必须保证传入的参数符合范围，程序不作判断
    //if ((chardata < ' ') || (chardata > '~') || (led7_var.bCoordinateX > 4))
    //{
    //    return;
    //}
    if ((chardata >= '0') && (chardata <= '9')) {
        led7_var.bShowBuff[led7_var.bCoordinateX++] |= LED_NUMBER[chardata - '0'];
    } else if ((chardata >= 'a') && (chardata <= 'z')) {
        led7_var.bShowBuff[led7_var.bCoordinateX++] |= LED_SMALL_LETTER[chardata - 'a'];
    } else if ((chardata >= 'A') && (chardata <= 'Z')) {
        led7_var.bShowBuff[led7_var.bCoordinateX++] |= LED_LARGE_LETTER[chardata - 'A'];
    } else if (chardata == ':') {
        //LED_STATUS |= LED_2POINT;
    } else if (chardata == ' ') {
        led7_var.bShowBuff[led7_var.bCoordinateX++] &= 0x80;
    } else { //if (chardata == '-')     //不可显示
        led7_var.bShowBuff[led7_var.bCoordinateX++] |= BIT(6);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 字符串显示函数
   @param   *str：字符串的指针   offset：显示偏移量
   @return  void
   @author  Change.tsai
   @note    void led7_show_string(u8 *str)
*/
/*----------------------------------------------------------------------------*/
void led7_show_string(u8 *str)
{
    while (0 != *str) {
        led7_show_char(*str++);
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
void led7_show_string_menu(u8 menu)
{

    if (menu >= (sizeof(menu_string) / 5)) {
        printf("*strid(%d) is over!\n", menu);
    } else {
        led7_show_string((u8 *)menu_string[menu]);
    }
    if (menu != MENU_POWER_UP)
    {
        led7_show_alm_time();
        led7_show_alm2_time();
    }
}

void led7_show_power(void)
{
    u16  temp;

    temp = get_power_external_value();

    itoa4(temp);
    if (temp <= 99) {
        bcd_number[0] = ' ';
        bcd_number[1] = ' ';
        led7_show_string((u8 *)bcd_number);
        //LED_STATUS |= LED_DOT;
    } else if (temp <= 365) {
        bcd_number[0] = ' ';
        led7_show_string((u8 *)bcd_number);
        //LED_STATUS |= LED_DOT;
    }
}

void led7_show_bt_main(u8 menu)
{
	u8 coordinate;
	coordinate = rtc_set.calendar_set.coordinate;
    if (rtc_display_cnt)
    {
        //led7_show_string_menu(menu);
        led7_var.bShowBuff[1] |= LED_SMALL_LETTER['b'-'a'];
        led7_var.bShowBuff[2] |= LED_SMALL_LETTER['t'-'a'];
    	//LED_STATUS |= LED_BT;
        led7_var.bShowBuff[4] |= LED_BT;
    	if (get_bt_connect_status() < BT_STATUS_CONNECTING)
    	{
    		if (get_sys_halfsec())
    		{
    			//LED_STATUS &=~ LED_BT;//led7_var.bFlashIcon |= LED_BT;
    	        led7_var.bShowBuff[0] &= 0x80;
    	        led7_var.bShowBuff[1] &= 0x80;
    	        led7_var.bShowBuff[2] &= 0x80;
    	        led7_var.bShowBuff[3] &= 0x80;
                led7_var.bShowBuff[4] &=~ LED_BT;
    		}
    	}
    }
	else
	{
	    led7_show_RTC_temp();
    	if ((coordinate == RTC_HOUR_SETTING)||(coordinate == RTC_MIN_SETTING))
    	{
            led7_var.bShowBuff[4] |= LED_BT;//LED_STATUS |= LED_BT;
        	if (get_bt_connect_status() < BT_STATUS_CONNECTING)
        	{
        		if (get_sys_halfsec())
        		{
                    led7_var.bShowBuff[4] &=~ LED_BT;
        		}
            }
        }
	}
    led7_show_alm_time();
    led7_show_alm2_time();
}

void led7_show_linin_main(u8 menu)
{
	u8 coordinate;
	coordinate = rtc_set.calendar_set.coordinate;
#if 1
    if (rtc_display_cnt)
    {
        //led7_show_string_menu(menu);
    	//led7_var.bShowBuff[1] |= LED_AUX;
	    led7_show_RTC_temp();
        led7_var.bShowBuff[5] |= LED_AUX;
    }
	else
#endif
	{
	    led7_show_RTC_temp();
    	if ((coordinate == RTC_HOUR_SETTING)||(coordinate == RTC_MIN_SETTING))
        	led7_var.bShowBuff[5] |= LED_AUX;
	}
    //led7_var.bShowBuff[1] |= LED_AUX;
    led7_show_alm_time();
    led7_show_alm2_time();

#if REC_EN
    RECORD_OP_API *rec_var_p;

    rec_var_p = *(RECORD_OP_API **)UI_var.ui_buf_adr;
    if ((UI_var.ui_buf_adr) && (rec_var_p)) {

        LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
        LED_STATUS &= ~(LED_SD | LED_USB);

        if (ENC_STOP != rec_get_enc_sta(rec_var_p)) {
            if (ENC_STAR == rec_get_enc_sta(rec_var_p)) {
                LED_STATUS |= LED_PLAY;
            } else if (ENC_PAUS == rec_get_enc_sta(rec_var_p)) {
                LED_STATUS |= LED_PAUSE;
            }

            led7_show_dev(rec_get_cur_dev(rec_var_p));
        }
    }
#endif
}
/*----------------------------------------------------------------------------*/
/**@brief   Music 播放文件号显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_filenumber(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_filenumber(void)
{
    MUSIC_DIS_VAR *music_var;

    music_var = (MUSIC_DIS_VAR *)UI_var.ui_buf_adr;

    if (music_var) {
        MUSIC_PLAYER *music_obj;
        music_obj = music_var->mapi;
        u32 file_num;

        file_num = music_player_get_file_number(music_obj);
        //itoa4(file_num);
        //bcd_number[0] = 'F';
        led7_show_dev(music_var->ui_curr_device);
        //led7_show_string((u8 *)bcd_number);
        #if 0
        if (((file_num/100)/10)> 2)
            led7_var.bShowBuff[0] |= LED_NUMBER[2];
        else
        {
            if ((file_num/100)/10)
                led7_var.bShowBuff[0] |= LED_NUMBER[(file_num/100)/10];
        }
        #else
        led7_var.bShowBuff[0] |= LED_SMALL_LETTER['t'-'a'];
        #endif
        led7_var.bShowBuff[1] |= LED_NUMBER[(file_num/100)%10];
        led7_var.bShowBuff[2] |= LED_NUMBER[(file_num%100)/10];
        led7_var.bShowBuff[3] |= LED_NUMBER[(file_num%100)%10];
    }
    led7_show_alm_time();
    led7_show_alm2_time();
}


/*----------------------------------------------------------------------------*/
/**@brief   红外输入文件号显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_IR_number(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_IR_number(s32 arg)
{
    u16 ir_num;
    ir_num = (u16)(arg & 0xffff);
    /*IR File Number info*/
    itoa4(ir_num);
    led7_show_string((u8 *)bcd_number);
}

/*----------------------------------------------------------------------------*/
/**@brief   Music模式 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_music_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_music_main(void)
{
    u32 play_time;
    MUSIC_DIS_VAR *music_var;

    music_var = (MUSIC_DIS_VAR *)UI_var.ui_buf_adr;

    if (rtc_display_cnt)
    {
        if (music_var) {
    	#if USE_MUSIC_STOP
    		if (music_stop_flag)
    		{
    	        led7_show_string((u8 *)spec_string[MUSIC_DECODER_STOP]);
    		}
    		else
    	#endif
    		{
    		#if USE_DISPLAY_WAIT
    	        if (music_var->ui_total_file == 0) {
    	            led7_show_string((u8 *)"Lod");
    	            return;
    	        }
    	    #endif
    	        /*Music Play time info*/
    	        //LED_STATUS &= ~(LED_SD | LED_USB);
    		#if 0
    	        play_time = music_var->play_time;
    	        itoa2(play_time / 60);
    	        led7_show_string((u8 *)bcd_number);
    	        itoa2(play_time % 60);
    	        led7_show_string((u8 *)bcd_number);
    		#endif
    
    	        /*Music Play dev info*/
    	        led7_show_dev(music_var->ui_curr_device);
    
    	        /*Music Play status info*/
    	        if (MUSIC_DECODER_ST_PLAY == music_var->curr_statu) {
    		        play_time = music_var->play_time;
    		        //itoa2(play_time / 60);
    		        //led7_show_string((u8 *)bcd_number);
    		        //itoa2(play_time % 60);
    		        //led7_show_string((u8 *)bcd_number);
                    //if (((play_time/60)/10)> 2)
                    //    led7_var.bShowBuff[0] |= LED_NUMBER[2];
                    //else
                    {
                    //    if ((play_time/60)/10)
                            led7_var.bShowBuff[0] |= LED_NUMBER[(play_time/60)/10];
                    }
                    led7_var.bShowBuff[1] |= LED_NUMBER[(play_time/60)%10];
                    led7_var.bShowBuff[2] |= LED_NUMBER[(play_time%60)/10];
                    led7_var.bShowBuff[3] |= LED_NUMBER[(play_time%60)%10];
    	            led7_var.bShowBuff[1] |= LED_2POINT;//LED_STATUS |= LED_PLAY | LED_2POINT;
                	//led7_var.bShowBuff[2] |= ICON_DOT_UP;
                	//led7_var.bShowBuff[3] |= ICON_DOT_DN;
    	        } else {
    	        	led7_show_string((u8 *)spec_string[MUSIC_DECODER_PAUSE]);
    	            //LED_STATUS |= LED_PAUSE;
    	        }
    	        //LED_STATUS |= LED_MP3;
    		}
        }
    }
	else
	{
	    led7_show_RTC_temp();
    	//if ((coordinate == RTC_HOUR_SETTING)||(coordinate == RTC_MIN_SETTING))
    	    led7_show_dev(music_var->ui_curr_device);
	}
    led7_show_alm_time();
    led7_show_alm2_time();
}

/*----------------------------------------------------------------------------*/
/**@brief   EQ显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_eq(s32 arg)
*/
/*----------------------------------------------------------------------------*/
void led7_show_eq(s32 arg)
{
    u8 eq_cnt;
    eq_cnt = (u8)arg;
    led7_show_string((u8 *)other_string[0]);
    led7_show_char(eq_cnt % 10 + '0');
}

/*----------------------------------------------------------------------------*/
/**@brief   循环模式显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_playmode(s32 arg)
*/
/*----------------------------------------------------------------------------*/
void led7_show_playmode(s32 arg)
{
    u8 pm_cnt;
    pm_cnt = (u8)arg;
    printf("paly mode %d\n", pm_cnt - FOP_MAX - 1);
    led7_show_string((u8 *)&playmodestr[pm_cnt - FOP_MAX - 1][0]);
    if ((pm_cnt - FOP_MAX - 1)== 0)
    {
        led7_var.bShowBuff[4] |= LED_ALL;
    }
    else if ((pm_cnt - FOP_MAX - 1)== 2)
    {
        led7_var.bShowBuff[4] |= LED_ONE;
    }
    else if ((pm_cnt - FOP_MAX - 1)== 3)
    {
        led7_var.bShowBuff[4] |= LED_RANDOM;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   音量显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_volume(s32 vol)
*/
/*----------------------------------------------------------------------------*/
void led7_show_volume(s32 vol)
{
    u8 tmp_vol;

    tmp_vol = (u8)vol;
    //led7_show_string((u8 *)other_string[1]);
    //itoa2(tmp_vol);
    //led7_show_string((u8 *)bcd_number);
    led7_var.bShowBuff[1] |= LED_LARGE_LETTER['V'-'A'];
    led7_var.bShowBuff[2] |= LED_NUMBER[tmp_vol/10];
    led7_var.bShowBuff[3] |= LED_NUMBER[tmp_vol%10];
    led7_show_alm_time();
    led7_show_alm2_time();
}

void led7_mute(void)
{
#if 1
    memset(led7_var.bShowBuff, 0x00, 5);
#else
    memset(led7_var.bShowBuff, 0x00, 4);
    LED_STATUS &=~ (LED_2POINT|LED_MHZ);
#endif
}

#if (FM_RADIO_EN == 1)
/*----------------------------------------------------------------------------*/
/**@brief   FM 模式主界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_fm_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_fm_main(void)
{
    /*FM - Frequency*/
#if 1
	u8 coordinate;
	coordinate = rtc_set.calendar_set.coordinate;
    FM_MODE_VAR *fm_var;

    if (!UI_var.ui_buf_adr) {
        return;
    }

    if (rtc_display_cnt)
    {
        fm_var = *(FM_MODE_VAR **)UI_var.ui_buf_adr;
    
        if (fm_var) {
            //itoa4(fm_var->wFreq);
    
            if (fm_var->wFreq <= 999) {
                bcd_number[0] &= 0x80;//= ' ';
            }
    
            //led7_show_string((u8 *)bcd_number);
            if ((fm_var->wFreq/100)/10)
                led7_var.bShowBuff[0] |= LED_NUMBER[(fm_var->wFreq/100)/10];
            led7_var.bShowBuff[1] |= LED_NUMBER[(fm_var->wFreq/100)%10];
            led7_var.bShowBuff[2] |= LED_NUMBER[(fm_var->wFreq%100)/10];
            led7_var.bShowBuff[3] |= LED_NUMBER[(fm_var->wFreq%100)%10];
            //LED_STATUS |= LED_MHZ | LED_FM;
            led7_var.bShowBuff[2] |= LED_MHZ;
            led7_var.bShowBuff[6] |= LED_MHZ_2;
            led7_var.bShowBuff[5] |= LED_FM;
            if (mute_flag)
            {
                if (get_sys_halfsec())
                {
                    led7_var.bShowBuff[0] &= 0x80;
                    led7_var.bShowBuff[1] &= 0x80;
                    led7_var.bShowBuff[2] &= 0x80;
                    led7_var.bShowBuff[3] &= 0x80;
                    //led7_var.bShowBuff[4] = 0;
                    led7_var.bShowBuff[2] &=~ LED_MHZ;
                    led7_var.bShowBuff[6] &=~ LED_MHZ_2;
                    led7_var.bShowBuff[5] &=~ LED_FM;
                }
            }
    
    #if  0 //REC_EN
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
        led7_show_alm_time();
        led7_show_alm2_time();
	    if ((rtc_set.alarm_set.alarm_sw)&&(rtc_set.alarm_set.alarm_flag))
    	{
    		led7_var.bShowBuff[5] |= ICON_ALM1;//LED_STATUS |= ICON_ALM1;
    		if (get_sys_halfsec())
    		{
    			if (rtc_set.alarm_set.alarm_flag)
    				led7_var.bShowBuff[5] &=~ ICON_ALM1;//LED_STATUS &=~ ICON_ALM1;
    		}
    		
    	}
	    else if ((rtc_set.alarm_set.alarm2_sw)&&(rtc_set.alarm_set.alarm2_flag))
    	{
    		led7_var.bShowBuff[5] |= ICON_ALM2;//LED_STATUS |= ICON_ALM2;
    		if (get_sys_halfsec())
    		{
    			if (rtc_set.alarm_set.alarm2_flag)
    				led7_var.bShowBuff[5] &=~ ICON_ALM2;//LED_STATUS &=~ ICON_ALM2;
    		}
    		
    	}
    }
	else
	{
        led7_show_alm_time();
        led7_show_alm2_time();
	    if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
	    {
	        //led7_show_alarm_temp();
    		led7_var.bShowBuff[5] |= ICON_ALM1;
    		if (get_sys_halfsec())
    		{
    			//if (rtc_set.alarm_set.alarm_flag)
    				led7_var.bShowBuff[5] &=~ ICON_ALM1;
    		}
	    }
		else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
		{
	        //led7_show_alarm2_temp();
    		led7_var.bShowBuff[5] |= ICON_ALM2;
    		if (get_sys_halfsec())
    		{
    			//if (rtc_set.alarm_set.alarm2_flag)
    				led7_var.bShowBuff[5] &=~ ICON_ALM2;
    		}
		}
		//else
		{
	        led7_show_RTC_temp();
		}
    	//if ((coordinate == RTC_HOUR_SETTING)||(coordinate == RTC_MIN_SETTING))
            led7_var.bShowBuff[5] |= LED_FM;//LED_STATUS |= LED_FM;
	}
    //LED_STATUS |= LED_FM;

#endif // 0
}

/*----------------------------------------------------------------------------*/
/**@brief   FM 模式主界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_fm_station(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_fm_station(void)
{
//    /*FM - Station*/
#if 1
    FM_MODE_VAR *fm_var;

    if (!UI_var.ui_buf_adr) {
        return;
    }

    fm_var = *(FM_MODE_VAR **)UI_var.ui_buf_adr;

    if (fm_var) {
        //led7_show_string((u8 *)other_string[2]);
        //itoa2(fm_var->wFreChannel);
        //led7_show_string((u8 *)bcd_number);
        led7_var.bShowBuff[1] |= LED_LARGE_LETTER['P'-'A'];
        led7_var.bShowBuff[2] |= LED_NUMBER[fm_var->wFreChannel/10];
        led7_var.bShowBuff[3] |= LED_NUMBER[fm_var->wFreChannel%10];
    }
    //LED_STATUS |= LED_FM;
    led7_var.bShowBuff[5] |= LED_FM;
#endif
    led7_show_alm_time();
    led7_show_alm2_time();
}
void led7_show_fm_preset_station(void)
{
    //led7_var.bShowBuff[0] |= LED_LARGE_LETTER['C'-'A'];
    led7_var.bShowBuff[1] |= LED_LARGE_LETTER['P'-'A'];
    led7_var.bShowBuff[2] |= LED_NUMBER[preset_station_num/10];
    led7_var.bShowBuff[3] |= LED_NUMBER[preset_station_num%10];
    //LED_STATUS |= LED_FM;
    led7_var.bShowBuff[5] |= LED_FM;
    led7_var.bShowBuff[5] |= LED_MEMORY;
    if (get_sys_halfsec())
    {
        led7_var.bShowBuff[0] &= 0x80;
        led7_var.bShowBuff[1] &= 0x80;
        led7_var.bShowBuff[2] &= 0x80;
        led7_var.bShowBuff[3] &= 0x80;
        //LED_STATUS &=~ LED_FM;
        led7_var.bShowBuff[5] &=~ LED_FM;
        led7_var.bShowBuff[5] &=~ LED_MEMORY;
    }
    led7_show_alm_time();
    led7_show_alm2_time();
}
void led7_show_fm_clr(void)
{
    led7_var.bShowBuff[1] |= LED_LARGE_LETTER['C'-'A'];
    led7_var.bShowBuff[2] |= LED_LARGE_LETTER['L'-'A'];
    led7_var.bShowBuff[3] |= LED_LARGE_LETTER['R'-'A'];
    led7_show_alm_time();
    led7_show_alm2_time();
}

#endif
#if (RTC_CLK_EN == 1)
#include "rtc_api.h"
/*----------------------------------------------------------------------------*/
/**@brief   RTC 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_RTC_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_RTC_time_format(void)
{
	if (time_format_flag == FORMAT_24)
	{
        led7_var.bShowBuff[0] |= LED_NUMBER[2];
        led7_var.bShowBuff[1] |= LED_NUMBER[4];
	}
	else
	{
        led7_var.bShowBuff[0] |= LED_NUMBER[1];
        led7_var.bShowBuff[1] |= LED_NUMBER[2];
	}
    led7_var.bShowBuff[2] |= LED_LARGE_LETTER['H'-'A'];
    if ((rtc_set.rtc_set_mode == RTC_SET_MODE)&&(rtc_time_set_cnt))
    {
    }
    else
    {
        if (get_sys_halfsec())
        {
            led7_var.bShowBuff[0] &= 0x80;
            led7_var.bShowBuff[1] &= 0x80;
            led7_var.bShowBuff[2] &=~ LED_LARGE_LETTER['H'-'A'];
        }
    }
    led7_show_alm_time();
    led7_show_alm2_time();
}
void led7_show_RTC_temp(void)
{
    RTC_TIME *ui_time;
	u8 hour_temp,coordinate;
	coordinate = rtc_set.calendar_set.coordinate;
	ui_time = rtc_set.calendar_set.curr_rtc_time;
	hour_temp = ui_time->bHour;
	if (time_format_flag == FORMAT_12)
	{
		if (hour_temp > 12)
		{
			hour_temp -= 12;
		}
		else if (hour_temp == 0)
		{
			hour_temp = 12;
		}
	}
#if 1
	if (coordinate == RTC_YEAR_SETTING) {
		//itoa4(ui_time->dYear);
		//led7_show_string((u8 *)bcd_number);
        led7_var.bShowBuff[0] |= LED_NUMBER[(ui_time->dYear/100)/10];
        led7_var.bShowBuff[1] |= LED_NUMBER[(ui_time->dYear/100)%10];
        led7_var.bShowBuff[2] |= LED_NUMBER[(ui_time->dYear%100)/10];
        led7_var.bShowBuff[3] |= LED_NUMBER[(ui_time->dYear%100)%10];
	} else if (coordinate == RTC_MONTH_SETTING) {
		//itoa2(ui_time->bMonth);
		//led7_show_string((u8 *)bcd_number);
		//if (((ui_time->bMonth % 100)/10)== 0)
		//{
		//    led7_var.bShowBuff[0] = 0;
		//}
		//itoa2(ui_time->bDay);
		//led7_show_string((u8 *)bcd_number);
		//led7_var.bShowBuff[3] |= ICON_DOT_DN;
		if (ui_time->bMonth/10)
            led7_var.bShowBuff[0] |= LED_NUMBER[ui_time->bMonth/10];
        led7_var.bShowBuff[1] |= LED_NUMBER[ui_time->bMonth%10];
        led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bDay/10];
        led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bDay%10];
		led7_var.bShowBuff[1] |= LED_2POINT;
	} else if (coordinate == RTC_DAT_SETTING) {
		//itoa2(ui_time->bMonth);
		//led7_show_string((u8 *)bcd_number);
		//if (((ui_time->bMonth % 100)/10)== 0)
		//{
		//    led7_var.bShowBuff[0] = 0;
		//}
		//itoa2(ui_time->bDay);
		//led7_show_string((u8 *)bcd_number);
		//led7_var.bShowBuff[3] |= ICON_DOT_DN;
		if (ui_time->bMonth/10)
            led7_var.bShowBuff[0] |= LED_NUMBER[ui_time->bMonth/10];
        led7_var.bShowBuff[1] |= LED_NUMBER[ui_time->bMonth%10];
        led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bDay/10];
        led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bDay%10];
		led7_var.bShowBuff[1] |= LED_2POINT;
#endif
	} else if (coordinate == RTC_HOUR_SETTING) {
		//itoa2(hour_temp);
		//led7_show_string((u8 *)bcd_number);
        //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
        //    led7_var.bShowBuff[0] = 0;
		//itoa2(ui_time->bMin);
		//led7_show_string((u8 *)bcd_number);
		//if (hour_temp/10)
            led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
        led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
        led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
        led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
        if (get_sys_halfsec())
        {
            led7_var.bShowBuff[1] &=~ LED_2POINT;
        }
        else
        {
		    led7_var.bShowBuff[1] |= LED_2POINT;//LED_STATUS |= LED_2POINT;
        }
		//led7_var.bShowBuff[2] |= ICON_DOT_UP;
		//led7_var.bShowBuff[3] |= ICON_DOT_DN;
		if (time_format_flag == FORMAT_12)
		{
    	    if (ui_time->bHour >= 12)
    	    {
    	        led7_var.bShowBuff[6] |= ICON_PM;
    	    }
	    }
	} else if (coordinate == RTC_MIN_SETTING) {
		//itoa2(hour_temp);
		//led7_show_string((u8 *)bcd_number);
        //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
        //    led7_var.bShowBuff[0] = 0;
		//itoa2(ui_time->bMin);
		//led7_show_string((u8 *)bcd_number);
		//if (hour_temp/10)
            led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
        led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
        led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
        led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
        if (get_sys_halfsec())
        {
            led7_var.bShowBuff[1] &=~ LED_2POINT;
        }
        else
        {
		    led7_var.bShowBuff[1] |= LED_2POINT;//LED_STATUS |= LED_2POINT;
        }
		//led7_var.bShowBuff[2] |= ICON_DOT_UP;
		//led7_var.bShowBuff[3] |= ICON_DOT_DN;
		if (time_format_flag == FORMAT_12)
		{
    	    if (ui_time->bHour >= 12)
    	    {
    	        led7_var.bShowBuff[6] |= ICON_PM;
    	    }
	    }
	}
	#if 0
	if ((coordinate == RTC_HOUR_SETTING)||(coordinate == RTC_MIN_SETTING))
	{
    	//led7_var.bShowBuff[5] |= wday_icon[rtc_week];
        if (rtc_set.alarm_set.alarm_sw)
    	{
            led7_var.bShowBuff[5] |= ICON_ALM1;//LED_STATUS |= ICON_ALM1;		
    	    if (alm_times)
    	    {
    	        if (get_sys_halfsec())
    	        {
                    led7_var.bShowBuff[5] &=~ ICON_ALM1;//LED_STATUS &=~ ICON_ALM1;
    	        }
    	    }
        }
        if (rtc_set.alarm_set.alarm2_sw)
    	{
            led7_var.bShowBuff[5] |= ICON_ALM2;//LED_STATUS |= ICON_ALM2;		
    	    if (alm2_times)
    	    {
    	        if (get_sys_halfsec())
    	        {
                    led7_var.bShowBuff[5] &=~ ICON_ALM2;//LED_STATUS &=~ ICON_ALM2;
    	        }
    	    }
        }
    }
    #endif
}

void led7_show_RTC_main(void)
{
    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time;
	u8 hour_temp,coordinate;

    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var) {
        //u8 coordinate =  rtc_var->calendar_set.coordinate;
        coordinate = rtc_var->calendar_set.coordinate;
        if (rtc_var->rtc_set_mode == RTC_SET_MODE) {
            //coordinate = rtc_var_temp->calendar_set.coordinate;
            ui_time = &rtc_time_set_temp;
    		hour_temp = ui_time->bHour;
        	if (time_format_flag == FORMAT_12)
        	{
        	    if (hour_temp > 12)
        	    {
        	        hour_temp -= 12;
        	    }
        		else if (hour_temp == 0)
        		{
        			hour_temp = 12;
        		}
        	}
		#if 1
            if (coordinate == RTC_YEAR_SETTING) {
                //itoa4(ui_time->dYear);
                //led7_show_string((u8 *)bcd_number);
                //led7_var.bFlashChar = BIT(0) | BIT(1) | BIT(2) | BIT(3);
                led7_var.bShowBuff[0] |= LED_NUMBER[(ui_time->dYear/100)/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[(ui_time->dYear/100)%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[(ui_time->dYear%100)/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[(ui_time->dYear%100)%10];
				if (!rtc_time_set_cnt)
				{
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[0] &= 0x80;
				        led7_var.bShowBuff[1] &= 0x80;
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				    }
				}
            } else if (coordinate == RTC_MONTH_SETTING) {
                //itoa2(ui_time->bMonth);
                //led7_show_string((u8 *)bcd_number);
        		//if (((ui_time->bMonth % 100)/10)== 0)
        		//{
        		//    led7_var.bShowBuff[0] = 0;
        		//}
                //itoa2(ui_time->bDay);
                //led7_show_string((u8 *)bcd_number);
				//led7_var.bShowBuff[3] |= ICON_DOT_DN;
                //led7_var.bFlashChar = BIT(0) | BIT(1);
        		if (ui_time->bMonth/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[ui_time->bMonth/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[ui_time->bMonth%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bDay/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bDay%10];
                led7_var.bShowBuff[1] |= LED_2POINT;
				if (!rtc_time_set_cnt)
				{
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[0] &= 0x80;
				        led7_var.bShowBuff[1] &= 0x80;
				    }
				}
            } else if (coordinate == RTC_DAT_SETTING) {
                //itoa2(ui_time->bMonth);
                //led7_show_string((u8 *)bcd_number);
        		//if (((ui_time->bMonth % 100)/10)== 0)
        		//{
        		//    led7_var.bShowBuff[0] = 0;
        		//}
                //itoa2(ui_time->bDay);
                //led7_show_string((u8 *)bcd_number);
				//led7_var.bShowBuff[3] |= ICON_DOT_DN;
                //led7_var.bFlashChar = BIT(2) | BIT(3);
        		if (ui_time->bMonth/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[ui_time->bMonth/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[ui_time->bMonth%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bDay/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bDay%10];
                led7_var.bShowBuff[1] |= LED_2POINT;
				if (!rtc_time_set_cnt)
				{
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				    }
				}
		#endif
            } else if (coordinate == RTC_HOUR_SETTING) {
                //itoa2(hour_temp);
                //led7_show_string((u8 *)bcd_number);
                //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
                //    led7_var.bShowBuff[0] = 0;
                //itoa2(ui_time->bMin);
                //led7_show_string((u8 *)bcd_number);
        		//if (hour_temp/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;
				    }
				}
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(0) | BIT(1);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[0] &= 0x80;
				        led7_var.bShowBuff[1] &= 0x80;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_STATUS |= LED_2POINT;
            	//led7_var.bShowBuff[2] |= ICON_DOT_UP;
            	//led7_var.bShowBuff[3] |= ICON_DOT_DN;
            } else if (coordinate == RTC_MIN_SETTING) {
                //itoa2(hour_temp);
                //led7_show_string((u8 *)bcd_number);
                //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
                //    led7_var.bShowBuff[0] = 0;
                //itoa2(ui_time->bMin);
                //led7_show_string((u8 *)bcd_number);
        		//if (hour_temp/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;
				    }
				}
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(2) | BIT(3);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_STATUS |= LED_2POINT;
            	//led7_var.bShowBuff[2] |= ICON_DOT_UP;
            	//led7_var.bShowBuff[3] |= ICON_DOT_DN;
            }
        } else {
            //coordinate =  rtc_var->calendar_set.coordinate;
            ui_time = rtc_var->calendar_set.curr_rtc_time;
    		hour_temp = ui_time->bHour;
        	if (time_format_flag == FORMAT_12)
        	{
        	    if (hour_temp > 12)
        	    {
        	        hour_temp -= 12;
        	    }
        		else if (hour_temp == 0)
        		{
        			hour_temp = 12;
        		}
        	}
		#if 1
            if (coordinate == RTC_YEAR_SETTING) {
                //itoa4(ui_time->dYear);
                //led7_show_string((u8 *)bcd_number);
                led7_var.bShowBuff[0] |= LED_NUMBER[(ui_time->dYear/100)/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[(ui_time->dYear/100)%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[(ui_time->dYear%100)/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[(ui_time->dYear%100)%10];
            } else if (coordinate == RTC_MONTH_SETTING) {
                //itoa2(ui_time->bMonth);
                //led7_show_string((u8 *)bcd_number);
         		//if (((ui_time->bMonth % 100)/10)== 0)
         		//{
         		//    led7_var.bShowBuff[0] = 0;
         		//}
                //itoa2(ui_time->bDay);
                //led7_show_string((u8 *)bcd_number);
				//led7_var.bShowBuff[3] |= ICON_DOT_DN;
        		if (ui_time->bMonth/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[ui_time->bMonth/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[ui_time->bMonth%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bDay/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bDay%10];
                led7_var.bShowBuff[1] |= LED_2POINT;
            } else if (coordinate == RTC_DAT_SETTING) {
                //itoa2(ui_time->bMonth);
                //led7_show_string((u8 *)bcd_number);
        		//if (((ui_time->bMonth % 100)/10)== 0)
        		//{
        		//    led7_var.bShowBuff[0] = 0;
        		//}
                //itoa2(ui_time->bDay);
                //led7_show_string((u8 *)bcd_number);
				//led7_var.bShowBuff[3] |= ICON_DOT_DN;
        		if (ui_time->bMonth/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[ui_time->bMonth/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[ui_time->bMonth%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bDay/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bDay%10];
                led7_var.bShowBuff[1] |= LED_2POINT;
		#endif
            } else if (coordinate == RTC_HOUR_SETTING) {
                //itoa2(hour_temp);
                //led7_show_string((u8 *)bcd_number);
                //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
                //    led7_var.bShowBuff[0] = 0;
                //itoa2(ui_time->bMin);
                //led7_show_string((u8 *)bcd_number);
        		//if (hour_temp/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
                if (get_sys_halfsec())
                {
                    led7_var.bShowBuff[1] &=~ LED_2POINT;
                }
                else
                {
                    led7_var.bShowBuff[1] |= LED_2POINT;//LED_STATUS |= LED_2POINT;
                }
            	//led7_var.bShowBuff[2] |= ICON_DOT_UP;
            	//led7_var.bShowBuff[3] |= ICON_DOT_DN;
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;
				    }
				}
            } else if (coordinate == RTC_MIN_SETTING) {
                //itoa2(hour_temp);
                //led7_show_string((u8 *)bcd_number);
                //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
                //    led7_var.bShowBuff[0] = 0;
                //itoa2(ui_time->bMin);
                //led7_show_string((u8 *)bcd_number);
        		//if (hour_temp/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
                if (get_sys_halfsec())
                {
                    led7_var.bShowBuff[1] &=~ LED_2POINT;
                }
                else
                {
                    led7_var.bShowBuff[1] |= LED_2POINT;//LED_STATUS |= LED_2POINT;
                }
            	//led7_var.bShowBuff[2] |= ICON_DOT_UP;
            	//led7_var.bShowBuff[3] |= ICON_DOT_DN;
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;
				    }
				}
            }
            if (rtc_set.alarm_set.alarm_sw)
        	{
                led7_var.bShowBuff[5] |= ICON_ALM1;//LED_STATUS |= ICON_ALM1;		
        	    if (alm_times)
        	    {
        	        if (get_sys_halfsec())
        	        {
                        led7_var.bShowBuff[5] &=~ ICON_ALM1;//LED_STATUS &=~ ICON_ALM1;
        	        }
        	    }
            }
            if (rtc_set.alarm_set.alarm2_sw)
        	{
                led7_var.bShowBuff[5] |= ICON_ALM2;//LED_STATUS |= ICON_ALM2;		
        	    if (alm2_times)
        	    {
        	        if (get_sys_halfsec())
        	        {
                        led7_var.bShowBuff[5] &=~ ICON_ALM2;//LED_STATUS &=~ ICON_ALM2;
        	        }
        	    }
            }
	        //led7_var.bShowBuff[5] |= wday_icon[rtc_week];
        }
    }
    led7_show_alm_time();
    led7_show_alm2_time();
}

/*----------------------------------------------------------------------------*/
/**@brief   Alarm 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_alarm(void)
*/
/*----------------------------------------------------------------------------*/
#if (RTC_ALM_EN == 1)
void led7_show_alarm_temp(void)
{
    led7_show_RTC_temp();
    led7_show_alm_time();
    led7_show_alm2_time();
	if (rtc_set.alarm_set.alarm_sw)
	{
		led7_var.bShowBuff[5] |= ICON_ALM1;//LED_STATUS |= ICON_ALM1;
		if (get_sys_halfsec())
		{
			if (rtc_set.alarm_set.alarm_flag)
				led7_var.bShowBuff[5] &=~ ICON_ALM1;//LED_STATUS &=~ ICON_ALM1;
		}
		
	}
}

void led7_show_alarm(void)
{
    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time;
	u8 hour_temp,coordinate;
	u8 fm_station_temp=0;
    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var) {
        coordinate = rtc_var->alarm_set.coordinate;
        if (rtc_var->rtc_set_mode == ALM_SET_MODE) {
            //coordinate = rtc_var_temp->alarm_set.coordinate;
            ui_time = &rtc_time_set_temp;
            printf("led7_show_alarm %d\n", rtc_var->rtc_set_mode);
    		hour_temp = ui_time->bHour;
        	if (time_format_flag == FORMAT_12)
        	{
        	    if (hour_temp > 12)
        	    {
        	        hour_temp -= 12;
        	    }
        		else if (hour_temp == 0)
        		{
        			hour_temp = 12;
        		}
        	}
		#if 0
            if (coordinate == RTC_YEAR_SETTING) {
                itoa4(ui_time->dYear);
                led7_show_string((u8 *)bcd_number);
                led7_var.bFlashChar = BIT(0) | BIT(1) | BIT(2) | BIT(3);
            } else if (coordinate == RTC_MONTH_SETTING) {
                itoa2(ui_time->bMonth);
                led7_show_string((u8 *)bcd_number);
                itoa2(ui_time->bDay);
                led7_show_string((u8 *)bcd_number);
                led7_var.bFlashChar = BIT(0) | BIT(1);
            } else if (coordinate == RTC_DAT_SETTING) {
                itoa2(ui_time->bMonth);
                led7_show_string((u8 *)bcd_number);
                itoa2(ui_time->bDay);
                led7_show_string((u8 *)bcd_number);
                led7_var.bFlashChar = BIT(2) | BIT(3);
		#endif
            if (coordinate == ALARM_RING_TYPE) {
				if (alarm_ring_type == ALARM_RING_BELL)
				{
                    //led7_var.bShowBuff[0] |= LED_SMALL_LETTER['b'-'a'];
                    led7_var.bShowBuff[1] |= LED_SMALL_LETTER['b'-'a'];
                    led7_var.bShowBuff[2] |= LED_SMALL_LETTER['b'-'a'];
                    //led7_var.bShowBuff[3] |= LED_NUMBER[2];
                    if (get_sys_halfsec())
                    {
                        led7_var.bShowBuff[0] &= 0x80;
                        led7_var.bShowBuff[1] &= 0x80;
                        led7_var.bShowBuff[2] &= 0x80;
                        led7_var.bShowBuff[3] &= 0x80;
                    }
				}
				else //if (alarm_ring_type == ALARM_RING_FM)
				{
    				if (alm_fm_station_set_flag == 0)
    				{
                        led7_var.bShowBuff[1] |= LED_SMALL_LETTER['r'-'a'];
                        led7_var.bShowBuff[2] |= LED_LARGE_LETTER['A'-'A'];
                        led7_var.bShowBuff[3] |= LED_SMALL_LETTER['d'-'a'];
                    }
                    else
                    {
                        fm_station_temp = alm_fm_station_temp;
                        led7_var.bShowBuff[1] |= LED_SMALL_LETTER['P'-'A'];
                        led7_var.bShowBuff[2] |= LED_NUMBER[fm_station_temp%100/10];
                        led7_var.bShowBuff[3] |= LED_NUMBER[fm_station_temp%10];
                    }
                    if (get_sys_halfsec())
                    {
                        led7_var.bShowBuff[1] &= 0x80;
                        led7_var.bShowBuff[2] &= 0x80;
                        led7_var.bShowBuff[3] &= 0x80;
                    }
				}
        		//if (hour_temp/10)
                //    led7_var.bShowBuff[4] |= LED_NUMBER[hour_temp/10];
                //led7_var.bShowBuff[5] |= LED_NUMBER[hour_temp%10];
                //led7_var.bShowBuff[6] |= LED_NUMBER[ui_time->bMin/10];
                //led7_var.bShowBuff[7] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM1_PM;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM1;
            } else if (coordinate == ALARM_HOUR_SETTING) {
                //itoa2(hour_temp);
                //led7_show_string((u8 *)bcd_number);
                //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
                //    led7_var.bShowBuff[0] = 0;
                //itoa2(ui_time->bMin);
                //led7_show_string((u8 *)bcd_number);
        		//if (hour_temp/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM1_PM;
				    }
				}
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(0) | BIT(1);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[0] &= 0x80;
				        led7_var.bShowBuff[1] &= 0x80;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM1;//LED_STATUS |= LED_2POINT;
            	//led7_var.bShowBuff[2] |= ICON_DOT_UP;
            	//led7_var.bShowBuff[3] |= ICON_DOT_DN;
            } else if (coordinate == ALARM_MIN_SETTING) {
                //itoa2(hour_temp);
                //led7_show_string((u8 *)bcd_number);
                //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
                //    led7_var.bShowBuff[0] = 0;
                //itoa2(ui_time->bMin);
                //led7_show_string((u8 *)bcd_number);
        		//if (hour_temp/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM1_PM;
				    }
				}
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(2) | BIT(3);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM1;//LED_STATUS |= LED_2POINT;
            	//led7_var.bShowBuff[2] |= ICON_DOT_UP;
            	//led7_var.bShowBuff[3] |= ICON_DOT_DN;
            } else if (coordinate == ALARM_RING_VOLUME) {
                led7_var.bShowBuff[1] |= LED_LARGE_LETTER['U'-'A'];
                led7_var.bShowBuff[2] |= LED_NUMBER[alarm_ring_volume/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[alarm_ring_volume%10];
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(2) | BIT(3);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[1] &= 0x80;
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				    }
				}
        		//if (hour_temp/10)
                //    led7_var.bShowBuff[4] |= LED_NUMBER[hour_temp/10];
                //led7_var.bShowBuff[5] |= LED_NUMBER[hour_temp%10];
                //led7_var.bShowBuff[6] |= LED_NUMBER[ui_time->bMin/10];
                //led7_var.bShowBuff[7] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM1_PM;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM1;
            } else if (coordinate == ALARM_MODE) {
            #if 0
                if (alarm_up_mode == ALARM_UP_1_5)
                {
                   led7_var.bShowBuff[1] |= LED_NUMBER[1];
                   led7_var.bShowBuff[2] |= LEDSEG__;
                   led7_var.bShowBuff[3] |= LED_NUMBER[5];
                   //led7_var.bShowBuff[5] = (wday_icon[WEEKDAY_1]|wday_icon[WEEKDAY_2]|wday_icon[WEEKDAY_3]|wday_icon[WEEKDAY_4]|wday_icon[WEEKDAY_5]);
                }
                else if (alarm_up_mode == ALARM_UP_1_7)
                {
                   led7_var.bShowBuff[1] |= LED_NUMBER[1];
                   led7_var.bShowBuff[2] |= LEDSEG__;
                   led7_var.bShowBuff[3] |= LED_NUMBER[7];
                   //led7_var.bShowBuff[5] = (wday_icon[WEEKDAY_1]|wday_icon[WEEKDAY_2]|wday_icon[WEEKDAY_3]|wday_icon[WEEKDAY_4]|wday_icon[WEEKDAY_5]|wday_icon[WEEKDAY_6]|wday_icon[WEEKDAY_7]);
                }
                else //if (alarm_up_mode == ALARM_UP_6_7)
                {
                   led7_var.bShowBuff[1] |= LED_NUMBER[6];
                   led7_var.bShowBuff[2] |= LEDSEG__;
                   led7_var.bShowBuff[3] |= LED_NUMBER[7];
                   //led7_var.bShowBuff[5] = (wday_icon[WEEKDAY_6]|wday_icon[WEEKDAY_7]);
                }
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(2) | BIT(3);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[1] &= 0x80;
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				        //led7_var.bShowBuff[5] &= 0x80;
				    }
				}
            #endif
            }
            //if (rtc_var->alarm_set.alarm_sw)
			{
                led7_var.bShowBuff[5] |= ICON_ALM1;
            }
        } else {
            //coordinate = rtc_var->alarm_set.coordinate;
            ui_time = rtc_var->alarm_set.curr_alm_time;
            printf("led7_show_alarm %d\n", rtc_var->rtc_set_mode);
    		hour_temp = ui_time->bHour;
        	if (time_format_flag == FORMAT_12)
        	{
        	    if (hour_temp > 12)
        	    {
        	        hour_temp -= 12;
        	    }
        		else if (hour_temp == 0)
        		{
        			hour_temp = 12;
        		}
        	}
            //itoa2(hour_temp);
            //led7_show_string((u8 *)bcd_number);
            //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
            //    led7_var.bShowBuff[0] = 0;
            //itoa2(ui_time->bMin);
            //led7_show_string((u8 *)bcd_number);
    		//if (hour_temp/10)
                led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
            led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
            led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
            led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
			if (time_format_flag == FORMAT_12)
			{
			    if (ui_time->bHour >= 12)
			    {
			        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM1_PM;
			    }
			}
            led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM1;//LED_STATUS |= LED_2POINT;
            //led7_var.bShowBuff[2] |= ICON_DOT_UP;
            //led7_var.bShowBuff[3] |= ICON_DOT_DN;
            if (rtc_var->alarm_set.alarm_sw)
			{
                led7_var.bShowBuff[5] |= ICON_ALM1;//LED_STATUS |= ICON_ALM1;
				//if (get_sys_halfsec())
				//{
				//    if (rtc_var->alarm_set.alarm_flag)
                //        led7_var.bShowBuff[5] &=~ ICON_ALM1;;//LED_STATUS &=~ ICON_ALM1;
				//}
				
            }
        }


        /*Alarm info - Switch On/Off*/
        //if (rtc_var->alarm_set.alarm_sw) {
        //    LED_STATUS |= ICON_ALM1;
        //} else {
            //LED_STATUS |= LED_PAUSE;
        //}
    }
    if ((rtc_var->rtc_set_mode == ALM_SET_MODE)&&((coordinate == ALARM_RING_TYPE)||(coordinate == ALARM_RING_VOLUME)))
    {
    }
    else
    {
        //led7_show_RTC_temp();
    }
    led7_show_alm2_time();
}
void led7_show_alm_time(void)
{
#if 0
    RTC_TIME *ui_time;
	u8 hour_temp,coordinate;
	ui_time = rtc_set.alarm_set.curr_alm_time;
	hour_temp = ui_time->bHour;
	if (rtc_set.alarm_set.alarm_sw)
	{
	if (time_format_flag == FORMAT_12)
	{
		if (hour_temp > 12)
		{
			hour_temp -= 12;
		}
		else if (hour_temp == 0)
		{
			hour_temp = 12;
		}
	}
	if (hour_temp/10)
        led7_var.bShowBuff[4] |= LED_NUMBER[hour_temp/10];
    led7_var.bShowBuff[5] |= LED_NUMBER[hour_temp%10];
    led7_var.bShowBuff[6] |= LED_NUMBER[ui_time->bMin/10];
    led7_var.bShowBuff[7] |= LED_NUMBER[ui_time->bMin%10];
	led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM1;
	if (time_format_flag == FORMAT_12)
	{
	    if (ui_time->bHour >= 12)
	    {
	        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM1_PM;
	    }
	}
    if (rtc_set.alarm_set.alarm_sw)
	{
        led7_var.bShowBuff[5] |= ICON_ALM1;//LED_STATUS |= ICON_ALM1;		
	    if (alm_times)
	    {
	        if (get_sys_halfsec())
	        {
                led7_var.bShowBuff[5] &=~ ICON_ALM1;//LED_STATUS &=~ ICON_ALM1;
	        }
	    }
    }
    }
    else
    {
        led7_var.bShowBuff[5] |= LED_LARGE_LETTER['O'-'A'];
        led7_var.bShowBuff[6] |= LED_LARGE_LETTER['F'-'A'];
        led7_var.bShowBuff[7] |= LED_LARGE_LETTER['F'-'A'];
    }
#endif
}
void led7_show_alarm2_temp(void)
{
    led7_show_RTC_temp();
    led7_show_alm_time();
    led7_show_alm2_time();
	if (rtc_set.alarm_set.alarm2_sw)
	{
		led7_var.bShowBuff[5] |= ICON_ALM2;//LED_STATUS |= ICON_ALM2;
		if (get_sys_halfsec())
		{
			if (rtc_set.alarm_set.alarm2_flag)
				led7_var.bShowBuff[5] &=~ ICON_ALM2;;//LED_STATUS &=~ ICON_ALM2;
		}
		
	}
}

void led7_show_alarm2(void)
{
    RTC_SETTING *rtc_var;
    RTC_TIME *ui_time;
	u8 hour_temp,coordinate;
	u8 fm_station_temp=0;
    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if (rtc_var) {
        coordinate = rtc_var->alarm_set.coordinate;
        if (rtc_var->rtc_set_mode == ALM2_SET_MODE) {
            //coordinate = rtc_var_temp->alarm_set.coordinate;
            ui_time = &rtc_time_set_temp;
            printf("led7_show_alarm2 %d\n", rtc_var->rtc_set_mode);
    		hour_temp = ui_time->bHour;
        	if (time_format_flag == FORMAT_12)
        	{
        	    if (hour_temp > 12)
        	    {
        	        hour_temp -= 12;
        	    }
        		else if (hour_temp == 0)
        		{
        			hour_temp = 12;
        		}
        	}
		#if 0
            if (coordinate == RTC_YEAR_SETTING) {
                itoa4(ui_time->dYear);
                led7_show_string((u8 *)bcd_number);
                led7_var.bFlashChar = BIT(0) | BIT(1) | BIT(2) | BIT(3);
            } else if (coordinate == RTC_MONTH_SETTING) {
                itoa2(ui_time->bMonth);
                led7_show_string((u8 *)bcd_number);
                itoa2(ui_time->bDay);
                led7_show_string((u8 *)bcd_number);
                led7_var.bFlashChar = BIT(0) | BIT(1);
            } else if (coordinate == RTC_DAT_SETTING) {
                itoa2(ui_time->bMonth);
                led7_show_string((u8 *)bcd_number);
                itoa2(ui_time->bDay);
                led7_show_string((u8 *)bcd_number);
                led7_var.bFlashChar = BIT(2) | BIT(3);
		#endif
            if (coordinate == ALARM_RING_TYPE) {
				if (alarm2_ring_type == ALARM_RING_BELL)
				{
                    //led7_var.bShowBuff[0] |= LED_SMALL_LETTER['b'-'a'];
                    led7_var.bShowBuff[1] |= LED_SMALL_LETTER['b'-'a'];
                    led7_var.bShowBuff[2] |= LED_SMALL_LETTER['b'-'a'];
                    //led7_var.bShowBuff[3] |= LED_NUMBER[2];
                    if (get_sys_halfsec())
                    {
                        led7_var.bShowBuff[0] &= 0x80;
                        led7_var.bShowBuff[1] &= 0x80;
                        led7_var.bShowBuff[2] &= 0x80;
                        led7_var.bShowBuff[3] &= 0x80;
                    }
				}
				else //if (alarm2_ring_type == ALARM_RING_FM)
				{
				    if (alm_fm_station_set_flag == 0)
				    {
                        led7_var.bShowBuff[1] |= LED_SMALL_LETTER['r'-'a'];
                        led7_var.bShowBuff[2] |= LED_LARGE_LETTER['A'-'A'];
                        led7_var.bShowBuff[3] |= LED_SMALL_LETTER['d'-'a'];
                    }
                    else
                    {
                        fm_station_temp = alm_fm_station_temp;
                        led7_var.bShowBuff[1] |= LED_SMALL_LETTER['P'-'A'];
                        led7_var.bShowBuff[2] |= LED_NUMBER[fm_station_temp%100/10];
                        led7_var.bShowBuff[3] |= LED_NUMBER[fm_station_temp%10];
                    }
                    if (get_sys_halfsec())
                    {
                        led7_var.bShowBuff[1] &= 0x80;
                        led7_var.bShowBuff[2] &= 0x80;
                        led7_var.bShowBuff[3] &= 0x80;
                    }
				}
        		//if (hour_temp/10)
                //    led7_var.bShowBuff[8] |= LED_NUMBER[hour_temp/10];
                //led7_var.bShowBuff[9] |= LED_NUMBER[hour_temp%10];
                //led7_var.bShowBuff[10] |= LED_NUMBER[ui_time->bMin/10];
                //led7_var.bShowBuff[11] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM2_PM;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM2;
            } else if (coordinate == ALARM_HOUR_SETTING) {
                //itoa2(hour_temp);
                //led7_show_string((u8 *)bcd_number);
                //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
                //    led7_var.bShowBuff[0] = 0;
                //itoa2(ui_time->bMin);
                //led7_show_string((u8 *)bcd_number);
        		//if (hour_temp/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM2_PM;
				    }
				}
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(0) | BIT(1);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[0] &= 0x80;
				        led7_var.bShowBuff[1] &= 0x80;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM2;//LED_STATUS |= LED_2POINT;
                //led7_var.bShowBuff[2] |= ICON_DOT_UP;
                //led7_var.bShowBuff[3] |= ICON_DOT_DN;
            } else if (coordinate == ALARM_MIN_SETTING) {
                //itoa2(hour_temp);
                //led7_show_string((u8 *)bcd_number);
                //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
                //    led7_var.bShowBuff[0] = 0;
                //itoa2(ui_time->bMin);
                //led7_show_string((u8 *)bcd_number);
        		//if (hour_temp/10)
                    led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
                led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
                led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM2_PM;
				    }
				}
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(2) | BIT(3);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM2;//LED_STATUS |= LED_2POINT;
                //led7_var.bShowBuff[2] |= ICON_DOT_UP;
                //led7_var.bShowBuff[3] |= ICON_DOT_DN;
            } else if (coordinate == ALARM_RING_VOLUME) {
                led7_var.bShowBuff[1] |= LED_LARGE_LETTER['U'-'A'];
                led7_var.bShowBuff[2] |= LED_NUMBER[alarm2_ring_volume/10];
                led7_var.bShowBuff[3] |= LED_NUMBER[alarm2_ring_volume%10];
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(2) | BIT(3);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[1] &= 0x80;
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				    }
				}
        		//if (hour_temp/10)
                //    led7_var.bShowBuff[8] |= LED_NUMBER[hour_temp/10];
                //led7_var.bShowBuff[9] |= LED_NUMBER[hour_temp%10];
                //led7_var.bShowBuff[10] |= LED_NUMBER[ui_time->bMin/10];
                //led7_var.bShowBuff[11] |= LED_NUMBER[ui_time->bMin%10];
				if (time_format_flag == FORMAT_12)
				{
				    if (ui_time->bHour >= 12)
				    {
				        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM2_PM;
				    }
				}
                led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM2;
            } else if (coordinate == ALARM_MODE) {
            #if 0
                if (alarm2_up_mode == ALARM_UP_1_5)
                {
                   led7_var.bShowBuff[1] |= LED_NUMBER[1];
                   led7_var.bShowBuff[2] |= LEDSEG__;
                   led7_var.bShowBuff[3] |= LED_NUMBER[5];
                   //led7_var.bShowBuff[5] = (wday_icon[WEEKDAY_1]|wday_icon[WEEKDAY_2]|wday_icon[WEEKDAY_3]|wday_icon[WEEKDAY_4]|wday_icon[WEEKDAY_5]);
                }
                else if (alarm2_up_mode == ALARM_UP_1_7)
                {
                   led7_var.bShowBuff[1] |= LED_NUMBER[1];
                   led7_var.bShowBuff[2] |= LEDSEG__;
                   led7_var.bShowBuff[3] |= LED_NUMBER[7];
                   //led7_var.bShowBuff[5] = (wday_icon[WEEKDAY_1]|wday_icon[WEEKDAY_2]|wday_icon[WEEKDAY_3]|wday_icon[WEEKDAY_4]|wday_icon[WEEKDAY_5]|wday_icon[WEEKDAY_6]|wday_icon[WEEKDAY_7]);
                }
                else //if (alarm2_up_mode == ALARM_UP_6_7)
                {
                   led7_var.bShowBuff[1] |= LED_NUMBER[6];
                   led7_var.bShowBuff[2] |= LEDSEG__;
                   led7_var.bShowBuff[3] |= LED_NUMBER[7];
                   //led7_var.bShowBuff[5] = (wday_icon[WEEKDAY_6]|wday_icon[WEEKDAY_7]);
                }
				if (!rtc_time_set_cnt)
				{
                	//led7_var.bFlashChar = BIT(2) | BIT(3);
				    if (get_sys_halfsec())
				    {
				        led7_var.bShowBuff[1] &= 0x80;
				        led7_var.bShowBuff[2] &= 0x80;
				        led7_var.bShowBuff[3] &= 0x80;
				        led7_var.bShowBuff[5] &= 0x80;
				    }
				}
			#endif
            }
            //if (rtc_var->alarm_set.alarm2_sw)
			{
                led7_var.bShowBuff[5] |= ICON_ALM2;
            }
        } else {
            //coordinate = rtc_var->alarm_set.coordinate;
            ui_time = rtc_var->alarm_set.curr_alm2_time;
            printf("led7_show_alarm2 %d\n", rtc_var->rtc_set_mode);
    		hour_temp = ui_time->bHour;
        	if (time_format_flag == FORMAT_12)
        	{
        	    if (hour_temp > 12)
        	    {
        	        hour_temp -= 12;
        	    }
        		else if (hour_temp == 0)
        		{
        			hour_temp = 12;
        		}
        	}
            //itoa2(hour_temp);
            //led7_show_string((u8 *)bcd_number);
            //if (led7_var.bShowBuff[0] == LED_NUMBER[0])
            //    led7_var.bShowBuff[0] = 0;
            //itoa2(ui_time->bMin);
            //led7_show_string((u8 *)bcd_number);
    		//if (hour_temp/10)
                led7_var.bShowBuff[0] |= LED_NUMBER[hour_temp/10];
            led7_var.bShowBuff[1] |= LED_NUMBER[hour_temp%10];
            led7_var.bShowBuff[2] |= LED_NUMBER[ui_time->bMin/10];
            led7_var.bShowBuff[3] |= LED_NUMBER[ui_time->bMin%10];
			if (time_format_flag == FORMAT_12)
			{
			    if (ui_time->bHour >= 12)
			    {
			        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM2_PM;
			    }
			}
            led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM2;//LED_STATUS |= LED_2POINT;
            //led7_var.bShowBuff[2] |= ICON_DOT_UP;
            //led7_var.bShowBuff[3] |= ICON_DOT_DN;
            if (rtc_var->alarm_set.alarm2_sw)
			{
                led7_var.bShowBuff[5] |= ICON_ALM2;//LED_STATUS |= ICON_ALM2;
				//if (get_sys_halfsec())
				//{
				//    if (rtc_var->alarm_set.alarm2_flag)
                //        led7_var.bShowBuff[5] &=~ ICON_ALM2;//LED_STATUS &=~ ICON_ALM2;
				//}
				
            }
        }


        /*Alarm info - Switch On/Off*/
        //if (rtc_var->alarm_set.alarm2_sw) {
        //    LED_STATUS |= ICON_ALM2;
        //} else {
            //LED_STATUS |= LED_PAUSE;
        //}
    }
    if ((rtc_var->rtc_set_mode == ALM2_SET_MODE)&&((coordinate == ALARM_RING_TYPE)||(coordinate == ALARM_RING_VOLUME)))
    {
    }
    else
    {
        //led7_show_RTC_temp();
    }
    led7_show_alm_time();
}
void led7_show_alm2_time(void)
{
#if 0
    RTC_TIME *ui_time;
	u8 hour_temp,coordinate;
	ui_time = rtc_set.alarm_set.curr_alm2_time;
	hour_temp = ui_time->bHour;
	if (rtc_set.alarm_set.alarm2_sw)
	{
	if (time_format_flag == FORMAT_12)
	{
		if (hour_temp > 12)
		{
			hour_temp -= 12;
		}
		else if (hour_temp == 0)
		{
			hour_temp = 12;
		}
	}
	if (hour_temp/10)
        led7_var.bShowBuff[8] |= LED_NUMBER[hour_temp/10];
    led7_var.bShowBuff[9] |= LED_NUMBER[hour_temp%10];
    led7_var.bShowBuff[10] |= LED_NUMBER[ui_time->bMin/10];
    led7_var.bShowBuff[11] |= LED_NUMBER[ui_time->bMin%10];
	led7_var.bShowBuff[1] |= LED_2POINT;//LED_2POINT_ALM2;
	if (time_format_flag == FORMAT_12)
	{
	    if (ui_time->bHour >= 12)
	    {
	        led7_var.bShowBuff[6] |= ICON_PM;//ICON_ALM2_PM;
	    }
	}
    if (rtc_set.alarm_set.alarm2_sw)
	{
        led7_var.bShowBuff[5] |= ICON_ALM2;//LED_STATUS |= ICON_ALM2;		
	    if (alm2_times)
	    {
	        if (get_sys_halfsec())
	        {
                led7_var.bShowBuff[5] &=~ ICON_ALM2;//LED_STATUS &=~ ICON_ALM2;
	        }
	    }
    }
    }
    else
    {
        led7_var.bShowBuff[9] |= LED_LARGE_LETTER['O'-'A'];
        led7_var.bShowBuff[10] |= LED_LARGE_LETTER['F'-'A'];
        led7_var.bShowBuff[11] |= LED_LARGE_LETTER['F'-'A'];
    }
#endif
}
void led7_show_alarm_up(void)
{
	if (rtc_set.alarm_set.alarm_flag == ALARM_ON)
	{
	    led7_show_alarm_temp();
	}
	else if (rtc_set.alarm_set.alarm2_flag == ALARM_ON)
	{
	    led7_show_alarm2_temp();
	}
}
#endif
#endif

void led7_show_auto_power_off(void)
{
    //led7_show_string((u8 *)auto_time_string[auto_power_off_type]);
    if (auto_power_off_type == AUTO_TIME_OFF)
    {
        led7_var.bShowBuff[1] |= LED_NUMBER[auto_time_string[auto_power_off_type][1] - '0'];
        led7_var.bShowBuff[2] |= LED_LARGE_LETTER[auto_time_string[auto_power_off_type][2] - 'A'];
        led7_var.bShowBuff[3] |= LED_LARGE_LETTER[auto_time_string[auto_power_off_type][3] - 'A'];
    }
    else
    {
        if (auto_time_string[auto_power_off_type][1] != ' ')
            led7_var.bShowBuff[1] |= LED_NUMBER[auto_time_string[auto_power_off_type][1] - '0'];
        led7_var.bShowBuff[2] |= LED_NUMBER[auto_time_string[auto_power_off_type][2] - '0'];
        if (auto_time_string[auto_power_off_type][3] != ' ')
            led7_var.bShowBuff[3] |= LED_NUMBER[auto_time_string[auto_power_off_type][3] - '0'];
        if (get_sys_halfsec())
        {
        }
        else
        {
            led7_var.bShowBuff[5] |= ICON_SLEEP;
        }
    }
    led7_show_alm_time();
    led7_show_alm2_time();
}

void led7_show_auto_power_off_time(void)
{
    u8 temp;
    temp = auto_power_off_cnt/60 + 1;
    if (auto_power_off_type == AUTO_TIME_OFF)
    {
        led7_var.bShowBuff[3] |= LED_NUMBER[0];
    }
    else
    {
        led7_var.bShowBuff[0] = 0;
        if (temp / 100)
            led7_var.bShowBuff[1] |= LED_NUMBER[temp / 100];
        if ((temp % 100)/10)
            led7_var.bShowBuff[2] |= LED_NUMBER[(temp % 100)/10];
        //if (temp % 10)
            led7_var.bShowBuff[3] |= LED_NUMBER[temp % 10];
    }
    led7_show_alm_time();
    led7_show_alm2_time();
}
void led7_show_auto_power_off_icon(void)
{
    if ((auto_power_off_type != AUTO_TIME_OFF)&&(auto_power_off_cnt))
        led7_var.bShowBuff[5] |= ICON_SLEEP;
}

void led7_show_display_version(void)
{
    led7_var.bShowBuff[1] |= LED_LARGE_LETTER['V'-'A'];
    //led7_var.bShowBuff[1] = LED_NUMBER[0];
    led7_var.bShowBuff[2] |= LED_NUMBER[0];
    led7_var.bShowBuff[3] |= LED_NUMBER[1];
}
void led7_show_display_test(u8 arg)
{
    u8 i;
    if ((arg > MENU_VERSION)&&(arg < MENU_ALL))
    {
        led7_show_display_version();
    }
    switch (arg)
    {
        case MENU_0000:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[0];
			}
			break;
        case MENU_1111:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[1];
			}
			break;
        case MENU_2222:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[2];
			}
			break;
        case MENU_3333:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[3];
			}
			break;
        case MENU_4444:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[4];
			}
			break;
        case MENU_5555:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[5];
			}
			break;
        case MENU_6666:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[6];
			}
			break;
        case MENU_7777:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[7];
			}
			break;
        case MENU_8888:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[8];
			}
			break;
        case MENU_9999:
			for (i=0;i<4;i++)
			{
                led7_var.bShowBuff[i] |= LED_NUMBER[9];
			}
			break;
        case MENU_VERSION:
            led7_show_display_version();
			break;
		#if 0
        case MENU_PM:
			led7_var.bShowBuff[1] |= ICON_PM;
			break;
        case MENU_FM:
            led7_var.bShowBuff[12] |= LED_FM;
			break;
        case MENU_ICON_BT:
            led7_var.bShowBuff[12] |= LED_BT;
			break;
        case MENU_AU:
			led7_var.bShowBuff[7] |= LED_AUX;
			break;
        case MENU_MON:
			led7_var.bShowBuff[5] |= ICON_WDAY_1;
			break;
        case MENU_TUE:
			led7_var.bShowBuff[5] |= ICON_WDAY_2;
			break;
        case MENU_WED:
			led7_var.bShowBuff[5] |= ICON_WDAY_3;
			break;
        case MENU_THU:
			led7_var.bShowBuff[5] |= ICON_WDAY_4;
			break;
        case MENU_FRI:
			led7_var.bShowBuff[5] |= ICON_WDAY_5;
			break;
        case MENU_SAT:
			led7_var.bShowBuff[5] |= ICON_WDAY_6;
			break;
        case MENU_SUN:
			led7_var.bShowBuff[5] |= ICON_WDAY_7;
			break;
        case MENU_A1:
			led7_var.bShowBuff[4] |= ICON_ALM1;
			break;
        case MENU_A2:
			led7_var.bShowBuff[4] |= ICON_ALM2;
			break;
        case MENU_DOT_UP:
			led7_var.bShowBuff[2] |= ICON_DOT_UP;
			break;
        case MENU_DOT_DN:
            led7_var.bShowBuff[3] |= ICON_DOT_DN;
			break;
        case MENU_MHZ:
            led7_var.bShowBuff[4] |= LED_MHZ;
			break;
		#else
        case MENU_2POINT:
            led7_var.bShowBuff[1] |= LED_2POINT;
			break;
        case MENU_MHZ:
            led7_var.bShowBuff[2] |= LED_MHZ;
			break;
        case MENU_CD:
            led7_var.bShowBuff[4] |= LED_CD;
			break;
        case MENU_USB_TEST:
            led7_var.bShowBuff[4] |= LED_USB;
			break;
        case MENU_SD_TEST:
            led7_var.bShowBuff[4] |= LED_SD;
			break;
        case MENU_BT:
            led7_var.bShowBuff[4] |= LED_BT;
			break;
        case MENU_FM:
            led7_var.bShowBuff[5] |= LED_FM;
			break;
        case MENU_AUX:
            led7_var.bShowBuff[5] |= LED_AUX;
			break;
        case MENU_MEMORY:
            led7_var.bShowBuff[5] |= LED_MEMORY;
			break;
        case MENU_AM:
            led7_var.bShowBuff[5] |= LED_AM;
			break;
        case MENU_PM:
            led7_var.bShowBuff[6] |= ICON_PM;
			break;
        case MENU_KHZ:
            led7_var.bShowBuff[6] |= LED_KHZ;
			break;
        case MENU_FOLDER:
            led7_var.bShowBuff[4] |= LED_FOLDER;
			break;
        case MENU_ALL_LOOP:
            led7_var.bShowBuff[4] |= LED_ALL;
			break;
        case MENU_RANDOM:
            led7_var.bShowBuff[4] |= LED_RANDOM;
			break;
        case MENU_ALM1:
            led7_var.bShowBuff[5] |= ICON_ALM1;
			break;
        case MENU_ALM2:
            led7_var.bShowBuff[5] |= ICON_ALM2;
			break;
        case MENU_SLEEP:
            led7_var.bShowBuff[5] |= ICON_SLEEP;
			break;
        case MENU_SNOOZ:
            led7_var.bShowBuff[5] |= ICON_SNOOZ;
			break;
        case MENU_BAT:
            led7_var.bShowBuff[6] |= LED_BAT;
			break;
        case MENU_MHZ_2:
            led7_var.bShowBuff[6] |= LED_MHZ_2;
			break;
		#endif
        case MENU_ALL:
			for (i=0;i<13;i++)
			{
                led7_var.bShowBuff[i] = 0xFF;
			}
			break;
		case MENU_HI:
            led7_var.bShowBuff[1] |= LED_LARGE_LETTER['H'-'A'];
            led7_var.bShowBuff[2] |= LED_LARGE_LETTER['I'-'A'];
		    break;
		case MENU_CLOCK:
            rtc_read_datetime(rtc_set.calendar_set.curr_rtc_time);
            led7_var.bShowBuff[0] |= LED_NUMBER[rtc_set.calendar_set.curr_rtc_time->bHour/10];
            led7_var.bShowBuff[1] |= LED_NUMBER[rtc_set.calendar_set.curr_rtc_time->bHour%10];
            led7_var.bShowBuff[2] |= LED_NUMBER[rtc_set.calendar_set.curr_rtc_time->bMin/10];
            led7_var.bShowBuff[3] |= LED_NUMBER[rtc_set.calendar_set.curr_rtc_time->bMin%10];
            led7_var.bShowBuff[1] |= LED_2POINT;
            //led7_var.bShowBuff[2] |= ICON_DOT_UP;
            //led7_var.bShowBuff[3] |= ICON_DOT_DN;
		    break;
    }
}

void led7_show_display_reset(void)
{
    led7_var.bShowBuff[1] |= LED_SMALL_LETTER['r'-'a'];
    led7_var.bShowBuff[2] |= LED_LARGE_LETTER['E'-'A'];
    led7_var.bShowBuff[3] |= LED_LARGE_LETTER['S'-'A'];
}

void led7_show_display_all(void)
{
    u8 i;
    for (i=0;i<7;i++)
    {
        led7_var.bShowBuff[i] = 0xFF;
    }
}

#if  REC_EN
/*----------------------------------------------------------------------------*/
/**@brief   REC 显示界面
   @param   void
   @return  void
   @note    void led7_show_rec_start(void)
*/
/*----------------------------------------------------------------------------*/
static void led7_show_rec_start(RECORD_OP_API *rec_api)
{
    u32 rec_time;

    if (rec_api) {
        rec_time = rec_get_enc_time(rec_api);

        itoa2(rec_time / 60);
        led7_show_string((u8 *)bcd_number);

        itoa2(rec_time % 60);
        led7_show_string((u8 *)bcd_number);

        LED_STATUS |= LED_2POINT; //| LED_PLAY;

        if (ENC_STAR == rec_get_enc_sta(rec_api)) {
            LED_STATUS |= LED_PLAY;
        } else if (ENC_PAUS == rec_get_enc_sta(rec_api)) {
            LED_STATUS |= LED_PAUSE;
        }

        led7_show_dev(rec_get_cur_dev(rec_api));
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   REC 显示界面
   @param   void
   @return  void
   @note    void led7_show_rec_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_rec_main(void)
{
    RECORD_OP_API *rec_var_p;

    LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
    LED_STATUS &= ~LED_2POINT; //| LED_PLAY;
    LED_STATUS &= ~(LED_SD | LED_USB);

    if (UI_var.ui_buf_adr) {
        rec_var_p = *(RECORD_OP_API **)UI_var.ui_buf_adr;
        if (rec_var_p) {
            led7_show_rec_start(rec_var_p);
            return;
        }
    }

    led7_show_string((u8 *)other_string[4]);
}
#endif

#if USE_SHOW_BAT
extern volatile u8 bat_level;
void menu_disp_bat(void)
{
#if USE_SHOW_BAT_1
	led7_var.bShowBuff[6] &=~ LED_BAT;//(LED_BAT | LED_BAT_LOW);
#if DCIN_DECT_IO
	if (dcin_status == true)					//充电电量图标闪烁
	{
		if (bat_level < battery_full)
		{
			if (get_sys_halfsec())//(half_second_flash_flag)
			{
				led7_var.bShowBuff[6] |= LED_BAT;
				//led7_var.bShowBuff[6] &=~ LED_BAT_LOW;
			}
			//else
			{
				//led7_var.bShowBuff[6] &=~ LED_BAT;
			//	led7_var.bShowBuff[6] |= LED_BAT_LOW;
			}
		}
		else
		{
			led7_var.bShowBuff[6] |= LED_BAT;
			//led7_var.bShowBuff[6] &=~ LED_BAT_LOW;
		}
	}
	else
#endif
	{
		//if (bat_level >= battery_full)
		{
		//	led7_var.bShowBuff[6] |= LED_BAT;
			//led7_var.bShowBuff[6] &=~ LED_BAT_LOW;
		}
		if (bat_level >= battery_1)
		{
			//led7_var.bShowBuff[6] &=~ LED_BAT;
			led7_var.bShowBuff[6] |= LED_BAT;//LED_BAT_LOW;
		}
		else
		{
			//led7_var.bShowBuff[4] &=~ LED_BAT;
			if (get_sys_halfsec())//(half_second_flash_flag)
				led7_var.bShowBuff[6] |= LED_BAT;//LED_BAT_LOW;
			//else
				//led7_var.bShowBuff[6] &=~ LED_BAT_LOW;
		}
	}
#elif USE_SHOW_BAT_2
	static u8 bat_level_cnt;
	led7_var.bShowBuff[4] &=~ (LED_BAT_1 | LED_BAT_2 | LED_BAT_3);
#if DCIN_DECT_IO
	if (dcin_status == true)					//未充电电量图标闪烁
	{
		bat_level_cnt++;
		if (bat_level >= battery_full)
		{
			led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2 | LED_BAT_3;
		}
		else if (bat_level >= battery_3)
		{
			if (bat_level_cnt > 3)
				bat_level_cnt = 0;
			switch( bat_level_cnt )
			{
				case 0:
					led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2;
					//led7_var.bShowBuff[4] &=~ LED_BAT_3;
					break;
				case 1:
					led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2 | LED_BAT_3;
					break;
				default:
					break;
			}
		}
		else if (bat_level >= battery_1)
		{
			if (bat_level_cnt > 2)
				bat_level_cnt = 0;
			switch (bat_level_cnt)
			{
				case 0:
					led7_var.bShowBuff[4] |= LED_BAT_1;
					//led7_var.bShowBuff[4] &=~ ( LED_BAT_2 | LED_BAT_3 );
					break;
				case 1:
					led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2;
					//led7_var.bShowBuff[4] &=~ LED_BAT_3;
					break;
				case 2:
					led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2 | LED_BAT_3;
					break;
				default:
					break;
			}
		}
		else//else if (bat_level >= battery_1)
		{
			if (bat_level_cnt > 1)
				bat_level_cnt = 0;
			switch (bat_level_cnt)
			{
				case 0:
					//led7_var.bShowBuff[4] &=~ ( LED_BAT_1 | LED_BAT_2 | LED_BAT_3 );
					break;
				case 1:
					led7_var.bShowBuff[4] |= LED_BAT_1;
					//led7_var.bShowBuff[4] &=~ ( LED_BAT_2 | LED_BAT_3 );
					break;
				case 2:
					led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2;
					//led7_var.bShowBuff[4] &=~ LED_BAT_3;
					break;
				case 3:
					led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2 | LED_BAT_3;
					break;
				default:
					break;
			}
		}
	}
	else
#endif
	{
		bat_level_cnt = 0;
		if (bat_level >= battery_full)
		{
			led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2 | LED_BAT_3;
		}
		else if (bat_level >= battery_3)
		{
			led7_var.bShowBuff[4] |= LED_BAT_1 | LED_BAT_2;
			led7_var.bShowBuff[4] &=~ LED_BAT_3;
		}
		else if (bat_level >= battery_1)
		{
			led7_var.bShowBuff[4] |= LED_BAT_1;
			led7_var.bShowBuff[4] &=~ (LED_BAT_2 | LED_BAT_3);
		}
		else//小于3.4V开始闪烁//else if( bat_level >= battery_1 )
		{
			//if (half_second_flash_flag)
			//	led7_var.bShowBuff[5] |= LED_BAT_1;
		}
		//else
		//{
		//	;						//3个图标都不亮
		//}
	}
#endif
}
#endif
#if 0
void LED_drive7_temp(void)
{
    u8 k, i, j, temp;
    k = 0;
#if 0
    //led7_var.bShowBuff[0]=0xff;
    //led7_var.bShowBuff[1]=0xff;
    //led7_var.bShowBuff[2]=0xff;
    //led7_var.bShowBuff[3]=0xff;
    //led7_var.bShowBuff[4]=0xff;

    led7_var.bShowBuff1[0] = 0;
    led7_var.bShowBuff1[1] = 0;
    led7_var.bShowBuff1[2] = 0;
    led7_var.bShowBuff1[3] = 0;
    led7_var.bShowBuff1[4] = 0;
    led7_var.bShowBuff1[5] = 0;
    led7_var.bShowBuff1[6] = 0;
#endif
    for (i = 0; i < 5; i++) {
        temp = led7_var.bShowBuff[i];
        if (get_sys_halfsec()) {
            if ((led7_var.bFlashIcon) && (i == 4)) {
                temp = LED_STATUS & (~led7_var.bFlashIcon);
            } else if (led7_var.bFlashChar & BIT(i)) {
                temp = 0x0;
            }
        }

        for (j = 0; j < 7; j++) {
            if (temp & bit_table[j]) {
                led7_var.bShowBuff1[led_7[k][0]] |= bit_table[led_7[k][1]];
            }
            k++;
        }
        if (j == 7) {
            if (temp & bit_table[j]) {
			#if THEME_LEDSEG_7PIN
                led7_var.bShowBuff1[led_7[34][0]] |= bit_table[led_7[34][1]];
			#else
                led7_var.bShowBuff1[led_7[35][0]] |= bit_table[led_7[35][1]];
			#endif
            }
        }
    }
}

void LED_drive7(void)
{
    led7_var.bShowBuff1[0] = 0;
    led7_var.bShowBuff1[1] = 0;
    led7_var.bShowBuff1[2] = 0;
    led7_var.bShowBuff1[3] = 0;
    led7_var.bShowBuff1[4] = 0;
    led7_var.bShowBuff1[5] = 0;
    led7_var.bShowBuff1[6] = 0;
#if 0//USE_MUTE_FLASH
	if (mute_flag)
	{
		if (get_sys_halfsec())
		{
			LED_drive7_temp();
		}
	}
	else
#endif
	{
		LED_drive7_temp();
	}
}
#endif
/*----------------------------------------------------------------------------*/
/**@brief   LED清屏函数
   @param   x：显示横坐标
   @return  void
   @author  Change.tsai
   @note    void led7_clear(void)
*/
/*----------------------------------------------------------------------------*/
void led7_clear(void)
{
#if 0
#if 0
    LEDN_PORT_OUT &= ~(BIT(LEDN_S0_BIT) | BIT(LEDN_S1_BIT) | BIT(LEDN_S2_BIT) | BIT(LEDN_S3_BIT) | BIT(LEDN_S4_BIT) | BIT(LEDN_S5_BIT) | BIT(LEDN_S6_BIT));
    LEDN_PORT_DIR |= (BIT(LEDN_S0_BIT) | BIT(LEDN_S1_BIT) | BIT(LEDN_S2_BIT) | BIT(LEDN_S3_BIT) | BIT(LEDN_S4_BIT) | BIT(LEDN_S5_BIT) | BIT(LEDN_S6_BIT));
    LEDN_PORT_PU  &= ~(BIT(LEDN_S0_BIT) | BIT(LEDN_S1_BIT) | BIT(LEDN_S2_BIT) | BIT(LEDN_S3_BIT) | BIT(LEDN_S4_BIT) | BIT(LEDN_S5_BIT) | BIT(LEDN_S6_BIT));
    LEDN_PORT_PD  &= ~(BIT(LEDN_S0_BIT) | BIT(LEDN_S1_BIT) | BIT(LEDN_S2_BIT) | BIT(LEDN_S3_BIT) | BIT(LEDN_S4_BIT) | BIT(LEDN_S5_BIT) | BIT(LEDN_S6_BIT));
    LEDN_PORT_HD  &= ~(BIT(LEDN_S0_BIT) | BIT(LEDN_S1_BIT) | BIT(LEDN_S2_BIT) | BIT(LEDN_S3_BIT) | BIT(LEDN_S4_BIT) | BIT(LEDN_S5_BIT) | BIT(LEDN_S6_BIT));
    LEDN_PORT_HD1  &= ~(BIT(LEDN_S0_BIT) | BIT(LEDN_S1_BIT) | BIT(LEDN_S2_BIT) | BIT(LEDN_S3_BIT) | BIT(LEDN_S4_BIT) | BIT(LEDN_S5_BIT) | BIT(LEDN_S6_BIT));
#else
    LEDN_PORT0_OUT &=~ BIT(LEDN_S0_BIT);
    LEDN_PORT0_DIR |= BIT(LEDN_S0_BIT);
    LEDN_PORT0_PU &=~ BIT(LEDN_S0_BIT);
    LEDN_PORT0_PD &=~ BIT(LEDN_S0_BIT);
    LEDN_PORT0_HD &=~ BIT(LEDN_S0_BIT);
    LEDN_PORT0_HD1 &=~ BIT(LEDN_S0_BIT);

    LEDN_PORT1_OUT &=~ BIT(LEDN_S1_BIT);
    LEDN_PORT1_DIR |= BIT(LEDN_S1_BIT);
    LEDN_PORT1_PU &=~ BIT(LEDN_S1_BIT);
    LEDN_PORT1_PD &=~ BIT(LEDN_S1_BIT);
    LEDN_PORT1_HD &=~ BIT(LEDN_S1_BIT);
    LEDN_PORT1_HD1 &=~ BIT(LEDN_S1_BIT);

    LEDN_PORT2_OUT &=~ BIT(LEDN_S2_BIT);
    LEDN_PORT2_DIR |= BIT(LEDN_S2_BIT);
    LEDN_PORT2_PU &=~ BIT(LEDN_S2_BIT);
    LEDN_PORT2_PD &=~ BIT(LEDN_S2_BIT);
    LEDN_PORT2_HD &=~ BIT(LEDN_S2_BIT);
    LEDN_PORT2_HD1 &=~ BIT(LEDN_S2_BIT);

    LEDN_PORT3_OUT &=~ BIT(LEDN_S3_BIT);
    LEDN_PORT3_DIR |= BIT(LEDN_S3_BIT);
    LEDN_PORT3_PU &=~ BIT(LEDN_S3_BIT);
    LEDN_PORT3_PD &=~ BIT(LEDN_S3_BIT);
    LEDN_PORT3_HD &=~ BIT(LEDN_S3_BIT);
    LEDN_PORT3_HD1 &=~ BIT(LEDN_S3_BIT);

    LEDN_PORT4_OUT &=~ BIT(LEDN_S4_BIT);
    LEDN_PORT4_DIR |= BIT(LEDN_S4_BIT);
    LEDN_PORT4_PU &=~ BIT(LEDN_S4_BIT);
    LEDN_PORT4_PD &=~ BIT(LEDN_S4_BIT);
    LEDN_PORT4_HD &=~ BIT(LEDN_S4_BIT);
    LEDN_PORT4_HD1 &=~ BIT(LEDN_S4_BIT);

    LEDN_PORT5_OUT &=~ BIT(LEDN_S5_BIT);
    LEDN_PORT5_DIR |= BIT(LEDN_S5_BIT);
    LEDN_PORT5_PU &=~ BIT(LEDN_S5_BIT);
    LEDN_PORT5_PD &=~ BIT(LEDN_S5_BIT);
    LEDN_PORT5_HD &=~ BIT(LEDN_S5_BIT);
    LEDN_PORT5_HD1 &=~ BIT(LEDN_S5_BIT);

    LEDN_PORT6_OUT &=~ BIT(LEDN_S6_BIT);
    LEDN_PORT6_DIR |= BIT(LEDN_S6_BIT);
    LEDN_PORT6_PU &=~ BIT(LEDN_S6_BIT);
    LEDN_PORT6_PD &=~ BIT(LEDN_S6_BIT);
    LEDN_PORT6_HD &=~ BIT(LEDN_S6_BIT);
    LEDN_PORT6_HD1 &=~ BIT(LEDN_S6_BIT);
#endif
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief   LED扫描函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_scan(void *param)
*/
/*----------------------------------------------------------------------------*/
void led7_scan(void *param)
{
#if 0
    static u8 cnt;
    u8 seg;

    LED_drive7();

    seg = led7_var.bShowBuff1[cnt];

    led7_clear();

    switch (cnt) {
    case 0:
        LEDN_PORT0_OUT |=  BIT(LEDN_S0_BIT);
        LEDN_PORT0_HD  |=  BIT(LEDN_S0_BIT);
        LEDN_PORT0_HD1 |=  BIT(LEDN_S0_BIT);
        LEDN_PORT0_DIR &= ~BIT(LEDN_S0_BIT);
        break;

    case 1:
        LEDN_PORT1_OUT |=  BIT(LEDN_S1_BIT);
        LEDN_PORT1_HD  |=  BIT(LEDN_S1_BIT);
        LEDN_PORT1_HD1 |=  BIT(LEDN_S1_BIT);
        LEDN_PORT1_DIR &= ~BIT(LEDN_S1_BIT);
        break;

    case 2:
        LEDN_PORT2_OUT |=  BIT(LEDN_S2_BIT);
        LEDN_PORT2_HD  |=  BIT(LEDN_S2_BIT);
        LEDN_PORT2_HD1 |=  BIT(LEDN_S2_BIT);
        LEDN_PORT2_DIR &= ~BIT(LEDN_S2_BIT);
        break;

    case 3:
        LEDN_PORT3_OUT |=  BIT(LEDN_S3_BIT);
        LEDN_PORT3_HD  |=  BIT(LEDN_S3_BIT);
        LEDN_PORT3_HD1 |=  BIT(LEDN_S3_BIT);
        LEDN_PORT3_DIR &= ~BIT(LEDN_S3_BIT);
        break;

    case 4:
        LEDN_PORT4_OUT |=  BIT(LEDN_S4_BIT);
        LEDN_PORT4_HD  |=  BIT(LEDN_S4_BIT);
        LEDN_PORT4_HD1 |=  BIT(LEDN_S4_BIT);
        LEDN_PORT4_DIR &= ~BIT(LEDN_S4_BIT);
        break;

    case 5:
        LEDN_PORT5_OUT |=  BIT(LEDN_S5_BIT);
        LEDN_PORT5_HD  |=  BIT(LEDN_S5_BIT);
        LEDN_PORT5_HD1 |=  BIT(LEDN_S5_BIT);
        LEDN_PORT5_DIR &= ~BIT(LEDN_S5_BIT);
        break;

    case 6:
        LEDN_PORT6_OUT |=  BIT(LEDN_S6_BIT);
        LEDN_PORT6_HD  |=  BIT(LEDN_S6_BIT);
        LEDN_PORT6_HD1 |=  BIT(LEDN_S6_BIT);
        LEDN_PORT6_DIR &= ~BIT(LEDN_S6_BIT);
        break;

    default :
        break;
    }

    if (seg & BIT(0)) {
        LEDN_PORT0_OUT &= ~BIT(LEDN_S0_BIT);
        LEDN_PORT0_DIR &= ~BIT(LEDN_S0_BIT);
    }

    if (seg & BIT(1)) {
        LEDN_PORT1_OUT &= ~BIT(LEDN_S1_BIT);
        LEDN_PORT1_DIR &= ~BIT(LEDN_S1_BIT);
    }

    if (seg & BIT(2)) {
        LEDN_PORT2_OUT &= ~BIT(LEDN_S2_BIT);
        LEDN_PORT2_DIR &= ~BIT(LEDN_S2_BIT);
    }

    if (seg & BIT(3)) {
        LEDN_PORT3_OUT &= ~BIT(LEDN_S3_BIT);
        LEDN_PORT3_DIR &= ~BIT(LEDN_S3_BIT);
    }

    if (seg & BIT(4)) {
        LEDN_PORT4_OUT &= ~BIT(LEDN_S4_BIT);
        LEDN_PORT4_DIR &= ~BIT(LEDN_S4_BIT);
    }

    if (seg & BIT(5)) {
        LEDN_PORT5_OUT &= ~BIT(LEDN_S5_BIT);
        LEDN_PORT5_DIR &= ~BIT(LEDN_S5_BIT);
    }

    if (seg & BIT(6)) {
        LEDN_PORT6_OUT &= ~BIT(LEDN_S6_BIT);
        LEDN_PORT6_DIR &= ~BIT(LEDN_S6_BIT);
    }
    cnt = (cnt >= 7) ? 0 : cnt + 1;
#endif
}
#endif
#if 0
u8 ht1628displaydate[HT1628_DIS_BUF_NUM];
bool flag_1628key_en=0;
//U8 icon1628[4];
u32 getht1628date=0;
u8 getht1628date1=0;
#define HT1628_DELAY   15//5

const u8 LED7_LEVEL_TABLE[4] =
{
	LIGHT_8,LIGHT_3,LIGHT_1,LIGHT_OFF
};

void SoftDelay_HT1628(u8 n)
{
    while (n--)
        __builtin_pi32_nop();
}

void Ht1628Write(u8 date,u8 num)
{
	u8 loop;
	for(loop = 0; loop < num; loop++)
	{		
		SoftDelay_HT1628(HT1628_DELAY);		
		if(date & 0x01)
		{
			HT1628_DIO_HIGH();
		}
		else
		{
			HT1628_DIO_LOW();
		}
			
        HT1628_CLK_LOW();
		SoftDelay_HT1628(HT1628_DELAY);
        HT1628_CLK_HIGH();		
		SoftDelay_HT1628(HT1628_DELAY);
		date >>= 1;	
	}
	SoftDelay_HT1628(HT1628_DELAY);
}
/*
void Ht1628Read(void)
{
	u8 loop;
	getht1628date = 0;
	getht1628date1 = 0;
        flag_1628key_en=0;
	
	HT1628STBLOW;
	SoftDelay_HT1628(HT1628_DELAY);

	Ht1628Write(HT1628READKEYDATE,8);

	HT1628DATEIN;	
	SoftDelay_HT1628(HT1628_DELAY);

		
	for(loop=0;loop<32;loop++)
	{	
		HT1628CLKLOW;
		getht1628date = getht1628date<<1;	
		SoftDelay_HT1628(HT1628_DELAY);

		if(GET_1628_DATA)			
			getht1628date ++;
		HT1628CLKHIGH;
		SoftDelay_HT1628(HT1628_DELAY);

	}
	
    for(loop=0;loop<8;loop++)
	{	
	    HT1628CLKLOW;
		getht1628date1 = getht1628date1<<1;	
		SoftDelay_HT1628(HT1628_DELAY);

		if(GET_1628_DATA)			
			getht1628date1 ++;
		HT1628CLKHIGH;
		SoftDelay_HT1628(HT1628_DELAY);
			
	}		
	
	SoftDelay_HT1628(HT1628_DELAY);

	HT1628STBHIGH;	
	SoftDelay_HT1628(HT1628_DELAY);

	HT1628DATEOUT;
    flag_1628key_en=1;

}
*/
void Ht1628Send(u8 sta)
{
	u8 i;
    HT1628_STB_HIGH();	
	HT1628_CLK_HIGH();
	HT1628_DIO_HIGH();		
//---------------------------------------	
	SoftDelay_HT1628(HT1628_DELAY);	
	HT1628_STB_LOW();
	Ht1628Write(HT1628_DIS_ADR00,8);	
	SoftDelay_HT1628(HT1628_DELAY);
	for(i=0;i<HT1628_DIS_BUF_NUM;i++)
	{
		Ht1628Write(ht1628displaydate[i],8);
	}		
	SoftDelay_HT1628(HT1628_DELAY);
	HT1628_STB_HIGH();	
//---------------------------------------		
	SoftDelay_HT1628(HT1628_DELAY);
	HT1628_STB_LOW();
	Ht1628Write(sta,8);
	SoftDelay_HT1628(HT1628_DELAY);
	HT1628_STB_HIGH();		
}


//#pragma location="DISP_LED_5C7S_SEG"
//void Ht1628DateUpdate()
//{	
   // Ht1628Send();
//}

void Clear1628_buf(void)
{
	my_memset((u8*)ht1628displaydate,0x00,HT1628_DIS_BUF_NUM);
}

void HT1628_value_set(void)
{
	if ((led7_var.bShowBuff[0] == led7_var.bShowBuff_temp[0])
		&&(led7_var.bShowBuff[1] == led7_var.bShowBuff_temp[1])
		&&(led7_var.bShowBuff[2] == led7_var.bShowBuff_temp[2])
		&&(led7_var.bShowBuff[3] == led7_var.bShowBuff_temp[3])
		&&(led7_var.bShowBuff[4] == led7_var.bShowBuff_temp[4])
		&&(led7_var.bShowBuff[5] == led7_var.bShowBuff_temp[5])
		&&(led7_var.bShowBuff[6] == led7_var.bShowBuff_temp[6])
		)
		return;
	//memcpy(led7_var.bShowBuff_temp,led7_var.bShowBuff,8);
	led7_var.bShowBuff_temp[0] = led7_var.bShowBuff[0];
	led7_var.bShowBuff_temp[1] = led7_var.bShowBuff[1];
	led7_var.bShowBuff_temp[2] = led7_var.bShowBuff[2];
	led7_var.bShowBuff_temp[3] = led7_var.bShowBuff[3];
	led7_var.bShowBuff_temp[4] = led7_var.bShowBuff[4];
	led7_var.bShowBuff_temp[5] = led7_var.bShowBuff[5];
	led7_var.bShowBuff_temp[6] = led7_var.bShowBuff[6];

	my_memset((u8*)ht1628displaydate,0x00,HT1628_DIS_BUF_NUM);
#if 1
    ht1628displaydate[0] = led7_var.bShowBuff_temp[0];
    ht1628displaydate[2] = led7_var.bShowBuff_temp[1];
    ht1628displaydate[4] = led7_var.bShowBuff_temp[2];
    ht1628displaydate[6] = led7_var.bShowBuff_temp[3];
    ht1628displaydate[8] = led7_var.bShowBuff_temp[4];
    ht1628displaydate[10] = led7_var.bShowBuff_temp[5];
    ht1628displaydate[12] = led7_var.bShowBuff_temp[6];
#else
    if(led7_var.bShowBuff[0] & LED_A)     ht1628displaydate[6] |= BIT(0);
	if(led7_var.bShowBuff[0] & LED_B)     ht1628displaydate[6] |= BIT(1);
    if(led7_var.bShowBuff[0] & LED_C)     ht1628displaydate[6] |= BIT(2);
    if(led7_var.bShowBuff[0] & LED_D)     ht1628displaydate[6] |= BIT(3);
    if(led7_var.bShowBuff[0] & LED_E)     ht1628displaydate[6] |= BIT(4);
    if(led7_var.bShowBuff[0] & LED_F)     ht1628displaydate[6] |= BIT(5);
    if(led7_var.bShowBuff[0] & LED_G)     ht1628displaydate[6] |= BIT(6);

    if(led7_var.bShowBuff[1] & LED_A)     ht1628displaydate[8] |= BIT(0);
    if(led7_var.bShowBuff[1] & LED_B)     ht1628displaydate[8] |= BIT(1);
    if(led7_var.bShowBuff[1] & LED_C)     ht1628displaydate[8] |= BIT(2);
    if(led7_var.bShowBuff[1] & LED_D)     ht1628displaydate[8] |= BIT(3);
    if(led7_var.bShowBuff[1] & LED_E)     ht1628displaydate[8] |= BIT(4);
    if(led7_var.bShowBuff[1] & LED_F)     ht1628displaydate[8] |= BIT(5);
    if(led7_var.bShowBuff[1] & LED_G)     ht1628displaydate[8] |= BIT(6);
	
    if(led7_var.bShowBuff[2] & LED_A)     ht1628displaydate[10] |= BIT(0);
    if(led7_var.bShowBuff[2] & LED_B)     ht1628displaydate[10] |= BIT(1);
    if(led7_var.bShowBuff[2] & LED_C)     ht1628displaydate[10] |= BIT(2);
    if(led7_var.bShowBuff[2] & LED_D)     ht1628displaydate[10] |= BIT(3);
    if(led7_var.bShowBuff[2] & LED_E)     ht1628displaydate[10] |= BIT(4);
    if(led7_var.bShowBuff[2] & LED_F)     ht1628displaydate[10] |= BIT(5);
    if(led7_var.bShowBuff[2] & LED_G)     ht1628displaydate[10] |= BIT(6);

    if(led7_var.bShowBuff[3] & LED_A)     ht1628displaydate[12] |= BIT(0);
    if(led7_var.bShowBuff[3] & LED_B)     ht1628displaydate[12] |= BIT(1);
    if(led7_var.bShowBuff[3] & LED_C)     ht1628displaydate[12] |= BIT(2);
    if(led7_var.bShowBuff[3] & LED_D)     ht1628displaydate[12] |= BIT(3);
    if(led7_var.bShowBuff[3] & LED_E)     ht1628displaydate[12] |= BIT(4);
    if(led7_var.bShowBuff[3] & LED_F)     ht1628displaydate[12] |= BIT(5);
    if(led7_var.bShowBuff[3] & LED_G)     ht1628displaydate[12] |= BIT(6);
#endif
	Ht1628Send(LED7_LEVEL_TABLE[led7_display_level]/*HT1628_DISPLAY_ON*/);
}

void set1628Display(void)
{
	Ht1628Send(LED7_LEVEL_TABLE[led7_display_level]/*HT1628_DISPLAY_ON*/);
}

void Clear1628Display(void)
{
	Clear1628_buf();
	Ht1628Write(HT1628_DISPLAY_OFF,8);
}

void HT1628Poweron_init(void)
{
	u8 i;
	Clear1628_buf();
    HT1628_DIO_OUT();
    HT1628_CLK_OUT();
    HT1628_STB_OUT();
    HT1628_DIO_HIGH();
    HT1628_CLK_HIGH();
    HT1628_STB_HIGH();
	SoftDelay_HT1628(HT1628_DELAY);
		
//---------------------------------------
    HT1628_STB_LOW();	
	SoftDelay_HT1628(HT1628_DELAY);
    Ht1628Write(HT1628_DISPLAY_MODE,8);  // HT1628DISPLAYMODE
	SoftDelay_HT1628(HT1628_DELAY);
    HT1628_STB_HIGH();
	
//---------------------------------------	
	SoftDelay_HT1628(HT1628_DELAY);
	HT1628_STB_LOW();
	SoftDelay_HT1628(HT1628_DELAY);
	Ht1628Write(HT1628_WRITE_DATA,8);
	SoftDelay_HT1628(HT1628_DELAY);
	HT1628_STB_HIGH();	
	for(i=0; i<HT1628_DIS_BUF_NUM; i++)
	{
		ht1628displaydate[i] = 0x00;	
	}
//---------------------------------------	
	SoftDelay_HT1628(HT1628_DELAY);	
	HT1628_STB_LOW();
	Ht1628Write(HT1628_DIS_ADR00,8);	
	SoftDelay_HT1628(HT1628_DELAY);
	for(i=0; i<HT1628_DIS_BUF_NUM; i++)
	{
		Ht1628Write(ht1628displaydate[i],8);
	}		
	SoftDelay_HT1628(HT1628_DELAY);
	HT1628_STB_HIGH();	
//---------------------------------------		
	SoftDelay_HT1628(HT1628_DELAY);
	HT1628_STB_LOW();
	Ht1628Write(HT1628_DISPLAY_ON,8);
	SoftDelay_HT1628(HT1628_DELAY);
	HT1628_STB_HIGH();	
}
#else
u8 Ht1621Data[MAX_RAM_NUM];
#define HT1621_DELAY   15//5

void SoftDelay_HT1621(u8 n)
{
    while (n--)
        __builtin_pi32_nop();
}
void SendBit(u8 sdata,u8 cnt)
{
	while(cnt--)              //send cnt bits of sdata highst bit
	{
		HT1621CLKLOW();
        SoftDelay_HT1621(HT1621_DELAY);
		if(sdata & 0x80)
			HT1621DATEHIGH();
		else
			HT1621DATELOW();
		sdata <<= 1;
        SoftDelay_HT1621(HT1621_DELAY);
		HT1621CLKHIGH();
        SoftDelay_HT1621(HT1621_DELAY);
	}

}
void SendCmd(u8 command)
{
	HT1621CSLOW();
    SoftDelay_HT1621(HT1621_DELAY);
	SendBit(CommandID,4);    //Command mode 100 3bit
	SendBit(command,8);      //command data 9 bit + above 1bit
	HT1621CSHIGH();
    SoftDelay_HT1621(HT1621_DELAY);
}

void Ht1621Send(void)
{
    u8 i;
    HT1621CSLOW();
    SoftDelay_HT1621(HT1621_DELAY);
    
	SendBit(DataID,3);     //data mode
	SendBit(StartReg,6);   //register address  A5-A0 SEG0

    for (i = 0; i < MAX_RAM_NUM; i++) 
    { 
        SendBit(Ht1621Data[i], 4);         // 写入数据  
    } 
    HT1621CSHIGH();
}

void Clear1621RAM(void)
{
    u8 i;
    for(i = 0; i < (MAX_RAM_NUM); i++)
        Ht1621Data[i] = 0x00;
}

void Ht1621DateUpdate(void)
{
    u8 i,j;
    for (i=0;i<7;i++)
    {
        if (led7_var.bShowBuff[i] != led7_var.bShowBuff_temp[i])
            break;
    }
    if (i == 7)
        return;
	memcpy(led7_var.bShowBuff_temp,led7_var.bShowBuff,13);

	my_memset(Ht1621Data,0x00,MAX_RAM_NUM);
#if 1
    for (i=0,j=0;i<2;i++,j++)
    {
        if(led7_var.bShowBuff[i] & LED_D)     Ht1621Data[31-2*j] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_E)     Ht1621Data[31-2*j] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_F)     Ht1621Data[31-2*j] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_A)     Ht1621Data[31-2*j] |= BIT(7);
        if(led7_var.bShowBuff[i] & LED_H)     Ht1621Data[30-2*j] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_C)     Ht1621Data[30-2*j] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_G)     Ht1621Data[30-2*j] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_B)     Ht1621Data[30-2*j] |= BIT(7);
    }
    for (i=2,j=0;i<4;i++,j++)
    {
        if(led7_var.bShowBuff[i] & LED_D)     Ht1621Data[22-2*j] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_E)     Ht1621Data[22-2*j] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_F)     Ht1621Data[22-2*j] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_A)     Ht1621Data[22-2*j] |= BIT(7);
        if(led7_var.bShowBuff[i] & LED_H)     Ht1621Data[21-2*j] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_C)     Ht1621Data[21-2*j] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_G)     Ht1621Data[21-2*j] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_B)     Ht1621Data[21-2*j] |= BIT(7);
    }
    for (i=4,j=0;i<6;i++,j++)
    {
        if(led7_var.bShowBuff[i] & LED_D)     Ht1621Data[27-2*j] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_E)     Ht1621Data[27-2*j] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_F)     Ht1621Data[27-2*j] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_A)     Ht1621Data[27-2*j] |= BIT(7);
        if(led7_var.bShowBuff[i] & LED_H)     Ht1621Data[26-2*j] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_C)     Ht1621Data[26-2*j] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_G)     Ht1621Data[26-2*j] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_B)     Ht1621Data[26-2*j] |= BIT(7);
    }
    for (i=6,j=0;i<7;i++,j++)
    {
        if(led7_var.bShowBuff[i] & LED_D)     Ht1621Data[23-2*j] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_E)     Ht1621Data[23-2*j] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_F)     Ht1621Data[23-2*j] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_A)     Ht1621Data[23-2*j] |= BIT(7);
        //if(led7_var.bShowBuff[i] & LED_H)     Ht1621Data[22-i] |= BIT(4);
        //if(led7_var.bShowBuff[i] & LED_C)     Ht1621Data[22-i] |= BIT(5);
        //if(led7_var.bShowBuff[i] & LED_G)     Ht1621Data[22-i] |= BIT(6);
        //if(led7_var.bShowBuff[i] & LED_B)     Ht1621Data[22-i] |= BIT(7);
    }
#else
	if(led7_var.bShowBuff[0] & LED_H)	  Ht1621Data[31] |= BIT(7);
	if(led7_var.bShowBuff[0] & LED_C)	  Ht1621Data[31] |= BIT(6);
	if(led7_var.bShowBuff[0] & LED_G)	  Ht1621Data[31] |= BIT(5);
	if(led7_var.bShowBuff[0] & LED_B)	  Ht1621Data[31] |= BIT(4);

    if(led7_var.bShowBuff[1] & LED_A)     Ht1621Data[29] |= BIT(4);
    if(led7_var.bShowBuff[1] & LED_B)     Ht1621Data[29] |= BIT(5);
    if(led7_var.bShowBuff[1] & LED_C)     Ht1621Data[29] |= BIT(6);
    if(led7_var.bShowBuff[1] & LED_D)     Ht1621Data[29] |= BIT(7);
    if(led7_var.bShowBuff[1] & LED_E)     Ht1621Data[30] |= BIT(6);
    if(led7_var.bShowBuff[1] & LED_F)     Ht1621Data[30] |= BIT(4);
    if(led7_var.bShowBuff[1] & LED_G)     Ht1621Data[30] |= BIT(5);
    if(led7_var.bShowBuff[1] & LED_H)     Ht1621Data[30] |= BIT(7);

    for (i=2;i<4;i++)
    {
        if(led7_var.bShowBuff[i] & LED_A)     Ht1621Data[14-2*i] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_B)     Ht1621Data[14-2*i] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_C)     Ht1621Data[14-2*i] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_D)     Ht1621Data[14-2*i] |= BIT(7);
        if(led7_var.bShowBuff[i] & LED_E)     Ht1621Data[15-2*i] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_F)     Ht1621Data[15-2*i] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_G)     Ht1621Data[15-2*i] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_H)     Ht1621Data[15-2*i] |= BIT(7);
    }

	if(led7_var.bShowBuff[4] & LED_H)	  Ht1621Data[28] |= BIT(7);
	if(led7_var.bShowBuff[4] & LED_C)	  Ht1621Data[28] |= BIT(6);
	if(led7_var.bShowBuff[4] & LED_G)	  Ht1621Data[28] |= BIT(5);
	if(led7_var.bShowBuff[4] & LED_B)	  Ht1621Data[28] |= BIT(4);

    for (i=5;i<8;i++)
    {
        if(led7_var.bShowBuff[i] & LED_A)     Ht1621Data[36-2*i] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_B)     Ht1621Data[36-2*i] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_C)     Ht1621Data[36-2*i] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_D)     Ht1621Data[36-2*i] |= BIT(7);
        if(led7_var.bShowBuff[i] & LED_E)     Ht1621Data[37-2*i] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_F)     Ht1621Data[37-2*i] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_G)     Ht1621Data[37-2*i] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_H)     Ht1621Data[37-2*i] |= BIT(7);
    }

	if(led7_var.bShowBuff[8] & LED_H)	  Ht1621Data[21] |= BIT(7);
	if(led7_var.bShowBuff[8] & LED_C)	  Ht1621Data[21] |= BIT(6);
	if(led7_var.bShowBuff[8] & LED_G)	  Ht1621Data[21] |= BIT(5);
	if(led7_var.bShowBuff[8] & LED_B)	  Ht1621Data[21] |= BIT(4);

    for (i=9;i<12;i++)
    {
        if(led7_var.bShowBuff[i] & LED_A)     Ht1621Data[37-2*i] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_B)     Ht1621Data[37-2*i] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_C)     Ht1621Data[37-2*i] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_D)     Ht1621Data[37-2*i] |= BIT(7);
        if(led7_var.bShowBuff[i] & LED_E)     Ht1621Data[38-2*i] |= BIT(6);
        if(led7_var.bShowBuff[i] & LED_F)     Ht1621Data[38-2*i] |= BIT(4);
        if(led7_var.bShowBuff[i] & LED_G)     Ht1621Data[38-2*i] |= BIT(5);
        if(led7_var.bShowBuff[i] & LED_H)     Ht1621Data[38-2*i] |= BIT(7);
    }

    if(led7_var.bShowBuff[12] & LED_A)     Ht1621Data[12] |= BIT(4);
    if(led7_var.bShowBuff[12] & LED_B)     Ht1621Data[12] |= BIT(5);
    if(led7_var.bShowBuff[12] & LED_C)     Ht1621Data[12] |= BIT(6);
    if(led7_var.bShowBuff[12] & LED_D)     Ht1621Data[12] |= BIT(7);
    if(led7_var.bShowBuff[12] & LED_E)     Ht1621Data[13] |= BIT(6);
    if(led7_var.bShowBuff[12] & LED_F)     Ht1621Data[13] |= BIT(4);
    if(led7_var.bShowBuff[12] & LED_G)     Ht1621Data[13] |= BIT(5);
    if(led7_var.bShowBuff[12] & LED_H)     Ht1621Data[13] |= BIT(7);
#endif
    Ht1621Send();
}

void Clear1621Display(void)
{
    Clear1621RAM();
    Ht1621Send();
}

void HT1621LCDInit(void)
{
    HT1621CSOUT();
    HT1621DATEOUT();
    HT1621CLKOUT();

    HT1621CSHIGH();
    HT1621DATEHIGH();
    HT1621CLKHIGH();
       
	SoftDelay_HT1621(100);
    SendCmd(Bias13Com4); 
    SendCmd(RC256K);                 //使用内部振荡器 
    SendCmd(SysDis); 
    SendCmd(WDTDis); 
    SendCmd(SysEn); 
    SendCmd(LcdOff);	
	SoftDelay_HT1621(100);
    
	Clear1621Display();
	
    SendCmd(LcdOn);
}
void set1628Display(void)
{
}
#endif

