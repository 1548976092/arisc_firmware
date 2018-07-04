/**
 * @file    mod_encoder.c
 *
 * @brief   quadrature encoder counter module
 *
 * This module implements an API
 * to make real-time counting of quadrature encoder pulses
 */

#include "mod_gpio.h"
#include "mod_encoder.h"




// private vars

static struct encoder_ch_t enc[ENCODER_CH_CNT] = {0}; // array of channels data

static uint8_t msg_buf[ENCODER_MSG_BUF_LEN] = {0};

static uint8_t state_list[4] =
{
    //      clockwise (CW) direction phase states sequence

    //      AB AB AB AB AB AB AB AB AB AB AB AB AB
    //      00 01 11 10 00 01 11 10 00 01 11 10 00

    // A    _____|`````|_____|`````|_____|`````|__

    // B    __|`````|_____|`````|_____|`````|_____

    //        AB    AB    AB    AB
    //      0b00, 0b01, 0b11, 0b10

    // prev    next
    //   AB      AB
    /* 0b00 */ 0b01,
    /* 0b01 */ 0b11,
    /* 0b10 */ 0b00,
    /* 0b11 */ 0b10
};




// public methods

/**
 * @brief   module base thread
 * @note    call this function anywhere in the main loop
 * @retval  none
 */
void encoder_module_base_thread()
{
    static uint8_t c, A, B, Z, AB;

    // check all channels
    for ( c = ENCODER_CH_CNT; c--; )
    {
        if ( !enc[c].enabled ) continue;

        if ( enc[c].using_Z ) // if we are using ABZ encoder
        {
            Z = (uint8_t) gpio_pin_get(enc[c].port[PH_Z], enc[c].pin[PH_Z]);

            if ( enc[c].state[PH_Z] != Z ) // on phase Z state change
            {
                if ( Z ) enc[c].counts = 0;
                enc[c].state[PH_Z] = Z;
            }
        }

        A = (uint8_t) gpio_pin_get(enc[c].port[PH_A], enc[c].pin[PH_A]);

        if ( enc[c].using_B ) // if we are using AB encoder
        {
            B = (uint8_t) gpio_pin_get(enc[c].port[PH_B], enc[c].pin[PH_B]);

            if ( enc[c].state[PH_A] != A || enc[c].state[PH_B] != B ) // on any phase change
            {
                AB = (A << 1) | B; // get encoder state

                if ( state_list[enc[c].AB_state] == AB ) enc[c].counts++; // CW
                else                                     enc[c].counts--; // CCW

                enc[c].AB_state = AB;
            }

            enc[c].state[PH_B] = B;
        }
        else if ( enc[c].state[PH_A] != A && A ) // if we are using A encoder and phase A is HIGH
        {
            enc[c].counts++; // CW
        }

        enc[c].state[PH_A] = A;
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
    // set GPIO pin function to INPUT
    gpio_pin_setup_for_input(port, pin);

    // set phase pin parameters
    enc[c].port[phase] = port;
    enc[c].pin[phase] = pin;
    enc[c].state[phase] = (uint8_t) gpio_pin_get(port, pin);
}




/**
 * @brief   setup selected channel of encoder counter
 *
 * @param   c           channel id
 * @param   enabled     enable encoder counter right now?
 * @param   using_B     use phase B input?
 * @param   using_Z     use phase Z index input?
 *
 * @retval  none
 */
void encoder_setup(uint8_t c, uint8_t enabled, uint8_t using_B, uint8_t using_Z)
{
    // set channel parameters
    enc[c].enabled  = enabled;
    enc[c].using_B  = using_B;
    enc[c].using_Z  = using_Z;

    // set encoder state
    enc[c].AB_state = (enc[c].state[PH_A] << 1) | enc[c].state[PH_B];
}

/**
 * @brief   enable/disable selected channel of encoder counter
 * @param   c       channel id
 * @retval  none
 */
void encoder_state_set(uint8_t c, uint8_t state)
{
    enc[c].enabled = state ? 1 : 0;
}

/**
 * @brief   reset number of counts for the selected channel
 * @param   c       channel id
 * @retval  none
 */
void encoder_counts_reset(uint8_t c)
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
uint8_t encoder_state_get(uint8_t c)
{
    return enc[c].enabled;
}

/**
 * @brief   get current counts for the selected channel
 * @param   c   channel id
 * @retval  signed 32-bit number
 */
int32_t encoder_counts_get(uint8_t c)
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
    static uint8_t c, p;

    switch (type)
    {
        case ENCODER_MSG_PINS_SETUP:
        {
            for ( c = ENCODER_CH_CNT; c--; )
            {
                if ( (uint8_t) ENCODER_MSG_BUF_PORT(msg,c,PH_A) >= GPIO_PORTS_CNT ) continue;

                for ( p = ENCODER_PH_CNT; p--; )
                {
                    encoder_pin_setup( c, p,
                        (uint8_t) ENCODER_MSG_BUF_PORT(msg,c,p),
                        (uint8_t) ENCODER_MSG_BUF_PIN (msg,c,p)
                    );
                }
            }

            break;
        }

        case ENCODER_MSG_SETUP:
        {
            for ( c = ENCODER_CH_CNT; c--; )
            {
                if ( (uint8_t) ENCODER_MSG_BUF_ENABLED(msg,c) > 1 ) continue;

                encoder_setup( c,
                    (uint8_t) ENCODER_MSG_BUF_ENABLED(msg,c),
                    (uint8_t) ENCODER_MSG_BUF_USING_B(msg,c),
                    (uint8_t) ENCODER_MSG_BUF_USING_Z(msg,c)
                );
            }

            break;
        }

        case ENCODER_MSG_COUNTS:
        {
            for ( c = ENCODER_CH_CNT; c--; )
            {
                ENCODER_MSG_BUF_COUNTS(&msg_buf,c) = ENCODER_MSG_BUF_COUNTS(msg,c) ?
                    encoder_counts_get(c) :
                    0 ;
            }

            msg_send(type, (uint8_t*)&msg_buf, ENCODER_MSG_COUNTS_LEN);

            break;
        }

        case ENCODER_MSG_ENABLE:
        {
            for ( c = ENCODER_CH_CNT; c--; )
            {
                if ( ENCODER_MSG_BUF_ENABLE(msg) & (1U << c) )  encoder_state_set(c, 1);
                else                                            encoder_state_set(c, 0);
            }

            break;
        }

        case ENCODER_MSG_RESET:
        {
            for ( c = ENCODER_CH_CNT; c--; )
            {
                if ( ENCODER_MSG_BUF_RESET(msg) & (1U << c) ) encoder_counts_reset(c);
            }

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
