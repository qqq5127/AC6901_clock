#ifndef __DEV_PC_H__
#define __DEV_PC_H__

#include "includes.h"
#include "usb/usb_slave_api.h"
#define USER_PC_DESCRIPTOR   0   //用户自定义USB字符串描述

s32 app_usb_slave_init(void);
s32 app_usb_slave_close(void);
s32 app_usb_slave_card_reader(u32 cmd);
s32 app_usb_slave_hid(u32 hid_cmd);
u8 	app_pc_set_speaker_vol(u32 pc_mute_status);
u32 app_usb_slave_online_status(void);
void pc_dac_mute(bool mute_status, u8 fade_en);
void pc_dac_channel_on(void);
void pc_dac_channel_off(void);
void pc_check_api();
void usb_slave_mic_input(s16 *buffer, u32 buffer_len);
void pc_speak_out_sw(u8 sw);
#endif/*__DEV_PC_H__*/
