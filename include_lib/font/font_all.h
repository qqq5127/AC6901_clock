#ifndef __FONT_ALL_H__
#define __FONT_ALL_H__

#include "font_GBK.h"
#include "font_BIG5.h"
#include "font_SJIS.h"
#include "font_KSC.h"
#include "font_OtherLanguage.h"
#include "font_utf16toansi.h"
//#include "font_out.h"

#define ntohl(x)	(u32)((x>>24)|((x>>8)&0xff00)|(x<<24)|((x&0xff00)<<8))
#define ntoh(x)		(u16)((x>>8&0x00ff)|x<<8&0xff00)

#define  Chinese_Simplified       1   //简体中文
#define  Chinese_Traditional      2   //繁体中文
#define  Japanese                 3   //日语
#define  Korean                   4   //韩语
#define  English                  5   //英语
#define  French                   6   //法语
#define  German                   7   //德语
#define  Italian                  8   //意大利语
#define  Dutch                    9   //荷兰语
#define  Portuguese               10  //葡萄牙语
#define  Spanish                  11  //西班牙语
#define  Swedish                  12  //瑞典语
#define  Czech                    13  //捷克语
#define  Danish                   14  //丹麦语
#define  Polish                   15  //波兰语
#define  Russian                  16  //俄语
#define  Turkey                   17  //土耳其语
#define  Hebrew                   18  //希伯来语
#define  Thai                     19  //泰语
#define  Hungarian                20  //匈牙利语
#define  Romanian                 21  //罗马尼亚语
#define  Arabic                   22  //阿拉伯语

#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif
typedef struct {
    u32 addr;
    u8 flag;
    u8 font_size;
    u8 font_nbytes;
} FONT_MATRIX;
#ifdef WIN32
#pragma pack(pop)
#endif

extern FONT_MATRIX fontinfo[3];
extern FONT_MATRIX ascfontinfo[3];
extern FONT_MATRIX *font;
extern FONT_MATRIX *ascfont;
extern u8 g_amplify;
extern u32 g_ConvertTableFileAddr;
extern bool IsGB2312;
extern u8  pixBuf[100];

//extern u32 g_AsciiPixDataFileAddr;
//extern u32 g_PixDataFileAddr;


#endif
