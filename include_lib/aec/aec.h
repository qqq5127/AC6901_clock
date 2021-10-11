#ifndef _AEC_H_
#define _AEC_H_

#include "typedef.h"

/*
* AEC ENABLE BITMAP
*/
#define AEC_BITMAP_SPEECH_DET	BIT(0)
#define AEC_BITMAP_BT_CALLING	BIT(1)
#define AEC_BITMAP_SYNC_IN		BIT(2)

/*
* AEC SYNC BITMAP
*/
#define AEC_SYNC_CODEC			BIT(0)	/*AEC codec ready		*/
#define AEC_SYNC_SEND			BIT(1)	/*AEC send data ready	*/
#define AEC_SYNC_ADC			BIT(2)	/*AEC adc ready			*/
#define AEC_SYNC_DAC			BIT(3)	/*AEC dac ready			*/
#define AEC_SYNC_AEC			BIT(4)	/*AEC aec task ready	*/
#define AEC_SYNC_NULL			0
#define AEC_SYNC_OK				(AEC_SYNC_SEND | AEC_SYNC_ADC | AEC_SYNC_DAC | AEC_SYNC_AEC)

typedef enum aec_err {

    AEC_ERR_NONE		= 0u,	/*aec err none			*/
    AEC_ERR_MEMORY			,	/*malloc buffer faild	*/
    AEC_ERR_TASK			,	/*create task faild		*/
    AEC_ERR_TASK_PRIO		,	/*task priority err		*/
    AEC_ERR_EXIST			,	/*aec exist				*/
} AEC_ERR;

typedef struct {
    volatile u16 kick_start;
    volatile u16 toggle;
    void (*fill_dac_echo_buf)(s16 *buf, u16 len);
    void (*fill_adc_ref_buf)(s16 *buf, u16 len);
    void (*aec_set_mic_gain)(u8 gain, u8 gx2);
    void (*aec_run)(void *p_arg);
} AEC_INTERFACE;
extern AEC_INTERFACE aec_interface;

typedef struct {
    unsigned short mic_analog_gain;
    unsigned short dac_analog_gain;
    unsigned short NDT_max_gain;			/*dagc max gain*/
    unsigned short NDT_min_gain;			/*dagc min gain*/
    unsigned short NDT_Fade_Speed;			/*dagc fade_in or fade_out speed*/

    unsigned short suppress_rough;
    unsigned short suppress_fine;

    unsigned short NearEnd_Begin_Threshold;	/* > Nearend_begin_threshold,NearEnd talking*/
    unsigned short FarEnd_Talk_Threshold;	/* > Farend_talk_threshold,FarEnd talking	*/

    unsigned short speak_detect_thr;
    unsigned short speak_detect_gain;

    unsigned short adc_pre_delay;
    unsigned short dac_pre_delay;

    int e_slow;
    int x_slow;
    int frame_cnt;

    const int *eq_tab;
    unsigned short eq_gain;

    unsigned short em_level;
    int aec_ctl;
} AEC_ARGV;
extern AEC_ARGV aec_param;

u32 aec_query_bufsize(u16 en_bitmap);
u32 aec_init(void *aecbuf, AEC_ARGV *argv);
void aec_close();
void aec_run(void *p_arg);
void aec_handle_register(void (*aec_wakeup)());
void Speech_dectection_run(short *data, int npoint);
void aec_buf_clear(void);

//aec sync input
u32 sync_in_querybuf();
u32 sync_in_init(void *priv);

//data stream api
void get_bt_send_data(s16 *buf, u16 len);
void fill_adc_ref_buf(s16 *buf, u16 len);
void fill_dac_echo_buf(s16 *buf, u16 len);

void aec_post_init_register(void (*callback_fun)());
int set_advanced_params(int *parNum, int nParSet, int *value);

#endif
