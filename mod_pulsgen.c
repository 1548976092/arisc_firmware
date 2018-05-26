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




// public methods

/**
 * @brief   module init
 * @note    call this function only once before pulsgen_module_base_thread()
 * @retval  none
 */
void pulsgen_module_init()
{
    TIMER_START();
}

/**
 * @brief   module base thread
 * @note    call this function in the main loop, before gpio_module_base_thread()
 * @retval  none
 */
void pulsgen_module_base_thread()
{
    static uint8_t c;
    static uint32_t tick, todo_tick;

    // do nothing if we have no enabled channels
    if ( !max_id ) return;

    // get current CPU tick
    tick = TIMER_CNT_GET();

    // check all working channels
    for ( c = max_id; c--; )
    {
        if ( !gen[c].task ) continue; // if channel disabled, goto next channel

        if ( !gen[c].task_infinite && !gen[c].task_pulses_todo ) // if we have no steps to do
        {
            gen[c].task = 0; // disable channel
            continue; // goto next channel
        }

        if // if it's time to make a pulse change
        (
            ( !gen[c].todo_tick_ovrfl &&             tick  >= gen[c].todo_tick ) ||
            (  gen[c].todo_tick_ovrfl && (UINT32_MAX-tick) >= gen[c].todo_tick)
        )
        {
            todo_tick = gen[c].todo_tick; // save current tick value

            if ( gen[c].pin_state ) // if current pin state is HIGH
            {
                gen[c].pin_state = 0; // set pin state to LOW
                if ( !gen[c].task_infinite ) --gen[c].task_pulses_todo; // decrease number of pulses to do
                gen[c].todo_tick += gen[c].low_ticks; // set new timestamp
            }
            else // if current pin state is LOW
            {
                gen[c].pin_state = 1; // set step state to HIGH
                gen[c].todo_tick += gen[c].high_ticks; // set new timestamp
            }

            // set timestamp overflow flag
            gen[c].todo_tick_ovrfl = gen[c].todo_tick < todo_tick ? 1 : 0;

            // toggle pin
            if ( gen[c].pin_state ^ gen[c].pin_inverted )
            {
                gpio_pin_clear(gen[c].port, gen[c].pin);
            }
            else
            {
                gpio_pin_set(gen[c].port, gen[c].pin);
            }
        }
    }
}
