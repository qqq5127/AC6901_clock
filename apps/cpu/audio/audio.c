#include "sdk_cfg.h"
#include "audio/audio.h"
#include "audio/dac.h"
#include "audio/dac_api.h"
#include "audio/tone.h"
#include "audio/src.h"
#include "audio/ladc.h"
#include "audio/digital_vol.h"
#include "audio/audio_stream.h"
#include "board_init.h"
#include "string.h"
#include "flash_api.h"
#include "task_manager.h"
#include "common.h"

struct SOUND_VAR sound;
static u8 sound_cbuffer[OUTPUT_BUF_SIZE] 	AT(.dac_buf_sec);	/* dac cbuffer	*/
static u32 output_buf_limit_val = OUTPUT_BUF_LIMIT;

extern void music_decoder_loop_resume(void);

void audio_set_output_buf_limit(u32 limit_val)
{
    output_buf_limit_val = limit_val;
}

static u32 audio_stream_write(AUDIO_STREAM_DAC *obj, void *buf, u32 len)
{
    if (obj == NULL) {
        return 0;
    }
#if 0
    extern u8 debug_open_dac_flag;
    if (debug_open_dac_flag) {
        debug_open_dac_flag++;
        if (debug_open_dac_flag < 10) {
            putchar('L');
            put_buf(buf, 30);

        } else {

            debug_open_dac_flag = 0;
        }
    }
#endif
    u32 wlen;
    u32 limit = cbuf_get_space(&(obj->cbuf)) / 2;
    /* printf("W"); */
    wlen = cbuf_write(&(obj->cbuf), buf, len);
    if (wlen == len) {
        /* music_decoder_loop_resume(); */
    } else {

    }
    if (obj->flag == 0) {
        if (cbuf_get_data_size(&(obj->cbuf)) >= limit) {
            /* log_printf("data enough !!\n"); */
            obj->flag = 1;
        }
    }

    return wlen;
}


static void audio_stream_clear(AUDIO_STREAM_DAC *obj)
{
    /* AUDIO_STREAM_DAC *obj = (AUDIO_STREAM_DAC *)priv; */
    cbuf_clear(&(obj->cbuf));
}


static tbool audio_stream_check(AUDIO_STREAM_DAC *obj)
{
    /* AUDIO_STREAM_DAC *obj = (AUDIO_STREAM_DAC *)priv; */
    if (obj->flag) {
        if (cbuf_get_space(&(obj->cbuf)) - cbuf_get_data_size(&(obj->cbuf)) > DAC_BUF_LEN) {
            return true;
        }
    }

    return false;
}

static u32 audio_stream_free_len(AUDIO_STREAM_DAC *obj)
{
    return (cbuf_get_space(&(obj->cbuf)) - cbuf_get_data_size(&(obj->cbuf)));
}

static u32 audio_stream_data_len(AUDIO_STREAM_DAC *obj)
{
    return cbuf_get_data_size(&(obj->cbuf));
}

static void audio_stream_samplerate(AUDIO_STREAM_DAC *obj, u16 sr, u8 wait)
{
    /* AUDIO_STREAM_DAC *obj = (AUDIO_STREAM_DAC *)priv; */
    dac_set_samplerate(sr, wait);
}

AT_AUDIO
static u32 audio_stream_read(AUDIO_STREAM_DAC *obj, void *buf, u32 len)
{
    u8 is_tws_working = 0;
    /* AUDIO_STREAM_DAC *obj = (AUDIO_STREAM_DAC *)priv; */
    if (obj == NULL) {
        return 0;
    }
    u32 read_len = 0;

#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
    is_tws_working = tws_get_conn_working();
#endif

    if (obj->flag || is_tws_working) { //data enough
        if (cbuf_get_data_size(&(obj->cbuf)) >= len) {
            cbuf_read(&(obj->cbuf), buf, len);
            read_len = len;
        } else {
            /*
            if(0 == get_vm_statu()){
               putchar('#');
            }
            */
            memset(buf, 0x0, len);
            if (cbuf_get_data_size(&(obj->cbuf))) {
                read_len = cbuf_get_data_size(&(obj->cbuf));
                cbuf_read(&(obj->cbuf), buf, read_len);
            }
#if (BT_TWS&BT_TWS_TRANSMIT||BT_TWS&BT_TWS_BROADCAST)
            tws_set_dac_ide();
#endif

            obj->flag = 0;
        }
    }
    if ((0 == get_vm_statu()) && (cbuf_is_write_able(&(obj->cbuf), len) >= output_buf_limit_val)) {

        music_decoder_loop_resume();
    }

    return read_len;
}

static tbool audio_stream_creat(AUDIO_STREAM_DAC *obj)
{
    if (obj == NULL) {
        return false;
    }

    if (obj->buf == NULL) {
        return false;
    }

    cbuf_init(&(obj->cbuf), obj->buf, obj->buf_size);
    obj->flag = 0;

    return true;
}

void audio_stream_destroy(AUDIO_STREAM_DAC *obj)
{

}

AUDIO_STREAM_DAC dac_stream = {
    .buf = 	sound_cbuffer,
    .buf_size = OUTPUT_BUF_SIZE,
};

const AUDIO_STREAM audio_stream_io = {
    .priv 		= &dac_stream,
    .output		= (void *)audio_stream_write,
    .clear 		= (void *)audio_stream_clear,
    .check 		= (void *)audio_stream_check,
    .free_len	= (void *)audio_stream_free_len,
    .data_len 	= (void *)audio_stream_data_len,
    .set_sr 	= (void *)audio_stream_samplerate,
};

u16 get_audio_stream_total_len()
{
    AUDIO_STREAM_DAC *obj = audio_stream_io.priv;
    if (obj) {
        return  cbuf_get_space(&(obj->cbuf));
    }
    return 0;

}
u16 get_audio_stream_dac_len()
{
    if (audio_stream_io.priv) {
        return  audio_stream_io.data_len(audio_stream_io.priv);
    }
    return 0;
}
void clear_audio_stream_clear()
{
    audio_stream_clear(audio_stream_io.priv);
}

REGISTER_SOUND_DRIVERV(dac_drv);
static void audio_init(void)
{

    sound_automute_set(AUTO_MUTE_CFG, 4, 1200, 200);//max_cnt已扩展到u16

#if SYS_DEFAULT_VOL
    sound.vol.sys_vol_l = SYS_DEFAULT_VOL;
#else
    if (vm_read(VM_SYS_VOL, (void *)&sound.vol.sys_vol_l, VM_SYS_VOL_LEN) == VM_SYS_VOL_LEN) {
        log_printf("mem_vol:%d\n", sound.vol.sys_vol_l);
        /* limit system vol */
        if (sound.vol.sys_vol_l < 10) {
            sound.vol.sys_vol_l = 10;
        }
        if (sound.vol.sys_vol_l > MAX_SYS_VOL_L) {
            sound.vol.sys_vol_l = MAX_SYS_VOL_L;
        }
    } else {
        sound.vol.sys_vol_l = 20;
        log_printf("default_vol:%d\n", sound.vol.sys_vol_l);
    }
#endif
	#if USE_16_LEVEL_VOLUME
		u8 i;
		for (i=0; i<=MAX_SYS_VOL_TEMP; i++)
		{
			if (sound.vol.sys_vol_l <= volume_table[i])
			{
				volume_temp = i;
				break;
			}
		}
	#endif
    sound.eq_mode   			= 0;/*如果eq有记忆，这里应该读取记忆eq出来赋值*/
    sound.vol.sys_vol_r    		= sound.vol.sys_vol_l;
    sound.vol.digital_vol_l 	= MAX_DIGITAL_VOL_L;
    sound.vol.digital_vol_r 	= MAX_DIGITAL_VOL_R;
    sound.vol.max_sys_vol_l 	= MAX_SYS_VOL_L;
    sound.vol.max_sys_vol_r 	= MAX_SYS_VOL_R;
    sound.vol.max_digit_vol_l 	= MAX_DIGITAL_VOL_L;
    sound.vol.max_digit_vol_r 	= MAX_DIGITAL_VOL_R;
    sound_buf_init(sound_cbuffer, sizeof(sound_cbuffer));

    digital_vol_init(&sound.vol);
    sin_tone_init();
    if (sound_drv && (strcmp(sound_drv->name, DAC_DRV_NAME) == 0)) {
        dac_init();
    }

    ladc_init();

    audio_stream_creat(&dac_stream);
}
no_sequence_initcall(audio_init);

void sound_automute_set(u8 ctl, u16 packet_cnt, u16 Threshold, u16 max_cnt)
{
#if DAC_AUTO_MUTE_EN
    if (sys_global_value.fast_test_flag == 0x1A) {
        ctl = AUTO_MUTE_DISABLE;
    }
    if ((u8) - 1 != ctl) {
        sound.automute_ctl = ctl;
    }
    /* dac_automute_init(ctl, packet_cnt, Threshold, max_cnt); */
    dac_automute_param_init(ctl, packet_cnt, Threshold, max_cnt);//by wuxu 20190729
#endif
}


//*****************************************************************************
//
//            				SOUND Buffer Operates
//
//*****************************************************************************
void sound_buf_init(void *cbuf, u32 cbuf_size)
{
    if (cbuf) {
        cbuf_init(&sound.cbuf, cbuf, cbuf_size);
    }
}

u32 get_sound_cbuf_len(void)
{
    return sound.cbuf.data_len;
}

AT_AUDIO
u32 audio_read(void *buf, u32 len)
{
    return audio_stream_read(&dac_stream, buf, len);
}


AT_AUDIO
void audio_read_vm(void *buf, u8 flag)
{
    AUDIO_STREAM_DAC *obj = &dac_stream;
    u32 len = DAC_BUF_LEN;

    /* AUDIO_STREAM_DAC *obj = (AUDIO_STREAM_DAC *)priv; */
    if (obj == NULL) {
        return;
    }
    u32 read_len = 0;
    if (obj->flag) { //data enough
        if (cbuf_get_data_size(&(obj->cbuf)) >= len) {
            cbuf_read(&(obj->cbuf), buf, len);
            read_len = len;
        } else {
            memset(buf, 0x0, len);
            if (cbuf_get_data_size(&(obj->cbuf))) {
                read_len = cbuf_get_data_size(&(obj->cbuf));
                cbuf_read(&(obj->cbuf), buf, read_len);
            }
            obj->flag = 0;
        }
    }

    if (read_len == 0) {
        memset(buf, 0x00, len);
    }
}

typedef struct __AD_TO_DA_STREAM {
    cbuffer_t 	cbuf;
    u8 			ram[DAC_BUF_LEN];		//double buffer
    u8 			enable;
} AD_TO_DA_STREAM;

static AD_TO_DA_STREAM ad2da_obj;

void ad2da_init(void)
{
    ad2da_obj.enable = 1;
    cbuf_init(&ad2da_obj.cbuf, ad2da_obj.ram, sizeof(ad2da_obj.ram));
}

s32 ad2da_write(u8 *buf, u32 len)
{
    s32 ret;
    if (ad2da_obj.enable == 0) {
        ret = 0;
    } else {
        ret = cbuf_write(&ad2da_obj.cbuf, buf, len);
        if (len != ret) {
            /* putchar('x'); */
        }
    }
    return ret;
}

s32 ad2da_read(u8 *buf, u32 len)
{
    s32 ret;
    if (ad2da_obj.enable == 0) {
        ret = 0;
    } else {
        ret = cbuf_read(&ad2da_obj.cbuf, buf, len);
        if (len != ret) {
            /* putchar('s'); */
        }
    }
    return ret;
}

