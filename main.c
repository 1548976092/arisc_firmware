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

    // add message handlers for the GPIO module
    msg_recv_callback_add(GPIO_MSG_GET,     (msg_recv_func_t) gpio_msg_recv);
    msg_recv_callback_add(GPIO_MSG_SET,     (msg_recv_func_t) gpio_msg_recv);
    msg_recv_callback_add(GPIO_MSG_SETUP,   (msg_recv_func_t) gpio_msg_recv);

    // add message handlers for the PULSGEN module
    msg_recv_callback_add(PULSGEN_MSG_TASK_SETUP,   (msg_recv_func_t) pulsgen_msg_recv);
    msg_recv_callback_add(PULSGEN_MSG_TASK_ABORT,   (msg_recv_func_t) pulsgen_msg_recv);
    msg_recv_callback_add(PULSGEN_MSG_TASK_TOGGLES, (msg_recv_func_t) pulsgen_msg_recv);
    msg_recv_callback_add(PULSGEN_MSG_TASK_STATE,   (msg_recv_func_t) pulsgen_msg_recv);
    msg_recv_callback_add(PULSGEN_MSG_PIN_SETUP,    (msg_recv_func_t) pulsgen_msg_recv);

    // main loop
    for(;;)
    {
        msg_module_base_thread();
        pulsgen_module_base_thread();
        gpio_module_base_thread();
    }

    return 0;
}
