#include "music_player.h"
#include "file_operate.h"
#include "dev_mg_api.h"
#include "file_op_err.h"
#include "fs_io.h"
#include "audio/dac_api.h"
#include "msg.h"
#include "task_manager.h"
#include "audio/dac.h"
#include "audio/audio.h"
#include "flash_api.h"
#include "resource_manage.h"
#include "warning_tone.h"
#include "music_decrypt.h"
#include "lyrics_api.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".music_decoder_bss")
#pragma data_seg(	".music_decoder_data")
#pragma const_seg(	".music_decoder_const")
#pragma code_seg(	".music_decoder_code")
#endif

//#define MUSIC_PLAYER_DEBUG_ENABLE

#ifdef MUSIC_PLAYER_DEBUG_ENABLE
#define music_player_printf	printf
#define music_player_puts	puts
#else
#define music_player_printf(...)
#define music_player_puts(...)
#endif


struct __MUSIC_PLAYER {
    MUSIC_DECODER 	*dop;
    FILE_OPERATE 	*fop;
};

static MUSIC_PLAYER music_player_hdl 			sec_used(.music_mem);
static u32 music_decode_ram[7 * 1024] 			sec_used(.music_mp3_dec);
static AB_RPT_STA ab_repeat_status = AB_RPT_NON;

extern AUDIO_STREAM_DAC audio_stream_t;
extern u8 music_play_get_rpt_mode(void);
#if WARNING_SD_USB
extern DEV_HANDLE cur_use_dev_real;
extern DEV_HANDLE cur_use_dev_temp;
extern u8 play_sd_usb_tone_flag;
#endif

#if LCD_128X64_EN
FS_DISP_NAME music_file_info;
#endif

const char dec_file_ext[][3] = {
#if DEC_TYPE_MP3_ENABLE
    {"MP1"},
    {"MP2"},
    {"MP3"},
#endif // DEC_TYPE_MP3_ENABLE

#if DEC_TYPE_WMA_ENABLE
    {"WMA"},
#endif // DEC_TYPE_WMA_ENABLE

#if DEC_TYPE_WAV_ENABLE
    {"WAV"},
#endif // DEC_TYPE_WAV_ENABLE

#if DEC_TYPE_FLAC_ENABLE
    {"FLA"},
#endif // DEC_TYPE_FLAC_ENABLE

#if DEC_TYPE_APE_ENABLE
    {"APE"},
#endif // DEC_TYPE_APE_ENABLE

#if DEC_TYPE_F1A_ENABLE
    {"F1A"},
#endif // DEC_TYPE_APE_ENABLE

#if MUSIC_DECRYPT_EN
    {"SMP"},
#endif //SMP_FILE

#if DEC_TYPE_AMR_ENABLE
    {"AMR"},
#endif

#if DEC_TYPE_M4A_ENABLE
    {"M4A"},
    {"AAC"},
    {"DTS"},
    {"TS?"},
#endif
    {'\0'},
};

static const DEC_CFG music_player_decoder_configs[] = {

#if DEC_TYPE_WAV_ENABLE
    {
        get_wav_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

#if DEC_TYPE_APE_ENABLE
    {
        get_ape_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

#if DEC_TYPE_WMA_ENABLE
    {
        get_wma_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

#if DEC_TYPE_FLAC_ENABLE
    {
        get_flac_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

#if DEC_TYPE_F1A_ENABLE
    {
        get_f1a_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        0,
    },
#endif

#if DEC_TYPE_DTS_ENABLE
    {
        get_dts_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif


#if DEC_TYPE_AMR_ENABLE
    {
        get_amr_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        0,
    },
#endif

#if DEC_TYPE_M4A_ENABLE
    {
        get_m4a_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

#if DEC_TYPE_MP3_ENABLE
    {
        get_mp3_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

};

static const DEC_CFG music_tone_decoder_configs[] = {

#if DEC_TYPE_WAV_ENABLE
    {
        get_wav_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

#if DEC_TYPE_MP3_ENABLE
    {
        get_mp3_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

};

#if REC_PLAY_EN
static const DEC_CFG rec_play_decoder_configs[] = {

#if DEC_TYPE_WAV_ENABLE
    {
        get_wav_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

#if DEC_TYPE_MP3_ENABLE
    {
        get_mp3_ops,
        NULL,
        music_decode_ram,
        sizeof(music_decode_ram),
        1,
        1,
    },
#endif

};
#endif

static u32 fs_get_file_size(void *priv)
{
    u32 size;
    if (fs_io_ctrl(NULL, (_FIL_HDL *)priv, FS_IO_GET_FILE_SIZE, &size)) {
        return 0;
    }
    /* printf("file size === %d\n", size); */
    return size;
}

extern CIPHER  dec_cipher;

u32 fs_cipher_seek(_FIL_HDL  *p_f_hdl, u8 type, u32 offsize)
{
    return fs_seek(p_f_hdl, type, offsize);
}

u32 fs_cipher_read(_FIL_HDL *p_f_hdl, u8 _xdata *buff, u16 len)
{
    u32 faddr = fs_tell(p_f_hdl);
    u16 r_len = fs_read(p_f_hdl, buff, len);
    if ((r_len != 0) && (r_len != (u16) - 1)) {
        cipher_analysis_buff(&dec_cipher, buff, faddr, len);
        return r_len;
    }
    return 0;
}

static const AUDIO_FILE music_player_file = {
    .seek = (void *)fs_seek,
    .read = (void *)fs_read,
    .get_size = (void *)fs_get_file_size,
};

static const AUDIO_FILE music_player_cipher_file = {
    .seek = (void *)fs_cipher_seek,
    .read = (void *)fs_cipher_read,
    .get_size = (void *)fs_get_file_size,
};

AUDIO_FILE *music_player_get_fs_hdl(u8 cipher_enable)
{
    if (cipher_enable) {
        return (AUDIO_FILE *)&music_player_cipher_file;
    } else {
        return (AUDIO_FILE *)&music_player_file;
    }
}

MUSIC_DECODER *music_player_get_dop(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return NULL;
    }

    return obj->dop;
}

FILE_OPERATE *music_player_get_fop(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return NULL;
    }

    return obj->fop;
}

void music_player_run(MUSIC_PLAYER *obj)
{
    music_decoder_decode_deal(obj->dop);
}

static void music_player_decoder_err_deal(void *priv, u32 err)
{
    u32 cur_dev_status = 1;
    u32 msg;
    s32 ret = 0;
    MUSIC_PLAYER *obj = (MUSIC_PLAYER *)priv;

    if (err) {
        ret = dev_get_online_status((DEV_HANDLE)file_operate_get_dev(obj->fop), &cur_dev_status);
        if (ret != 0) {
            msg = SYS_EVENT_DEC_DEVICE_ERR;
        } else {
            if (cur_dev_status == 0) {
                msg = SYS_EVENT_DEC_DEVICE_ERR;
            } else {
                /* mapi_printf("music player dec err = %x\n", err); */
                switch (err) {
                case MAD_ERROR_FILE_END://file end
                    msg = SYS_EVENT_DEC_END;
                    break;
                //case MAD_ERROR_SYNC_LIMIT:// 文件错误 */
                //	break;
                case MAD_ERROR_FF_FR_FILE_END://快进结束 */
                    msg = SYS_EVENT_DEC_FF_END;
                    break;
                case MAD_ERROR_FF_FR_FILE_START://快退结束 */
                    msg = SYS_EVENT_DEC_FR_END;
                    break;
                case MAD_ERROR_NODATA:
                    msg = SYS_EVENT_DEC_END;
                    break;
                default:
                    msg = SYS_EVENT_DEC_END;
                    break;
                }
            }
        }
        music_player_printf("music player dec err ===================== %x\n", err);
        task_post_msg(NULL, 1, msg);
    }
}

static tbool music_player_get_vm_index(MUSIC_PLAYER *obj, u8 *index)
{
    if (obj == NULL) {
        return false;
    }
    DEV_HANDLE dev = file_operate_get_dev(obj->fop);
    if (dev == sd0) {
        *index = VM_DEV0_BREAKPOINT;
    } else if (dev == sd1) {
        *index = VM_DEV1_BREAKPOINT;
    } else if (dev == usb) {
        *index = VM_DEV2_BREAKPOINT;
    } else {
        return false;
    }
    return true;
}

static void music_player_save_break_point(MUSIC_PLAYER *obj)
{
    MUSIC_PLAYER_BP bp_info;

    if (obj == NULL) {
        return ;
    }

    if (music_player_get_break_point_info(obj, &bp_info) == false) {
        return ;
    }
    printf("fun %s\nsclust = %d\nf_size = %d\n", __func__, bp_info.f_info.sclust, bp_info.f_info.f_size);

    u8 vm_index = 0;
    if (music_player_get_vm_index(obj, &vm_index) == false) {
        return ;
    }

    s32 ret;
    u32 bp_size = bp_info.bp_size + sizeof(bp_info) - sizeof(bp_info.buf);
    ret = vm_write(vm_index, (const void *)&bp_info, bp_size);
    if (0 < ret) {
        printf("save bp ok , len = %d\n", ret);
        /* printf_buf((u8 *)&bp_info, ret); */
    } else {
        printf("warnning : save bp err\n");
    }
}

static tbool music_player_read_break_point(MUSIC_PLAYER *obj, MUSIC_PLAYER_BP *bp_info)
{
    if (obj == NULL || bp_info == NULL) {
        return false;
    }

    if (file_operate_get_file_sel_mode(obj->fop) != PLAY_BREAK_POINT) {
        return false;
    }

    memset((u8 *)bp_info, 0, sizeof(MUSIC_PLAYER_BP));

    u8 vm_index = 0;
    if (music_player_get_vm_index(obj, &vm_index) == false) {
        return false;
    }

    s32 ret;
    u32 bp_size = sizeof(MUSIC_PLAYER_BP);		//read max break_point len
    ret = vm_read(vm_index, (void *)bp_info, bp_size);
    if (0 < ret) {
        printf("read bp ok!!! , len = %d\n", ret);
    } else {
        printf("warnning : read bp err\n");
        return false;
    }

    /* printf_buf((u8 *)bp_info, ret); */
    return true;
}

static void music_player_clear_break_point(MUSIC_PLAYER *obj, u32 err)
{
    if (obj == NULL) {
        return ;
    }

    if ((err == MUSIC_DECODER_ERR_NONE) || (err == FILE_OP_ERR_END_FILE) || (err == FILE_OP_ERR_PRE_FILE)) { //解码成功和设备最后一个文件才需要清除断点
        MUSIC_PLAYER_BP bp_info;
        ///只是记录文件簇号和文件大小
        memset((u8 *)&bp_info, 0, sizeof(MUSIC_PLAYER_BP));
        if (err == MUSIC_DECODER_ERR_NONE) {
            if (fs_io_ctrl(NULL, (_FIL_HDL *)file_operate_get_file_hdl(obj->fop), FS_IO_GET_FILE_SIZE, &(bp_info.f_info.f_size))) {
                return ;
            }
            if (fs_io_ctrl(NULL, (_FIL_HDL *)file_operate_get_file_hdl(obj->fop), FS_IO_GET_FILE_SCLUST, &(bp_info.f_info.sclust))) {
                return ;
            }
        }

        ///可以根据不同设备保存断点到不同的存储空间
        u8 vm_index = 0;
        if (music_player_get_vm_index(obj, &vm_index) == false) {
            return ;
        }

        s32 ret;
        u32 bp_size = sizeof(bp_info) - sizeof(bp_info.buf);
        ret = vm_write(vm_index, (const void *)&bp_info, bp_size);
        if (0 < ret) {
            printf("clear bp ok, len = %d\n", ret);
        } else {
            printf("warnning : clear bp err\n");
        }
    }
}


MUSIC_PLAYER *music_player_creat(void)
{
    tbool ret = true;
    int err;

    memset((u8 *)&music_player_hdl, 0x0, sizeof(MUSIC_PLAYER));
    MUSIC_PLAYER *obj = (MUSIC_PLAYER *)&music_player_hdl;
    ///music op creat
    obj->dop = music_decoder_creat();
    if (obj->dop == NULL) {
        music_player_printf("music player creat fail ! fun = %s, line = %d\n", __func__, __LINE__);
        ret = false;
        goto __exit;
    }

    ///file op creat
    obj->fop = file_operate_creat();
    if (obj->fop == NULL) {
        music_player_printf("file operate creat fail ! fun = %s, line = %d\n", __func__, __LINE__);
        ret = false;
        goto __exit;
    }

    music_decoder_set_configs(obj->dop, (DEC_CFG *)music_player_decoder_configs, sizeof(music_player_decoder_configs) / sizeof(music_player_decoder_configs[0]));

    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_MUSIC;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    music_decoder_set_output(obj->dop, audio_stream_init(&stream_param, NULL));
    music_decoder_set_err_deal_interface(obj->dop, music_player_decoder_err_deal, obj);
    music_decoder_set_callback(obj->dop, music_decoder_before_callback, music_decoder_after_callback);
    file_operate_set_file_ext(obj->fop, (const char *)dec_file_ext);//WAV
    file_operate_set_repeat_mode(obj->fop, music_play_get_rpt_mode());
    file_operate_set_auto_next(obj->fop, 1);


// PLAY_FIRST_FILE
    file_operate_set_file_sel_mode(obj->fop, PLAY_FIRST_FILE);
// PLAY_LAST_FILE
    /* file_operate_set_file_sel_mode(obj->fop, PLAY_LAST_FILE); */
//PLAY_SPEC_FILE
    /* file_operate_set_file_sel_mode(obj->fop, PLAY_SPEC_FILE); */
    /* file_operate_set_file_number(obj->fop, 3); */
//PLAY_FILE_BYPATH
    /* file_operate_set_file_sel_mode(obj->fop, PLAY_FILE_BYPATH); */
    /* file_operate_set_path(obj->fop, (void *)"/test.mp3", 0); */


__exit:
    if (ret == false) {
        music_player_destroy(&obj);
    }

    return obj;
}

void music_player_destroy(MUSIC_PLAYER **hdl)
{
    int err;
    if (hdl == NULL || *hdl == NULL) {
        return;
    }
    MUSIC_PLAYER *obj = *hdl;

    music_player_save_break_point(obj);
    //music op destroy
    music_decoder_destroy(&(obj->dop));
    //file op destroy
    file_operate_destroy(&(obj->fop));

    *hdl = NULL;
}

static tbool music_player_err_deal(MUSIC_PLAYER *obj, u32 err)
{
    tbool ret = false;
    if (obj == NULL) {
        return false;
    }

    music_player_printf("----------------err = %x---------------\n", err);

    if (err == MUSIC_DECODER_ERR_START_FAIL) {
        ///音乐开始解码不成功， 例如格式检查不对等， 认为文件错位处理，执行错误文件统计
        music_player_printf(" @@@@@@@@@@@@@@@@@@@@@@@@@@DEC_ERR\n");
        err = FILE_OP_ERR_OPEN_FILE;
    }

    if (err < MUSIC_DECODER_ERR_NONE) {
        ret = file_operate_err_deal(obj->fop, err);
    }

    return ret;
}



tbool music_player_get_break_point_info(MUSIC_PLAYER *obj, MUSIC_PLAYER_BP *bp_info)
{
    if (obj == NULL || bp_info == NULL) {
        return false;
    }

    if (music_decoder_get_status(obj->dop) != MUSIC_DECODER_ST_STOP) {
        memset((u8 *)bp_info, 0, sizeof(MUSIC_PLAYER_BP));

        u32 bp_size = music_decoder_get_break_point_size(obj->dop);
        if ((bp_size > sizeof(bp_info->buf)) || (bp_size == 0)) {
            music_player_printf("break point buf is too small!! bp_size = %d\n\n", bp_size);
            return false;
        } else {
            u8 *bp_buf = music_decoder_get_break_point_info(obj->dop);
            memcpy(bp_info->buf, bp_buf, bp_size);
            bp_info->bp_size = bp_size;

            //printf("\n\n\n------get_break point buf bp_size = %d\n\n", bp_size);
            //printf_buf(bp_buf, bp_info->bp_size);

        }

        if (fs_io_ctrl(NULL, (_FIL_HDL *)file_operate_get_file_hdl(obj->fop), FS_IO_GET_FILE_SIZE, &(bp_info->f_info.f_size))) {
            return false;
        }
        if (fs_io_ctrl(NULL, (_FIL_HDL *)file_operate_get_file_hdl(obj->fop), FS_IO_GET_FILE_SCLUST, &(bp_info->f_info.sclust))) {
            return false;
        }

        ///可以根据不同设备保存断点到不同的存储空间
        /* bp_info->dev = file_operate_get_dev(obj->fop);	 */
        return true;
    }
    return false;
}


tbool music_player_play(MUSIC_PLAYER *obj, MUSIC_PLAYER_BP *bp_info, u8 is_auto)
{
    MUSIC_PLAYER_BP bp_info_rd;
    tbool ret = false;
    u32 err;
    if (obj == NULL) {
        return false;
    }

    music_player_ab_repeat_close(obj);

#if LRC_LYRICS_EN
    lrc_set_analysis_flag(0);
#endif

    //save break point before decode stop
    if (file_operate_get_dev_sel_mode(obj->fop) != DEV_SEL_CUR) {
        music_player_save_break_point(obj);
    }

    ///stop dec
    music_decoder_stop(obj->dop);

    vm_check_api(1);

    do {

        ///dev sel
        err = file_operate_dev_sel(obj->fop);

        if (!err) {
            //read break
            if (bp_info == NULL) {
                if (music_player_read_break_point(obj, &bp_info_rd) == true) {
                    bp_info = &bp_info_rd;
                }
            }
            //get_break_point
            err = file_operate_dev_bp(obj->fop, &(bp_info->f_info));

            if (!err) {
                ///file sel
                err = file_operate_file_sel(obj->fop, &(bp_info->f_info));
                if (!err) {
#if MUSIC_DECRYPT_EN
                    char *f_path = NULL;
                    file_operate_get_file_name(obj->fop, &f_path);
                    music_player_printf("music_get_filename = %s, line = %d\n", f_path, __LINE__);
                    if (cipher_check_file_type(&dec_cipher, f_path)) {
                        music_player_printf("\n\n######  is SMP file !!! \n\n");
                    }
#endif
#if LCD_128X64_EN
                    memset(&music_file_info, 0, sizeof(music_file_info));
                    file_operate_get_file_info(obj->fop, &music_file_info);
                    music_file_info.update_flag = 1;
#if LRC_LYRICS_EN
                    if (lrc_analysis_api(obj->fop)) {
                        lrc_set_analysis_flag(1);
                    } else {
                        lrc_set_analysis_flag(0);
                    }
#endif
#endif
                    ///dec
				#if WARNING_SD_USB
					cur_use_dev_real = music_player_get_cur_dev(obj);
					if (cur_use_dev_real != cache)
					{
						if (cur_use_dev_temp != cur_use_dev_real)
						{
							dac_mute(1,1);
							play_sd_usb_tone_flag = 1;
							cur_use_dev_temp = cur_use_dev_real;
						}
					}
					music_player_printf("~~~cur_use_dev_real = %s\n", dev_get_name_by_handle(cur_use_dev_real));
				#endif
                    music_decoder_set_file_interface(obj->dop, music_player_get_fs_hdl(dec_cipher.cipher_enable), file_operate_get_file_hdl(obj->fop));
                    if (bp_info) {
                        DEC_BP dec_bp_info;
                        dec_bp_info.bp_buf = bp_info->buf;
                        dec_bp_info.bp_size = bp_info->bp_size;
                        err = music_decoder_play(obj->dop, &dec_bp_info);

                    } else {
                        err = music_decoder_play(obj->dop, NULL);
                    }

                    if (err == MUSIC_DECODER_ERR_NONE) {
                        music_normal_end_flag = 0;
                        music_player_clear_break_point(obj, err);

                        music_player_printf("music_player_play ok !! fun = %s, line = %d\n", __func__, __LINE__);
#if FILTER_NULL_TIME_MUSIC
                        if (music_player_get_total_time(obj) != 0)
#endif
                        {
                            file_operate_set_auto_next(obj->fop, 1);
                            file_operate_clean_total_err_file(obj->fop);
                        }
				#if USE_MUSIC_STOP
					if (music_stop_flag)
		            	music_stop_flag = 0;
				#endif
                        return true;
                    } else {
                        music_player_printf("music_player_play dec fail %x !! fun = %s, line = %d\n", err, __func__, __LINE__);
                    }
                }
            }
        }



        if (is_auto) {
            bp_info = NULL;

            music_player_clear_break_point(obj, err);  //设备最后一个文件需要清除断点
            ret = music_player_err_deal(obj, err);
            if (ret == false) {
                music_player_printf("music_player_play fail !! fun = %s, line = %d\n", __func__, __LINE__);
                return false;
            } else {
                music_player_printf("auto err deal continue !!, fun = %s, line = %d\n", __func__, __LINE__);
            }
        }
    } while (is_auto);

    music_player_printf("music_player_play fail !! fun = %s, line = %d\n", __func__, __LINE__);
    return false;
}

tbool music_player_operation(MUSIC_PLAYER *obj, ENUM_FILE_SELECT_MODE op)
{
    if (obj == NULL) {
        return false;
    }

    music_player_printf("music_player_operation, %x\n", op);
    file_operate_set_file_sel_mode(obj->fop, op);
    return music_player_play(obj, NULL, 1);
}

tbool music_player_play_spec_num_file(MUSIC_PLAYER *obj, u32 filenum)
{
    if (obj == NULL) {
        return false;
    }

    file_operate_set_file_sel_mode(obj->fop, PLAY_SPEC_FILE);
    file_operate_set_file_number(obj->fop, filenum);
    return music_player_play(obj, NULL, 1);
}

tbool music_player_play_spec_break_point_file(MUSIC_PLAYER *obj, MUSIC_PLAYER_BP *bp_info)
{
    if (obj == NULL) {
        return false;
    }

    file_operate_set_file_sel_mode(obj->fop, PLAY_BREAK_POINT);
    return music_player_play(obj, bp_info, 1);
}

tbool music_player_play_path_file(MUSIC_PLAYER *obj, u8 *path, u32 index)
{
    if (obj == NULL) {
        return false;
    }

    file_operate_set_file_sel_mode(obj->fop, PLAY_FILE_BYPATH);
    file_operate_set_path(obj->fop, path, index);
    return music_player_play(obj, NULL, 1);
}


tbool music_player_play_spec_dev(MUSIC_PLAYER *obj, u32 dev)
{
    if (obj == NULL) {
        return false;
    }

    file_operate_set_file_sel_mode(obj->fop, PLAY_BREAK_POINT);
    file_operate_set_dev_sel_mode(obj->fop, DEV_SEL_SPEC);
    file_operate_set_dev(obj->fop, dev);
    return music_player_play(obj, NULL, 1);
}



tbool music_player_play_next_dev(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return false;
    }

    file_operate_set_file_sel_mode(obj->fop, PLAY_BREAK_POINT);
    file_operate_set_dev_sel_mode(obj->fop, DEV_SEL_NEXT);
    return music_player_play(obj, NULL, 1);
}



tbool music_player_play_prev_dev(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return false;
    }

    file_operate_set_file_sel_mode(obj->fop, PLAY_FIRST_FILE);
    file_operate_set_dev_sel_mode(obj->fop, DEV_SEL_PREV);
    return music_player_play(obj, NULL, 1);
}

tbool music_player_play_first_dev(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return false;
    }

    file_operate_set_file_sel_mode(obj->fop, PLAY_FIRST_FILE);
    file_operate_set_dev_sel_mode(obj->fop, DEV_SEL_FIRST);
    return music_player_play(obj, NULL, 1);
}

tbool music_player_play_last_dev(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return false;
    }

    file_operate_set_file_sel_mode(obj->fop, PLAY_FIRST_FILE);
    file_operate_set_dev_sel_mode(obj->fop, DEV_SEL_LAST);
    return music_player_play(obj, NULL, 1);
}


void music_player_set_repeat_mode(MUSIC_PLAYER *obj, ENUM_PLAY_MODE mode)
{
    if (obj == NULL) {
        return;
    }
    file_operate_set_repeat_mode(obj->fop, mode);
}

void music_player_set_auto_next(MUSIC_PLAYER *obj, u8 auto_next_flag)
{
    if (obj == NULL) {
        return;
    }
    file_operate_set_auto_next(obj->fop, auto_next_flag);
}

void music_player_set_decoder_init_sta(MUSIC_PLAYER *obj, MUSIC_DECODER_ST sta)
{
    if (obj == NULL) {
        return;
    }
    music_decoder_set_ini_sta(obj->dop, sta);
}
void music_player_ff(MUSIC_PLAYER *obj, u8 second)
{
    if (obj == NULL) {
        return;
    }
    music_player_ab_repeat_close(obj);
    music_decoder_fffr(obj->dop, PLAY_MOD_FF, second);
}

void music_player_fr(MUSIC_PLAYER *obj, u8 second)
{
    if (obj == NULL) {
        return;
    }
    music_player_ab_repeat_close(obj);
    music_decoder_fffr(obj->dop, PLAY_MOD_FB, second);
}


MUSIC_DECODER_ST music_player_pp(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return MUSIC_DECODER_ST_STOP;
    }
    return music_decoder_pp(obj->dop);
}

MUSIC_DECODER_ST music_player_pause(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return MUSIC_DECODER_ST_STOP;
    }
    return music_decoder_pause(obj->dop);
}

MUSIC_DECODER_ST music_player_play_resume(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return MUSIC_DECODER_ST_STOP;
    }
    return music_decoder_play_resume(obj->dop);
}


MUSIC_DECODER_ST music_player_get_status(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return MUSIC_DECODER_ST_STOP;
    }

    return music_decoder_get_status(obj->dop);
}

u32 music_player_get_total_file(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return 0;
    }
    return file_operate_get_file_total(obj->fop);
}

u32 music_player_get_cur_time(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return 0;
    }

    return music_decoder_get_decode_cur_time(obj->dop);
}


u32 music_player_get_total_time(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return 0;
    }

    return music_decoder_get_decode_total_time(obj->dop);
}


void *music_player_get_cur_dev(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return NULL;
    }

    return file_operate_get_dev(obj->fop);

}


tbool music_player_delete_playing_file(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return false;
    }

    music_decoder_stop(obj->dop);
    ///del file
    file_operate_op(obj->fop, FOP_DEL_FILE, NULL, NULL);
    //scan file
    file_operate_op(obj->fop, FOP_SCAN_FILE, NULL, NULL);
    return true;
}

u32 music_player_get_file_number(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return 0;
    }

    return file_operate_get_file_number(obj->fop);
}

u8 music_player_get_auto_next_flag(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return 0;
    }

    return file_operate_get_auto_next_flag(obj->fop);
}

FS_DISP_NAME *music_player_get_file_info(void)
{
#if LCD_128X64_EN
    return &music_file_info;
#else
    return NULL;
#endif
}


tbool music_player_ab_repeat_set(MUSIC_PLAYER *obj, u8 cmd, void *parm)
{
    if (obj == NULL) {
        return false;
    }
    if (music_decoder_get_status(obj->dop) != MUSIC_DECODER_ST_PLAY) {
        return false;
    }
    return	music_decoder_set_ab_repeat(obj->dop, cmd, parm);
}

void music_player_ab_repeat_close(MUSIC_PLAYER *obj)
{
    /* music_player_printf("%s\n",__func__); */

    if (obj == NULL) {
        return;
    }

    PARM_RECOVERMODE recover_mode;
    recover_mode.RECOVER_MODE_value = RECOVER_MODE_CUR;

    if (ab_repeat_status != AB_RPT_NON) {
        if (ab_repeat_status == AB_RPT_ASTA) {
            if (strcmp(music_decoder_get_decode_name(obj->dop), "FLA") == 0 ||
                strcmp(music_decoder_get_decode_name(obj->dop), "DTS") == 0 ||
                strcmp(music_decoder_get_decode_name(obj->dop), "APE") == 0) {
                music_decoder_set_ab_repeat(obj->dop, SET_BREAKPOINT_B, NULL);
            }
        }
        music_decoder_set_ab_repeat(obj->dop, SET_RECOVER_MODE, &recover_mode);
        ab_repeat_status = AB_RPT_NON;
    }
}

void music_player_ab_repeat_switch(MUSIC_PLAYER *obj)
{
    if (obj == NULL) {
        return;
    }

    PARM_RECOVERMODE recover_mode;
    recover_mode.RECOVER_MODE_value = RECOVER_MODE_CUR;

    switch (ab_repeat_status) {
    case AB_RPT_NON:
        if (music_player_ab_repeat_set(obj, SET_BREAKPOINT_A, NULL)) {
            music_player_printf("SET_BREAKPOINT_A_SUCCESS\n");
            ab_repeat_status = AB_RPT_ASTA;
        } else {
            music_player_printf("SET_BREAKPOINT_A_FAIL\n");
        }
        break;

    case AB_RPT_ASTA:
        if (music_player_ab_repeat_set(obj, SET_BREAKPOINT_B, NULL)) {
            music_player_printf("SET_BREAKPOINT_B_SUCCESS\n");
            ab_repeat_status = AB_RPT_BSTA;
        } else {
            music_player_printf("SET_BREAKPOINT_B_FAIL\n");
        }
        break;

    case AB_RPT_BSTA:
        if (music_player_ab_repeat_set(obj, SET_RECOVER_MODE, &recover_mode)) {
            music_player_printf("SET_RECOVER_MODE_SUCCESS\n");
            ab_repeat_status = AB_RPT_NON;
        } else {
            music_player_printf("SET_RECOVER_MODE_FAIL\n");
        }
        break;

    default:
        break;
    }
}


static void music_tone_err_deal(void *priv, u32 err)
{
    u32 msg;
    MUSIC_PLAYER *obj = (MUSIC_PLAYER *)priv;
    volatile u16 dac_len;

    if (err) {
        switch (err) {
        case MAD_ERROR_FILE_END:
            music_player_puts("music_tone END ok\n");
            if (tone_var.rpt_mode == 0) {
                mutex_resource_release("tone");
                msg = SYS_EVENT_PLAY_SEL_END;
                task_post_msg(NULL, 1, msg);

#if (ECHO_EN == 1)
                /* 播放提示音结束，回复高采样率，混响效果更好 */
                do {
                    dac_len = get_audio_stream_dac_len();
                } while (dac_len >= DAC_BUF_LEN);
                dac_set_samplerate(48000, 0);
#endif
            } else {
                tone_var.status = 0;
            }
            break;

        case MAD_ERROR_SYNC_LIMIT:
        case MAD_ERROR_NODATA:
            music_player_puts("music_tone NODATA ok\n");
            mutex_resource_release("tone");
            msg = SYS_EVENT_PLAY_SEL_END;
            task_post_msg(NULL, 1, msg);

#if (ECHO_EN == 1)
            /* 播放提示音结束，回复高采样率，混响效果更好 */
            do {
                dac_len = get_audio_stream_dac_len();
            } while (dac_len >= DAC_BUF_LEN);
            dac_set_samplerate(48000, 0);
#endif
            break;
        default:
            mutex_resource_release("tone");
            log_printf("music_tone END err:0x%x\n", err);
            break;
        }
    }
}

void music_tone_play(void *name)
{
    tbool ret = true;
    int err;
    music_player_puts("\n====play music tone start1===\n");
    dac_toggle(1);
    dac_channel_off(LINEIN_CHANNEL, 0);
    dac_channel_off(FM_IIC_CHANNEL, 0);
    dac_channel_on(MUSIC_CHANNEL, 0);
#if TONE_DEFAULT_VOL
    if (alarm_tone_flag == 0)//((alarm_ring_mode_flag == 0)||(vol_maxmin_play_flag))
    {
        sound.tmp_sys_vol_l = sound.vol.sys_vol_l;
        sound.tmp_sys_vol_r = sound.vol.sys_vol_r;
        if (tone_var.idx == TONE_RING)
        {
            sound.vol.sys_vol_l = 2*TONE_DEFAULT_VOL;
            sound.vol.sys_vol_r = 2*TONE_DEFAULT_VOL;
        }
        else
        {
            sound.vol.sys_vol_l = TONE_DEFAULT_VOL;
            sound.vol.sys_vol_r = TONE_DEFAULT_VOL;
        }
    }
    else
    {
    }
#else
    sound.tmp_sys_vol_l = sound.vol.sys_vol_l;
    sound.tmp_sys_vol_r = sound.vol.sys_vol_r;
#endif
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, 1);
#if USE_MUTE_PALYTONE_ENABLE
	if (mute_flag)
	{
		dac_mute(0,1);
	}
#endif
	AMP_UNMUTE();
    memset((u8 *)&music_player_hdl, 0x0, sizeof(MUSIC_PLAYER));
    MUSIC_PLAYER *obj = (MUSIC_PLAYER *)&music_player_hdl;
    tone_var.status = 1;
    ///music op creat
    obj->dop = music_decoder_creat();
    if (obj->dop == NULL) {
        music_player_printf("music player creat fail ! fun = %s, line = %d\n", __func__, __LINE__);
        ret = false;
        goto __exit;
    }

    ///file op creat
    obj->fop = file_operate_creat();
    if (obj->fop == NULL) {
        music_player_printf("file operate creat fail ! fun = %s, line = %d\n", __func__, __LINE__);
        ret = false;
        goto __exit;
    }

    memset((u8 *)music_decode_ram, 0x0, sizeof(music_decode_ram));
    music_decoder_set_configs(obj->dop, (DEC_CFG *)music_tone_decoder_configs, sizeof(music_tone_decoder_configs) / sizeof(music_tone_decoder_configs[0]));

    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_NULL;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    music_decoder_set_output(obj->dop, audio_stream_init(&stream_param, NULL));
    music_decoder_set_err_deal_interface(obj->dop, music_tone_err_deal, obj);
    file_operate_set_file_ext(obj->fop, "MP3WAV");
    file_operate_set_repeat_mode(obj->fop, REPEAT_ONE);

    file_operate_set_file_sel_mode(obj->fop, PLAY_FILE_BYPATH);
    file_operate_set_path(obj->fop, (void *)name, 0);

    file_operate_set_dev_sel_mode(obj->fop, DEV_SEL_SPEC);
    file_operate_set_dev(obj->fop, (u32)cache);
    ret = music_player_play(obj, NULL, 0);
    if (ret == false) {
        puts(">>>music_tone play faild\n");
    }

__exit:
    if (ret == false) {
        music_tone_err_deal(obj, MAD_ERROR_NODATA);
    }
    music_player_puts("====play music tone start2===\n");
}

void music_tone_end(void)
{
    music_player_puts("\n====play music tone end===\n");
    MUSIC_PLAYER *obj = (MUSIC_PLAYER *)&music_player_hdl;
    music_decoder_stop(obj->dop);
    music_player_destroy(&obj);
    tone_var.status = 0;
	//delay_2ms(100);
#if 1//TONE_DEFAULT_VOL
    if (alarm_tone_flag == 0)//((alarm_ring_mode_flag == 0)||(vol_maxmin_play_flag))
    {
        sound.vol.sys_vol_l = sound.tmp_sys_vol_l;
        sound.vol.sys_vol_r = sound.tmp_sys_vol_r;
    }
#endif
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
#if (WARNING_VOL_MAX || WARNING_VOL_MIN)
	//#if (WARNING_VOL_ONCE == 0)
	    //vol_maxmin_play_flag = 0;
	//#else
    	if ((vol_maxmin_play_flag == MSG_VOL_UP_SHORT)||(vol_maxmin_play_flag == MSG_VOL_DOWN_SHORT))
    		vol_maxmin_play_flag = 0;
	//#endif
#endif
    if (tone_display_flag)
        tone_back_cnt = 2;
	tone_display_flag = 0;
	alarm_tone_flag = 0;
}

void music_tone_stop(void)
{
    music_player_puts("\n====stop play music tone===\n");
    mutex_resource_release("tone");
}

#if REC_PLAY_EN
static void rec_play_err_deal(void *priv, u32 err)
{
    u32 msg;
    MUSIC_PLAYER *obj = (MUSIC_PLAYER *)priv;

    if (err) {
        switch (err) {
        case MAD_ERROR_FILE_END:
            music_player_puts("rec play END ok\n");
            msg = MSG_REC_PLAY_END;
            task_post_msg(NULL, 1, msg);
            break;
        default:
            log_printf("rec paly END err:0x%x\n", err);
            msg = MSG_REC_PLAY_ERR;
            task_post_msg(NULL, 1, msg);
            break;
        }
    }
}

tbool rec_player_play(MUSIC_PLAYER *obj)
{
    tbool ret = false;
    u32 err;
    if (obj == NULL) {
        return false;
    }

    ///stop dec
    music_decoder_stop(obj->dop);

    vm_check_api(1);

    ///dev sel
    err = file_operate_dev_sel(obj->fop);

    if (!err) {
        err = file_operate_dev_bp(obj->fop, NULL);

        if (!err) {
            ///file sel
            err = file_operate_file_sel(obj->fop,  NULL);
            if (!err) {
                ///dec
                music_decoder_set_file_interface(obj->dop, (AUDIO_FILE *)&music_player_file, file_operate_get_file_hdl(obj->fop));
                err = music_decoder_play(obj->dop, NULL);

                if (err == MUSIC_DECODER_ERR_NONE) {
                    music_player_printf("music_player_play ok !! fun = %s, line = %d\n", __func__, __LINE__);
                    return true;
                } else {
                    music_player_printf("music_player_play dec fail %x !! fun = %s, line = %d\n", err, __func__, __LINE__);
                }
            }
        }
    }
    music_player_printf("rec_player_play fail !! fun = %s, line = %d\n", __func__, __LINE__);
    return false;
}


void *rec_play(u32 dev, u32 file_sclust)
{
    tbool ret = true;
    int err;
    dac_channel_off(LINEIN_CHANNEL, 0);
    dac_channel_on(MUSIC_CHANNEL, 0);
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, 1);

    memset((u8 *)&music_player_hdl, 0x0, sizeof(MUSIC_PLAYER));
    MUSIC_PLAYER *obj = (MUSIC_PLAYER *)&music_player_hdl;
    ///music op creat
    obj->dop = music_decoder_creat();
    if (obj->dop == NULL) {
        music_player_printf("music player creat fail ! fun = %s, line = %d\n", __func__, __LINE__);
        ret = false;
        goto __exit;
    }

    ///file op creat
    obj->fop = file_operate_creat();
    if (obj->fop == NULL) {
        music_player_printf("file operate creat fail ! fun = %s, line = %d\n", __func__, __LINE__);
        ret = false;
        goto __exit;
    }

    memset((u8 *)music_decode_ram, 0x0, sizeof(music_decode_ram));
    music_decoder_set_configs(obj->dop, (DEC_CFG *)rec_play_decoder_configs, sizeof(rec_play_decoder_configs) / sizeof(rec_play_decoder_configs[0]));

    AUDIO_STREAM_PARAM stream_param;
    stream_param.ef = AUDIO_EFFECT_NULL;
    stream_param.ch = 2;
    stream_param.sr = SR44100;
    music_decoder_set_output(obj->dop, audio_stream_init(&stream_param, NULL));
    music_decoder_set_err_deal_interface(obj->dop, rec_play_err_deal, obj);
    file_operate_set_file_ext(obj->fop, "MP3MP2WAV");
    file_operate_set_repeat_mode(obj->fop, REPEAT_ONE);

    file_operate_set_file_sel_mode(obj->fop, PLAY_SCLUCT_FILE);
    file_operate_set_file_sclust(obj->fop, file_sclust);

    file_operate_set_dev_sel_mode(obj->fop, DEV_SEL_SPEC);
    file_operate_set_dev(obj->fop, dev);
    //music_player_play(obj, NULL, 0);
    if (false == rec_player_play(obj)) {
        ret = false;
        goto __exit;
    }

    return obj;
__exit:
    if (ret == false) {
        music_player_destroy(&obj);
    }
    return NULL;
}

void rec_play_stop(void)
{
    music_player_puts("\n====rec paly end===\n");
    MUSIC_PLAYER *obj = (MUSIC_PLAYER *)&music_player_hdl;
    music_decoder_stop(obj->dop);
    music_player_destroy(&obj);
    set_sys_vol(sound.vol.sys_vol_l, sound.vol.sys_vol_r, FADE_ON);
}
#endif
///demo
/*
 * get break point and play
 */
/*
MUSIC_PLAYER_BP bp_info;
if(music_player_get_break_point_info(obj, &bp_info) == true)
{
	music_player_play_spec_break_point_file(obj, &bp_info);
}
*/

