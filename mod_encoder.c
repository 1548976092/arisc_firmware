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

// uses with GPIO module macros
extern volatile uint32_t * gpio_port_data[GPIO_PORTS_CNT];




// public methods

/**
 * @brief   module init
 * @note    call this function only once before encoder_module_base_thread()
 * @retval  none
 */
void encoder_module_init()
{
    uint8_t i = 0;

    // add message handlers
    for ( i = ENCODER_MSG_PIN_SETUP; i <= ENCODER_MSG_COUNTS_GET; i++ )
    {
        msg_recv_callback_add(i, (msg_recv_func_t) encoder_msg_recv);
    }
}

/**
 * @brief   module base thread
 * @note    call this function anywhere in the main loop
 * @retval  none
 */
void encoder_module_base_thread()
{
    static uint8_t c = (ENCODER_CH_CNT - 1);
    static uint32_t A, B, Z, AB;

    if ( enc[c].enabled )
    {
        if ( enc[c].using_Z ) // if we are using ABZ encoder
        {
            Z = GPIO_PIN_GET(enc[c].port[PH_Z], enc[c].pin_mask[PH_Z]);

            if ( enc[c].state[PH_Z] != Z ) // on phase Z state change
            {
                if ( Z ) enc[c].counts = 0;
                enc[c].state[PH_Z] = Z;
            }
        }

        A = GPIO_PIN_GET(enc[c].port[PH_A], enc[c].pin_mask[PH_A]);

        if ( enc[c].using_B ) // if we are using AB encoder
        {
            B = GPIO_PIN_GET(enc[c].port[PH_B], enc[c].pin_mask[PH_B]);

            if ( enc[c].state[PH_A] != A || enc[c].state[PH_B] != B ) // on any phase change
            {
                AB = (A ? 0b10 : 0) | (A ? 0b01 : 0); // get encoder state

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

    --c;
    if ( !c ) c = (ENCODER_CH_CNT - 1);
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
    enc[c].pin_mask[phase] = 1U << pin;
    enc[c].state[phase] = GPIO_PIN_GET(port, enc[c].pin_mask[phase]);
}




/**
 * @brief   setup selected channel of encoder counter
 *
 * @param   c           channel id
 * @param   using_B     use phase B input?
 * @param   using_Z     use phase Z index input?
 *
 * @retval  none
 */
void encoder_setup(uint8_t c, uint8_t using_B, uint8_t using_Z)
{
    // set channel parameters
    enc[c].using_B  = using_B;
    enc[c].using_Z  = using_Z;

    // set encoder state
    enc[c].AB_state = (enc[c].state[PH_A] ? 0b10 : 0) |
                      (enc[c].state[PH_B] ? 0b01 : 0);
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
 * @brief   change number of counts for the selected channel
 * @param   c       channel id
 * @param   counts  new value for encoder channel counts
 * @retval  none
 */
void encoder_counts_set(uint8_t c, int32_t counts)
{
    enc[c].counts = counts;
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
    switch (type)
    {
        case ENCODER_MSG_PIN_SETUP:
        {
            struct encoder_msg_pin_setup_t in = *((struct encoder_msg_pin_setup_t *) msg);
            encoder_pin_setup(in.ch, in.phase, in.port, in.pin);
            break;
        }
        case ENCODER_MSG_SETUP:
        {
            struct encoder_msg_setup_t in = *((struct encoder_msg_setup_t *) msg);
            encoder_setup(in.ch, in.using_B, in.using_Z);
            break;
        }

        case ENCODER_MSG_STATE_SET:
        {
            struct encoder_msg_state_set_t in = *((struct encoder_msg_state_set_t *) msg);
            encoder_state_set(in.ch, in.state);
            break;
        }
        case ENCODER_MSG_STATE_GET:
        {
            struct encoder_msg_ch_t in = *((struct encoder_msg_ch_t *) msg);
            struct encoder_msg_state_get_t out = *((struct encoder_msg_state_get_t *) &msg_buf);
            out.state = encoder_state_get(in.ch);
            msg_send(type, (uint8_t*)&out, 4);
            break;
        }

        case ENCODER_MSG_COUNTS_SET:
        {
            struct encoder_msg_counts_set_t in = *((struct encoder_msg_counts_set_t *) msg);
            encoder_counts_set(in.ch, in.counts);
            break;
        }
        case ENCODER_MSG_COUNTS_GET:
        {
            struct encoder_msg_ch_t in = *((struct encoder_msg_ch_t *) msg);
            struct encoder_msg_counts_get_t out = *((struct encoder_msg_counts_get_t *) &msg_buf);
            out.counts = encoder_counts_get(in.ch);
            msg_send(type, (uint8_t*)&out, 4);
            break;
        }

        default: return -1;
    }

    return 0;
}




/**
    @example mod_encoder.c

    @code
        #include <stdint.h>
        #include "mod_gpio.h"
        #include "mod_encoder.h"

        int main(void)
        {
            // module init
            encoder_module_init();

            // setup phase pins of the channel 0
            encoder_pin_setup(0, PHASE_A, PA, 3);
            encoder_pin_setup(0, PHASE_B, PA, 5);
            encoder_pin_setup(0, PHASE_Z, PA, 8);

            // setup channel 0 parameters
            encoder_setup(0, 1, 1);

            // reset counter value of the channel 0
            if ( encoder_counts_get(0) ) encoder_counts_set(0, 0);

            // enable channel 0
            if ( !encoder_state_get(0) ) encoder_state_set(0, 1);

            // main loop
            for(;;)
            {
                // reset channel 0 counts after 100 CW counts
                if ( encoder_counts_get(0) > 100 ) encoder_counts_set(0, 0);

                // real update of channel states
                encoder_module_base_thread();
            }

            return 0;
        }
    @endcode
*/
