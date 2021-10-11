#ifndef __FM_INSIDE_API_H_
#define __FM_INSIDE_API_H_
#include "typedef.h"

enum {
    SET_FM_INSIDE_SCAN_ARG1,
    SET_FM_INSIDE_SCAN_ARG2,
    SET_FM_INSIDE_PRINTF,
    SET_FM_INSIDE_SYS_CLK,
};
void fm_inside_io_ctrl(int ctrl, ...);

void fm_inside_default_config(void);
void fm_inside_on(void);
void fm_inside_off(void);
u8   fm_inside_freq_set(u32 freq);
void fm_inside_int_set(u8 mute);
u16  fm_inside_id_read(void);
s32  fm_inside_rssi_read(void); //unit DB
void fm_inside_scan_info_printf(u16 freq_start, u16 freq_end);
//more inside fm api function, see file fm_inside_api.h

//Following function Call is valid Only after fm_inside_on();
void fm_inside_set_stereo(u8 set);  //set[0,127], 0 mono, 127 stereo.
void fm_inside_set_abw(u8 set);  //audio bandwidth set //set[0,127] <=>2k~16k
void fm_inside_deempasis_set(u8 set);// set[0/1], 0:50us, 1:75us
void set_fm_pcm_out_fun(void *fun);
#endif

