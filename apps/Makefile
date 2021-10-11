

CC_ARG = \
-DNOFLOAT \
-DHAVE_MALLOC \

ifeq ($(TYPE), FPGA)
CC_ARG += \
-DFPGA
endif



INCLUDES = \
-I../include_lib \
-I../include_lib/cpu \
-I../include_lib/cpu/$(RELEASE) \
-I../include_lib/power_manage \
-I../include_lib/mcpwm \
-I../include_lib/rtc \
-I../include_lib/common \
-I../include_lib/cbuf \
-I../include_lib/aec \
-I../include_lib/dec \
-I../include_lib/echo \
-I../include_lib/fs \
-I../include_lib/fs/fat \
-I../include_lib/fs/sydf \
-I../include_lib/dev_mg \
-I../include_lib/sdmmc \
-I../include_lib/usb \
-I../include_lib/vm \
-I../include_lib/speed_pitch \
-I../include_lib/mcpwm \
-I../include_lib/bluetooth \
-I../include_lib/encode \
-I../include_lib/font \
-I../include_lib/uicon \
-Iinclude/bluetooth \
-Iinclude/bluetooth/ble \
-Iinclude \
-Iinclude/common \
-Iinclude/common/music_player \
-Iinclude/common/file_operate \
-Iinclude/common/fs \
-Iinclude/common/pitchshifter \
-Iinclude/common/audio_sync \
-Iinclude/common/pc_device \
-Iinclude/common/record \
-Iinclude/common/echo \
-Iinclude/common/fmtx \
-Iinclude/common/fmtx/qn8007 \
-Iinclude/common/fmtx/qn8027 \
-Iinclude/common/ui \
-Iinclude/cpu \
-Iinclude/cpu/$(RELEASE) \
-Iinclude/cpu/$(RELEASE)/audio \
-Iinclude/cpu/$(RELEASE)/power \
-Iinclude/cpu/$(RELEASE)/key_drv \
-Iinclude/cpu/$(RELEASE)/led \
-Iinclude/cpu/$(RELEASE)/nv_mem \
-Iinclude/cpu/$(RELEASE)/iic \
-Iinclude/cpu/$(RELEASE)/uart \
-Iinclude/cpu/$(RELEASE)/ui \
-Iinclude/cpu/$(RELEASE)/ui/led \
-Iinclude/cpu/$(RELEASE)/ui/lcd \
-Iinclude/cpu/$(RELEASE)/ui/task_ui \
-Itask_manager \
-Itask_manager/task_bt \
-Itask_manager/task_idle \
-Itask_manager/task_music \
-Itask_manager/task_linein \
-Itask_manager/task_pc \
-Itask_manager/task_fm \
-Itask_manager/task_rtc\
-Itask_manager/task_echo\
-Itask_manager/task_spdif\
-Itask_manager/task_fm/bk1080 \
-Itask_manager/task_fm/fm_inside \
-Itask_manager/task_fm/qn8035 \
-Itask_manager/task_fm/rda5807 \
-Icpu/audio/sw_eq_table \
-Itask_manager/task_rec\
-Iauto_test \


SRCS_C = \
auto_test/auto_test.c \
btstack/trunk/fcc_test.c \
btstack/trunk/aec_main.c \
btstack/trunk/PLC_main.c \
btstack/user/bt_configs_user.c \
btstack/user/aec_user.c \
btstack/user/a2dp_decode.c \
btstack/user/bt_tws.c \
ble_stack/user/le_server_module.c \
ble_stack/user/le_client_module.c \
ble_stack/user/att_send.c \
cpu/$(RELEASE)/clock.c \
cpu/$(RELEASE)/board_init.c \
cpu/$(RELEASE)/wdt.c \
cpu/$(RELEASE)/irq.c \
cpu/$(RELEASE)/timer.c \
cpu/$(RELEASE)/rotate_dec.c \
cpu/$(RELEASE)/key_drv/key.c \
cpu/$(RELEASE)/key_drv/key_drv_io.c \
cpu/$(RELEASE)/key_drv/key_drv_ad_rtcvdd.c \
cpu/$(RELEASE)/key_drv/key_drv_ad_vddio.c \
cpu/$(RELEASE)/key_drv/key_drv_ir.c \
cpu/$(RELEASE)/key_drv/key_drv_tch.c \
cpu/$(RELEASE)/key_drv/key_drv_uart.c \
cpu/$(RELEASE)/key_drv/key_drv_encoder.c \
cpu/$(RELEASE)/board/board.c \
cpu/$(RELEASE)/uart/uart.c \
cpu/$(RELEASE)/power/power.c \
cpu/$(RELEASE)/power/charge.c \
cpu/$(RELEASE)/adc_api.c \
cpu/$(RELEASE)/audio/alink.c \
cpu/$(RELEASE)/audio/dac.c \
cpu/$(RELEASE)/audio/ladc.c \
cpu/$(RELEASE)/audio/tone.c \
cpu/$(RELEASE)/audio/eq.c \
cpu/$(RELEASE)/audio/src.c \
cpu/$(RELEASE)/audio/pdm_link.c \
cpu/$(RELEASE)/audio/sin_tab.c \
cpu/$(RELEASE)/audio/digital_vol.c \
cpu/$(RELEASE)/audio/audio.c \
cpu/$(RELEASE)/audio/audio_stream.c \
cpu/$(RELEASE)/led/led.c \
cpu/$(RELEASE)/led/pwm_led.c \
cpu/$(RELEASE)/iic/iic_hw.c \
cpu/$(RELEASE)/iic/iic.c \
cpu/$(RELEASE)/iis/wm8978.c \
cpu/$(RELEASE)/spdif.c \
cpu/$(RELEASE)/ui/led/led7_drv.c \
cpu/$(RELEASE)/ui/led/led_1888_drv.c \
cpu/$(RELEASE)/ui/lcd/lcd128X64.c \
cpu/$(RELEASE)/ui/lcd/lcd_disp.c \
cpu/$(RELEASE)/ui/lcd/lcd_drv_api.c \
cpu/$(RELEASE)/ui/lcd/lcd_drv_interface.c \
cpu/$(RELEASE)/ui/lcd/lcd_spi.c \
cpu/$(RELEASE)/ui/task_ui/bt_ui.c \
cpu/$(RELEASE)/ui/task_ui/linein_ui.c \
cpu/$(RELEASE)/ui/task_ui/music_ui.c \
cpu/$(RELEASE)/ui/task_ui/pc_ui.c \
cpu/$(RELEASE)/ui/task_ui/fm_ui.c \
cpu/$(RELEASE)/ui/task_ui/rtc_ui.c \
cpu/$(RELEASE)/ui/task_ui/rec_ui.c \
cpu/$(RELEASE)/ui/task_ui/echo_ui.c \
cpu/$(RELEASE)/ui/ui_api.c \
cpu/$(RELEASE)/ui/ui_common.c \
cpu/$(RELEASE)/updata.c \
common/common.c \
common/msg.c \
common/flash_api.c \
common/dev_manage.c \
common/file_operate/file_io.c \
common/file_operate/file_operate.c \
common/fs/fs.c \
common/music_player/music_decoder.c \
common/music_player/music_player.c \
common/music_player/warning_tone.c \
common/pitchshifter/pitchshifter.c \
common/audio_sync/sync_hardware.c \
common/audio_sync/sync_software.c \
common/audio_sync/sync_tws.c \
common/record/rec_file_op.c \
common/record/rec_api.c \
common/record/rec_input.c \
common/record/rec_output.c \
common/record/rec_play.c \
common/echo/echo_api.c \
common/fmtx/qn8027/qn8027.c \
common/fmtx/qn8007/qn8007.c \
common/fmtx/fmtx_api.c \
common/ui/font_api.c \
common/ui/uicon_api.c \
task_manager/task_common.c \
task_manager/task_manager.c \
task_manager/task_bt/task_bt.c \
task_manager/task_bt/task_bt_key.c \
task_manager/task_idle/task_idle.c \
task_manager/task_idle/task_idle_key.c \
task_manager/task_music/music_decrypt.c \
task_manager/task_music/lyrics_api.c \
task_manager/task_music/task_music.c \
task_manager/task_music/task_music_key.c \
task_manager/task_fm/bk1080/Bk1080.c \
task_manager/task_fm/fm_inside/fm_inside.c \
task_manager/task_fm/qn8035/QN8035.c \
task_manager/task_fm/rda5807/RDA5807.c \
task_manager/task_fm/fm_api.c \
task_manager/task_fm/fm_common.c \
task_manager/task_fm/task_fm.c \
task_manager/task_fm/task_fm_key.c \
task_manager/task_linein/task_linein.c \
task_manager/task_linein/task_linein_key.c \
task_manager/task_rtc/task_rtc.c \
task_manager/task_rtc/task_rtc_key.c \
task_manager/task_rtc/rtc_setting.c \
task_manager/task_rec/task_rec_key.c \
task_manager/task_rec/task_rec.c \
task_manager/task_echo/task_echo.c \
task_manager/task_echo/task_echo_key.c \
task_manager/task_spdif/task_spdif.c \
task_manager/task_spdif/task_spdif_key.c \

SRCS_LS = \


SRCS_BS = \

LIBS = \
--start-group  \
--whole-archive \
../include_lib/bluetooth_lib$(SUFFIX).a  \
../include_lib/bluetooth/aes_lib_pi32.a 		  \
../include_lib/bluetooth/crc_ibm_lib.a 		  \
../include_lib/bluetooth/msbc_enc_dec.a 		  \
--no-whole-archive \
../include_lib/rtc_lib$(SUFFIX).a  \
../include_lib/power_manage_lib$(SUFFIX).a  \
../include_lib/periphy_lib$(SUFFIX).a  \
../include_lib/aec_lib$(SUFFIX).a  \
../include_lib/audio_lib$(SUFFIX).a   \
../include_lib/common_lib$(SUFFIX).a   \
../include_lib/font_lib$(SUFFIX).a   \
../include_lib/uicon_lib$(SUFFIX).a   \
../include_lib/aec/libBtCalling.a   \
../include_lib/aec/libDAGC.a   \
../include_lib/aec/libSpeech_dection.a   \
../include_lib/aec/lib_esco_repair.a     \
../include_lib/aec/lib_esco_sync.a       \
../include_lib/dec/sbc_dec_lib.a         \
../include_lib/dec/udsbc_enc_lib.a         \
../include_lib/dec/udsbc_dec_lib.a         \
../include_lib/dec/sbc_dec_lib_for_EngDect.a \
../include_lib/dec/wt_dec_lib.a 		  \
../include_lib/dec/mp3_dec_lib.a 		  \
../include_lib/dec/flac_dec_lib.a 		  \
../include_lib/dec/ape_dec_lib.a 		  \
../include_lib/dec/wav_dec_lib.a 		  \
../include_lib/dec/wma_dec_lib.a 		  \
../include_lib/dec/dts_dec_lib.a 		  \
../include_lib/dec/aac_dec_lib.a 		  \
../include_lib/dec/amr_dec_lib.a 		  \
../include_lib/dev_mg_lib.a 		  \
../include_lib/encode/lib_adpcm_encode.a \
../include_lib/encode/lib_mp2_encode.a \
../include_lib/encode_lib.a 		  \
../include_lib/mcpwm_lib.a 		  \
../include_lib/fs_lib.a 		  \
../include_lib/fm_lib.a 		  \
../include_lib/speed_pitch/lib_pitch_speed.a 		  \
../include_lib/flash_lib.a 		  \
../include_lib/audio/libSOSIIR.a 		  \
../include_lib/audio/libLimiter.a 		  \
../include_lib/audio/lib_sync_frame.a 		\
../include_lib/echo_lib.a 		\
../include_lib/echo/lib_audio_reverb_H.a 		\
../include_lib/echo/lib_audio_howlingsupression.a 		\
../include_lib/sdmmc_lib.a 		  \
../include_lib/usb_lib.a 		  \
../include_lib/usb/lib_usb_syn.a 		  \
--end-group  \






#------------------------function--------------------#
SRCS_C += \
common/pc_device/dev_pc.c \
common/pc_device/card_reader_io.c \

#------------------------task--------------------#
SRCS_C += \
task_manager/task_pc/task_pc.c \
task_manager/task_pc/task_pc_key.c \



-include $(MAKE_RULE)
