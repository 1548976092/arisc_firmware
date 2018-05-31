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
#include "mod_pulsgen.h"

#include "debug.h"
#include "uart.h"




int main(void)
{
    // startup settings
    enable_caches();
    clk_enable(CLK_R_PIO);
    clk_set_rate(CLK_CPUS, CPU_FREQ);
    uart0_init();

    // module init
    pulsgen_module_init();

    // use GPIO pin PA15 (ERD led) for the channel 0 output
    pulsgen_pin_setup(0, PA, 15, 1);

    uint8_t type = 0;

    // main loop
    for(;;)
    {
        if ( ! pulsgen_task_state(0) )
        {
            if ( type )
            {
                pulsgen_task_setup(0, 1, 5*2, 10, 0);
                type = 0;
            }
            else
            {
                pulsgen_task_setup(0, 10, 50*2, 10, 0);
                type = 1;
            }
        }

        // real update of channel states
        pulsgen_module_base_thread();
        // real update of pin states
        gpio_module_base_thread();
    }

    return 0;
}
