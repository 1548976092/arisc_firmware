/**
 * @file        main.c
 *
 * @mainpage    CNC firmware for the Allwinner H3 ARISC co-processor
 *
 * http://orange-cnc.ru/
 *
 * The firmware uses by Orange Pi boards to help the
 *      <a href="http://linuxcnc.org">LinuxCNC</a>
 * and
 *      <a href="http://machinekit.io">Machinekit</a>
 * make a real-time @b GPIO manipulations.
 *
 * http://github.com/orange-cnc/h3_arisc_firmware
 *
 */

#include "sys.h"
#include "mod_gpio.h"
#include "mod_msg.h"
#include "mod_stepgen.h"
#include "mod_encoder.h"




int main(void)
{
    // startup settings
    enable_caches();
    clk_set_rate(CPU_FREQ);

    // modules init
    msg_module_init();
    gpio_module_init();
    stepgen_module_init();
    encoder_module_init();

    // main loop
    for(;;)
    {
        msg_module_base_thread();
        encoder_module_base_thread();
        stepgen_module_base_thread();
    }

    return 0;
}
