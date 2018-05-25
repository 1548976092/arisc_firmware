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
#include "sys.h"

// modules
#include "mod_gpio.h"
#include "mod_msg.h"




int main(void)
{
    // startup settings
    enable_caches();
    clk_set_rate(CLK_CPUS, CPU_FREQ);

    // modules init
    gpio_module_init();
    msg_module_init();

    // main loop
    for(;;)
    {
        msg_module_base_thread();
        gpio_module_base_thread();
    }

    return 0;
}

