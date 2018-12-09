/**
 * @file    mod_pulsgen.c
 *
 * @brief   pulses generator module
 *
 * This module implements an API
 * to make real-time pulses generation using GPIO
 */

#include "mod_timer.h"
#include "mod_gpio.h"
#include "mod_pulsgen.h"




// private vars

static uint8_t max_id = 0; // maximum channel id
static struct pulsgen_ch_t gen[PULSGEN_CH_CNT] = {0}; // array of channels data
static uint8_t msg_buf[PULSGEN_MSG_BUF_LEN] = {0};

// uses with GPIO module macros
extern volatile uint32_t * gpio_port_data[GPIO_PORTS_CNT];




// public methods

/**
 * @brief   module init
 * @note    call this function only once before pulsgen_module_base_thread()
 * @retval  none
 */
void pulsgen_module_init()
{
    uint8_t i = 0;

    // start sys timer
    TIMER_START();

    // add message handlers
    for ( i = PULSGEN_MSG_PIN_SETUP; i <= PULSGEN_MSG_TASK_TOGGLES; i++ )
    {
        msg_recv_callback_add(i, (msg_recv_func_t) pulsgen_msg_recv);
    }
}

/**
 * @brief   module base thread
 * @note    call this function in the main loop, before gpio_module_base_thread()
 * @retval  none
 */
void pulsgen_module_base_thread()
{
    static uint8_t c;
    static uint64_t tick;

    // get current CPU tick
    tick = timer_cnt_get_64();

    // check all working channels
    for ( c = max_id + 1; c--; )
    {
        if ( !gen[c].task || tick < gen[c].todo_tick ) continue;

        if ( !gen[c].task_infinite && !gen[c].task_toggles_todo ) // if we have no steps to do
        {
            gen[c].task = 0; // disable channel
            if ( max_id && c == max_id ) --max_id; // if needed decrease channels max ID value
            continue; // goto next channel
        }

        if ( GPIO_PIN_GET(gen[c].port, gen[c].pin_mask) ^ gen[c].pin_inverted )
        {
            GPIO_PIN_CLEAR(gen[c].port, gen[c].pin_mask_not);
            gen[c].todo_tick += (uint64_t)gen[c].setup_ticks;
        }
        else
        {
            GPIO_PIN_SET(gen[c].port, gen[c].pin_mask);
            gen[c].todo_tick += (uint64_t)gen[c].hold_ticks;
        }

        --gen[c].task_toggles_todo; // decrease number of pin changes to do
    }
}




/**
 * @brief   setup GPIO pin for the selected channel
 *
 * @param   c           channel id
 * @param   port        GPIO port number
 * @param   pin         GPIO pin number
 * @param   inverted    invert pin state?
 *
 * @retval  none
 */
void pulsgen_pin_setup(uint8_t c, uint8_t port, uint8_t pin, uint8_t inverted)
{
    gpio_pin_setup_for_output(port, pin);

    gen[c].port = port;
    gen[c].pin_mask = 1U << pin;
    gen[c].pin_mask_not = ~(gen[c].pin_mask);
    gen[c].pin_inverted = inverted ? gen[c].pin_mask : 0;

    // set pin state
    if ( gen[c].pin_inverted )  GPIO_PIN_SET    (port, gen[c].pin_mask);
    else                        GPIO_PIN_CLEAR  (port, gen[c].pin_mask_not);
}




/**
 * @brief   setup a new task for the selected channel
 *
 * @param   c               channel id
 * @param   toggles         number of pin state changes
 * @param   pin_setup_time  pin state setup_time (in nanoseconds)
 * @param   pin_hold_time   pin state hold_time (in nanoseconds)
 * @param   start_delay     task start delay (in nanoseconds)
 *
 * @retval  none
 */
void pulsgen_task_setup
(
    uint32_t c,
    uint32_t toggles,
    uint32_t pin_setup_time,
    uint32_t pin_hold_time,
    uint32_t start_delay
)
{
    if ( c > max_id ) ++max_id;

    // set task data
    gen[c].task = 1;
    gen[c].task_infinite = toggles ? 0 : 1;
    gen[c].task_toggles = toggles ? toggles : UINT32_MAX;
    gen[c].task_toggles_todo = gen[c].task_toggles;

    gen[c].setup_ticks = (uint32_t) ( (uint64_t)pin_setup_time *
        (uint64_t)TIMER_FREQUENCY_MHZ / (uint64_t)1000 );
    gen[c].hold_ticks = (uint32_t) ( (uint64_t)pin_hold_time *
        (uint64_t)TIMER_FREQUENCY_MHZ / (uint64_t)1000 );

    gen[c].todo_tick = timer_cnt_get_64();

    // if we need a delay before task start
    if ( start_delay )
    {
        gen[c].todo_tick += (uint64_t)start_delay *
            (uint64_t)TIMER_FREQUENCY_MHZ / (uint64_t)1000;
    }
}

/**
 * @brief   abort current task for the selected channel
 * @param   c       channel id
 * @retval  none
 */
void pulsgen_task_abort(uint8_t c)
{
    gen[c].task = 0;

    if ( max_id && c == max_id ) --max_id;
}




/**
 * @brief   get current task state for the selected channel
 *
 * @param   c   channel id
 *
 * @retval  0   (channel have no task)
 * @retval  1   (channel have a task)
 */
uint8_t pulsgen_task_state(uint8_t c)
{
    return gen[c].task;
}

/**
 * @brief   get current pin state changes since task start
 * @param   c   channel id
 * @retval  0..0xFFFFFFFF
 */
uint32_t pulsgen_task_toggles(uint8_t c)
{
    return gen[c].task_toggles - gen[c].task_toggles_todo;
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
int8_t volatile pulsgen_msg_recv(uint8_t type, uint8_t * msg, uint8_t length)
{
    static uint8_t i = 0;

    switch (type)
    {
        case PULSGEN_MSG_PIN_SETUP:
        {
            struct pulsgen_msg_pin_setup_t in = *((struct pulsgen_msg_pin_setup_t *) msg);
            pulsgen_pin_setup(in.ch, in.port, in.pin, in.inverted);
            break;
        }
        case PULSGEN_MSG_TASK_SETUP:
        {
            struct pulsgen_msg_task_setup_t in = *((struct pulsgen_msg_task_setup_t *) msg);
            pulsgen_task_setup(in.ch, in.toggles, in.pin_setup_time, in.pin_hold_time, in.start_delay);
            break;
        }
        case PULSGEN_MSG_TASK_ABORT:
        {
            struct pulsgen_msg_ch_t in = *((struct pulsgen_msg_ch_t *) msg);
            pulsgen_task_abort(in.ch);
            break;
        }
        case PULSGEN_MSG_TASK_STATE:
        {
            struct pulsgen_msg_ch_t in = *((struct pulsgen_msg_ch_t *) msg);
            struct pulsgen_msg_state_t out = *((struct pulsgen_msg_state_t *) &msg_buf);
            out.state = pulsgen_task_state(in.ch);
            msg_send(type, (uint8_t*)&out, 4);
            break;
        }
        case PULSGEN_MSG_TASK_TOGGLES:
        {
            struct pulsgen_msg_ch_t in = *((struct pulsgen_msg_ch_t *) msg);
            struct pulsgen_msg_toggles_t out = *((struct pulsgen_msg_toggles_t *) &msg_buf);
            out.toggles = pulsgen_task_toggles(in.ch);
            msg_send(type, (uint8_t*)&out, 4);
            break;
        }

        default: return -1;
    }

    return 0;
}




/**
    @example mod_pulsgen.c

    <b>Usage example 1</b>: enable infinite PWM signal on GPIO pin PA3

    @code
        #include <stdint.h>
        #include "mod_gpio.h"
        #include "mod_pulsgen.h"

        int main(void)
        {
            // module init
            pulsgen_module_init();

            // use GPIO pin PA3 for the channel 0 output
            pulsgen_pin_setup(0, PA, 3, 0);

            // enable infinite PWM signal on the channel 0
            // PWM frequency = 20 kHz, duty cycle = 50%
            pulsgen_task_setup(0, 0, 25000, 25000, 0);

            // main loop
            for(;;)
            {
                // real update of channel states
                pulsgen_module_base_thread();
                // real update of pin states
                gpio_module_base_thread();
            }

            return 0;
        }
    @endcode

    <b>Usage example 2</b>: output of STEP/DIR signal

    @code
        #include <stdint.h>
        #include "mod_gpio.h"
        #include "mod_pulsgen.h"

        #define STEP_CHANNEL 0
        #define DIR_CHANNEL 1

        int main(void)
        {
            // uses to switch between DIR an STEP output
            uint8_t dir_output = 0; // 0 = STEP output, 1 = DIR output

            // module init
            pulsgen_module_init();

            // use GPIO pin PA3 for the STEP output on the channel 0
            pulsgen_pin_setup(STEP_CHANNEL, PA, 3, 0);

            // use GPIO pin PA5 for the DIR output on the channel 1
            pulsgen_pin_setup(DIR_CHANNEL, PA, 5, 0);

            // main loop
            for(;;)
            {
                if // if both channels aren't busy
                (
                    ! pulsgen_task_state(STEP_CHANNEL) &&
                    ! pulsgen_task_state(DIR_CHANNEL)
                )
                {
                    if ( dir_output ) // if it's time to make a DIR change
                    {
                        // make a DIR change with 20 kHz rate and 50% duty cycle
                        pulsgen_task_setup(DIR_CHANNEL, 1, 25000, 25000, 0);
                        dir_output = 0;
                    }
                    else // if it's time to make a STEP output
                    {
                        // start output of 1000 steps with 20 kHz rate,
                        // 50% duty cycle and startup delay = 50 us
                        pulsgen_task_setup(STEP_CHANNEL, 2000, 25000, 25000, 50000);
                        dir_output = 1;
                    }
                }

                // real update of channel states
                pulsgen_module_base_thread();
                // real update of pin states
                gpio_module_base_thread();
            }

            return 0;
        }
    @endcode
*/
