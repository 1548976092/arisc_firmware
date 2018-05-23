/**
 * @file        main.c
 *
 * @mainpage    Firmware for the Allwinner H3 ARISC co-processor
 *
 * This firmware for the @b OrangePi boards.
 *
 * The firmware uses to help the
 * <a href="http://linuxcnc.org">LinuxCNC</a> and
 * <a href="http://machinekit.io">Machinekit</a>
 * makes a real-time @b GPIO manipulations.
 *
 * http://github.com/orangecnc/h3_arisc_firmware
 *
 */

#include <or1k-support.h>
#include <or1k-sprs.h>
#include <stdint.h>
#include <string.h>

#include "cfg.h"
#include "clk.h"
#include "timer.h"
#include "sys.h"

// modules
#include "gpio_ctrl.h"
#include "msg_ctrl.h"




int main(void)
{
    // startup settings
    enable_caches();
    clk_set_rate(CLK_CPUS, CPU_FREQ);
    timer_start();

    // modules init
    gpio_ctrl_init();
    msg_ctrl_init();

    // main loop
    for(;;)
    {
        msg_ctrl_base_thread();
        gpio_ctrl_base_thread();
    }

    return 0;
}

