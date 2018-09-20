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
#include "mod_msg.h"
#include "mod_timer.h"




#define PULSGEN_CH_CNT      64  ///< maximum number of pulse generator channels




/// a channel parameters
struct pulsgen_ch_t
{
    uint32_t    port;               // GPIO port number
    uint32_t    pin_mask;           // GPIO pin mask
    uint32_t    pin_mask_not;       // GPIO pin ~mask
    uint32_t    pin_inverted;       // same as `pin_mask` or 0

    uint8_t     task;               // 0 = "channel disabled"
    uint8_t     task_infinite;      // 0 = "make task_toggles and disable the channel"
    uint32_t    task_toggles;       // total number of pin state changes
    uint32_t    task_toggles_todo;  // current number of pin state changes we must to do

    uint32_t    setup_ticks;        // number of CPU ticks to prepare pin toggle
    uint32_t    hold_ticks;         // number of CPU ticks to hold pin state

    uint64_t    todo_tick;          // timestamp (in CPU ticks) to change pin state
};




/// messages types
enum
{
    PULSGEN_MSG_PIN_SETUP = 0x20,
    PULSGEN_MSG_TASK_SETUP,
    PULSGEN_MSG_TASK_ABORT,
    PULSGEN_MSG_TASK_STATE,
    PULSGEN_MSG_TASK_TOGGLES
};

/// the message data sizes
#define PULSGEN_MSG_BUF_LEN MSG_LEN

/// the message data access
struct pulsgen_msg_pin_setup_t { uint32_t ch; uint32_t port; uint32_t pin; uint32_t inverted; };
struct pulsgen_msg_task_setup_t { uint32_t ch; uint32_t toggles;
    uint32_t pin_setup_time; uint32_t pin_hold_time; uint32_t start_delay; };
struct pulsgen_msg_ch_t { uint32_t ch; };
struct pulsgen_msg_state_t { uint32_t state; };
struct pulsgen_msg_toggles_t { uint32_t toggles; };




// export public methods

void pulsgen_module_init();
void pulsgen_module_base_thread();

void pulsgen_pin_setup(uint8_t c, uint8_t port, uint8_t pin, uint8_t inverted);

void pulsgen_task_setup(uint32_t c, uint32_t toggles, uint32_t pin_setup_time,
    uint32_t pin_hold_time, uint32_t start_delay);

void pulsgen_task_abort(uint8_t c);

uint8_t pulsgen_task_state(uint8_t c);
uint32_t pulsgen_task_toggles(uint8_t c);
int8_t volatile pulsgen_msg_recv(uint8_t type, uint8_t * msg, uint8_t length);




#endif
