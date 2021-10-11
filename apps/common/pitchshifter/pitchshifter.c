#include "sdk_cfg.h"
#include "pitchshifter.h"
#include "speed_pitch/ps_for69_api.h"
#include "string.h"

#define PITCHSHIFTER_DEBUG_ENABLE
#ifdef PITCHSHIFTER_DEBUG_ENABLE
#define pitchshifter_printf log_printf
#else
#define pitchshifter_printf(...)
#endif

/*********************************************模块说明*************************************************************/
/*input------>pitchshifter------>output****************************************************************************/
/*input:  AUDIO_STREAM类型， 是该模块提供给上级模块输出使用的,在该模块生效后，上级模块的输出由该模块提供***********/
/*output: AUDIO_STREAM类型， 是上级模块的输出接口, 在该模块生效后，该模块的输出由上级模块提供**********************/
/*
 * 1、pitchshifter是一个可以重入的模块，多个应用同时调用时，只要为该模块分配多个控制句柄即可
 * 2、可以隐式定义的结构体，尽量隐式定义，struct __PITCHSHIFTER这个结构体是在模块c文件中定义的， 在h文件通过typedef
 * 的方式给外部使用，这样的好处是很好地隐藏模块内部处理，用户使用的时候只需要关心如何使用接口，不需要关心接口如何实现，
 * 接口有哪些变量
 */
/******************************************************************************************************************/


typedef struct __PS69_OPS {
    void 				*hdl;
    PS69_API_CONTEXT	*_io;
} PS69_OPS;

///可以隐式定义的结构体，尽量隐式定义，以下这个结构体是在模块c文件中定义的， 在h文件通过typedef的方式给外部使用，这样
///的好处是很好地隐藏模块内部处理，用户使用的时候只需要关心如何使用接口，不需要关心接口如何实现， 接口有哪些变量
struct __PITCHSHIFTER {
    PS69_OPS			ops;
    PS69_CONTEXT_CONF	config;
    AUDIO_STREAM		input;
    AUDIO_STREAM		*output;
    /* PS69_audio_IO 		ps_output; */
    /* OS_MUTEX		 	mutex;		 */
    volatile u8			set_flag;
};


#if SPEED_PITCH_EN

static u8 pitchshifter_buf[5084] sec(.dac_buf_sec) __attribute__((aligned(4)));
static PITCHSHIFTER *pitchshifter_obj = NULL;

/*----------------------------------------------------------------------------*/
/** @brief: 模块数据写入接口，主要是提供给上一级流控作为输出使用
    @param: obj 模块控制句柄
    @param: buf 写入数据基地址
    @param: len 写入数据长度
    @return: 真正写入模块处理的数据长度，这个长度是要反馈给上级流控
    @note:
*/
/*----------------------------------------------------------------------------*/
static u32 pitchshifter_stream_write(PITCHSHIFTER *obj, void *buf, u32 len)
{
    if (obj == NULL) {
        return 0;
    }

    /* os_mutex_pend(&obj->mutex, 0); */
    if (obj->set_flag == 1) {
        obj->ops._io->dconfig(obj->ops.hdl, &(obj->config));
        obj->set_flag = 0;
    }
    /* os_mutex_post(&obj->mutex); */

    u32 out_len;
    out_len = obj->ops._io->run(obj->ops.hdl, buf, len);//确保run的返回值是已使用了多少数据
    return out_len;
}


/*----------------------------------------------------------------------------*/
/** @brief: 模块采样率设置
    @param: obj 模块控制句柄
    @param: sr 要设置的采样率
    @param: wait 是否等待
    @return:
    @note:
*/
/*----------------------------------------------------------------------------*/
static void pitchshifter_stream_set_sr(PITCHSHIFTER *obj, u16 sr, u8 wait)
{
    if (obj == NULL) {
        return ;
    }

    if (obj->output->set_sr) {
        obj->output->set_sr(obj->output->priv, sr, wait);
    }


    /* os_mutex_pend(&obj->mutex, 0); */
    obj->config.sr = sr;
    obj->set_flag = 1;
    /* os_mutex_post(&obj->mutex); */
}



/*----------------------------------------------------------------------------*/
/** @brief: 模块数据清空处理
    @param: obj 模块控制句柄
    @return:
    @note:
*/
/*----------------------------------------------------------------------------*/
static void pitchshifter_stream_clear(PITCHSHIFTER *obj)
{

    if (obj == NULL || obj->output == NULL || obj->output->clear == NULL) {
        return ;
    }
    obj->output->clear(obj->output->priv);
}


/*----------------------------------------------------------------------------*/
/** @brief: 模块数据是否输出完判读
    @param: obj 模块控制句柄
    @return:  true: 还有数据， false：已经没有数据了
    @note:
*/
/*----------------------------------------------------------------------------*/
static tbool pitchshifter_stream_check(PITCHSHIFTER *obj)
{
    if (obj == NULL || obj->output == NULL || obj->output->check == NULL) {
        return false;
    }

    return obj->output->check(obj->output->priv);
}


/*----------------------------------------------------------------------------*/
/** @brief: 模块流控拼接接口
    @param: obj 模块控制句柄
    @param: output 本模块的输出，即拼接前是上一级流控模块的输出
    @return:  本模块的输入， 拼接后是上一级流控的输出
    @note: 该接口主要是负责模块串接， 例如拼接两个水管的拼接口
*/
/*----------------------------------------------------------------------------*/
AUDIO_STREAM *pitchshifter_set_output(PITCHSHIFTER *obj, AUDIO_STREAM *output)
{
    if (obj == NULL || output == NULL || output->output == NULL) {
        return output;
    }

    obj->output = output;

    obj->input.priv  = (void *)obj;
    obj->input.output = (void *)pitchshifter_stream_write;
    obj->input.clear = (void *)pitchshifter_stream_clear;
    obj->input.check = (void *)pitchshifter_stream_check;
    obj->input.set_sr = (void *)pitchshifter_stream_set_sr;

    /* set  module output*/
    PS69_audio_IO ps_output;
    ps_output.outpriv = output->priv;
    ps_output.output = (void *)output->output;
    /* obj->ps_output.outpriv = output->priv; */
    /* obj->ps_output.output = (void *)output->output; */

    pitchshifter_printf("pitchshifter_stream_write = %x, %x, line = %d\n", pitchshifter_stream_write, obj->input.output, __LINE__);
    pitchshifter_printf("obj->ops.hdlxxx = %d\n", obj->ops.hdl);
    obj->ops._io->open(obj->ops.hdl, &(ps_output));

    pitchshifter_printf("pitchshifter_stream_write = %x, %x, line = %d\n", pitchshifter_stream_write, obj->input.output, __LINE__);

    return &(obj->input);
}

/*----------------------------------------------------------------------------*/
/** @brief: 模块通道个数设置
    @param: obj 模块控制句柄
    @param: track 通道个数
    @return:
    @note: 模块私有接口
*/
/*----------------------------------------------------------------------------*/
void pitchshifter_set_track_nums(u16 track)
{
    PITCHSHIFTER *obj = (PITCHSHIFTER *)pitchshifter_obj;
    if (obj == NULL) {
        return ;
    }
    /* os_mutex_pend(&obj->mutex, 0); */
    obj->config.chn = track;
    obj->set_flag = 1;
    /* os_mutex_post(&obj->mutex); */
}

/*----------------------------------------------------------------------------*/
/** @brief: 模块变速参数设置
    @param: obj 模块控制句柄
    @param: val 参数值
    @return:
    @note: 模块私有接口
*/
/*----------------------------------------------------------------------------*/
void pitchshifter_set_speed(PITCHSHIFTER *obj, u16 val)
{
    if (obj == NULL) {
        return ;
    }

    /* os_mutex_pend(&obj->mutex, 0); */
    obj->config.speedV = val;
    obj->set_flag = 1;
    /* os_mutex_post(&obj->mutex); */
}


/*----------------------------------------------------------------------------*/
/** @brief: 模块变调参数设置
    @param: obj 模块控制句柄
    @param: val 参数值
    @return:
    @note: 模块私有接口
*/
/*----------------------------------------------------------------------------*/
void pitchshifter_set_pitch(PITCHSHIFTER *obj, u16 val)
{
    if (obj == NULL) {
        return ;
    }

    /* os_mutex_pend(&obj->mutex, 0); */
    obj->config.pitchV = val;
    obj->set_flag = 1;
    /* os_mutex_post(&obj->mutex); */
}

/*----------------------------------------------------------------------------*/
/** @brief: 模块获取当前变速参数
    @param: obj 模块控制句柄
    @param: val 参数值
    @return:
    @note: 模块私有接口
*/
/*----------------------------------------------------------------------------*/
u16 pitchshifter_get_cur_speed(void)
{
    PITCHSHIFTER *obj = (PITCHSHIFTER *)pitchshifter_obj;
    if (obj == NULL) {
        return 0;
    }

    return obj->config.speedV;
}

/*----------------------------------------------------------------------------*/
/** @brief: 模块获取当前变调参数
    @param: obj 模块控制句柄
    @param: val 参数值
    @return:
    @note: 模块私有接口
*/
/*----------------------------------------------------------------------------*/
u16 pitchshifter_get_cur_pitch(void)
{
    PITCHSHIFTER *obj = (PITCHSHIFTER *)pitchshifter_obj;
    if (obj == NULL) {
        return 0;
    }

    return obj->config.pitchV;
}


/*----------------------------------------------------------------------------*/
/** @brief: 模块所需空间获取
    @param:
    @return:  所需空间获取大小
    @note: 模块私有接口
*/
/*----------------------------------------------------------------------------*/
u32 pitchshifter_get_need_buf_size(void)
{
    PS69_API_CONTEXT *tmp_ops = get_ps69_api();
    u32 need_len = SIZEOF_ALIN(sizeof(PITCHSHIFTER), 4) + SIZEOF_ALIN(tmp_ops->need_size(), 4);
    return need_len;
}


/*----------------------------------------------------------------------------*/
/** @brief: 模块对象创建
    @param:	need_buf 外部通过模块所需空间分配的内存地址
    @return:  模块控制句柄
    @note: 模块私有接口
*/
/*----------------------------------------------------------------------------*/
PITCHSHIFTER *pitchshifter_creat(void)
{
    pitchshifter_printf("pitchshifter_need_buf_size=%d\n", pitchshifter_get_need_buf_size());
    /* ASSERT(sizeof(pitchshifter_buf) >= pitchshifter_get_need_buf_size()); */
    u8 *need_buf = pitchshifter_buf;
    memset(pitchshifter_buf, 0x0, sizeof(pitchshifter_buf));

    PITCHSHIFTER *obj = (PITCHSHIFTER *)need_buf;
    need_buf += SIZEOF_ALIN(sizeof(PITCHSHIFTER), 4);

    obj->ops.hdl = (void *)need_buf;

    obj->ops._io = get_ps69_api();
    pitchshifter_printf("obj->ops.hdl = %d\n", obj->ops.hdl);

    ///set some default config
    pitchshifter_set_speed(obj, PS_SPEED_DEFAULT_VAL);
    pitchshifter_set_pitch(obj, PS_PITCHT_DEFAULT_VAL);

    return obj;
}


/*----------------------------------------------------------------------------*/
/** @brief: 模块对象释放
    @param:	obj 模块控制句柄
    @return:
    @note: 模块私有接口, 注意这里的句柄是双重指针
*/
/*----------------------------------------------------------------------------*/
void pitchshifter_destroy(PITCHSHIFTER **obj)
{
    if (obj == NULL || *obj == NULL) {
        return ;
    }

    /* free(*obj); */
    *obj = NULL;
}


AUDIO_STREAM *pitchshifter_input(AUDIO_STREAM *output)
{
    pitchshifter_obj = pitchshifter_creat();

    return pitchshifter_set_output((PITCHSHIFTER *)pitchshifter_obj, output);
}

void pitchshifter_set_speed_val(u16 val)
{
    pitchshifter_set_speed((PITCHSHIFTER *)pitchshifter_obj, val);
}

void pitchshifter_set_pitch_val(u16 val)
{
    pitchshifter_set_pitch((PITCHSHIFTER *)pitchshifter_obj, val);
}

#if 0
///demo
{
    ///根据变速变调模块所需的空间大小申请空间
    u8 *need_buf = (u8 *)malloc(pitchshifter_get_need_buf_size());
    ///创建变速变调模块对象
    PITCHSHIFTER *ps_obj = pitchshifter_creat(need_buf);
    ///将原解码的输出dac流控接口提供给变速变调模块， 作为变数变调的输出接口
    AUDIO_STREAM *pitchshifter_input = pitchshifter_set_output(ps_obj, (AUDIO_STREAM *)&dac_stream_io);
    ///将变速变调的输入接口设置成解码的输出接口
    music_decoder_set_output(music_obj, pitchshifter_input);
    ///流控结束删除变速变调模块对象
    pitchshifter_destroy(&ps_obj);
}
#endif

#else
AUDIO_STREAM *pitchshifter_input(AUDIO_STREAM *output)
{
    return output;
}
void pitchshifter_set_speed_val(u16 val)
{

}
void pitchshifter_set_pitch_val(u16 val)
{

}
u16 pitchshifter_get_cur_pitch(void)
{
    return 0;
}
u16 pitchshifter_get_cur_speed(void)
{
    return 0;
}

#endif
