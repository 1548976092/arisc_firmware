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
#include "mod_timer.h"




int main(void)
{
    // startup settings
    enable_caches();
    clk_set_rate(CPU_FREQ);

    TIMER_START();

    // configure pin PA15 (RED led) as output
    gpio_pin_setup_for_output(PA,15);

    for(;;) // main loop
    {
        // make the 1s delay
        TIMER_CNT_SET(0);
        while ( TIMER_CNT_GET() < (TIMER_FREQUENCY) );

        // PA15 pin toggling
        if ( gpio_pin_get(PA,15) )  gpio_pin_clear(PA,15);
        else                        gpio_pin_set  (PA,15);

        gpio_module_base_thread(); // real update of pin states
    }
}
