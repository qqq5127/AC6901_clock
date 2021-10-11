#include "clock.h"
#include "clock_api.h"
#include "common.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(	".clk_app_bss")
#pragma data_seg(	".clk_app_data")
#pragma const_seg(	".clk_app_const")
#pragma code_seg(	".clk_app_code")
#endif

void clock_init_app(SYS_CLOCK_INPUT sys_in, u32 input_freq, u32 out_freq)
{
    clock_init(sys_in, input_freq, out_freq);

    init_t *init_func;

    list_for_each_clock_switcher(init_func) {
        (*init_func)();
    }
}

//AT(.common)
void set_sys_freq(u32 out_freq)
{
    static u32 last_clk = SYS_Hz;
    if (out_freq == last_clk) {
        return;
    }
    log_printf("[SYS_CLOCK]:%d -> %d\n", last_clk, out_freq);
    last_clk = out_freq;

    system_enter_critical();

    clock_switching(out_freq);

    system_exit_critical();
}

AT(.common)
void set_apc_clk(u32 freq)
{
    /*pll_64m(0)/pll_apc_clk(1)/lsb_clk(2)/disable(3)*/
    clock_set_apc_freq(freq, 0);
}


void clock_debug(void)
{
    puts("---Source BTOSC 24M\n");
    clock_init(SYS_CLOCK_INPUT_BT_OSC, 24000000L, 24000000L);
    clock_dump();

    clock_init(SYS_CLOCK_IN, OSC_Hz, SYS_Hz);

    puts("---Switching 48M\n");
    clock_switching(48000000L);
    clock_dump();
    puts("---Switching 80M\n");
    clock_switching(80000000L);
    clock_dump();

    puts("---480M/3\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 3);
    clock_dump();
    puts("---480M/5\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 5);
    clock_dump();
    puts("---480M/7\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 7);
    clock_dump();
    puts("---480M/2/3\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 2 / 3);
    clock_dump();
    puts("---480M/2/5\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 2 / 5);
    clock_dump();
    puts("---480M/2/7\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 2 / 7);
    clock_dump();
    puts("---480M/4/1\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 4 / 1);
    clock_dump();
    puts("---480M/4/3\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 4 / 3);
    clock_dump();
    puts("---480M/4/5\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 4 / 5);
    clock_dump();
    puts("---480M/4/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L/4/7); */
    /* clock_dump(); */
    puts("---480M/8/1\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L / 8 / 1);
    clock_dump();
    puts("---480M/8/3\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L/8/3); */
    /* clock_dump(); */
    puts("---480M/8/5\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L/8/5); */
    /* clock_dump(); */
    puts("---480M/8/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 480000000L/8/7); */
    /* clock_dump(); */

    puts("---192M/3\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L / 3);
    clock_dump();
    puts("---192M/5\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L / 5);
    clock_dump();
    puts("---192M/7\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L / 7);
    clock_dump();
    puts("---192M/2/1\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L / 2 / 1);
    clock_dump();
    puts("---192M/2/3\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L / 2 / 3);
    clock_dump();
    puts("---192M/2/5\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L/2/5); */
    /* clock_dump(); */
    puts("---192M/2/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L/2/7); */
    /* clock_dump(); */
    puts("---192M/4/1\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L / 4 / 1);
    clock_dump();
    puts("---192M/4/3\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L/4/3); */
    /* clock_dump(); */
    puts("---192M/4/5\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L/4/5); */
    /* clock_dump(); */
    puts("---192M/4/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L/4/7); */
    /* clock_dump(); */
    puts("---192M/8/1\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L / 8 / 1);
    clock_dump();
    puts("---192M/8/3\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L/8/3); */
    /* clock_dump(); */
    puts("---192M/8/5\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L/8/5); */
    /* clock_dump(); */
    puts("---192M/8/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 192000000L/8/7); */
    /* clock_dump(); */

    puts("---137M/3\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L / 3);
    clock_dump();
    puts("---137M/5\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L / 5);
    clock_dump();
    puts("---137M/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/7); */
    /* clock_dump(); */
    puts("---137M/2/1\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L / 2 / 1);
    clock_dump();
    puts("---137M/2/3\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/2/3); */
    /* clock_dump(); */
    puts("---137M/2/5\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/2/5); */
    /* clock_dump(); */
    puts("---137M/2/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/2/7); */
    /* clock_dump(); */
    puts("---137M/4/1\n");
    clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L / 4 / 1);
    clock_dump();
    puts("---137M/4/3\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/4/3); */
    /* clock_dump(); */
    puts("---137M/4/5\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/4/5); */
    /* clock_dump(); */
    puts("---137M/4/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/4/7); */
    /* clock_dump(); */
    puts("---137M/8/1\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/8/1); */
    /* clock_dump(); */
    puts("---137M/8/3\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/8/3); */
    /* clock_dump(); */
    puts("---137M/8/5\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/8/5); */
    /* clock_dump(); */
    puts("---137M/8/7\n");
    /* clock_init(SYS_CLOCK_IN, OSC_Hz, 137000000L/8/7); */
    /* clock_dump(); */
}
