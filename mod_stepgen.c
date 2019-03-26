/**
 * @file    mod_stepgen.c
 * @brief   steps generator module
 * This module implements an API to make real-time step/dir pulses via GPIO
 */

#include "mod_timer.h"
#include "mod_gpio.h"
#include "mod_stepgen.h"




#define SG gen[c]               // current channel
#define SLOT SG.task_slot       // current task slot
#define TASK SG.tasks[SLOT]     // current task




// private vars

static uint8_t max_id = 0; // uses to speedup idle channels processing
static stepgen_ch_t gen[STEPGEN_CH_CNT] = {0}; // array of channels data
static uint8_t msg_buf[STEPGEN_MSG_BUF_LEN] = {0}; // message buffer
static uint64_t tick = 0; // last CPU tick

// uses with GPIO module macros
extern volatile uint32_t * gpio_port_data[GPIO_PORTS_CNT];




// private functions

static void busy(uint8_t c)
{
    // we need to increase max channel ID?
    if ( c > max_id ) max_id = c;
}

static void idle(uint8_t c)
{
    // no need to decrease max channel ID?
    if ( !max_id || c != max_id ) return;
    // decrease max channel ID
    for ( max_id--; gen[max_id].tasks[SLOT].pulses; max_id-- );
}

static void toggle_pin(uint8_t c, uint8_t t)
{
    if ( SG.pin_state[t] ^ SG.pin_invert[t] )
        GPIO_PIN_SET(SG.pin_port[t], SG.pin_mask[t]);
    else
        GPIO_PIN_CLEAR(SG.pin_port[t], SG.pin_mask_not[t]);
}

static void goto_next_task(uint8_t c)
{
    static uint8_t i, slot;

    // find next task
    for ( i = STEPGEN_FIFO_SIZE, slot = SLOT; i--; slot++ )
    {
        if ( slot >= STEPGEN_FIFO_SIZE ) slot = 0;
        if ( SG.tasks[slot].pulses ) break;
    }

    // no more tasks to do?
    if ( !SG.tasks[slot].pulses ) { idle(c); return; }

    // save new task slot
    SLOT = slot;

    // start task
    if ( SG.tasks[slot].type ) // DIR task
    {
        SG.tasks[slot].pulses = 2;
        SG.task_tick += SG.tasks[slot].low_ticks;
    }
    else // STEP task
    {
        SG.task_infinite = SG.tasks[slot].pulses > INT32_MAX ? 1 : 0;
        SG.pin_state[SG.tasks[slot].type] = 1;
        SG.task_tick += SG.tasks[slot].high_ticks;
        toggle_pin(c, SG.tasks[slot].type);
    }
}

static void abort(uint8_t c)
{
    if ( SG.abort > 1 )
    {
        // fifo cleanup
        uint8_t i;
        for ( i = STEPGEN_FIFO_SIZE; i--; )
        {
            // abort tasks added before abort command only
            if ( SG.tasks[i].tick > SG.abort_tick ) SG.tasks[i].pulses = 0;
        }
        // free channel id
        idle(c);
    }
    else goto_next_task(c);

    SG.abort = 0;
}




// public methods

/**
 * @brief   module init
 * @note    call this function only once before stepgen_module_base_thread()
 * @retval  none
 */
void stepgen_module_init()
{
    // start sys timer
    TIMER_START();

    // add message handlers
    uint8_t i = 0;
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
    static uint8_t c;

    // get current CPU tick
    tick = timer_cnt_get_64();

    // check all working channels
    for ( c = max_id + 1; c--; )
    {
        // channel disabled?
        if ( !TASK.pulses ) continue;
        // we need to stop? && (it's DIR task || step pin is LOW)
        if ( SG.abort && (TASK.type || !SG.pin_state[0]) ) { abort(c); continue; }
        // it's not a time for a pulse?
        if ( tick < SG.task_tick ) continue;

        if ( TASK.type ) // DIR task
        {
            if ( TASK.pulses > 1 ) // hold
            {
                SG.pin_state[TASK.type] = SG.pin_state[TASK.type] ? 0 : 1;
                SG.task_tick += TASK.high_ticks;
            }
            else goto_next_task(c); // dir task done

            TASK.pulses--;
        }
        else // STEP task
        {
            if ( SG.pin_state[TASK.type] ) // high
            {
                SG.pin_state[TASK.type] = 0;
                SG.task_tick += TASK.low_ticks;
            }
            else // low
            {
                SG.pos += SG.pin_state[1] ? -1 : 1;
                if ( !SG.task_infinite ) TASK.pulses--;

                if ( TASK.pulses ) // have we more steps to do?
                {
                    SG.pin_state[TASK.type] = 1;
                    SG.task_tick += TASK.high_ticks;
                }
                else goto_next_task(c); // step task done
            }
        }

        toggle_pin(c, TASK.type);
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
 *
 * @retval  none
 */
void stepgen_pin_setup(uint8_t c, uint8_t type, uint8_t port, uint8_t pin, uint8_t invert)
{
    gpio_pin_setup_for_output(port, pin);

    SG.pin_state[type] = 0;
    SG.pin_port[type] = port;
    SG.pin_mask[type] = 1U << pin;
    SG.pin_mask_not[type] = ~(SG.pin_mask[type]);
    SG.pin_invert[type] = invert ? 1 : 0;

    toggle_pin(c, type);
}




/**
 * @brief   add a new task for the selected channel
 *
 * @param   c               channel id
 * @param   type            0:step, 1:dir
 * @param   pulses          number of pulses (ignored for DIR task)
 * @param   pin_low_time    pin LOW state duration (in nanoseconds)
 * @param   pin_high_time   pin HIGH state duration (in nanoseconds)
 *
 * @retval  none
 */
void stepgen_task_add(uint8_t c, uint8_t type, uint32_t pulses, uint32_t pin_low_time, uint32_t pin_high_time)
{
    uint8_t i, slot;

    // find free fifo slot for the new task
    for ( i = STEPGEN_FIFO_SIZE, slot = SLOT; i--; slot++ )
    {
        if ( slot >= STEPGEN_FIFO_SIZE ) slot = 0;
        if ( !SG.tasks[slot].pulses ) break;
    }

    // no free slots?
    if ( SG.tasks[slot].pulses ) return;

    busy(c);

    SG.tasks[slot].tick = tick;
    SG.tasks[slot].type = type;
    SG.tasks[slot].pulses = type ? 2 : pulses;
    SG.tasks[slot].low_ticks = (uint32_t) ( (uint64_t)pin_low_time *
        (uint64_t)TIMER_FREQUENCY_MHZ / (uint64_t)1000 );
    SG.tasks[slot].high_ticks = (uint32_t) ( (uint64_t)pin_high_time *
        (uint64_t)TIMER_FREQUENCY_MHZ / (uint64_t)1000 );

    // start a task right now?
    if ( slot == SLOT )
    {
        SG.task_tick = tick + 9000;

        if ( type )
        {
            SG.task_tick += SG.tasks[slot].low_ticks;
        }
        else
        {
            SG.task_infinite = SG.tasks[slot].pulses > INT32_MAX ? 1 : 0;
            SG.pin_state[type] = 1;
            SG.task_tick += SG.tasks[slot].high_ticks;
            toggle_pin(c, type);
        }
    }
}




/**
 * @brief   abort all tasks for the selected channel
 * @param   c       channel id
 * @param   all     abort all task?
 * @retval  none
 */
void stepgen_abort(uint8_t c, uint8_t all)
{
    SG.abort = all ? 2 : 1;
    SG.abort_tick = tick;
}




/**
 * @brief   set channel steps position
 * @param   c   channel id
 * @retval  integer 4-bytes
 */
int32_t stepgen_pos_get(uint8_t c)
{
    return SG.pos;
}

/**
 * @brief   set channel steps position
 * @param   c       channel id
 * @param   pos     integer 4-bytes
 * @retval  none
 */
void stepgen_pos_set(uint8_t c, int32_t pos)
{
    SG.pos = pos;
}




/**
 * @brief   task count left to do
 * @param   c           channel id
 * @retval  uint8_t     task count left to do (including current task)
 */
uint8_t stepgen_tasks_left(uint8_t c)
{
    uint8_t i, slot, cnt = 0;

    for ( i = STEPGEN_FIFO_SIZE, slot = SLOT; i--; slot++ )
    {
        if ( slot >= STEPGEN_FIFO_SIZE ) slot = 0;
        if ( SG.tasks[slot].pulses ) cnt++;
    }

    return cnt;
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
    u32_10_t *in = (u32_10_t*) msg;
    u32_10_t *out = (u32_10_t*) msg_buf;

    switch (type)
    {
        case STEPGEN_MSG_PIN_SETUP:
            stepgen_pin_setup(in->v[0], in->v[1], in->v[2], in->v[3], in->v[4]);
            break;
        case STEPGEN_MSG_TASK_ADD:
            stepgen_task_add(in->v[0], in->v[1], in->v[2], in->v[3], in->v[4]);
            break;
        case STEPGEN_MSG_ABORT:
            stepgen_abort(in->v[0], in->v[1]);
            break;
        case STEPGEN_MSG_POS_GET:
            out->v[0] = (uint32_t) stepgen_pos_get(in->v[0]);
            msg_send(type, msg_buf, 4);
            break;
        case STEPGEN_MSG_POS_SET:
            stepgen_pos_set(in->v[0], (int32_t)in->v[1]);
            break;
        case STEPGEN_MSG_TASKS_LEFT:
            out->v[0] = (uint32_t) stepgen_tasks_left(in->v[0]);
            msg_send(type, msg_buf, 4);
            break;

        default: return -1;
    }

    return 0;
}




/**
    @example mod_stepgen.c

    <b>Usage example 1</b>: make two PWM tasks using GPIO pin PA3

    @code
        #include <stdint.h>
        #include "mod_gpio.h"
        #include "mod_stepgen.h"

        int main(void)
        {
            // module init
            stepgen_module_init();

            // use GPIO pin PA3 for the channel 0 output
            stepgen_pin_setup(0, 1, PA, 3, 0);

            // 1st PWM task - 20 kHz, duty cycle = 50%, duration 1 second
            stepgen_task_add(0, 1, 40000, 25000, 25000);
            // 2nd PWM task - 10 kHz, duty cycle = 20%, duration 1 second
            stepgen_task_add(0, 1, 20000, 80000, 20000);

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
*/
