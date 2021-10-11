#include "flash_api.h"
#include "string.h"
#include "uart.h"
#define MAX_VM_CACHE 3
_VM_CACHE vm_cache[MAX_VM_CACHE]; //sizeof(_VM_CACHE)*MAX_VM_CACHE

/*----------------------------------------------------------------------------*/
/** @brief: 初始化VM，并打开所有存储单元
    @param: void
    @return:void
    @author:lj
    @note:  void vm_open_all(void
*/
/*----------------------------------------------------------------------------*/
static void vm_open_all(void)
{
    u32 i;
    s32 err;
    vm_cache[0].cnt = 0;
    vm_cache[0].index = VM_SYS_VOL;
    vm_cache[0].dat_len = VM_SYS_VOL_LEN;

    vm_cache[1].cnt = 0;
    vm_cache[1].index = VM_PC_VOL;
    vm_cache[1].dat_len = VM_PC_VOL_LEN;

    vm_cache[2].cnt = 0;
    vm_cache[2].index = VM_SYS_EQ;
    vm_cache[2].dat_len = VM_SYS_EQ_LEN;

    for (i = 0; i < VM_MAX_SIZE; i++) {
        err = vm_open(i + VM_START_INDEX);
        if (err < 0) {
            printf("warning : index : %x  open err : 0x%x\n", i, err);
        }
    }
}
/*----------------------------------------------------------------------------*/
/** @brief: 按照索引号缓存存储信息，但是不直接写入VM. only support 1 or 2 bytes
    @param: index：存储单元索引
    @param: data_buf：需要读取的数据的指针
    @param: cnt ：在cnt个节拍之后，将数据存入VM
    @return:vm_err：操作结果
    @author:lj
    @note:  vm_err vm_cache_write(u8 index ,const void *data_buf,s16 cnt)
*/
/*----------------------------------------------------------------------------*/
int vm_cache_write(u8 index, const void *data_buf, u8 cnt)
{
    u32 i;
    for (i = 0; i < MAX_VM_CACHE; i++) {
        if (vm_cache[i].index == index) {		//this index has cache
            memcpy(&vm_cache[i].data, data_buf, vm_cache[i].dat_len);
            vm_cache[i].cnt = cnt;
            return FLASH_ERR_NONE;
        }
    }
    return FLASH_NOT_SUPPORT_CACHE;
}

/*----------------------------------------------------------------------------*/
/** @brief: 扫描缓存，提交需要写入VM的数据
    @param: void
    @return:vm_err：操作结果
    @author:lj
    @note:  vm_err vm_cache_submit(void)
*/
/*----------------------------------------------------------------------------*/
int vm_cache_submit(void)
{
    u32 i;

    for (i = 0; i < MAX_VM_CACHE; i++) {
        if (vm_cache[i].cnt > 0) {
            vm_cache[i].cnt--;
            if (0 == vm_cache[i].cnt) {
                if (vm_write(vm_cache[i].index, &vm_cache[i].data, vm_cache[i].dat_len) == vm_cache[i].dat_len) {
                    vm_cache[i].cnt = 0;
                    puts("submit vm succ\n");
                } else {
                    puts("submit vm err and retry\n");
                    vm_cache[i].cnt = 4;
                }
            }
        }
    }
    return FLASH_ERR_NONE;
}

/*----------------------------------------------------------------------------*/
/** @brief: This function called to check vm space if it's necessary to tidy the
  			vm area to avoid vm full.
    @param: void
    @return:void
    @author:
    @note:  vm will no do check operates when the code can't been load to cache
			at a time.
*/
/*----------------------------------------------------------------------------*/
/*vm check api depends functions*/
#include "audio/tone.h"
#include "aec_main.h"
void vm_check_api(u8 level)
{
    /** Follow situations unapply:
      * 1.sin_tone busy
      *
      */
#if TONE_EN
    if (is_sin_tone_busy()) {
        puts("sin_tone busy!\n");
        return;
    }
#endif

    vm_check_all(0);
}

void flash_storage_init(u32 addr)
{
    spi_port_hd(0);/*range:0~3*/
    set_g_sys_cfg(addr);

    //0：vm写、整理等特殊操作期间，不允许DAC工作
    //1：vm写、整理等特殊操作期间，允许DAC工作
    vm_init_api(1);

    vm_open_all();

}

/*----------------------------------------------------------------------------*/
/** @brief: This function called to set spi driver of ports
    @param: level:0(weak)~3(strong)
    @return:
    @author:
    @note:	CS		CLK		D0		D1		D2		D3
	PORT A: PD3		PD0		PD1		PD2		PB4		PB3
	PORT B: PD3		PB2  	PD2		PB6		PB4		PB3
*/
/*----------------------------------------------------------------------------*/
void spi_port_hd(u8 level)
{
    switch (level) {
    case 0:
        JL_PORTB->HD  &= ~(BIT(2) | BIT(3) | BIT(4) | BIT(6));
        JL_PORTB->HD1 &= ~(BIT(2) | BIT(3) | BIT(4) | BIT(6));

        JL_PORTD->HD  &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(3));
        JL_PORTD->HD1 &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(3));
        break;
    case 1:
        JL_PORTB->HD  |= (BIT(2) | BIT(3) | BIT(4) | BIT(6));
        JL_PORTB->HD1 &= ~(BIT(2) | BIT(3) | BIT(4) | BIT(6));

        JL_PORTD->HD  |= (BIT(0) | BIT(1) | BIT(2) | BIT(3));
        JL_PORTD->HD1 &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(3));
        break;
    case 2:
        JL_PORTB->HD  &= ~(BIT(2) | BIT(3) | BIT(4) | BIT(6));
        JL_PORTB->HD1 |= (BIT(2) | BIT(3) | BIT(4) | BIT(6));

        JL_PORTD->HD  &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(3));
        JL_PORTD->HD1 |= (BIT(0) | BIT(1) | BIT(2) | BIT(3));
        break;
    case 3:
        JL_PORTB->HD  |= (BIT(2) | BIT(3) | BIT(4) | BIT(6));
        JL_PORTB->HD1 |= (BIT(2) | BIT(3) | BIT(4) | BIT(6));

        JL_PORTD->HD  |= (BIT(0) | BIT(1) | BIT(2) | BIT(3));
        JL_PORTD->HD1 |= (BIT(0) | BIT(1) | BIT(2) | BIT(3));
        break;
    }
}

