#ifndef _DIGITAL_VOL_H_
#define _DIGITAL_VOL_H_

#include "typedef.h"
#include "cpu/audio_param.h"

#define SYS_VOL_EXT_NULL	0	/*默认直接设置模拟音量				*/
#define SYS_VOL_EXT_RANGE	1	/*给定最大值最小值，然后平均		*/
#define SYS_VOL_EXT_STEP	2	/*给定最大值，然后按照给定步进平均	*/
#define SYS_VOL_EXT			SYS_VOL_EXT_NULL

#define D_FADE_NULL	0
#define D_FADE_IN	1
#define D_FADE_OUT	2

void digital_vol_init(struct SOUND_VOL *p);
void set_digital_vol(u32 vol_l, u32 vol_r);
void digital_vol_ctrl(void *buffer, u32 len, u8 analog_vol);
void ad_vol_mix(void *buffer, u32 len, u8 sys_vol);
void digital_vol_fade(u8 fade);
void set_dac_vol_ext(u32 l_vol, u32 r_vol, u8 fade_en);


#endif
