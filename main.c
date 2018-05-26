/**
 * @file        main.c
 *
 * @mainpage    Firmware for the Allwinner H3 ARISC co-processor
 *
 * The firmware uses by Orange Pi boards to help the
 *      <a href="http://linuxcnc.org">LinuxCNC</a>
 * and
 *      <a href="http://machinekit.io">Machinekit</a>
 * make a real-time @b GPIO manipulations.
 *
 * http://github.com/orangecnc/h3_arisc_firmware
 *
 */

#include "sys.h"
#include "mod_gpio.h"
#include "mod_msg.h"
#include "mod_pulsgen.h"




int main(void)
{
    // startup settings
    enable_caches();
    clk_set_rate(CPU_FREQ);

    // modules init
    msg_module_init();
    pulsgen_module_init();

    // main loop
    for(;;)
    {
        msg_module_base_thread();
        pulsgen_module_base_thread();
        gpio_module_base_thread();
    }

    return 0;
}
