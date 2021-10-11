#ifndef _TONE_H_
#define _TONE_H_

#include "typedef.h"
typedef struct _TONE_VAR {
    const s16 *sr_tab;		/*tone tab						*/
    u16 toggle;				/*按键音总开关*/
    u16 sr_tab_size;		/*tone tab size					*/
    u16 step;
    s16 vol;				/*tone volume(max:1<<14)		*/
    u32 dac_sr;				/*select corresponding tone tab	*/
    volatile u16 tab_cnt;	/*total tone len				*/
    volatile u16 point_cnt;	/*current point cnt of tone tab	*/
    volatile u8 dac_lr;		/*tone mono to dac dual counter	*/
} SIN_TONE_VAR;
extern SIN_TONE_VAR sin_tone_var;

void sin_tone_init(void);
void sin_tone_toggle(u16 en);
void sin_tone_sr_set(u16 sr);
void sin_tone_play(u16 cnt);
void sin_tone_2_dac(s16 *buff, u32 len);
u16 is_sin_tone_busy(void);

#endif
