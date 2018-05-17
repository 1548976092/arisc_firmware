#include <or1k-support.h>
#include <or1k-sprs.h>
#include <stdint.h>
#include <string.h>

#include "cfg.h"
#include "io.h"
#include "gpio.h"
#include "clk.h"
#include "timer.h"
#include "sys.h"

// modules
#include "gpio_ctrl.h"




int main(void)
{
    // system settings
    enable_caches();
    gpio_init();
    clk_set_rate(CLK_CPUS, CPU_FREQ);
    timer_start();

    // main loop
    for(;;)
    {
        // gpio control module base thread
        gpio_ctrl_base_thread();
    }

    return 0;
}

