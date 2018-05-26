#ifndef _PULSGEN_H
#define _PULSGEN_H

#include <stdint.h>




#define PULSGEN_CH_CNT 32 ///< maximum number of pulse generator channels




struct pulsgen_ch_t
{
    uint8_t     port;               // GPIO port number
    uint8_t     pin;                // GPIO pin number
    uint8_t     pin_state;          // 0/1
    uint8_t     pin_inverted;       // 0/1

    uint8_t     task;               // 0 = "channel disabled"
    uint8_t     task_infinite;      // 0 = "make task_pulses and disable the channel"
    uint32_t    task_pulses;        // total number of pulses for the task
    uint32_t    task_pulses_todo;   // current number of pulses we must to do

    uint32_t    low_ticks;          // number of CPU ticks when pin must be in LOW state
    uint32_t    high_ticks;         // number of CPU ticks when pin must be in HIGH state

    uint32_t    todo_tick;          // timestamp (in CPU ticks) to change pin state
    uint8_t     todo_tick_ovrfl;    // timestamp overflow flag
};




void pulsgen_module_init();
void pulsgen_module_base_thread();




#endif
