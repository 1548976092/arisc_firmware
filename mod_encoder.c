/**
 * @file    mod_encoder.c
 *
 * @brief   quadrature encoder counter module
 *
 * This module implements an API
 * to make real-time counting of quadrature encoder pulses
 */

#include "mod_timer.h"
#include "mod_gpio.h"
#include "mod_encoder.h"




// private vars

static struct encoder_ch_t enc[ENCODER_CH_CNT] = {0}; // array of channels data
static uint8_t msg_buf[ENCODER_MSG_BUF_LEN] = {0};




// public methods

/**
 * @brief   module base thread
 * @note    call this function anywhere in the main loop
 * @retval  none
 */
void encoder_module_base_thread()
{
    static uint8_t c, phase_A_pin_state, phase_B_pin_state, phase_Z_pin_state;
    static uint8_t phase_A_changed, phase_B_changed;

    // check all channels
    for ( c = ENCODER_CH_CNT; c--; )
    {
        // if channel disabled, goto next channel
        if ( !enc[c].enabled ) continue;

        // if we are using ABZ encoder
        if ( enc[c].using_Z )
        {
            phase_Z_pin_state = (uint8_t) gpio_pin_get(enc[c].phase_Z_port, enc[c].phase_Z_pin);

            // on phase Z state change
            if ( enc[c].phase_Z_pin_state != phase_Z_pin_state )
            {
                // reset counter if state of phase Z is HIGH
                if ( (!enc[c].edge && phase_Z_pin_state) || (enc[c].edge && !phase_Z_pin_state) )
                {
                    enc[c].counts = 0;
                }

                // save phase state for future checks
                enc[c].phase_Z_pin_state = phase_Z_pin_state;
            }
        }

        // get phase A state
        phase_A_pin_state = (uint8_t) gpio_pin_get(enc[c].phase_A_port, enc[c].phase_A_pin);

        // on phase A state change
        if ( enc[c].phase_A_pin_state != phase_A_pin_state )
        {
            // if we are using AB encoder
            if ( enc[c].using_B )
            {
                // get phase B state
                phase_B_pin_state = (uint8_t) gpio_pin_get(enc[c].phase_B_port, enc[c].phase_B_pin);

                // on phase A rising edge
                if ( (!enc[c].edge && phase_A_pin_state) || (enc[c].edge && !phase_A_pin_state) )
                {
                    // if state of phase B is LOW, direction = CW
                    if ( (!enc[c].edge && !phase_B_pin_state) || (enc[c].edge && phase_B_pin_state) )
                    {
                        enc[c].counts += enc[c].inverted ? -1 : 1;
                    }
                    // if state of phase B is HIGH, direction = CCW
                    else
                    {
                        enc[c].counts += enc[c].inverted ? 1 : -1;
                    }
                }
            }
            // if we are using A encoder and phase A is HIGH
            else if ( (!enc[c].edge && phase_A_pin_state) || (enc[c].edge && !phase_A_pin_state) )
            {
                enc[c].counts += enc[c].inverted ? -1 : 1;
            }

            // save phase state for future checks
            enc[c].phase_A_pin_state = phase_A_pin_state;
        }
    }
}




/**
 * @brief   setup encoder pin for the selected channel and phase
 *
 * @param   c           channel id
 * @param   phase       PHASE_A..PHASE_Z
 * @param   port        GPIO port number
 * @param   pin         GPIO pin number
 *
 * @retval  none
 */
void encoder_pin_setup(uint8_t c, uint8_t phase, uint8_t port, uint8_t pin)
{
    gpio_pin_setup_for_input(port, pin);

    switch (phase)
    {
        case PHASE_A:
        {
            enc[c].phase_A_port = port;
            enc[c].phase_A_pin = pin;
            enc[c].phase_A_pin_state = 0;
            break;
        }
        case PHASE_B:
        {
            enc[c].phase_B_port = port;
            enc[c].phase_B_pin = pin;
            enc[c].phase_B_pin_state = 0;
            break;
        }
        case PHASE_Z:
        {
            enc[c].phase_Z_port = port;
            enc[c].phase_Z_pin = pin;
            enc[c].phase_Z_pin_state = 0;
            break;
        }
    }
}




/**
 * @brief   setup selected channel of encoder counter
 *
 * @param   c           channel id
 * @param   inverted    invert counter direction?
 * @param   using_B     use phase B input?
 * @param   using_Z     use phase Z index input?
 * @param   edge        pulse detection type, 0 = rising edge, non 0 = falling edge
 *
 * @retval  none
 */
void encoder_setup(uint8_t c, uint8_t inverted, uint8_t using_B, uint8_t using_Z, uint8_t edge)
{
    enc[c].inverted = inverted;
    enc[c].using_B  = using_B;
    enc[c].using_Z  = using_Z;
    enc[c].edge     = edge;
}

/**
 * @brief   enable/disable selected channel of encoder counter
 * @param   c       channel id
 * @retval  none
 */
void encoder_enable(uint8_t c, uint8_t state)
{
    enc[c].enabled = state ? 1 : 0;
}

/**
 * @brief   reset number of counts for the selected channel
 * @param   c       channel id
 * @retval  none
 */
void encoder_reset(uint8_t c)
{
    enc[c].counts = 0;
}




/**
 * @brief   get state for the selected channel
 *
 * @param   c   channel id
 *
 * @retval  0   (channel is disabled)
 * @retval  1   (channel is enabled)
 */
uint8_t encoder_state(uint8_t c)
{
    return enc[c].enabled;
}

/**
 * @brief   get current counts for the selected channel
 * @param   c   channel id
 * @retval  signed 32-bit number
 */
int32_t encoder_counts(uint8_t c)
{
    return enc[c].counts;
}




/**
 * @brief   "message received" callback
 *
 * @note    this function will be called automatically
 *          when a new message will arrive for this module.
 *
 * @param   type    user defined message type (0..0xFF)
 * @param   msg     pointer to the message buffer
 * @param   length  the length of a message (0 .. MSG_LEN)
 *
 * @retval   0 (message read)
 * @retval  -1 (message not read)
 */
int8_t volatile encoder_msg_recv(uint8_t type, uint8_t * msg, uint8_t length)
{
    static uint8_t i = 0;

    switch (type)
    {
        case ENCODER_MSG_PINS_SETUP:
        {
            // setup channel pins
            for ( i = ENCODER_CH_CNT; i--; )
            {
                // TODO
            }

            // TODO

            break;
        }

        default: return -1;
    }

    return 0;
}




/**
    @example mod_encoder.c

    <b>Usage example 1</b>: UJHHJBHBKJN

    @code
        #include <stdint.h>
        #include "mod_encoder.h"

        int main(void)
        {


            // main loop
            for(;;)
            {
                // real update of channel states
                encoder_module_base_thread();
            }

            return 0;
        }
    @endcode
*/
