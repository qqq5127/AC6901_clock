#ifndef _FLASH_API_H_
#define _FLASH_API_H_

#include "typedef.h"
enum {
    FLASH_ERR_NONE  		= 0,
    FLASH_INDEX_ERR 		= -0x100,
    FLASH_DATA_LEN_ERR		= -0x101,
    FLASH_READ_DATA_ERR		= -0x102,
    FLASH_WRITE_ERR  		= -0x103,
    FLASH_NOT_SUPPORT_CACHE = -0x104,
};
enum {
    // SYSCFG_BTSTACK = 0 ,
    // SYSCFG_FILE_BREAKPOINT,
    VM_REMOTE_DB = 1,
    VM_REMOTE_DB_END = (VM_REMOTE_DB + 20),

    //vm_start index
    VM_SYS_VOL,
    VM_SYS_EQ,
    VM_DEV0_BREAKPOINT,
    VM_DEV1_BREAKPOINT,
    VM_DEV2_BREAKPOINT,
    VM_DEV3_BREAKPOINT,
    VM_MUSIC_DEVICE,
    VM_PC_VOL,
    VM_FM_INFO,
    VM_PHONE_VOL,
    VM_TWS_INFO,
    VM_BT_OSC_INT_INFO,
    VM_FM_TRANSMIT_FREQ,
    //add ble new
    VM_BLE_ADDR_INFO,
    VM_XM_BLE_MAC_INDEX,
    VM_XM_BLE_MAC_ADDR0,
    VM_XM_BLE_MAC_ADDR1,
    VM_XM_BLE_MAC_ADDR2,
    VM_XM_BLE_MAC_ADDR3,
    VM_XM_BLE_MAC_ADDR4,
    VM_XM_BLE_MAC_ADDR5,
    VM_XM_BLE_MAC_ADDR6,
    VM_XM_BLE_MAC_ADDR7,
    VM_XM_BLE_MAC_ADDR8,
    VM_XM_BLE_MAC_ADDR9,
    VM_XM_BLE_BOND_ID_ADDR0,
    VM_XM_BLE_DEBUG_INFO,
    VM_XM_DEBUG_INFO,
//------请在次分界线下添加新的VM项-----//
//双闹钟相关
#if (USE_FM_SAVE_FRE_POINT == 0)
    VM_FM_FRE_CUR,
	VM_FM_FRE_ALL,
    VM_FM_STATION_CUR,
	VM_FM_STATION_ALL,
#endif
	VM_TIME_FORMAT,
	VM_ALARM_TIME,
	VM_ALARM2_TIME,
	VM_ALARM_SWITCH,	//闹钟1是否开启
	VM_ALARM2_SWITCH,	//闹钟2是否开启
	VM_ALARM_MODE,		//闹钟1模式
	VM_ALARM2_MODE,		//闹钟2模式
	VM_ALARM_ALARM2_FLAG,	//当前是哪个闹钟
	VM_ALARM_VOLUME,		//闹钟1音量
	VM_ALARM2_VOLUME,		//闹钟2音量
	VM_ALARM_RING,		//闹钟1铃声模式
	VM_ALARM2_RING,		//闹钟2铃声模式
	VM_ALARM_RING_FM_STATION,
	VM_ALARM2_RING_FM_STATION,

    VM_MAX_INDEX,
};

#define VM_START_INDEX      VM_SYS_VOL
#define VM_MAX_SIZE			(VM_MAX_INDEX - VM_SYS_VOL)

#define VM_SYS_VOL_LEN          1
#define VM_SYS_EQ_LEN           1

#define VM_DEV0_BK_LEN          VM_FILE_BK_LEN///(VM_FILE_BK_LEN+8)
#define VM_DEV1_BK_LEN          VM_FILE_BK_LEN///(VM_FILE_BK_LEN+8)
#define VM_DEV2_BK_LEN          VM_FILE_BK_LEN///(VM_FILE_BK_LEN+8)
#define VM_DEV3_BK_LEN          VM_FILE_BK_LEN///(VM_FILE_BK_LEN+8)

// #define VM_DEV0_FLACBK_LEN          VM_FILE_FLACBK_LEN///(VM_FILE_BK_LEN+8)
// #define VM_DEV1_FLACBK_LEN          VM_FILE_FLACBK_LEN///(VM_FILE_BK_LEN+8)
// #define VM_DEV2_FLACBK_LEN          VM_FILE_FLACBK_LEN///(VM_FILE_BK_LEN+8)
// #define VM_DEV3_FLACBK_LEN          VM_FILE_FLACBK_LEN///(VM_FILE_BK_LEN+8)

#define VM_MUSIC_DEVICE_LEN		1

#define VM_PC_VOL_LEN			1
#define VM_FM_INFO_LEN			32
#define VM_PHONE_VOL_LEN		1
#define VM_TWS_INFO_LEN			1
#define VM_OSC_INT_INFO_LEN		4
#define VM_FM_TRANSMIT_FREQ_LEN 2
#if (USE_FM_SAVE_FRE_POINT == 0)
#define VM_FM_FRE_CUR_LEN		1
#define VM_FM_FRE_ALL_LEN		MAX_STATION
#define VM_FM_STATION_CUR_LEN	1
#define VM_FM_STATION_ALL_LEN	1
#endif
//双闹钟相关
#define VM_TIME_FORMAT_LEN		1
#define VM_ALARM_TIME_LEN		8
#define VM_ALARM2_TIME_LEN		8
#define VM_ALARM_SWITCH_LEN		1
#define VM_ALARM2_SWITCH_LEN	1
#define VM_ALARM_MODE_LEN		1
#define VM_ALARM2_MODE_LEN		1
#define VM_ALARM_ALARM2_FLAG_LEN	1
#define VM_ALARM_VOLUME_LEN	    1
#define VM_ALARM2_VOLUME_LEN	1
#define VM_ALARM_RING_LEN		1
#define VM_ALARM2_RING_LEN		1
#define VM_ALARM_RING_FM_STATION_LEN		1
#define VM_ALARM2_RING_FM_STATION_LEN		1

#define VM_BLE_ADDR_INFO_LEN	6
#define XM_PHONE_BLE_LIST_MAX       (10)

typedef struct __VM_CACHE {
    u8 data;
    u8 index;
    u8 dat_len;
    u8 cnt;
} _VM_CACHE;

typedef struct _APP_USE_FLASH_SYS_CFG {
    UINT cfg_zone_addr;
    UINT flash_size;
    UINT cfg_zone_size;
    UINT btif_addr;
    UINT btif_len;
    UINT vmif_addr;
    UINT vmif_len;
} APP_USE_FLASH_SYS_CFG;
extern APP_USE_FLASH_SYS_CFG app_use_flash_cfg;

extern void flash_storage_init(u32 addr);
extern void set_g_sys_cfg(u32 cfg_addr);
extern void vm_check_api(u8 level);

extern u8 get_vm_statu(void);

/*----------------------------------------------------------------------------*/
/**@brief  VM初始化函数
   @param  vm_mode：
   			0：vm写、整理等特殊操作期间，不允许DAC工作
			1：vm写、整理等特殊操作期间，允许DAC工作
   @return NON
   @note   外部必须定时通过此接口查询VM是否需要整理。
*/
/*----------------------------------------------------------------------------*/
extern int vm_init_api(u8 vm_mode);

/*----------------------------------------------------------------------------*/
/**@brief  VM碎片检测函数
   @param  level：0：以最快速度整理VM区域，但不一定全区域整理，1：整理全VM区域
   @return NON
   @note   外部必须定时通过此接口查询VM是否需要整理。
*/
/*----------------------------------------------------------------------------*/
void vm_check_all(u8 level);

s32 vm_open(u8 index);
s32 vm_write(u8 hdl, const void *data_buf, u16 len);
s32 vm_read(u8 hdl, void *data_buf, u16 len);
int vm_cache_write(u8 index, const void *data_buf, u8 cnt);
int vm_cache_submit(void);
u32 vm_write_by_addr(const u8 *buf, u32 addr, u32 len);
void vm_read_by_addr(u8 *buf, u32 addr, u32 len);
extern void *bt_vm_interface();
void spi_port_hd(u8 level);

#endif
