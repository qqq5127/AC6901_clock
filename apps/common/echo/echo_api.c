#include "sdk_cfg.h"
#include "common/msg.h"
#include "common/common.h"
#include "audio/dac_api.h"
#include "audio/audio.h"
#include "ladc.h"
#include "howlingsupression_api.h"
#include "echo_api.h"
#include "echo_deal.h"
#include "audio/linein_api.h"

#ifdef HAVE_MALLOC
#undef HAVE_MALLOC
#endif

#define ECHO_MAX_DEEP  				(200)	//单位ms，最大深度越大，需要echo_algorithm_ram越多 : 100ms~4.4K		200ms~7.8K	300ms~11K
#define ECHO_INPUT_RAM_SIZE			(1*1024)
#define ECHO_OUTPUT_RAM_SIZE		(2*1024 - 200)
#define ECHO_ALGORITHM_RAM_SIZE		(7900)
#define ECHO_PITCH_BUF_SIZE			(3376)

//echo_ram
#ifdef HAVE_MALLOC
static u8 *echo_input_ram;
static u8 *echo_output_ram;
static u8 *echo_algorithm_ram;
static u8 *echo_pitch_buf;
#else
static u8 echo_input_ram[ECHO_INPUT_RAM_SIZE] ALIGNED(4);
static u8 echo_output_ram[ECHO_OUTPUT_RAM_SIZE] ALIGNED(4);
static u8 echo_algorithm_ram[ECHO_ALGORITHM_RAM_SIZE] ALIGNED(4);
static u8 echo_pitch_buf[ECHO_PITCH_BUF_SIZE] ALIGNED(4);
#endif

// ECHO 效果参数初始化.
ECHO_CONTROL_VAR  echo_var = {
    .music_vol = 13384,           //0 ~ 16384   //背景音乐音量大小. 16384(0 db)
    .mic_vol_digital = 13384,     //0 ~ 16384   //MIC数字音量大小.  16384(0 db)
    .deep = 900,                  //0 ~ 1024    //值越大, 混响深度越深
    .mic_vol = 50,                //0 ~ 63      //MIC模拟音量(放大)大小
    .pitch = -2,                  //-127 ~ 127  //变调, 0为正常声音, 负值越小频率越低, 正值越大频率越高. (howlingsupression)
    .decay = 110,                 //0 ~ 128     //混响强度衰减: 值越大, 混响的后一声比前一声衰减越多.
    .run_flag = 0,
    .first_echo_sel = FIRST_ECHO_ANALOG,
};

// ECHO 效果参数初始化.
REVERB_PARM_SET rev_parm = {
    /*-----------------------------------------------------*/
    /* 以下参数用于调试使用，请勿修改 */
    .reverb_kind					= 0,
    .qualityn						= 1,
    .max_sr							= 16000UL,
    .max_predelay					= 90,	//200ms (analog 90, digital 30)
    .bufcal							= NULL,
    .ef_reverb_parm_2.filter_mode	= 0,
    .ef_reverb_parm_2.lp_para		= {0, 0, 0, 0, 0},
    .sr_out							= 0,
    .rs_enable						= 1,
    .handle_flag					= 3,   //ad sync da
    .ef_reverb_parm.predelay		= 128,
    .ef_reverb_parm.roomscale		= 128,
    .ef_reverb_parm.damping			= 5000,
    .ef_reverb_parm.reverberance	= 8000,
    .ef_reverb_parm.wet_gain		= 256,
    .ef_reverb_parm.dry_gain		= 256,

    /*-----------------------------------------------------*/
    .ef_reverb_parm_2.deepval		= 1024,		/* 混响深度，范围:(0-1024），(1024->max_ms)) */
    .ef_reverb_parm_2.decayval		= 128,		/* 混响强度，范围:(0-128) */
    .ef_reverb_parm_2.gainval		= 4000,		/* 音量增益，范围:(0-4096) */
    .ef_reverb_parm_2.rs_mode		= 0x100,	/* reserved */
    /*-----------------------------------------------------*/
};

void echo_mem_init(void)
{
#ifdef HAVE_MALLOC
    echo_input_ram = malloc(ECHO_INPUT_RAM_SIZE);
    ASSERT(echo_input_ram);

    echo_output_ram = malloc(ECHO_OUTPUT_RAM_SIZE);
    ASSERT(echo_output_ram);

    echo_algorithm_ram = malloc(ECHO_ALGORITHM_RAM_SIZE);
    ASSERT(echo_algorithm_ram);

#if PITCH_EN
    echo_pitch_buf = malloc(ECHO_PITCH_BUF_SIZE);
    ASSERT(echo_pitch_buf);
#endif

#endif
}

void echo_mem_free(void)
{
#ifdef HAVE_MALLOC
    if (echo_input_ram) {
        free(echo_input_ram);
    }

    if (echo_output_ram) {
        free(echo_output_ram);
    }

    if (echo_algorithm_ram) {
        free(echo_algorithm_ram);
    }

#if PITCH_EN
    if (echo_pitch_buf) {
        free(echo_pitch_buf);
    }
#endif
#endif
}

static bool echo_open_flag;

bool echo_open_flag_get(void)
{
    return echo_open_flag;
}

void echo_open_flag_set(bool flag)
{
    echo_open_flag = flag;
}

//app function
s32 echo_reverb_init(void)
{
    s32 ret;

    ret = echo_input_init(echo_input_ram, ECHO_INPUT_RAM_SIZE);
    ASSERT(ret == 0);

    ret = echo_output_init(echo_output_ram, ECHO_OUTPUT_RAM_SIZE);
    ASSERT(ret == 0);

    ret = echo_algorithm_init(&rev_parm, ECHO_MAX_DEEP, echo_algorithm_ram, ECHO_ALGORITHM_RAM_SIZE);
    ASSERT(ret == 0);

    ret = echo_deal_init();
    ASSERT(ret == 0);

    return ret;
}

void echo_audio_ctrl(u8 en)
{
    if (en) {
        u16 samplerate = dac_get_samplerate();
        ladc_ch_open(LADC_MIC_CHANNEL, samplerate); /*open mic_adc*/
        ladc_mic_gain(echo_var.mic_vol, 0);						/*set mic gain*/
        dac_set_samplerate(samplerate, 1);
        if (FIRST_ECHO_ANALOG == echo_var.first_echo_sel) {
            mic_2_dac(1, 1);   //MIC声音
        }
    } else {
        ladc_ch_close(LADC_MIC_CHANNEL);			/*close mic_adc*/
        if (FIRST_ECHO_ANALOG == echo_var.first_echo_sel) {
            mic_2_dac(0, 0);   //MIC声音
        }
    }
}

void echo_dac_adc_sr_sync(void)
{
    if (!echo_var.run_flag) {
        return;
    }
    u16 ladc_sr = ladc_get_samplerate();
    u16 dac_sr = dac_get_samplerate();
    printf("echo_sr_sync, ad_sr = %d, da_sr = %d\n", ladc_sr, dac_sr);
    if (ladc_sr != dac_sr) {
        ladc_set_samplerate(dac_sr);
    }
}

//-128 ~ 127  //变调, 0为正常声音, 负值越小频率越低, 正值越大频率越高
void echo_pitch_set(s8 pitch)
{
    echo_var.pitch = pitch;
    echo_pitch_set_lib(pitch);
}

//0 ~ 1024    //值越大, 混响深度越深
void echo_deep_set(u16 deep)
{
    echo_var.deep = deep;
    echo_deep_set_lib(deep);
}

//0 ~ 128     //混响强度衰减: 值越大, 混响的后一声比前一声衰减越多.
void echo_decay_set(u8 decay)
{
    if (decay > 128) {
        decay = 128;
    }
    echo_var.decay = decay;
    echo_decay_set_lib(128 - echo_var.decay);
}

//0 ~ 63      //MIC模拟音量(放大)大小
void echo_mic_vol_set(u8 vol)
{
    echo_var.mic_vol = vol;
    ladc_mic_gain(vol, 0);						/*set mic gain*/
    if (vol == 0) {
        ladc_mic_mute(1);
    } else {
        ladc_mic_mute(0);
    }
}

//0 ~ 16384   //背景音乐音量大小. //16384 is 0 db
void echo_music_vol_set(u16 vol)
{
    echo_var.music_vol = vol;
    echo_var.music_vol_bk = vol;
}


void echo_output_mix_dac(s16 *buf, u32 len)	//unit:byte
{
    if (echo_var.run_flag == false) {
        return;
    }
    //putchar('X');
    u32 cnt;
    u16 tmp_len = len >> 1;
    u8 echo_1_ram[tmp_len];
    u8 echo_n_ram[tmp_len];
    /*  */
    /* u8 echo_1_ram[64]; */
    /* u8 echo_n_ram[64]; */
    /*  */

    memset(echo_1_ram, 0x00, tmp_len);
    memset(echo_n_ram, 0x00, tmp_len);

    ad2da_read((void *)echo_1_ram, tmp_len);
    echo_output_pop_data((void *)echo_n_ram, tmp_len);

    /* memset(buf, 0x00, len); */
    /* memset(echo_1_ram, 0x00, tmp_len); */
    /* memset(echo_n_ram, 0x00, tmp_len); */
    /*  */
    s32 valuetemp;
    s16 *chl_1, *chl_2, *chl_3;
    chl_1 = (void *)buf;			//stereo_channel
    chl_2 = (void *)echo_1_ram;		//mono_channel
    chl_3 = (void *)echo_n_ram;		//mono_channel
    tmp_len = tmp_len / 2;

    for (cnt = 0; cnt < (tmp_len); cnt++) {
        valuetemp = ((chl_1[cnt * 2] * echo_var.music_vol) >> 14) + chl_2[cnt] + chl_3[cnt];
        if (valuetemp < -32768) {
            valuetemp = -32768;
        } else if (valuetemp > 32767) {
            valuetemp = 32767;
        }
        chl_1[cnt * 2] = valuetemp;

        valuetemp = ((chl_1[cnt * 2 + 1] * echo_var.music_vol) >> 14) + chl_2[cnt] + chl_3[cnt];
        if (valuetemp < -32768) {
            valuetemp = -32768;
        } else if (valuetemp > 32767) {
            valuetemp = 32767;
        }
        chl_1[cnt * 2 + 1] = valuetemp;
    }
}

void echo_input(s16 *buf, u32 len)	//unit:byte
{
    if (!echo_var.run_flag) {
        return;
    }

#if PITCH_EN
    echo_pitch_run((void *)buf, len);	//1channel
#endif
    if (echo_var.first_echo_sel == FIRST_ECHO_DIGITAL) {
        ad2da_write((void *)buf, len);
    }
    echo_input_api((void *)buf, len);
}

void echo_start(void)
{
    if (echo_var.run_flag) {
        puts("echo already running\n");
    } else {
        puts("echo_start\n");

        echo_mem_init();

        echo_reverb_init();  //
        ad2da_init();
#if PITCH_EN
        pitch_on(echo_pitch_buf, echo_var.pitch);
#endif
        echo_audio_ctrl(1);
        echo_var.run_flag = true;
        echo_mic_vol_set(echo_var.mic_vol);
    }
}

void echo_stop(void)
{
    if (echo_var.run_flag) {
        puts("echo_stop\n");
        echo_deal_release();
        echo_audio_ctrl(0);
#if PITCH_EN
        pitch_off();
#endif
        echo_var.run_flag = false;

        echo_mem_free();

    } else {
        puts("echo already stop!!!\n");
    }
}

tbool echo_msg_deal(void *p, u32 msg)
{
    tbool flag = true;

    switch (msg) {
    case MSG_ECHO_START:
        printf("MSG_ECHO_START\n");
        echo_start();
        echo_open_flag_set(true);
        break;

    case MSG_ECHO_STOP:
        printf("MSG_ECHO_STOP\n");
        echo_stop();
        echo_open_flag_set(false);
        break;

    case MSG_ECHO_SW :
        printf("MSG_ECHO_SW\n");

        if (echo_var.run_flag) {
            echo_stop();
            echo_open_flag_set(false);
        } else {
            echo_start();
            echo_open_flag_set(true);
        }

        break;

    case MSG_ECHO_DEEP_SET:
        echo_var.deep = echo_var.deep + (1024 / 10) > 1024  ? 0 : echo_var.deep + (1024 / 10);
        printf("deep = %d\n", echo_var.deep);
        echo_deep_set(echo_var.deep);
        break;

    case MSG_ECHO_PITCH_SET:
        echo_var.pitch = echo_var.pitch + (256 / 10) > 127 ? -127 : echo_var.pitch + (256 / 10);
        printf("pitch = %d\n", echo_var.pitch);
        echo_pitch_set(echo_var.pitch);  //+-128
        break;

    case MSG_ECHO_DECAY_SET:
        echo_var.decay = echo_var.decay + (128 / 10) > 128 ? 0 : echo_var.decay + (128 / 10);
        printf("decay = %d\n", echo_var.decay);
        echo_decay_set(echo_var.decay);
        break;

    case MSG_ECHO_MIC_VOL_SET:
        echo_var.mic_vol = echo_var.mic_vol + (63 / 10) > 63 ? 0 : echo_var.mic_vol + (63 / 10);
        printf("mic_vol = %d\n", echo_var.mic_vol);
        echo_mic_vol_set(echo_var.mic_vol);
        break;

    case MSG_ECHO_MUSIC_VOL_SET:
        echo_var.music_vol = echo_var.music_vol + (16384 / 10) > 16384 ? 0 : echo_var.music_vol + (16384 / 10);
        printf("music_vol = %d\n", echo_var.music_vol);
        echo_music_vol_set(echo_var.music_vol);
        break;

    case MSG_ECHO_MUSIC_VOL_SW:
        printf("music vol switch\n");
        if (echo_var.music_vol) {
            echo_var.music_vol_bk = echo_var.music_vol;
            echo_var.music_vol = 0;
        } else {
            echo_music_vol_set(echo_var.music_vol_bk);
        }
        break;

    default:
        flag = 0;
        break;
    }
    return flag;
}



/*----------------------------------------------------------------------------*/
/** @brief: echo init functions of group
    @param: buf : data buff
    @param: len : data len (unit : byte)
    @return: negative:err code. zero:succ
    @note:
*/
/*----------------------------------------------------------------------------*/
#include "irq_api.h"

/* #define IRQ_SOFT_ECHO_IDX		IRQ_SOFT2_IDX */

IRQ_REGISTER(IRQ_SOFT_ECHO_IDX, echo_soft_run);

s32 echo_deal_init(void)
{
    //init soft echo irq
    IRQ_REQUEST(IRQ_SOFT_ECHO_IDX, echo_soft_run);

    return 0;
}

s32 echo_deal_release(void)
{
    IRQ_RELEASE(IRQ_SOFT_ECHO_IDX);
    return 0;
}

void echo_kick_run(void)
{
    irq_set_pending(IRQ_SOFT_ECHO_IDX);
}

