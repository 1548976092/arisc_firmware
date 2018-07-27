/**
 * @file    mod_encoder.h
 *
 * @brief   quadrature encoder counter module
 *
 * This module implements an API
 * to make real-time counting of quadrature encoder pulses
 */

#ifndef _MOD_ENCODER_H
#define _MOD_ENCODER_H

#include <stdint.h>
#include "mod_msg.h"




#define ENCODER_CH_CNT 8  ///< maximum number of encoder counter channels
#define ENCODER_PH_CNT 3  ///< number of encoder phases




/// a channel parameters
struct encoder_ch_t
{
    uint8_t     enabled;
    uint8_t     using_B;
    uint8_t     using_Z;

    uint8_t     port[ENCODER_PH_CNT];
    uint8_t     pin[ENCODER_PH_CNT];
    uint8_t     state[ENCODER_PH_CNT];

    int32_t     counts;
    uint8_t     AB_state;
};




enum { PHASE_A, PHASE_B, PHASE_Z };
enum { PH_A, PH_B, PH_Z };

/// messages types
enum
{
    ENCODER_MSG_PIN_SETUP = 0x30,
    ENCODER_MSG_SETUP,
    ENCODER_MSG_STATE_SET,
    ENCODER_MSG_STATE_GET,
    ENCODER_MSG_COUNTS_SET,
    ENCODER_MSG_COUNTS_GET,

    ENCODER_MSG_MODULE_STATE_SET,
    ENCODER_MSG_MODULE_STATE_GET
};

/// the message data sizes
#define ENCODER_MSG_BUF_LEN MSG_LEN

/// the message data access
struct encoder_msg_ch_t { uint32_t ch; };
struct encoder_msg_pin_setup_t { uint32_t ch; uint32_t phase; uint32_t port; uint32_t pin; };
struct encoder_msg_setup_t { uint32_t ch; uint32_t using_B; uint32_t using_Z; };
struct encoder_msg_state_set_t { uint32_t ch; uint32_t state; };
struct encoder_msg_counts_set_t { uint32_t ch; int32_t counts; };
struct encoder_msg_state_get_t { uint32_t state; };
struct encoder_msg_counts_get_t { int32_t counts; };
struct encoder_msg_state_t { uint32_t state; };





// export public methods

void encoder_module_init();
void encoder_module_deinit();
void encoder_module_base_thread();

void encoder_pin_setup(uint8_t c, uint8_t phase, uint8_t port, uint8_t pin);

void encoder_setup(uint8_t c, uint8_t using_B, uint8_t using_Z);
void encoder_state_set(uint8_t c, uint8_t state);
void encoder_counts_set(uint8_t c, int32_t counts);

uint8_t encoder_state_get(uint8_t c);
int32_t encoder_counts_get(uint8_t c);
int8_t volatile encoder_msg_recv(uint8_t type, uint8_t * msg, uint8_t length);




#endif
