#ifndef __LINEIN_API_H__
#define __LINEIN_API_H__

#include "typedef.h"

/*
***********************************************************************************
*					OPEN AUX CHANNEL
*
*Description: This function is called by application to enable a aux channel
*
*Argument(s): chl		aux input channel enable
*			  amux_en	aux output to dac  enable
*
*Returns	: 0			successfully
*			  others 	failed
*
*Note(s)	: 1)if linein supports AUX_AD_ENABLE,param amux_en should set to Zero
***********************************************************************************
*/
u8 linein_channel_open(u16 chl, u8 amux_en);
u8 linein_channel_close(u16 chl, u8 amux_en);

u16 linein_channel_status();
void linein_mute(u8 mute);
u8 linein_mute_status(void);
void linein_gain_en(u8 en);
void aux_2_dac(u8 lr2l, u8 lr2r);
void mic_2_dac(u8 mic2l, u8 mic2r);
/*
***********************************************************************************
*					AMUX BIAS EN
*
*Description: This function is called by application to open amux bias
*
*Argument(s): en	Enable(1) or Disable(0)
*
*Returns	: none
*
*Note(s)	: 1)if you want to enable line bias,amux bias should be enabled first
***********************************************************************************
*/
void amux_bias_en(u8 en);
/*
***********************************************************************************
*					AUX	BIAS EN
*
*Description: This function is called by application to open aux bias
*
*Argument(s): chl	linein channel
*			  en	Enable(1) or Disable(0)
*
*Returns	: none
*
*Note(s)	: 1)if you want to enable line bias,amux bias should be enabled first
***********************************************************************************
*/
void aux_bias_en(u8 chl, u8 en);


#endif /*__LINEIN_API_H__*/
