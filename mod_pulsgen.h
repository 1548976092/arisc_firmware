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
#define PULSGEN_FIFO_SIZE   4   ///< size of channel's fifo buffer




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

    uint8_t     abort_on_setup;
    uint8_t     abort_on_hold;

    uint64_t    todo_tick;          // timestamp (in CPU ticks) to change pin state
};

struct pulsgen_fifo_item_t
{
    uint8_t used;
    uint32_t toggles;
    uint32_t pin_setup_time;
    uint32_t pin_hold_time;
    uint32_t start_delay;
};



/// messages types
enum
{
    PULSGEN_MSG_PIN_SETUP = 0x20,
    PULSGEN_MSG_TASK_SETUP,
    PULSGEN_MSG_TASK_ABORT,
    PULSGEN_MSG_TASK_STATE,
    PULSGEN_MSG_TASK_TOGGLES,
    PULSGEN_MSG_WATCHDOG_SETUP
};

/// the message data sizes
#define PULSGEN_MSG_BUF_LEN MSG_LEN

/// the message data access
struct pulsgen_msg_pin_setup_t { uint32_t ch; uint32_t port; uint32_t pin; uint32_t inverted; };
struct pulsgen_msg_task_setup_t { uint32_t ch; uint32_t toggles;
    uint32_t pin_setup_time; uint32_t pin_hold_time; uint32_t start_delay; };
struct pulsgen_msg_ch_t { uint32_t ch; };
struct pulsgen_msg_abort_t { uint32_t ch; uint32_t when; };
struct pulsgen_msg_state_t { uint32_t state; };
struct pulsgen_msg_toggles_t { uint32_t toggles; };
struct pulsgen_msg_watchdog_setup_t { uint32_t enable; uint32_t time; };




// export public methods

void pulsgen_module_init();
void pulsgen_module_base_thread();

void pulsgen_pin_setup(uint8_t c, uint8_t port, uint8_t pin, uint8_t inverted);

void pulsgen_task_setup(uint32_t c, uint32_t toggles, uint32_t pin_setup_time,
    uint32_t pin_hold_time, uint32_t start_delay);

void pulsgen_task_abort(uint8_t c, uint8_t when);

uint8_t pulsgen_task_state(uint8_t c);
uint32_t pulsgen_task_toggles(uint8_t c);
int8_t volatile pulsgen_msg_recv(uint8_t type, uint8_t * msg, uint8_t length);

void pulsgen_watchdog_setup(uint8_t enable, uint32_t time);




#endif
