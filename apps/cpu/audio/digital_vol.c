#include "digital_vol.h"
#include "audio/dac.h"
#include "audio/audio.h"


/*----------------------------------------------------------------------------*/
/** @brief:数字音量表，必须通过注册函数注册到库内，
                而且长度必须是DIGITAL_VOL_LIMIT+1
    @param:
    @return:
    @author:Juntham
    @note:
*/
/*----------------------------------------------------------------------------*/
static const u16 digital_vol_tab[MAX_DIGITAL_VOL_L + 1] AT_AUDIO = {
    0	,
    93	,
    111	,
    132	,
    158	,
    189	,
    226	,
    270	,
    323	,
    386	,  //9
    462	,
    552	,
    660	,
    789	,
    943	,
    1127,
    1347,
    1610,
    1925,
    2301,  //19
    2751,
    3288,
    3930,
    4698,
    5616,
    6713,
    8025,
    9592,
    11466,
    15200, //29
    16000, //30
    16384  //31
};
#if (SYS_VOL_EXT == SYS_VOL_EXT_RANGE)
//mode = 1
//step = 30
//gainmin = -45
//vppmax = 2.6
static const u32 ad_vol_tab[31][2] AT_AUDIO = {
    {0, 0    },//0
    {0, 27151},//1
    {0, 32461},//2
    {1, 31185},//3
    {2, 30258},
    {3, 29371},
    {4, 28432},
    {5, 27488},
    {6, 26253},
    {6, 31388},
    {7, 29926},
    {8, 28986},
    {9, 27741},
    {10, 26593},
    {10, 31794},
    {11, 30824},
    {12, 32068},
    {14, 28877},
    {15, 30119},
    {17, 29750},
    {18, 30600},
    {19, 31478},
    {20, 32508},
    {22, 28939},
    {23, 29896},
    {24, 30889},
    {25, 31940},
    {27, 28577},
    {28, 29553},
    {29, 30584},
    {30, 31621}

};
#endif

#if (SYS_VOL_EXT == SYS_VOL_EXT_STEP)
//mode = 2
//step = 30
//gainstep = -1
//vppmax = 2.6
static const u32 ad_vol_tab[31][2] AT_AUDIO = {
    {30, 31621},
    {29, 32590},
    {29, 29046},
    {28, 29907},
    {27, 30816},
    {26, 31731},
    {25, 32710},
    {25, 29153},
    {24, 30042},
    {23, 30983},
    {22, 31958},		//20
    {22, 28483},
    {21, 29450},
    {20, 30386},
    {19, 31353},
    {18, 32477},
    {18, 28945},
    {17, 29987},
    {15, 32350},
    {15, 28832},
    {14, 29456},		//10
    {13, 30156},
    {12, 31066},
    {11, 31818},
    {11, 28358},
    {10, 31169},
    {10, 27780},
    {9,	30880},
    {9,	27522},
    {8,	30643},
    {0,	0}

};
#endif


void digital_vol_init(struct SOUND_VOL *vol)
{
    sound.vol.digital_vol_l 	= vol->digital_vol_l;
    sound.vol.digital_vol_r 	= vol->digital_vol_r;
    sound.vol.max_digit_vol_l 	= vol->max_digit_vol_l;
    sound.vol.max_digit_vol_r 	= vol->max_digit_vol_r;
    sound.digital_fade_vol_l	= sound.vol.digital_vol_l;
    sound.digital_fade_vol_r	= sound.vol.digital_vol_r;
    sound.digital_fade			= D_FADE_NULL;
}


/*----------------------------------------------------------------------------*/
/**@brief   Set Digital volume
   @param   l_vol/r_vol:volume value
			fade_en:fade toggle
   @author
   @note    void set_sys_vol(u32 l_vol, u32 r_vol, u8 fade_en)
 */
/*----------------------------------------------------------------------------*/
void set_digital_vol(u32 vol_l, u32 vol_r)
{
    if (vol_l > sound.vol.max_digit_vol_l) {
        vol_l = sound.vol.max_digit_vol_l;
    }
    if (vol_r > sound.vol.max_digit_vol_r) {
        vol_r = sound.vol.max_digit_vol_r;
    }

    sound.digital_fade	 	= D_FADE_NULL;
    sound.vol.digital_vol_l = vol_l;
    sound.vol.digital_vol_r = vol_r;
}
/*----------------------------------------------------------------------------*/
/**@brief   Set DAC volume EXT API
   @param   l_vol/r_vol:volume value
  			fade_en:fade toggle
   @author
   @note    void set_dac_vol_ext(u32 l_vol, u32 r_vol, u8 fade_en)
 */
/*----------------------------------------------------------------------------*/
#if SYS_VOL_EXT
void set_dac_vol_ext(u32 l_vol, u32 r_vol, u8 fade_en)
{
    set_dac_vol(ad_vol_tab[l_vol][0], ad_vol_tab[r_vol][0], fade_en);
    set_digital_vol(l_vol, r_vol);
}
#endif

/*----------------------------------------------------------------------------*/
/**@brief   digital volume fade control
   @param   fade = D_FADE_IN
				 = D_FADE_OUT
   @author
   @note
 */
/*----------------------------------------------------------------------------*/
void digital_vol_fade(u8 fade)
{
    sound.digital_fade = fade;
}

AT_AUDIO
void digital_vol_ctrl(void *buffer, u32 len, u8 analog_vol)
{
    s32 valuetemp;
    u32 i;

    u16 curtabvol;
    u16 curtabvor;

    s16 *buf = buffer;
    len >>= 1;

    if (sound.digital_fade == D_FADE_NULL) {
        if (sound.digital_fade_vol_l > sound.vol.digital_vol_l) {
            sound.digital_fade_vol_l --;
            //printf("[%d]",sound.digital_fade_vol_l);
        } else if (sound.digital_fade_vol_l < sound.vol.digital_vol_l) {
            sound.digital_fade_vol_l++;
            //printf("<%d>",sound.digital_fade_vol_l);
        }
    } else if (sound.digital_fade == D_FADE_IN) {
        sound.digital_fade_vol_l++;
        if (sound.digital_fade_vol_l >= sound.vol.digital_vol_l) {
            sound.digital_fade_vol_l = sound.vol.digital_vol_l;
            sound.digital_fade = D_FADE_NULL;
        }
    } else if (sound.digital_fade == D_FADE_OUT) {
        if (sound.digital_fade_vol_l) {
            sound.digital_fade_vol_l--;
            if (sound.digital_fade_vol_l == 0) {
                sound.digital_fade = D_FADE_NULL;
            }
        }
    }
    sound.digital_fade_vol_r = sound.digital_fade_vol_l;

#if SYS_VOL_EXT
    curtabvol = ad_vol_tab[sound.digital_fade_vol_l][1];
    curtabvor = ad_vol_tab[sound.digital_fade_vol_r][1];
#else
    if (analog_vol) {
        curtabvol = digital_vol_tab[sound.digital_fade_vol_l];
        curtabvor = digital_vol_tab[sound.digital_fade_vol_r];
    } else {
        curtabvol = 0;
        curtabvor = 0;
    }
#endif

    for (i = 0; i < len; i += 2) {
        ///left channel
        valuetemp = buf[i];
#if SYS_VOL_EXT
        valuetemp = (valuetemp * curtabvol) >> 15 ;
#else
        valuetemp = (valuetemp * curtabvol) >> 14 ;
#endif
        if (valuetemp < -32768) {
            valuetemp = -32768;
        } else if (valuetemp > 32767) {
            valuetemp = 32767;
        }
        buf[i] = (s16)valuetemp;

        ///right channel
        valuetemp = buf[i + 1];
#if SYS_VOL_EXT
        valuetemp = (valuetemp * curtabvor) >> 15 ;
#else
        valuetemp = (valuetemp * curtabvor) >> 14 ;
#endif
        if (valuetemp < -32768) {
            valuetemp = -32768;
        } else if (valuetemp > 32767) {
            valuetemp = 32767;
        }
        buf[i + 1] = (s16)valuetemp;
    }
}

#define VOL_ONE_THIRD	14885	/* 1/3 */
#define VOL_TWO_THIRD	15617	/* 2/3 */
#define VOL_ONE_HALF	15247	/* 1/2 */
#define VOL_FULL		16384	/* 1/1 */
/**
  *	Analog and Digital vol mix ctrl
  *
  */
void ad_vol_mix(void *buffer, u32 len, u8 sys_vol)
{
    s32 valuetemp;
    u32 i;
    s16 *buf = buffer;
    u16 digital_vol;

#if 1	/*0-60*/
    if (sys_vol / 2 == 0) {
        digital_vol = VOL_FULL;/* 1<<14 */
    } else {
        digital_vol = VOL_ONE_HALF;

    }
#else /* 0-62 */
    if (sys_vol < 57) {
        if (sys_vol / 2 == 0) {
            digital_vol = VOL_FULL;/* 1<<14 */
        } else {
            digital_vol = VOL_ONE_HALF;
        }
    } else {
        if ((sys_vol == 57) || (sys_vol == 60)) {
            digital_vol = VOL_ONE_THIRD;
        } else if ((sys_vol == 58) || (sys_vol == 61)) {
            digital_vol = VOL_TWO_THIRD;
        } else {
            digital_vol = VOL_FULL;
        }
    }
#endif

    len >>= 1;
    for (i = 0; i < len; i += 2) {
        ///left channel
        valuetemp = buf[i];
        valuetemp = (valuetemp * digital_vol) >> 14 ;
        if (valuetemp < -32768) {
            valuetemp = -32768;
        } else if (valuetemp > 32767) {
            valuetemp = 32767;
        }
        buf[i] = (s16)valuetemp;

        ///right channel
        valuetemp = buf[i + 1];
        valuetemp = (valuetemp * digital_vol) >> 14 ;
        if (valuetemp < -32768) {
            valuetemp = -32768;
        } else if (valuetemp > 32767) {
            valuetemp = 32767;
        }
        buf[i + 1] = (s16)valuetemp;
    }
}

