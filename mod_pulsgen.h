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
#define PULSGEN_MSG_TASK_SETUP_CH_CNT   16

#pragma pack(push, 1)
struct pulsgen_msg_pin_setup_t
{
    uint32_t    channels_mask; // "bit 0" means "don't touch this channel"
    uint8_t     port[PULSGEN_CH_CNT];
    uint8_t     pin[PULSGEN_CH_CNT];
    uint32_t    inverted_mask; // "bit 1" means "inverted"
};

struct pulsgen_msg_task_setup_t
{
    uint16_t    channels_mask1; // channels bits from  0 to 15, "0" means "use channels_mask2"
    uint16_t    channels_mask2; // channels bits from 16 to 31
    uint32_t    period[PULSGEN_MSG_TASK_SETUP_CH_CNT];
    uint32_t    delay[PULSGEN_MSG_TASK_SETUP_CH_CNT];
    uint32_t    toggles[PULSGEN_MSG_TASK_SETUP_CH_CNT];
    uint8_t     duty[PULSGEN_MSG_TASK_SETUP_CH_CNT];
};

struct pulsgen_msg_task_abort_t
{
    uint32_t    channels_mask; // "bit 0" means "don't touch this channel"
};

struct pulsgen_msg_task_state_t
{
    uint32_t    channels_mask; // "bit 0" means "don't touch this channel"
};

struct pulsgen_msg_task_toggles_t
{
    uint32_t    channels_mask; // "bit 0" means "don't touch this channel"
    uint32_t    toggles[PULSGEN_CH_CNT];
};
#pragma pack(pop)




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
