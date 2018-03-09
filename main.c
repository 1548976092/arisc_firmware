#include <or1k-support.h>
#include <or1k-sprs.h>
#include <stdint.h>
#include <string.h>

#include "cfg.h"
#include "io.h"
#include "gpio.h"
#include "debug.h"
#include "clk.h"
#include "timer.h"
#include "sys.h"
#include "shmem.h"
#include "gpio_ctrl.h"
// TODO - test these modules
#include "stepgen.h"




int main(void)
{
    // system settings
    enable_caches();
    gpio_init();
    uart0_init();
    clk_set_rate(CLK_CPUS, CPU_FREQ);
    timer_start();

    // init and clean up shared memory
    shmem_clear_all();

    // main loop
    for(;;)
    {
        // ARISC core is alive
        if ( !shm(arisc_alive) ) shm(arisc_alive) = 1ul;

        // process stepgen thread
//        stepgen_base_thread();

        // process gpio control thread
        gpio_ctrl_base_thread();
    }


    return 0;
}

