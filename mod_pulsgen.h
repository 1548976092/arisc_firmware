/**
 * @file    mod_pulsgen.h
 *
 * @brief   pulses generator module header
 *
 * This module implements an API
 * to make real-time pulses generation using GPIO
 */

#ifndef _MOD_PULSGEN_H
#define _MOD_PULSGEN_H

#include <stdint.h>




#define PULSGEN_CH_CNT      32  ///< maximum number of pulse generator channels
#define PULSGEN_MAX_DUTY    100 ///< maximum percent of pulse duty cycle




/// a channel parameters
struct pulsgen_ch_t
{
    uint8_t     port;               // GPIO port number
    uint8_t     pin;                // GPIO pin number
    uint8_t     pin_state;          // 0/1
    uint8_t     pin_inverted;       // 0/1

    uint8_t     task;               // 0 = "channel disabled"
    uint8_t     task_infinite;      // 0 = "make task_toggles and disable the channel"
    uint32_t    task_toggles;       // total number of pin state changes
    uint32_t    task_toggles_todo;  // current number of pin state changes we must to do

    uint32_t    low_ticks;          // number of CPU ticks when pin must be in LOW state
    uint32_t    high_ticks;         // number of CPU ticks when pin must be in HIGH state

    uint32_t    todo_tick;          // timestamp (in CPU ticks) to change pin state
    uint8_t     todo_tick_ovrfl;    // timestamp overflow flag
};




// export public methods

void pulsgen_module_init();
void pulsgen_module_base_thread();

void pulsgen_pin_setup(uint8_t c, uint8_t port, uint8_t pin, uint8_t inverted);

void pulsgen_task_setup(uint8_t c, uint32_t frequency, uint32_t toggles, uint32_t duty, uint8_t infinite);
void pulsgen_task_abort(uint8_t c);

uint8_t pulsgen_task_state(uint8_t c);
uint32_t pulsgen_task_toggles(uint8_t c);



#endif
