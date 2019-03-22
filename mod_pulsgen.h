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




#define PULSGEN_CH_CNT      32  ///< maximum number of pulse generator channels
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
    uint32_t    task_toggles;       // pin toggles for this task
    uint32_t    task_toggles_todo;  // pin toggles left to do for this task

    uint8_t     toggles_dir;        // 0 = toggles_done++, !0 = toggles_done--
    uint32_t    toggles_done;       // total number of pin toggles
    uint32_t    tasks_done;         // total number of tasks done

    uint32_t    setup_ticks;        // number of CPU ticks to prepare pin toggle
    uint32_t    hold_ticks;         // number of CPU ticks to hold pin state

    uint8_t     abort_on_setup;
    uint8_t     abort_on_hold;

    uint64_t    todo_tick;          // timestamp (in CPU ticks) to change pin state
};

struct pulsgen_fifo_item_t
{
    uint8_t used;
    uint8_t toggles_dir;
    uint32_t toggles;
    uint32_t pin_setup_time;
    uint32_t pin_hold_time;
    uint32_t start_delay;
};



/// messages types
enum
{
    PULSGEN_MSG_PIN_SETUP = 0x20,
    PULSGEN_MSG_TASK_ADD,
    PULSGEN_MSG_ABORT,
    PULSGEN_MSG_STATE_GET,
    PULSGEN_MSG_TASK_TOGGLES_GET,
    PULSGEN_MSG_TOGGLES_DONE_GET,
    PULSGEN_MSG_TOGGLES_DONE_SET,
    PULSGEN_MSG_TASKS_DONE_GET,
    PULSGEN_MSG_TASKS_DONE_SET,
    PULSGEN_MSG_WATCHDOG_SETUP,
    PULSGEN_MSG_CNT
};

/// the message data sizes
#define PULSGEN_MSG_BUF_LEN MSG_LEN




// export public methods

void pulsgen_module_init();
void pulsgen_module_base_thread();
void pulsgen_pin_setup(uint8_t c, uint8_t port, uint8_t pin, uint8_t inverted);
void pulsgen_task_add(uint32_t c, uint32_t toggles_dir, uint32_t toggles, uint32_t pin_setup_time, uint32_t pin_hold_time, uint32_t start_delay);
void pulsgen_abort(uint8_t c, uint8_t on_hold);
uint8_t pulsgen_state_get(uint8_t c);
uint32_t pulsgen_task_toggles_get(uint8_t c);
uint32_t pulsgen_toggles_done_get(uint8_t c);
void pulsgen_toggles_done_set(uint8_t c, uint32_t toggles);
uint32_t pulsgen_tasks_done_get(uint8_t c);
void pulsgen_tasks_done_set(uint8_t c, uint32_t tasks);
int8_t volatile pulsgen_msg_recv(uint8_t type, uint8_t * msg, uint8_t length);
void pulsgen_watchdog_setup(uint8_t enable, uint32_t time);




#endif
