#include "file_operate.h"
#include "file_op_err.h"
#include "fs_io.h"
#include "file_io.h"
#include "string.h"

#include "dev_mg_api.h"
#include "msg.h"
#include "sdk_cfg.h"

#include "dev_manage.h"


//#define FILE_OPERATE_DEBUG_ENABLE
#ifdef FILE_OPERATE_DEBUG_ENABLE
#define fop_printf printf
#else
#define fop_printf(...)
#endif//FILE_OPERATE_DEBUG_ENABLE


/* #define FILE_OPERATE_MALLOC_EN */
#ifdef  FILE_OPERATE_MALLOC_EN
#include "mem/malloc.h"
#define file_operate_malloc	malloc
#define file_operate_calloc	calloc
#define file_operate_free	free
#endif//FILE_OPERATE_MALLOC_EN



typedef struct __FILE_OPERATE_INFO {
    ENUM_DEV_SEL_MODE      cur_dev_mode;        ///<设备选择模式
    ENUM_PLAY_MODE         cur_play_mode;       ///<文件播放模式
    ENUM_FILE_SELECT_MODE  cur_sel_mode;        ///<文件选择模式
    const char     		  *filetype;			///文件后缀配置, 例如"MP3WAVWMA"
    u8		 			  *filepath;            ///<文件路径
    u32 				   filenum;
    u32                    filesclust;          ///<文件簇
    u32                    dev;                 ///<指定设备
    u32					   dev_part_index;		///设备当前分区
    u32   	  			   total_file;
    u32 				   total_err_file;
    /* u8					   total_err_dev; */
    u8					   auto_file_next;
} FILE_OPERATE_INFO;



struct __FILE_OPERATE {
    FILE_OPERATE_INFO 	  *fop_info;        ///<文件操作器操作文件属性
    _FILE				  *fop_file;
};



static u32 random_number(u32 start, u32 end)
{
    if (end <= start) {
        return start;
    }

    return JL_TIMER0->CNT % (end - start + 1) + start;
}


static FILE_OPERATE file_operate_hdl;
static FILE_OPERATE_INFO file_operate_info_hdl;
static _FILE cur_file_hdl;

FILE_OPERATE *file_operate_creat(void)
{
    u8 *need_buf;
    u32 need_buf_len;
    FILE_OPERATE *obj;
#if 0
    need_buf_len = SIZEOF_ALIN(sizeof(FILE_OPERATE), 4)
                   + SIZEOF_ALIN(sizeof(FILE_OPERATE_INFO), 4)
                   + SIZEOF_ALIN(sizeof(_FILE), 4);

    need_buf = (u8 *)file_operate_calloc(1, need_buf_len);
    if (need_buf == NULL) {
        return NULL;
    }
    obj = (FILE_OPERATE *)need_buf;

    need_buf += SIZEOF_ALIN(sizeof(FILE_OPERATE), 4);
    obj->fop_info = (FILE_OPERATE_INFO *)need_buf;

    need_buf += SIZEOF_ALIN(sizeof(FILE_OPERATE_INFO), 4);
    obj->fop_file = (_FILE *)need_buf;


#else

    memset((u8 *)&file_operate_hdl, 0x0, sizeof(FILE_OPERATE));
    memset((u8 *)&file_operate_info_hdl, 0x0, sizeof(FILE_OPERATE_INFO));
    memset((u8 *)&cur_file_hdl, 0x0, sizeof(_FILE));
    /* memset((u8 *)&br_point_hdl, 0x0, sizeof(FS_BRK_POINT)); */

    obj = (FILE_OPERATE *)&file_operate_hdl;

    obj->fop_info = (FILE_OPERATE_INFO *)&file_operate_info_hdl;

    obj->fop_file = (_FILE *)&cur_file_hdl;
#endif


    return obj;
}




void file_operate_destroy(FILE_OPERATE **hdl)
{
    if (hdl == NULL || *hdl == NULL) {
        return ;
    }
    FILE_OPERATE *obj = *hdl;
    file_api_destroy(obj->fop_file);

    dev_power_off_spec_dev((DEV_HANDLE)obj->fop_info->dev); //不使用设备，关闭设备电源

#if ((USB_SD0_MULT_EN == 1)||(USB_SD1_MULT_EN == 1))
    io_clean();
#endif

#if 0
    file_operate_free(obj);
#endif
    *hdl = NULL;
}


void file_operate_set_dev_sel_mode(FILE_OPERATE *obj, ENUM_DEV_SEL_MODE mode)
{
    if (obj == NULL) {
        return ;
    }
    if (mode >= MAX_DEV_MODE || mode < DEV_SEL_CUR) {
        fop_printf("dev sel mode is over limit !!! fun = %s, line = %d\n", __func__, __LINE__);
        return;
    }
    obj->fop_info->cur_dev_mode = mode;
}

void file_operate_set_repeat_mode(FILE_OPERATE *obj, ENUM_PLAY_MODE mode)
{
    if (obj == NULL) {
        return ;
    }
    if (mode >= MAX_PLAY_MODE || mode < REPEAT_ALL) {
        fop_printf("repeat mode is over limit !!! fun = %s, line = %d\n", __func__, __LINE__);
    }
    obj->fop_info->cur_play_mode = mode;
}

void file_operate_set_file_sel_mode(FILE_OPERATE *obj, ENUM_FILE_SELECT_MODE mode)
{
    if (obj == NULL) {
        return ;
    }
    if (mode >= MAX_FILE_SEL_MODE || mode < PLAY_NEXT_FILE) {
        fop_printf("file sel mode is over limit !!! fun = %s, line = %d\n", __func__, __LINE__);
    }
    obj->fop_info->cur_sel_mode = mode;
}

void file_operate_set_file_number(FILE_OPERATE *obj, u32 index)
{
    if (obj == NULL) {
        return ;
    }

    obj->fop_info->filenum = index;
}

void file_operate_set_file_sclust(FILE_OPERATE *obj, u32 sclust)
{
    if (obj == NULL) {
        return ;
    }
    obj->fop_info->filesclust = sclust;
}


void file_operate_set_path(FILE_OPERATE *obj, u8 *path, u32 index)
{
    if (obj == NULL) {
        return ;
    }

    obj->fop_info->filepath = path;
    obj->fop_info->filenum = index;
}

void file_operate_set_dev(FILE_OPERATE *obj, u32 dev)
{
    if (obj == NULL) {
        return ;
    }

    obj->fop_info->dev = dev;
}

void file_operate_set_file_ext(FILE_OPERATE *obj, const char *ext)
{
    if (obj == NULL) {
        return ;
    }
    obj->fop_info->filetype = ext;
}


void file_operate_set_auto_next(FILE_OPERATE *obj, u32 auto_auto_flag)
{
    if (obj == NULL) {
        return ;
    }

    obj->fop_info->auto_file_next = auto_auto_flag;
}

void file_operate_clean_total_err_file(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return ;
    }

    obj->fop_info->total_err_file = 0;
}


ENUM_DEV_SEL_MODE file_operate_get_dev_sel_mode(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return MAX_DEV_MODE;
    }
    return obj->fop_info->cur_dev_mode;
}



ENUM_PLAY_MODE file_operate_get_repeat_mode(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return MAX_PLAY_MODE;
    }
    return obj->fop_info->cur_play_mode;
}


ENUM_FILE_SELECT_MODE file_operate_get_file_sel_mode(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return MAX_FILE_SEL_MODE;
    }
    return obj->fop_info->cur_sel_mode;
}


u32 file_operate_get_file_number(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return 0;
    }
    return obj->fop_info->filenum;
}

u32 file_operate_get_file_total(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return 0;
    }
    return obj->fop_info->total_file;
}


void *file_operate_get_dev(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return 0;
    }
    return (void *)obj->fop_info->dev;
}


void *file_operate_get_file_hdl(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return NULL;
    }
    return (void *) & (obj->fop_file->file_hdl);
}

void *file_operate_get_lrc_hdl(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return NULL;
    }
    return (void *) & (obj->fop_file->lrc_hdl);
}

u8 file_operate_get_auto_next_flag(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return 0;
    }

    return obj->fop_info->auto_file_next;
}

void *file_operate_get_fs_hdl(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return NULL;
    }
    return (void *) & (obj->fop_file->fs_hdl);
}
#if 1

u32 file_operate_op(FILE_OPERATE *obj, u32 cmd, void *input, void *output)
{
    s32 ret;
    u32 status = FILE_OP_NO_ERR;
    if (obj == NULL) {
        return FILE_OP_ERR_OP_HDL;
    }

    switch (cmd) {
    case FOP_FIND_SPEC_LGDEV:

        break;
    case FOP_GET_TOTALFILE_NUM:
        fop_printf("FOP_GET_TOTALFILE_NUM\n");
        obj->fop_info->total_file = fs_get_file_total(&(obj->fop_file->fs_hdl), obj->fop_info->filetype, 0, NULL);
        break;

    case FOP_OPEN_FILE_BYNUM:
        fop_printf("FOP_OPEN_FILE_BYNUM\n");
        ret = fs_get_file_byindex(&(obj->fop_file->fs_hdl), &(obj->fop_file->file_hdl), obj->fop_info->filenum);
        if (!ret) {
        } else {
            status = FILE_OP_ERR_NUM;
        }
        break;


    case FOP_OPEN_FILE_BYPATH:
        ///需要添加路径匹配 需要获取当前文件号!!!!!!!!!!
        fop_printf("FOP_OPEN_FILE_BYPATH\n");
        ret = fs_get_file_bypath(&(obj->fop_file->fs_hdl), &(obj->fop_file->file_hdl), (u8 *)obj->fop_info->filepath);
        if (!ret) {
            fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FILE_NUMBER, &(obj->fop_info->filenum));
        } else {
            status = FILE_OP_ERR_NUM;
        }
        break;
    case FOP_OPEN_FILE_BYBREAKPOINT:
        fop_printf("FOP_OPEN_FILE_BYBREAKPOINT\n");
        ret = fs_get_file_bysclust(&(obj->fop_file->fs_hdl), &(obj->fop_file->file_hdl), ((FOP_BP *)input)->sclust);
        if (!ret) {
            u32 f_size;
            fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FILE_SIZE, &f_size);
            fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FILE_NUMBER, &(obj->fop_info->filenum));  //获取当前文件序号
            if ((((FOP_BP *)input)->f_size == 0) || (f_size == ((FOP_BP *)input)->f_size)) {
                fop_printf("open break point file ok!!\n");
            } else {
                fop_printf("open break point file fail!!, line = %d\n", __LINE__);
                status = FILE_OP_ERR_OPEN_BPFILE;
                /* fs_close(&(obj->fop_file->file_hdl));	 */
            }
        } else {
            fop_printf("open break point file fail!!, line = %d\n", __LINE__);
            status = FILE_OP_ERR_OPEN_BPFILE;
        }
        break;

    case FOP_OPEN_FILE_BYSCLUCT:    //指定簇号播放，用于
        ret = fs_get_file_bysclust(&(obj->fop_file->fs_hdl), &(obj->fop_file->file_hdl), obj->fop_info->filesclust);
        if (!ret) {
            fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FILE_NUMBER, &(obj->fop_info->filenum));
        } else {
            status = FILE_OP_ERR_NUM;
        }
        break;

    case FOP_OPEN_FILE_BYNAME:
        ret = fs_get_file_byname_indir(&(obj->fop_file->fs_hdl), &(obj->fop_file->file_hdl), &(obj->fop_file->lrc_hdl), input);
        if (ret) {
            status = FILE_OP_ERR_OPEN_FILE;
        }
        break;

    case FOP_DEV_STATUS:
        /* status=lg_dev_online_status(cur_fop->cur_lgdev_info); */
        break;

    case FOP_CLOSE_LOGDEV:
        fop_printf("close all lg dev \n");
        /* lg_dev_close_all(); */
        break;

    case FOP_GET_FILE_NAME:
        status = fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FILE_NAME, output);
        break;

    case FOP_GET_FILE_INFO:
        status = fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FILE_INFO, output);
        break;

    case FOP_ALLFILE_ERR_LGDEV:
        /* cur_fop->cur_lgdev_info->allfileerr=1; */
        break;

    case FOP_GET_PHYDEV_INFO:
        /* status=lg_dev_get_phydev_type(cur_fop->cur_lgdev_info); */
        break;

    case FOP_GET_FOLDER_INFO:
        /* status=lg_dev_get_folder_info(cur_fop->cur_lgdev_info,input,output); */
        break;

    /* #if GET_LFN */
    /* case FOP_GET_FILE_NAME: */
    /* status = fs_file_name(cur_fop->cur_lgdev_info->lg_hdl->fs_hdl,cur_fop->cur_lgdev_info->lg_hdl->file_hdl, output); */
    /* break; */
    /* #endif */
    case FOP_CREAT_FOLDER:
        fop_printf("FOP_CREAT_FOLDER \n");
        //status = fs_mk_dir(&(obj->fop_file->fs_hdl), input, FA_CREATE_NEW);
        status = fs_io_ctrl(&(obj->fop_file->fs_hdl), NULL, FS_IO_MKDIR, input, FA_CREATE_NEW);
        break;
    case FOP_DEL_FILE:
        fop_printf("FOP_DEL_FILE \n");
        status = fs_delete(&(obj->fop_file->file_hdl));
        break;

    case FOP_SCAN_FILE:
        obj->fop_info->total_file = fs_get_file_total(
                                        &(obj->fop_file->fs_hdl),
                                        obj->fop_info->filetype,
                                        0,
                                        0
                                    );
        status = obj->fop_info->total_file;
        fop_printf("FOP_SCAN_FILE %d\n", status);
        break;
    default:
        status = FILE_OP_ERR_CMD;     ///<不能识别的命令
        break;
    }
    return status;
}

u32 file_operate_get_file_name(FILE_OPERATE *obj, void *f_path)
{
    return file_operate_op(obj,  FOP_GET_FILE_NAME, NULL, f_path);
}
u32 file_operate_get_file_info(FILE_OPERATE *obj, void *f_info)
{
    return file_operate_op(obj,  FOP_GET_FILE_INFO, NULL, f_info);
}

u32 file_operate_open_file_byname(FILE_OPERATE *obj, void *f_name)
{
    return file_operate_op(obj,  FOP_OPEN_FILE_BYNAME, f_name, NULL);
}

extern s32 fs_folder_file(_FIL_HDL *p_f_hdl, u32 *start_file, u32 *end_file);

u32 file_operate_file_sel(FILE_OPERATE *obj, FOP_BP *bp_info)
{
    FILE_OPERATE *cur_fop;
    s32 status;
    s32 step = 0;
    u32 dev_type;
    /* u32 curr_file = *((u32*)input); */
    u32 curr_file = obj->fop_info->filenum;
    u32 first_file, last_file;
    u32 file_total;
    u32 f_file = 0, e_file = 0;
    char *f_path = NULL;

    /* if(obj == NULL) */
    /* return FILE_OP_ERR_OP_HDL; */


    file_total = obj->fop_info->total_file;
    first_file = 1;
    last_file = file_total;

    ///----------选择播放模式-------------------------------
    fop_printf("file_operate_file_sel cur_play_mode = 0x%x  \n", obj->fop_info->cur_play_mode);
    switch (obj->fop_info->cur_play_mode) {
    case REPEAT_FOLDER:
        //获取当前文件夹first、last
        if (PLAY_BREAK_POINT == obj->fop_info->cur_sel_mode) {
            break;
        }
        status = fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FOLDER_FILE, &first_file, &last_file);
        if (status) {
            first_file = 1;
            last_file = file_total;
        }
        break;

    case REPEAT_ONE:
        if (PLAY_AUTO_NEXT == obj->fop_info->cur_sel_mode) {
            first_file = curr_file;
            last_file = curr_file;
        }
        break;

    case REPEAT_RANDOM:
        //随机数获取
        curr_file = random_number(first_file, last_file);
        break;

    case REPEAT_ONE_LGDEV:
    case REPEAT_ALL:
    case REPEAT_NORMAL:
    default :
        break;
    }

    fop_printf("cur_sel_mode = 0x%x  \n", obj->fop_info->cur_sel_mode);
    ///----------选择文件-------------------------------
    switch (obj->fop_info->cur_sel_mode) {
    case PLAY_NEXT_FILE:
    case PLAY_AUTO_NEXT:
        step = 1;
        obj->fop_info->auto_file_next = 1;
        break;
    case PLAY_PREV_FILE:
        step = -1;
        obj->fop_info->auto_file_next = 0;
        break;
    case PLAY_FIRST_FILE:
        if (obj->fop_info->cur_play_mode == REPEAT_FOLDER) {
            curr_file = first_file;
            goto _open_file;
        }
        curr_file = 1;
        goto _open_file;
    case PLAY_LAST_FILE:
        if (obj->fop_info->cur_play_mode == REPEAT_FOLDER) {
            curr_file = last_file;
            goto _open_file;
        }
        curr_file = file_total;
        goto _open_file;
    case PLAY_SPEC_FILE:
        if (curr_file == 0 || curr_file > file_total) {
            return FILE_OP_ERR_NUM;
        }
        goto _open_sel_file;
    case PLAY_BREAK_POINT:
    case PLAY_SCLUCT_FILE:
        goto _open_file;
    case PLAY_NEXT_FOLDER:
        status = fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_CHANGE_FOLDER, 1, &file_total, &curr_file);
        if (status) {
            return FILE_OP_ERR_NUM;
        } else {
            goto _open_file;
        }
        break;

    case PLAY_PRE_FOLDER:
        status = fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_CHANGE_FOLDER, 0, &file_total, &curr_file);
        if (status) {
            return FILE_OP_ERR_NUM;
        } else {
            goto _open_file;
        }
        break;
    default :
        break;
    }
    curr_file += step;
    if ((REPEAT_RANDOM == obj->fop_info->cur_play_mode)
        && curr_file == obj->fop_info->filenum)
        /* && curr_file==cur_fop->cur_lgdev_info->last_op_file_num) */
    {
        ///随机数不要等于本身
        curr_file++;
    }
_open_sel_file:
    ///文件范围检测
    if (curr_file > last_file) {
        if (obj->fop_info->cur_play_mode != REPEAT_ALL) {
            curr_file =  first_file;
        } else {
            return FILE_OP_ERR_END_FILE;
        }
    } else if (curr_file < first_file) {
        if (obj->fop_info->cur_play_mode != REPEAT_ALL) {
            curr_file =  last_file;
        } else {
            return FILE_OP_ERR_PRE_FILE;
        }
    }

_open_file:

    fop_printf("curr_file num = 0x%x\n", curr_file);
    if (obj->fop_info->cur_sel_mode == PLAY_BREAK_POINT) {
        status = file_operate_op(obj, FOP_OPEN_FILE_BYBREAKPOINT, bp_info, NULL);
    } else if (obj->fop_info->cur_sel_mode == PLAY_FILE_BYPATH) {
        fop_printf("PLAY_FILE_BYPATH  = %s\n", obj->fop_info->filepath);
        status = file_operate_op(obj, FOP_OPEN_FILE_BYPATH, NULL, NULL);
    } else if (obj->fop_info->cur_sel_mode == PLAY_SCLUCT_FILE) {
        fop_printf("PLAY_FILE_BYSCLUST  = %d\n", obj->fop_info->filesclust);
        status = file_operate_op(obj, FOP_OPEN_FILE_BYSCLUCT, NULL, NULL);
    } else {
        obj->fop_info->filenum = curr_file;
        status = file_operate_op(obj, FOP_OPEN_FILE_BYNUM, NULL, NULL);
    }
    if (!status) {
        if (obj->fop_info->cur_sel_mode == PLAY_PRE_FOLDER) {
            status = fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FOLDER_FILE, &f_file, &e_file);
            /* status = fs_folder_file(&(obj->fop_file->file_hdl), &f_file, &e_file); */
            if (status) {
                curr_file = f_file;
                obj->fop_info->cur_sel_mode = PLAY_SPEC_FILE;
                goto _open_file;
            }
        }
        fop_printf("open file ok  \n");
        fop_printf("file type %s  \n", (obj->fop_info->filetype));

        //file path
        status = fs_io_ctrl(NULL, &(obj->fop_file->file_hdl), FS_IO_GET_FILE_NAME, &f_path);
        if (f_path) {
            printf("fat_get_file_name :%s\n", f_path);
        }

    } else {
        fop_printf("open file err  \n");
    }

    /* *((u32*)input) = curr_file; */

    return status;
}


u32 file_operate_dev_sel(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return FILE_OP_ERR_NOT_INIT;
    }
    tbool ret;
    void *tmp_dev = (void *)obj->fop_info->dev;
    u32 parm = 0;
    /* if (obj->fop_info->cur_dev_mode == DEV_SEL_CUR) { */

    /* } */

    switch (obj->fop_info->cur_dev_mode) {
    case DEV_SEL_CUR:
        fop_printf("DEV_SEL_CUR, fun = %s, line = %d\n", __func__, __LINE__);
        ///check dev online status
        u32 dev_status = 0;
        if (dev_get_online_status(tmp_dev, &dev_status)) {
            obj->fop_info->cur_dev_mode = DEV_SEL_FIRST;
            return FILE_OP_ERR_LGDEV_HDL;
        } else {
            if (dev_status == DEV_ONLINE) {
                return FILE_OP_NO_ERR;
            } else {
                fop_printf("cur dev offline !! fun = %s, line = %d\n",  __func__, __LINE__);
                return FILE_OP_ERR_LGDEV_OFFLINE;
            }
        }
        break;

    case DEV_SEL_SPEC:
        fop_printf("DEV_SEL_SPEC, fun = %s, line = %d\n", __func__, __LINE__);
        if (dev_get_online_status(tmp_dev, &dev_status)) {
            obj->fop_info->cur_dev_mode = DEV_SEL_FIRST;
            return FILE_OP_ERR_LGDEV_HDL;
        } else {
            if (dev_status == DEV_ONLINE) {
                obj->fop_info->dev_part_index = 0;
                break;
            } else {
                fop_printf("spec dev offline !! fun = %s, line = %d\n",  __func__, __LINE__);
                return FILE_OP_ERR_LGDEV_OFFLINE;
            }
        }

        break;

    case DEV_SEL_FIRST:
        fop_printf("last dev = %x,  fun = %s, line = %d\n", tmp_dev, __func__, __LINE__);
        tmp_dev = (void *)dev_get_fisrt(MUSIC_DEV_TYPE, DEV_ONLINE);
        obj->fop_info->dev_part_index = 0;
        fop_printf("cur dev = %x,  fun = %s, line = %d\n", tmp_dev, __func__, __LINE__);
        /* dev_part_get_base_addr(tmp_dev, obj->fop_info->dev_part_index, 0); */
        break;

    case DEV_SEL_LAST:
        fop_printf("last dev = %x,  fun = %s, line = %d\n", tmp_dev, __func__, __LINE__);
        tmp_dev = (void *)dev_get_last(MUSIC_DEV_TYPE, DEV_ONLINE);
        if (dev_get_specific_part_total(tmp_dev, &parm)) {
            return FILE_OP_ERR_LGDEV_NO_FIND;
        }
        if (parm) {
            obj->fop_info->dev_part_index = parm - 1;
        }
        break;

    case DEV_SEL_PREV:
        fop_printf("last dev = %x,  fun = %s, line = %d\n", tmp_dev, __func__, __LINE__);
        if (obj->fop_info->dev_part_index) {
            obj->fop_info->dev_part_index--;
        } else {
            tmp_dev = (void *)dev_get_prev(tmp_dev, MUSIC_DEV_TYPE, DEV_ONLINE);
            if (tmp_dev) {
                if (dev_get_specific_part_total(tmp_dev, &parm)) {
                    return FILE_OP_ERR_LGDEV_NO_FIND;
                }
                if (parm) {
                    obj->fop_info->dev_part_index = parm - 1;
                }
            } else {
                ///返回到链表尾部
                if (dev_get_phydev_total(MUSIC_DEV_TYPE, DEV_ONLINE)) {
                    tmp_dev = (void *)dev_get_last(MUSIC_DEV_TYPE, DEV_ONLINE);
                    if (dev_get_specific_part_total(tmp_dev, &parm)) {
                        return FILE_OP_ERR_LGDEV_NO_FIND;
                    }
                    if (parm) {
                        obj->fop_info->dev_part_index = parm - 1;
                    }
                } else {
                    fop_printf("FILE_OP_ERR_LGDEV_NULL,  fun = %s, line = %d\n",  __func__, __LINE__);
                    return FILE_OP_ERR_LGDEV_NULL;
                }
            }
        }
        break;

    case DEV_SEL_NEXT:
        fop_printf("last dev = %x,  fun = %s, line = %d\n", tmp_dev, __func__, __LINE__);
        obj->fop_info->dev_part_index++;
        if (dev_get_specific_part_total(tmp_dev, &parm)) {
            fop_printf("get dev = %x total fail !!!,  fun = %s, line = %d\n", tmp_dev, __func__, __LINE__);
            return FILE_OP_ERR_LGDEV_HDL;
        }

        if (obj->fop_info->dev_part_index >= parm) {
            obj->fop_info->dev_part_index = 0;
            tmp_dev = (void *)dev_get_next(tmp_dev, MUSIC_DEV_TYPE, DEV_ONLINE);
            if (tmp_dev == NULL) {
                if (dev_get_phydev_total(MUSIC_DEV_TYPE, DEV_ONLINE)) {
                    tmp_dev = (void *)dev_get_fisrt(MUSIC_DEV_TYPE, DEV_ONLINE);
                    obj->fop_info->dev_part_index = 0;
                } else {
                    fop_printf("FILE_OP_ERR_LGDEV_NULL,  fun = %s, line = %d\n",  __func__, __LINE__);
                    return FILE_OP_ERR_LGDEV_NULL;
                }
            }
        }
        break;

    default:
        fop_printf("default,  fun = %s, line = %d\n",  __func__, __LINE__);
        break;
    }

    fop_printf("dev mode = %x, dev = %x,  fun = %s, line = %d\n", obj->fop_info->cur_dev_mode, tmp_dev, __func__, __LINE__);

    /* file_api_destroy(obj->fop_file); */
    /* ret = file_api_creat(obj->fop_file, (void *)tmp_dev, obj->fop_info->dev_part_index); */
    /* if (ret == false) { */
    /* return FILE_OP_ERR_FS; */
    /* } */

    dev_power_off_spec_dev((DEV_HANDLE)obj->fop_info->dev); //不使用设备，关闭设备电源

    obj->fop_info->dev = (u32)tmp_dev;
    obj->fop_info->total_err_file = 0;


    return FILE_OP_NO_ERR;
}

u32 file_operate_dev_bp(FILE_OPERATE *obj, FOP_BP *bp_info)
{
    u32 find_bp_file = 0;
    u32 parm = 0;
    /* u32 tmp_dev_total; */
    tbool ret;

    if (obj->fop_info->cur_dev_mode !=  DEV_SEL_CUR) {
        /* void *tmp_dev = (void *)obj->fop_info->dev; */
        /* if (dev_get_all_dev_part_total(MUSIC_DEV_TYPE, &tmp_dev_total)) { */
        /* return FILE_OP_ERR_LGDEV_HDL; */
        /* } */
        /* fop_printf("tmp dev total %d\n", tmp_dev_total); */

        file_api_destroy(obj->fop_file);
        dev_mult_sel_deal((DEV_HANDLE)obj->fop_info->dev);  //设备IO复用时，打开设备前先处理设备开关
        dev_power_on_spec_dev((DEV_HANDLE)obj->fop_info->dev);   //打开设备电源
        ret = file_api_creat(obj->fop_file, (void *)obj->fop_info->dev, obj->fop_info->dev_part_index);
        if (ret == false) {
            dev_rm_specific_part((void *)obj->fop_info->dev, obj->fop_info->dev_part_index);
#if ((USB_SD0_MULT_EN == 1)||(USB_SD1_MULT_EN == 1))
            io_clean();
#endif
            return FILE_OP_ERR_LGDEV_NO_FIND;
        } else {
            //find updata_file
            extern const char updata_file_name[];
            ret = fs_get_file_bypath(&(obj->fop_file->fs_hdl), &(obj->fop_file->file_hdl), (void *)updata_file_name);
            if (ret == FILE_OP_NO_ERR) {
                fop_printf("\n\n\n\n----------------fs_get_file_bypath, ret = %d\n", ret);
                task_post_msg(NULL, 1, MSG_UPDATA);
            }

            obj->fop_info->total_file = fs_get_file_total(
                                            &(obj->fop_file->fs_hdl),
                                            obj->fop_info->filetype,
                                            ((bp_info != NULL) ? bp_info->sclust : 0),
                                            ((bp_info != NULL) ? & (find_bp_file) : 0)
                                        );

            if (obj->fop_info->total_file) {
                /* obj->fop_info->total_err_dev = 0; */
                obj->fop_info->cur_dev_mode = DEV_SEL_CUR;
                /* obj->fop_info->dev = (u32)tmp_dev; */

                fop_printf("file total = %d, dev = %x, fun = %s, line = %d\n", obj->fop_info->total_file, obj->fop_info->dev, __func__, __LINE__);

                if ((obj->fop_info->cur_sel_mode == PLAY_BREAK_POINT) && (find_bp_file == 0)) {
                    fop_printf("bp_info->sclust = %d\n", bp_info->sclust);
                    return FILE_OP_ERR_OPEN_BPFILE;
                }

                return FILE_OP_NO_ERR;
            } else {
                dev_rm_specific_part((void *)obj->fop_info->dev, obj->fop_info->dev_part_index);
                return FILE_OP_ERR_LGDEV_NO_FIND;
            }
        }

        /*
        if (dev_get_all_dev_part_total(MUSIC_DEV_TYPE, &parm)) {
            obj->fop_info->total_err_dev = 0;
            return FILE_OP_ERR_LGDEV_HDL;
        } else {
            if (tmp_dev_total != parm) {
                obj->fop_info->total_err_dev = 0;
            } else {
                obj->fop_info->total_err_dev++;
            }
        }*/


        /* if (obj->fop_info->total_err_dev >= parm) { */
        /* fop_printf("total err dev %d-%d", obj->fop_info->total_err_dev, parm); */
        /* return FILE_OP_ERR_LGDEV_NULL; */
        /* } */
        /* return FILE_OP_ERR_LGDEV_NO_FIND; */
    }

    return FILE_OP_NO_ERR;
}

tbool file_operate_err_file_statistics_deal(FILE_OPERATE *obj)
{
    if (obj == NULL) {
        return false;
    }

    u32 dev_status = 0;
    if (dev_get_online_status(file_operate_get_dev(obj), &dev_status)) {
        obj->fop_info->total_err_file = 0;
        file_operate_set_file_sel_mode(obj, PLAY_FIRST_FILE);
        file_operate_set_dev_sel_mode(obj, DEV_SEL_NEXT);
        return true;
    } else {
        obj->fop_info->total_err_file++;
        fop_printf("total_err_file = %d\n", obj->fop_info->total_err_file);
        if ((dev_status == 0) || (obj->fop_info->total_err_file >= file_operate_get_file_total(obj))) {
            fop_printf("dev sel next!!! fun = %s, line = %d\n", __func__, __LINE__);
            dev_rm_specific_part((void *)obj->fop_info->dev, obj->fop_info->dev_part_index);
            obj->fop_info->total_err_file = 0;
            /*
            	file_operate_set_file_sel_mode(obj, PLAY_BREAK_POINT);
            	file_operate_set_dev_sel_mode(obj, DEV_SEL_NEXT);
            */
            if (obj->fop_info->auto_file_next) {
                file_operate_set_file_sel_mode(obj, PLAY_BREAK_POINT);
                file_operate_set_dev_sel_mode(obj, DEV_SEL_NEXT);
            } else {
                file_operate_set_file_sel_mode(obj, PLAY_BREAK_POINT);
                file_operate_set_dev_sel_mode(obj, DEV_SEL_PREV);
            }
            return true;
        }
    }

    if (obj->fop_info->auto_file_next) {
        file_operate_set_file_sel_mode(obj, PLAY_NEXT_FILE);
    } else {
        file_operate_set_file_sel_mode(obj, PLAY_PREV_FILE);
    }

    return true;
}

tbool file_operate_err_deal(FILE_OPERATE *obj, u32 err)
{
    if (obj == NULL) {
        return false;
    }
    switch (err) {
    case FILE_OP_ERR_NOT_INIT:
        return false;

    case FILE_OP_ERR_FS:
        return false;
    case FILE_OP_ERR_END_FILE:
        fop_printf("FILE_OP_ERR_END_FILE -----\n");
        file_operate_set_file_sel_mode(obj, PLAY_FIRST_FILE);
        file_operate_set_dev_sel_mode(obj, DEV_SEL_NEXT);
        break;
    case FILE_OP_ERR_OPEN_BPFILE:
        if (obj->fop_info->auto_file_next) {
            file_operate_set_file_sel_mode(obj, PLAY_FIRST_FILE);
        } else {
            file_operate_set_file_sel_mode(obj, PLAY_LAST_FILE);
        }
        break;

    case FILE_OP_ERR_PRE_FILE:
        fop_printf("FILE_OP_ERR_PRE_FILE-----\n");
        file_operate_set_dev_sel_mode(obj, DEV_SEL_PREV);
        file_operate_set_file_sel_mode(obj, PLAY_LAST_FILE);
        break;

    case FILE_OP_ERR_NUM:
    case FILE_OP_ERR_OPEN_FILE:
        if (file_operate_err_file_statistics_deal(obj) == false) {
            return false;
        }
        break;

    case FILE_OP_ERR_LGDEV_NO_FIND:
        fop_printf("FILE_OP_ERR_LGDEV_NO_FIND\n");
        if (obj->fop_info->auto_file_next) {
            file_operate_set_file_sel_mode(obj, PLAY_BREAK_POINT);
            file_operate_set_dev_sel_mode(obj, DEV_SEL_NEXT);
        } else {
            file_operate_set_file_sel_mode(obj, PLAY_BREAK_POINT);
            file_operate_set_dev_sel_mode(obj, DEV_SEL_PREV);
        }
        break;

    case FILE_OP_ERR_LGDEV_OFFLINE:
        file_operate_set_file_sel_mode(obj, PLAY_BREAK_POINT);
        file_operate_set_dev_sel_mode(obj, DEV_SEL_NEXT);
        break;

    case FILE_OP_ERR_LGDEV_HDL:
        return false;
    case FILE_OP_ERR_LGDEV_NULL:
        return false;

    default:
        break;
    }
    return true;
}


#endif

u32 file_operate_rec_file_api_creat(FILE_OPERATE *obj)
{
    u32 err;
    if (obj == NULL) {
        return FILE_OP_ERR_NOT_INIT;
    }
    file_api_destroy(obj->fop_file);
    dev_mult_sel_deal((DEV_HANDLE)obj->fop_info->dev);  //设备IO复用时，打开设备前先处理设备开关
    dev_power_on_spec_dev((DEV_HANDLE)obj->fop_info->dev);
    err = file_api_creat(obj->fop_file, (void *)obj->fop_info->dev, obj->fop_info->dev_part_index);
    if (err == false) {
        return FILE_OP_ERR_NO_MEM;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

struct _file_opr_hdl {
    FILE_OPERATE 		opr;
    FILE_OPERATE_INFO 	info;
    _FILE 				fil;
    u8					flag;
};
#define FILE_OPR_HDL_NUM		2
static struct _file_opr_hdl 	g_fhdl[FILE_OPR_HDL_NUM];

FILE_OPERATE *file_opr_creat(void)
{
    FILE_OPERATE *obj = NULL;

    u8 i;
    for (i = 0; i < FILE_OPR_HDL_NUM; i++) {
        if (!g_fhdl[i].flag) {
            memset(&g_fhdl[i], 0, sizeof(struct _file_opr_hdl));
            g_fhdl[i].flag = 1;
            obj = (FILE_OPERATE *)&g_fhdl[i].opr;
            obj->fop_info = (FILE_OPERATE_INFO *)&g_fhdl[i].info;
            obj->fop_file = (_FILE *)&g_fhdl[i].fil;
            printf("creat file hdl : 0x%x \n", obj);
            return obj;
        }
    }
    printf("creat file hdl err \n");
    return obj;
}

void file_opr_destroy(FILE_OPERATE **hdl)
{
    if (hdl == NULL || *hdl == NULL) {
        return ;
    }
    FILE_OPERATE *obj = *hdl;
    *hdl = NULL;

    file_api_destroy(obj->fop_file);

    u8 i;
    for (i = 0; i < FILE_OPR_HDL_NUM; i++) {
        if ((int)&g_fhdl[i].opr == (int)obj) {
            if (g_fhdl[i].flag) {
                g_fhdl[i].flag = 0;
                printf("file hdl:0x%x destroy ok \n", (int)obj);
            } else {
                printf("file hdl:0x%x already destroy !!! \n\n", (int)obj);
            }
            return;
        }
    }
    printf("file hdl:0x%x destroy err !!! \n\n", (int)obj);

}


#if 1

#define TEST_FNAME_MAX		2
static u8 test_fname_cnt = 0;
static const u8 *test_fname[TEST_FNAME_MAX] = {
    (const u8 *)"/123.txt",
    (const u8 *)"/456.txt",
};

void fat_file_test(void)
{
    int err;
    FILE_OPERATE *fopr;

    fopr = file_opr_creat();
    if (!fopr) {
        printf("\n[%s,%d], \n", __func__, __LINE__);
        return ;
    }

    /* file_operate_set_dev_sel_mode(fopr, DEV_SEL_SPEC); */
    /* file_operate_set_dev(fopr, (u32)dev); */
    file_operate_set_dev_sel_mode(fopr, DEV_SEL_LAST);

    err = file_operate_dev_sel(fopr);
    if (err) {
        printf("\n[%s,%d], err:%d,0x%x \n", __func__, __LINE__, err, err);
        goto _end;
    }

    err = file_api_creat(fopr->fop_file, (void *)fopr->fop_info->dev, fopr->fop_info->dev_part_index);
    if (err == false) {
        printf("\n[%s,%d], err:%d,0x%x \n", __func__, __LINE__, err, err);
        goto _end;
    }

    _FIL_HDL *f_p = file_operate_get_file_hdl(fopr);
    _FS_HDL  *fs_p = file_operate_get_fs_hdl(fopr);

    if (++test_fname_cnt >= TEST_FNAME_MAX) {
        test_fname_cnt = 0;
    }
    char *file_name = (char *)test_fname[test_fname_cnt];

    err = fs_open(fs_p, f_p, file_name, 0);
    if (err) {
        printf("\n[%s,%d], err:%d,0x%x \n", __func__, __LINE__, err, err);
        goto _end;
    }

    u8 buf[0x20];
    u32 len, f_size, addr, step;
    fs_io_ctrl(fs_p, f_p, FS_IO_GET_FILE_SIZE, &f_size);
    step = f_size / 10;

    printf("fname:%s, fsize:%d \n", file_name, f_size);
    for (addr = 0; addr < f_size; addr += step) {
        memset(buf, 0, sizeof(buf));
        err = fs_seek(f_p, 0, addr);
        if (err) {
            printf("\n[%s,%d], err:%d,0x%x \n", __func__, __LINE__, err, err);
            goto _end;
        }
        len = fs_read(f_p, buf, sizeof(buf));
        printf("\n read len:%d, addr:%d, tell:%d  \n", len, addr, fs_tell(f_p));
        printf_buf(buf, len);
    }

_end:
    file_opr_destroy(&fopr);
}
#endif


