#include "iic_hw.h"
#include "irq_api.h"
#include "common.h"
#include "string.h"

#include "uart.h"


void iic_hw_sendbyte(u8 byte)
{
    IIC_DATA_OUT();						//DIR_OUT

    JL_IIC->BUF = byte;					//start and send data
    CFG_DONE();							//CFG_DONE
    while (!(JL_IIC->CON0 & BIT(15)));	//8bit pend
    JL_IIC->CON0 |= (BIT(14));
}

u8 iic_hw_revbyte(u8 para)
{
    u8 byte;
    IIC_DATA_IN();						//DIR_IN
    IIC_WR_NACK_ACK(para);

    JL_IIC->BUF = 0xff;					//clear buf
    CFG_DONE();							//CFG_DONE
    while (!(JL_IIC->CON0 & BIT(15)));	//8bit pend
    JL_IIC->CON0 |= (BIT(14));

    byte = JL_IIC->BUF;
    return byte;
}

void iic_hw_read_eerom(u8 iic_addr, u8 *iic_dat, u8 n)
{
    u8  i;
    M_SET_RSTART();

    iic_hw_sendbyte(0xA0);				//send chip_id
    iic_hw_sendbyte(iic_addr);			//send addr

    M_SET_RSTART();						//restart
    iic_hw_sendbyte(0xA1);				//读命令

    for (i = 0; i < n - 1; i++) {
        iic_dat[i] = iic_hw_revbyte(0);
    }
    iic_dat[i] = iic_hw_revbyte(1);

    M_SET_END();						//send end
    CFG_DONE();							//CFG_DONE
    while (!(JL_IIC->CON0 & BIT(13)));	//end pend
    JL_IIC->CON0 |= (BIT(12));
}

void iic_hw_write_eerom(u8 iic_addr, u8 *iic_dat, u8 n)
{
    u8  i;
    M_SET_RSTART();

    iic_hw_sendbyte(0xA0);				//send chip_id
    iic_hw_sendbyte(iic_addr);			//send addr

    for (i = 0; i < n; i++) {
        iic_hw_sendbyte(iic_dat[i]);
    }

    M_SET_END();						//send end
    CFG_DONE();							//CFG_DONE
    while (!(JL_IIC->CON0 & BIT(13)));	//end pend
    JL_IIC->CON0 |= (BIT(12));

    delay_2ms(5);						//wait data save completed
}

void iic_isr(void)
{
    static u8 dat[8];
    static u8 cnt = 0;

    u8 a;
    if (JL_IIC->CON1 & BIT(15)) {			//rev_start pending
        JL_IIC->CON1 |= BIT(14);
        IIC_DATA_IN();
        memset(dat, 0x00, sizeof(dat));
        cnt = 0;
    }

    if (JL_IIC->CON0 & BIT(15)) {			//send or recieve pending
        JL_IIC->CON0 |= BIT(14);

        delay(2);							//延时一下让数据稳定
        dat[cnt] = JL_IIC->BUF;
        cnt++;

        if (cnt >= 6) { 					//wait end signal
            IIC_DATA_IN();
        } else if (cnt >= 2) {
            IIC_DATA_OUT();
            JL_IIC->BUF = cnt;				//send data to master
        }
    }

    if (JL_IIC->CON0 & BIT(13)) {			//rev_end pending
        JL_IIC->CON0 |= BIT(12);
    }

    /* slave_deal_handle(a); */
    //delay(4800000);  //测试长时间进中断是否有问题

    //delay(2);
    CFG_DONE();
}
IRQ_REGISTER(IRQ_IIC_IDX, iic_isr);


void iic_hw_init(void)
{
    u8 i;

    puts("IIC_HW_TEST\n");

    //IOMC_SEL
    IIC_PORT_SEL(IIC_PORTB);
    JL_PORTC->PU |= (BIT(4) | BIT(5));

#if 0	//slave
    IRQ_REQUEST(IRQ_IIC_IDX, iic_isr) ;

    JL_IIC->BAUD = 0xA0;
    JL_IIC->CON1 = BIT(13); //ignore invaild addr

    JL_IIC->CON0 = BIT(0);
    delay(1);					//fix by YY
    IIC_DATA_IN();
    JL_IIC->CON0 |= (BIT(14) | BIT(12) | BIT(10) | BIT(9) | BIT(8) | BIT(1));

    while (1) {
        u8 dat[8];
        delay(0xffff);					//fix by YY
        extern void iic_readn(u8 chip_id, u8 iic_addr, u8 * iic_dat, u8 n);
        iic_readn(0xA1, 0x55, dat, 4);

        puts("iic master rev dat \n");
        put_buf(dat, 4);

        delay_2ms(500);
    }
#endif

#if 1 //master
    JL_IIC->BAUD = 10000;
    JL_IIC->CON0 |= BIT(0);
    delay(1);					//fix by YY
    JL_IIC->CON0 |= (BIT(14) | BIT(12) | BIT(9));

    u8 dat[8];
    memset(dat, 0xA3, 4);

    iic_hw_write_eerom(0, dat, 4);
    iic_hw_read_eerom(0, dat, 4);

    puts("iic master rev dat \n");
    put_buf(dat, 4);
    while (1);
#endif

}

