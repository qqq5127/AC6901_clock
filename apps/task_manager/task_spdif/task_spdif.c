#include "msg.h"
#include "task_spdif.h"
#include "task_manager.h"
#include "task_common.h"
#include "audio/dac_api.h"
#include "dac.h"
#include "power_manage_api.h"
#include "dev_manage.h"
#include "warning_tone.h"
#include "music_decoder.h"
#include "string.h"
#include "uart.h"
#include "audio/audio.h"
#include "sdk_cfg.h"
#include "circular_buf.h"
#include "audio_stream.h"
#include "clock.h"
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".system_bss")
#pragma data_seg(	".system_data")
#pragma const_seg(	".system_const")
#pragma code_seg(	".system_code")
#endif

#define TASK_SPDIF_DEBUG_ENABLE

#ifdef TASK_SPDIF_DEBUG_ENABLE
#define task_spdif_printf log_printf
#else
#define task_spdif_printf(...)
#endif// TASK_SPDIF_DEBUG_ENABLE

static u32 spdif_decode_ram[7 * 1024] 			sec_used(.spdif_mem_pool);
static u32 spdif_data_buf[2 * 1024] 			sec_used(.spdif_data_pool);
static u8 spdif_none_linear_step = NONE_LINEAR_STATUS_STOP;
static void *spdif_decoder_init(void *priv, const char *format, void *inbuf);

static const DEC_CFG spdif_dts_decoder_configs[] = {
    ///dts
#if DEC_TYPE_DTS_ENABLE
    {
        get_dts_ops,
        NULL,
        spdif_decode_ram,
        sizeof(spdif_decode_ram),
        1,
        1,
    },
#endif
};

enum {
    SPDIF_DECODER_STOP,
    SPDIF_DECODER_WAITING,
    SPDIF_DECODER_START,
};

struct spdif_decoder_file {
    cbuffer_t cbuf;
    void **decode_hdl;
    volatile int state;
    u32 decode_data_buf_len;
    u32 decode_data_buf_ptr[0];
};
struct spdif_decoder_file *g_spdif_ptr = (struct spdif_decoder_file *)spdif_data_buf;
static u32 spdif_file_read(void *priv, u8 *buf, u16 len)
{
    u16 read_len;
    struct spdif_decoder_file *file = (struct spdif_decoder_file *)priv;
    int data_len;
    if (file->state != SPDIF_DECODER_START) {
        printf("{r_%d}", len);
        return 0;
    }
    while (file->state == SPDIF_DECODER_START) {
        data_len = cbuf_get_data_size(&file->cbuf);
        if (data_len <= len) {
            len = data_len;
            /*数据不够的时候，重新等到指定大小再启动解码，优化卡顿问题*/
            file->state = SPDIF_DECODER_WAITING;
        }
        read_len = cbuf_read(&file->cbuf, buf, len);
        return read_len;
    }
    return len;
}

static bool spdif_file_seek(void *priv, u8 type, u32 offsiz)
{
    return 1;
}
static int spdif_file_write(void *priv, void *data, u32 len)
{
    struct spdif_decoder_file *file = (struct spdif_decoder_file *)priv;
    if (len == 0) {
        return 0;
    }
    if (cbuf_is_write_able(&file->cbuf, len)) {
        cbuf_write(&file->cbuf, data, len);
    } else {
        putchar('R');
    }
    if (file->state == SPDIF_DECODER_WAITING) {
        if (cbuf_get_data_size(&file->cbuf) >= 5 * 1024) {
            file->state = SPDIF_DECODER_START;
            //DTs一定要有数据才能启动
            task_post_msg(NULL, 1, SYS_EVENT_BEGIN_DEC);
            //task_spdif_printf("spdif_start_cbuf %d\n", file->cbuf.data_len);
        }
    }
    return 1;
}
/*使用者自行实现， 主要是作为解码器的输入*/
const AUDIO_FILE spdif_media_file_io = {
    .seek = spdif_file_seek,
    .read = spdif_file_read,
    .get_size = NULL,
};

static void *spdif_decoder_init(void *priv, const char *format, void *inbuf)
{
    int err;
    MUSIC_DECODER *obj = NULL;
    obj = music_decoder_creat();
    if (obj == NULL) {
        task_spdif_printf("spdif player creat fail ！\n");
        return NULL;
    }
    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_SPDIF;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    music_decoder_set_output(obj, audio_stream_init(&stream_param, inbuf));
    music_decoder_set_file_interface(obj, (AUDIO_FILE *)&spdif_media_file_io, priv/*使用者主注册的file_io的私有属性*/);
    music_decoder_set_err_deal_interface(obj, NULL, NULL);

    if (0 == strcmp(format, "DTS")) {
        task_spdif_printf("is dts, fun = %s, line = %d\n",  __func__, __LINE__);
        music_decoder_set_configs(obj, (DEC_CFG *)spdif_dts_decoder_configs, 1);
    } else {
        task_spdif_printf("no support fomart yet!!\n");
        music_decoder_destroy(&obj);
        return NULL;
    }
    sound.vol.sys_vol_l = sound.vol.sys_vol_r = 22;
    //task_spdif_printf("spdif vol = %d, %d\n", sound.vol.sys_vol_l, sound.vol.sys_vol_r);
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
    err = music_decoder_play(obj, NULL);
    if (err != MUSIC_DECODER_ERR_NONE) {
        music_decoder_destroy(&obj);
        task_spdif_printf("music_player_play dec fail %x !! fun = %s, line = %d\n", err, __func__, __LINE__);
        return NULL;
    }
    return (void *)obj;
}
void spdif_media_stop(void **hdl)
{
    puts("spdif_media_stop\n");
    music_decoder_destroy((MUSIC_DECODER **)&hdl);
}
/*用于解码的ram是复用的，所以要有一个资源管理流程*/
static void spdif_stop_resource(void *priv)
{
    task_spdif_printf("spdif_stop_resource\n");
    if (spdif_none_linear_step == NONE_LINEAR_STATUS_STOP) {
        return ;
    }
    struct spdif_decoder_file *file = (struct spdif_decoder_file *)g_spdif_ptr;
    file->state = SPDIF_DECODER_STOP;
    if (file->decode_hdl) {
        spdif_media_stop(file->decode_hdl);
    }

    spdif_none_linear_step = NONE_LINEAR_STATUS_STOP;
}
static void spdif_init_resource(void *priv)
{
    if (priv == NULL) {
        return ;
    }
    task_spdif_printf("spdif_init_resource\n");
    struct spdif_decoder_file *file = (struct spdif_decoder_file *)g_spdif_ptr;
    memset(file, 0, sizeof(*file));
    file->state = SPDIF_DECODER_WAITING;
    file->decode_data_buf_len =	sizeof(spdif_data_buf) - sizeof(struct spdif_decoder_file);
    cbuf_init(&file->cbuf, (u8 *)file->decode_data_buf_ptr, file->decode_data_buf_len);
    spdif_none_linear_step = NONE_LINEAR_STATUS_WORKING;
}
static void spdif_start_decoder()
{
    struct spdif_decoder_file *file = (struct spdif_decoder_file *)g_spdif_ptr;
    if (file->state == SPDIF_DECODER_START) {
        //printf("spdif_start_decoder\n");
        file->decode_hdl = spdif_decoder_init(file, "DTS", &file->cbuf);
    }
}
int get_spdif_none_linear_sta()
{
    return spdif_none_linear_step;
}
int spdif_none_linear_init(u16 burst_info)
{
    spdif_none_linear_step = NONE_LINEAR_STATUS_WAIT_INIT;
    printf("<<<<spdif_codec_init\n");
    mutex_resource_apply("spdif", 1, spdif_init_resource, spdif_stop_resource, g_spdif_ptr);
    return 0;
}

int spdif_none_linear_stop(void *priv)
{
    spdif_none_linear_step = NONE_LINEAR_STATUS_WAIT_STOP;
    mutex_resource_release("spdif");
    return 0;
}
int spdif_data_write(void *data, u32 len)
{
    return  spdif_file_write(g_spdif_ptr, data, len);
}
extern void spdif_open();
extern void spdif_close();
static tbool task_spdif_skip_check(void)
{
    task_spdif_printf("task_spdif_skip_check !!\n");
    return false;
}
static void *task_spdif_init(void *priv)
{
    task_spdif_printf("task_spdif_init !!\n");
    set_sys_freq(MUSIC_DECODE_Hz);
    spdif_open();
    return priv;
}

static void task_spdif_exit(void **hdl)
{
    spdif_close();
    spdif_none_linear_stop(NULL);
    task_spdif_printf("task_spdif_exit !!\n");
    task_clear_all_message();
    set_sys_freq(SYS_Hz);
}

static void task_spdif_deal(void *hdl)
{
    int error = MSG_NO_ERROR;
    int msg = NO_MSG;
    task_spdif_printf("task_spdif_deal !!\n");
    while (1) {
        error = task_get_msg(0, 1, &msg);
        if (task_common_msg_deal(hdl, msg) == false) {
            return ;
        }
        if (NO_MSG == msg) {
            continue;
        }
        //task_spdif_printf("spdif msg = %x\n", msg);
        switch (msg) {
        case MSG_HALF_SECOND:
            //task_spdif_printf("-H-");
            break;

        case SYS_EVENT_DEC_END:
            task_spdif_printf("\nspdif notice SYS_EVENT_DEC_END\n");
            break;

        case SYS_EVENT_PLAY_SEL_END:
            task_spdif_printf("SYS_EVENT_PLAY_TONE_END\n");
            break;
        case SYS_EVENT_BEGIN_DEC:
            spdif_start_decoder();
            break;
        default:
            break;
        }
    }
}

extern const KEY_REG task_spdif_key;
const TASK_APP task_spdif_info = {
    .skip_check = NULL,//task_spdif_skip_check,//
    .init 		= task_spdif_init,
    .exit 		= task_spdif_exit,
    .task 		= task_spdif_deal,
    .key 		= &task_spdif_key,
};

