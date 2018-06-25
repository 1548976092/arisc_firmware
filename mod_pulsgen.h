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
#define PULSGEN_MAX_DUTY    100 ///< maximum percent of pulse duty cycle (255 is max)
#define PULSGEN_MAX_PERIOD  (UINT32_MAX/(TIMER_FREQUENCY/1000000))




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

    uint32_t    setup_ticks;        // number of CPU ticks to prepare pin toggle
    uint32_t    hold_ticks;         // number of CPU ticks to hold pin state

    uint32_t    todo_tick;          // timestamp (in CPU ticks) to change pin state
    uint8_t     todo_tick_ovrfl;    // timestamp overflow flag
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

#define PULSGEN_MSG_BUF_LEN             MSG_LEN
#define PULSGEN_MSG_CH_CNT              12
#define PULSGEN_MSG_PIN_SETUP_LEN       (4*4*PULSGEN_MSG_CH_CNT)
#define PULSGEN_MSG_TASK_SETUP_LEN      (5*4*PULSGEN_MSG_CH_CNT)
#define PULSGEN_MSG_TASK_ABORT_LEN      (4)
#define PULSGEN_MSG_TASK_STATE_LEN      (4)
#define PULSGEN_MSG_TASK_TOGGLES_LEN    (2*4*PULSGEN_MSG_CH_CNT)

/// the message data access
#define PULSGEN_MSG_BUF_CHANNEL_ID(LINK,SLOT)       (*((uint32_t*)(LINK) + SLOT))
#define PULSGEN_MSG_BUF_PORT(LINK,SLOT)             (*((uint32_t*)(LINK) + SLOT + 1*PULSGEN_MSG_CH_CNT))
#define PULSGEN_MSG_BUF_PIN(LINK,SLOT)              (*((uint32_t*)(LINK) + SLOT + 2*PULSGEN_MSG_CH_CNT))
#define PULSGEN_MSG_BUF_INVERTED(LINK,SLOT)         (*((uint32_t*)(LINK) + SLOT + 3*PULSGEN_MSG_CH_CNT))

#define PULSGEN_MSG_BUF_PERIOD(LINK,SLOT)           (*((uint32_t*)(LINK) + SLOT + 1*PULSGEN_MSG_CH_CNT))
#define PULSGEN_MSG_BUF_DELAY(LINK,SLOT)            (*((uint32_t*)(LINK) + SLOT + 2*PULSGEN_MSG_CH_CNT))
#define PULSGEN_MSG_BUF_TOGGLES(LINK,SLOT)          (*((uint32_t*)(LINK) + SLOT + 3*PULSGEN_MSG_CH_CNT))
#define PULSGEN_MSG_BUF_DUTY(LINK,SLOT)             (*((uint32_t*)(LINK) + SLOT + 4*PULSGEN_MSG_CH_CNT))

#define PULSGEN_MSG_BUF_CHANNELS_MASK(LINK)         (*((uint32_t*)(LINK)))

#define PULSGEN_MSG_BUF_TOGGLES_MADE(LINK,SLOT)     (*((uint32_t*)(LINK) + SLOT + 1*PULSGEN_MSG_CH_CNT))




// export public methods

void pulsgen_module_init();
void pulsgen_module_base_thread();

void pulsgen_pin_setup(uint8_t c, uint8_t port, uint8_t pin, uint8_t inverted);

void pulsgen_task_setup(uint8_t c, uint32_t period, uint32_t toggles, uint8_t duty, uint32_t delay);
void pulsgen_task_abort(uint8_t c);

uint8_t pulsgen_task_state(uint8_t c);
uint32_t pulsgen_task_toggles(uint8_t c);
int8_t volatile pulsgen_msg_recv(uint8_t type, uint8_t * msg, uint8_t length);




#endif
