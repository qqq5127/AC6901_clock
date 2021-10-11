#ifndef __FILE_OPERATE_H__
#define __FILE_OPERATE_H__
#include "typedef.h"
;
#include "dev_mg_api.h"
#include "fat/tff.h"

#define MUSIC_DEV_TYPE	(DEV_SDCRAD_0 | DEV_SDCRAD_1 | DEV_UDISK_F0)


//------file op cmd------------------
typedef enum {
    FOP_INIT = 0,
    FOP_GET_TOTALFILE_NUM,
    FOP_OPEN_FILE_BYNUM,
    FOP_OPEN_FILE_BYPATH,
    FOP_OPEN_FILE_BYSCLUCT,
    FOP_OPEN_FILE_BYBREAKPOINT,
    FOP_SAVE_LASTOP_DEV,
    FOP_FIND_SPEC_LGDEV,
    FOP_ALLFILE_ERR_LGDEV,
    FOP_DEV_STATUS,
    FOP_CLOSE_LOGDEV,
    FOP_GET_PHYDEV_INFO,
    FOP_GET_FOLDER_INFO,
    FOP_GET_FILE_NAME,
    FOP_GET_FILE_INFO,
    FOP_CREAT_FOLDER,
    FOP_OPEN_FILE,
    FOP_OPEN_FILE_BYNAME,
    FOP_DEL_FILE,
    FOP_SCAN_FILE,
    FOP_MAX,
} ENUM_FILE_CMD;

//------play mode------------------
typedef enum {
    REPEAT_ALL = FOP_MAX + 1,                 ///<全部循环
    REPEAT_ONE_LGDEV,                       ///<单个逻辑设备循环
    REPEAT_ONE,                             ///<单曲循环
    REPEAT_RANDOM,                          ///<单设备随机播放
    REPEAT_FOLDER,                          ///<文件夹循环
    REPEAT_NORMAL,

//    REPEAT_ONE_PHDEV,                       ///<单个物理设备循环
    MAX_PLAY_MODE,
} ENUM_PLAY_MODE;

//-----file sel mode----------------
typedef enum {						//播放文件的方式定义
    PLAY_NEXT_FILE = MAX_PLAY_MODE + 1,     ///<播放下一首
    PLAY_AUTO_NEXT,                         ///<播放下一首，播放结束时，自动下一首控制
    PLAY_FIRST_FILE,  		                ///<从第一个文件播放
    PLAY_BREAK_POINT,                       ///<从记忆文件和位置开始播放
    PLAY_SCLUCT_FILE,                       ///<从记忆文件和位置开始播放
    PLAY_LAST_FILE,				            ///<从最后一个文件播放
    PLAY_PREV_FILE,                         ///<播放上一首
    PLAY_SPEC_FILE,                         ///<输入指定文件序号
    PLAY_FILE_BYPATH,                       ///<输入包含文件名的路径(无文件名时，打开此路径下的第一个文件)
    PLAY_NEXT_FOLDER,                       ///<下一个文件夹
    PLAY_PRE_FOLDER,                        ///<上一个文件夹
    // PLAY_AB_REPEAT,							///AB断点
    MAX_FILE_SEL_MODE,
} ENUM_FILE_SELECT_MODE;

//-----dev sel mode-----------------
typedef enum {
    DEV_SEL_CUR = MAX_FILE_SEL_MODE + 1, ///<当前活动设备
    DEV_SEL_NEXT,                        ///<下一个设备
    DEV_SEL_PREV,                        ///<上一个设备
    DEV_SEL_FIRST,                       ///<第一个设备
    DEV_SEL_LAST,                        ///<最后一个设备
    DEV_SEL_SPEC,                        ///<指定设备
    MAX_DEV_MODE, //0x26
} ENUM_DEV_SEL_MODE;


typedef struct __FOP_BP {
    u32 sclust;
    u32 f_size;
} FOP_BP;


typedef struct __FILE_OPERATE FILE_OPERATE;



FILE_OPERATE *file_operate_creat(void);
void file_operate_destroy(FILE_OPERATE **hdl);
void file_operate_set_dev_sel_mode(FILE_OPERATE *obj, ENUM_DEV_SEL_MODE mode);
void file_operate_set_repeat_mode(FILE_OPERATE *obj, ENUM_PLAY_MODE mode);
void file_operate_set_file_sel_mode(FILE_OPERATE *obj, ENUM_FILE_SELECT_MODE mode);
void file_operate_set_file_number(FILE_OPERATE *obj, u32 index);
void file_operate_set_file_sclust(FILE_OPERATE *obj, u32 sclust);
void file_operate_set_path(FILE_OPERATE *obj, u8 *path, u32 index);
void file_operate_set_dev(FILE_OPERATE *obj, u32 dev);
void file_operate_set_file_ext(FILE_OPERATE *obj, const char *ext);
void file_operate_clean_total_err_file(FILE_OPERATE *obj);
void file_operate_set_auto_next(FILE_OPERATE *obj, u32 auto_auto_flag);
ENUM_DEV_SEL_MODE file_operate_get_dev_sel_mode(FILE_OPERATE *obj);
ENUM_PLAY_MODE file_operate_get_repeat_mode(FILE_OPERATE *obj);
ENUM_FILE_SELECT_MODE file_operate_get_file_sel_mode(FILE_OPERATE *obj);
u32 file_operate_get_file_number(FILE_OPERATE *obj);
void *file_operate_get_dev(FILE_OPERATE *obj);
void *file_operate_get_file_hdl(FILE_OPERATE *obj);
void *file_operate_get_lrc_hdl(FILE_OPERATE *obj);
void *file_operate_get_fs_hdl(FILE_OPERATE *obj);
u32 file_operate_get_file_name(FILE_OPERATE *obj, void *f_path);
u32 file_operate_get_file_total(FILE_OPERATE *obj);
u8 file_operate_get_auto_next_flag(FILE_OPERATE *obj);
u32 file_operate_file_sel(FILE_OPERATE *obj, FOP_BP *bp_info);
u32 file_operate_dev_sel(FILE_OPERATE *obj);
u32 file_operate_dev_bp(FILE_OPERATE *obj, FOP_BP *bp_info);
u32 file_operate_rec_file_api_creat(FILE_OPERATE *obj);
tbool file_operate_err_deal(FILE_OPERATE *obj, u32 err);
u32 file_operate_op(FILE_OPERATE *obj, u32 cmd, void *input, void *output);
u32 file_operate_get_file_info(FILE_OPERATE *obj, void *f_info);
u32 file_operate_open_file_byname(FILE_OPERATE *obj, void *f_name);
#endif// __FILE_OPERATE_H__
