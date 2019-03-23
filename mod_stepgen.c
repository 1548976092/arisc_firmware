/**
 * @file    mod_stepgen.c
 * @brief   steps generator module
 * This module implements an API to make real-time step/dir pulses via GPIO
 */

#include "mod_timer.h"
#include "mod_gpio.h"
#include "mod_stepgen.h"




// private vars

static uint8_t max_id = 0; // maximum channel id
static stepgen_ch_t gen[STEPGEN_CH_CNT] = {0}; // array of channels data
static uint8_t msg_buf[STEPGEN_MSG_BUF_LEN] = {0};
static uint64_t tick = 0;

// uses with GPIO module macros
extern volatile uint32_t * gpio_port_data[GPIO_PORTS_CNT];




// private function prototypes

static void abort(uint8_t c);




// public methods

/**
 * @brief   module init
 * @note    call this function only once before stepgen_module_base_thread()
 * @retval  none
 */
void stepgen_module_init()
{
    uint8_t i = 0;

    // start sys timer
    TIMER_START();

    // add message handlers
    for ( i = STEPGEN_MSG_PIN_SETUP; i < STEPGEN_MSG_CNT; i++ )
    {
        msg_recv_callback_add(i, (msg_recv_func_t) stepgen_msg_recv);
    }
}

/**
 * @brief   module base thread
 * @note    call this function in the main loop, before gpio_module_base_thread()
 * @retval  none
 */
void stepgen_module_base_thread()
{
    static uint8_t c, t;
    static stepgen_ch_t *g;
    static stepgen_fifo_slot_t *s;

    // get current CPU tick
    tick = timer_cnt_get_64();

    // check all working channels
    for ( c = max_id + 1; c--; )
    {
        g = &gen[c];                    // stepgen channel
        s = &g->tasks[g->task_slot];    // channel's fifo slot
        t = s->type;                    // channel's task type (0=step,1=dir)

        // channel disabled?
        if ( !s->toggles ) continue;
        // we need to stop? && (it's DIR task || step pin is LOW)
        if ( g->abort && (t || !g->pin_state[t]) ) { abort(c); continue; }
        // it's not a time for a pulse?
        if ( tick < g->task_tick ) continue;

        // pin state is HIGH?
        if ( g->pin_state[t] )
        {
            if ( g->pin_invert[t] ) GPIO_PIN_SET(g->pin_port[t], g->pin_mask[t]);
            else GPIO_PIN_CLEAR(g->pin_port[t], g->pin_mask_not[t]);
            g->pin_state[t] = 0;
            g->task_tick += g->pin_low_ticks[t];
        }
        else // pin state is LOW
        {
            if ( !g->pin_invert[t] ) GPIO_PIN_SET(g->pin_port[t], g->pin_mask[t]);
            else GPIO_PIN_CLEAR(g->pin_port[t], g->pin_mask_not[t]);
            g->pin_state[t] = 1;
            g->task_tick += g->pin_high_ticks[t];
        }

        // it's a STEP task?
        if ( !t ) g->pos += g->pin_state[1] ? -1 : 1; // update position

        // update toggles
        s->toggles--;
        // no more toggles to do?
        if ( !s->toggles )
        {
            // go to the next fifo slot
            g->task_slot++;
            if ( g->task_slot >= STEPGEN_FIFO_SIZE ) g->task_slot = 0;
        }
    }
}




/**
 * @brief   setup GPIO pin for the selected channel
 *
 * @param   c               channel id
 * @param   type            0:step, 1:dir
 * @param   port            GPIO port number
 * @param   pin             GPIO pin number
 * @param   invert          invert pin state?
 * @param   pin_low_time    pin LOW state duration (in nanoseconds)
 * @param   pin_high_time   pin HIGH state duration (in nanoseconds)
 *
 * @retval  none
 */
void stepgen_pin_setup(
    uint8_t c,
    uint8_t type,
    uint8_t port,
    uint8_t pin,
    uint8_t invert,
    uint32_t pin_low_time,
    uint32_t pin_high_time)
{
    gpio_pin_setup_for_output(port, pin);

    gen[c].pin_port[type] = port;
    gen[c].pin_mask[type] = 1U << pin;
    gen[c].pin_mask_not[type] = ~(gen[c].pin_mask[type]);
    gen[c].pin_invert[type] = invert;

    gen[c].pin_low_ticks[type] = (uint32_t) ( (uint64_t)pin_low_time *
        (uint64_t)TIMER_FREQUENCY_MHZ / (uint64_t)1000 );
    gen[c].pin_high_ticks[type] = (uint32_t) ( (uint64_t)pin_high_time *
        (uint64_t)TIMER_FREQUENCY_MHZ / (uint64_t)1000 );

    // reset pin state
    gen[c].pin_state[type] = 0;
    if ( gen[c].pin_invert[type] ) GPIO_PIN_SET(port, gen[c].pin_mask[type]);
    else GPIO_PIN_CLEAR(port, gen[c].pin_mask_not[type]);
}




/**
 * @brief   add a new task for the selected channel
 *
 * @param   c               channel id
 * @param   type            0:step, 1:dir
 * @param   toggles         number of pin state changes
 *
 * @retval  none
 */
void stepgen_task_add(uint8_t c, uint8_t type, uint32_t toggles)
{
    uint8_t i, slot;

    // find free fifo slot for the new task
    for ( i = STEPGEN_FIFO_SIZE, slot = gen[c].task_slot; i--; slot++ )
    {
        if ( slot >= STEPGEN_FIFO_SIZE ) slot = 0;
        if ( !gen[c].tasks[slot].toggles ) break;
    }

    // no free slots?
    if ( gen[c].tasks[slot].toggles ) return;

    if ( c > max_id ) ++max_id;

    gen[c].tasks[slot].type = type;
    gen[c].tasks[slot].toggles = toggles;

    // need to start task right now?
    if ( slot == gen[c].task_slot ) gen[c].task_tick = tick +
        gen[c].pin_state[type] ?
            gen[c].pin_high_ticks[type] :
            gen[c].pin_low_ticks[type] ;
}




/**
 * @brief   abort all tasks for the selected channel
 * @param   c   channel id
 * @retval  none
 */
void stepgen_abort(uint8_t c)
{
    gen[c].abort = 1;
}

static void abort(uint8_t c)
{
    uint8_t i;

    gen[c].abort = 0;

    if ( max_id && c == max_id ) --max_id;

    // fifo cleanup
    for ( i = STEPGEN_FIFO_SIZE; i--; ) gen[c].tasks[i].toggles = 0;
}




/**
 * @brief   set channel steps position
 * @param   c   channel id
 * @retval  integer 4-bytes
 */
int32_t stepgen_pos_get(uint8_t c)
{
    return (int32_t)(gen[c].pos / 2);
}

/**
 * @brief   set channel steps position
 * @param   c       channel id
 * @param   pos     integer 4-bytes
 * @retval  none
 */
void stepgen_pos_set(uint8_t c, int32_t pos)
{
    gen[c].pos = 2 * ((int64_t)pos);
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
int8_t volatile stepgen_msg_recv(uint8_t type, uint8_t * msg, uint8_t length)
{
    static uint8_t i = 0;

    u32_10_t *in = (u32_10_t*) msg;
    u32_10_t *out = (u32_10_t*) msg_buf;

    switch (type)
    {
        case STEPGEN_MSG_PIN_SETUP:
            stepgen_pin_setup(in->v[0], in->v[1], in->v[2], in->v[3], in->v[4], in->v[5], in->v[6]);
            break;
        case STEPGEN_MSG_TASK_ADD:
            stepgen_task_add(in->v[0], in->v[1], in->v[2]);
            break;
        case STEPGEN_MSG_ABORT:
            stepgen_abort(in->v[0]);
            break;
        case STEPGEN_MSG_POS_GET:
            out->v[0] = (uint32_t) stepgen_pos_get(in->v[0]);
            msg_send(type, msg_buf, 4);
            break;
        case STEPGEN_MSG_POS_SET:
            stepgen_pos_set(in->v[0], (int32_t)in->v[1]);
            break;

        default: return -1;
    }

    return 0;
}




/**
    @example mod_stepgen.c

    <b>Usage example 1</b>: enable infinite PWM signal on GPIO pin PA3

    @code
        #include <stdint.h>
        #include "mod_gpio.h"
        #include "mod_stepgen.h"

        int main(void)
        {
            // module init
            stepgen_module_init();

            // use GPIO pin PA3 for the channel 0 output
            stepgen_pin_setup(0, PA, 3, 0);

            // enable infinite PWM signal on the channel 0
            // PWM frequency = 20 kHz, duty cycle = 50%
            stepgen_task_add(0, 0, 0, 25000, 25000, 0);

            // main loop
            for(;;)
            {
                // real update of channel states
                stepgen_module_base_thread();
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
        #include "mod_stepgen.h"

        #define STEP_CHANNEL 0
        #define DIR_CHANNEL 1

        int main(void)
        {
            // uses to switch between DIR an STEP output
            uint8_t dir_output = 0; // 0 = STEP output, 1 = DIR output

            // module init
            stepgen_module_init();

            // use GPIO pin PA3 for the STEP output on the channel 0
            stepgen_pin_setup(STEP_CHANNEL, PA, 3, 0);

            // use GPIO pin PA5 for the DIR output on the channel 1
            stepgen_pin_setup(DIR_CHANNEL, PA, 5, 0);

            // main loop
            for(;;)
            {
                if // if both channels aren't busy
                (
                    ! stepgen_state_get(STEP_CHANNEL) &&
                    ! stepgen_state_get(DIR_CHANNEL)
                )
                {
                    if ( dir_output ) // if it's time to make a DIR change
                    {
                        // make a DIR change with 20 kHz rate and 50% duty cycle
                        stepgen_task_add(DIR_CHANNEL, 0, 1, 25000, 25000, 0);
                        dir_output = 0;
                    }
                    else // if it's time to make a STEP output
                    {
                        // start output of 1000 steps with 20 kHz rate,
                        // 50% duty cycle and startup delay = 50 us
                        stepgen_task_add(STEP_CHANNEL, 0, 2000, 25000, 25000, 50000);
                        dir_output = 1;
                    }
                }

                // real update of channel states
                stepgen_module_base_thread();
                // real update of pin states
                gpio_module_base_thread();
            }

            return 0;
        }
    @endcode
*/
