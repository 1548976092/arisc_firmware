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
#include "mod_msg.h"

#include "debug.h"
#include "uart.h"
#include <string.h>




int callback_id = 0; // messages callback id
int msg_counter = 0; // messages counter




// callback for the `message received` event
int32_t volatile msg_received(uint8_t type, uint8_t * msg, uint8_t length)
{
    // send mirror message
    msg_send(type, msg, length);
    // increase messages count
    msg_counter++;
    // abort messages receiving after 100 incoming messages
    if ( msg_counter >= 10 ) msg_recv_callback_remove(callback_id);
    // normal exit
    return 0;
}




int main(void)
{
    // startup settings
    enable_caches();
    clk_enable(CLK_R_PIO);
    clk_set_rate(CLK_CPUS, CPU_FREQ);
    uart0_init();

    // module init
    msg_module_init();

    // assign incoming messages callback for the message type 123
    callback_id = msg_recv_callback_add(123, (msg_recv_func_t) &msg_received);

    // main loop
    for(;;)
    {
        // real reading/sending of a messages
        msg_module_base_thread();
    }

    return 0;
}
