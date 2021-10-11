#include "key_drv_uart.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".key_drv_io_bss")
#pragma data_seg(	".key_drv_io_data")
#pragma const_seg(	".key_drv_io_const")
#pragma code_seg(	".key_drv_io_code")
#endif


#if KEY_UART_EN

/*----------------------------------------------------------------------------*/
/**@brief   uart按键初始化
   @param   void
   @param   void
   @return  void
   @note    void uart_key_init(void)
*/
/*----------------------------------------------------------------------------*/
void uart_key_init(void)
{
}

/*----------------------------------------------------------------------------*/
/**@brief   uart hex接受，高位是按键类型
   @param   void
   @param   void
   @note    u8 key_type2cnt(u8 type)
*/
/*----------------------------------------------------------------------------*/
u8 key_type2cnt(u8 type)
{
    if (type) {
        return  KEY_LONG_CNT + (type - 1) * KEY_HOLD_CNT;
    } else {
        return KEY_BASE_CNT;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   获取uart按键电平值
   @param   void
   @param   void
   @return  key_num:uart按键号
   @note    u8 get_uartkey_value(void)
*/
/*----------------------------------------------------------------------------*/
extern void reset_debug(u8 aa);
u8 get_uartkey_value(void)
{
    //key_puts("get_uartkey_value\n");
    u8 u_key_dat;
    u8 key_num = NO_KEY;
    static u8 key_cnt = 0;
    static u8 key_last = NO_KEY;

    if (key_cnt) {
        key_cnt--;
        key_num = key_last;
    } else {
        u_key_dat = getbyte();
        if (u_key_dat) {
            key_last = u_key_dat & 0x0f;
            key_cnt = key_type2cnt(u_key_dat >> 4);
            key_num = key_last;
            reset_debug(u_key_dat);
            /* log_printf("u_key_dat = 0x%02x\n", u_key_dat); */
            /* log_printf("key_last = 0x%x, key_cnt = 0x%x\n", key_last, key_cnt); */
        }
    }

    return key_num;
}

const key_interface_t key_uart_info = {
    .key_type = KEY_TYPE_UART,
    .key_init = uart_key_init,
    .key_get_value = get_uartkey_value,
};

#endif/*KEY_UART_EN*/
