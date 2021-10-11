#ifndef __ECHO_DEAL_H__
#define __ECHO_DEAL_H__

#include "typedef.h"
#include "reverb_api.h"

//head file define
s32 echo_input_init(u8 *buf, u32 len);
s32 echo_input_push_data(u8 *buf, u32 len);
s32 echo_input_pop_data(u8 *buf, u32 len);

s32 echo_output_init(u8 *buf, u32 len);
s32 echo_output_push_data(u8 *buf, u32 len);
s32 echo_output_pop_data(u8 *buf, u32 len);

s32 echo_input_api(u8 *buf, u32 len);
s32 echo_output_api(u8 *buf, u32 len);

//echo_soft_irq
s32 echo_algorithm_init(REVERB_PARM_SET *rev_parm, s16 max_ms, u8 *buf, u32 len);

//echo_deal_function
void echo_soft_kick(void);
void echo_soft_run(void);

//echo_deal_init
s32 echo_deal_init(void);
s32 echo_deal_release(void);



///echo spec define src
#define ECHO_11K_SR    10500
#define ECHO_12K_SR    11424
#define ECHO_08K_SR    7616

#define ECHO_22K_SR    21000
#define ECHO_24K_SR    22848
#define ECHO_16K_SR    15232

#define ECHO_44K_SR    42000
#define ECHO_48K_SR    45696
#define ECHO_32K_SR    30464

///normal src
#define NOR_11K_SR    11025
#define NOR_12K_SR    12000
#define NOR_08K_SR    8000

#define NOR_22K_SR    22050
#define NOR_24K_SR    24000
#define NOR_16K_SR    16000

#define NOR_44K_SR    44100
#define NOR_48K_SR    48000
#define NOR_32K_SR    32000

//sr exchange
u16 echo_reg2sr(u16 sr_reg);
void echo_decay_set_lib(u16 decay);
void echo_deep_set_lib(u16 deep);
void echo_pitch_set_lib(u8 pitch);
void pitch_on(void *pitch_buf, s16 pitch);
void pitch_off(void);
void echo_pitch_run(short *indata, short len);

#endif //__ECHO_DEAL_H__

