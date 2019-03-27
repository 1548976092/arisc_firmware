/**
 * @file    mod_stepgen.h
 * @brief   steps generator module header
 * This module implements an API to make real-time step/dir pulses via GPIO
 */

#ifndef _MOD_STEPGEN_H
#define _MOD_STEPGEN_H

#include <stdint.h>
#include "mod_msg.h"
#include "mod_timer.h"




#define STEPGEN_CH_CNT          24  ///< maximum number of pulse generator channels
#define STEPGEN_FIFO_SIZE       4   ///< size of channel's fifo buffer
#define STEPGEN_MSG_BUF_LEN     MSG_LEN

enum
{
    STEPGEN_MSG_PIN_SETUP = 0x20,
    STEPGEN_MSG_TASK_ADD,
    STEPGEN_MSG_TASK_UPDATE,
    STEPGEN_MSG_ABORT,
    STEPGEN_MSG_POS_GET,
    STEPGEN_MSG_POS_SET,
    STEPGEN_MSG_CNT
};




typedef struct
{
    uint8_t     type; // 0:step, 1:dir
    uint32_t    pulses; // 0:empty slot, !0:used
    uint32_t    low_ticks;
    uint32_t    high_ticks;
    uint64_t    tick;

} stepgen_fifo_slot_t;

typedef struct
{
    uint8_t     pin_state[2];
    uint8_t     pin_port[2];
    uint32_t    pin_mask[2];
    uint32_t    pin_mask_not[2];
    uint32_t    pin_invert[2];

    int32_t     pos; // in pulses

    uint8_t     abort;
    uint64_t    abort_tick;

    uint8_t                 task_infinite;
    uint8_t                 task_slot;
    uint64_t                task_tick;
    stepgen_fifo_slot_t     tasks[STEPGEN_FIFO_SIZE];

} stepgen_ch_t;




void stepgen_module_init();
void stepgen_module_base_thread();
void stepgen_pin_setup(uint8_t c, uint8_t type, uint8_t port, uint8_t pin, uint8_t invert);
void stepgen_task_add(uint8_t c, uint8_t type, uint32_t pulses, uint32_t pin_low_time, uint32_t pin_high_time);
void stepgen_abort(uint8_t c, uint8_t all);
int32_t stepgen_pos_get(uint8_t c);
void stepgen_pos_set(uint8_t c, int32_t pos);
int8_t volatile stepgen_msg_recv(uint8_t type, uint8_t * msg, uint8_t length);




#endif
