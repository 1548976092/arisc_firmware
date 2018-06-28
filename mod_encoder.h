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




/// a channel parameters
struct encoder_ch_t
{
    uint8_t     enabled;
    uint8_t     inverted; // non zero = invert counter direction
    uint8_t     using_B;
    uint8_t     using_Z;
    uint8_t     edge; // pulse detection type, 0 = rising edge

    int32_t     counts;

    uint8_t     phase_A_port;
    uint8_t     phase_A_pin;
    uint8_t     phase_A_pin_state;

    uint8_t     phase_B_port;
    uint8_t     phase_B_pin;
    uint8_t     phase_B_pin_state;

    uint8_t     phase_Z_port;
    uint8_t     phase_Z_pin;
    uint8_t     phase_Z_pin_state;
};




enum { PHASE_A, PHASE_B, PHASE_Z };

/// messages types
enum
{
    ENCODER_MSG_PINS_SETUP = 0x30,
    ENCODER_MSG_SETUP,
    ENCODER_MSG_COUNTS,
    ENCODER_MSG_ENABLE,
    ENCODER_MSG_RESET
};

#define ENCODER_MSG_BUF_LEN             MSG_LEN
#define ENCODER_MSG_PINS_SETUP_LEN      (6*4*ENCODER_CH_CNT)
#define ENCODER_MSG_SETUP_LEN           (4*4)
#define ENCODER_MSG_COUNTS_LEN          (4*ENCODER_CH_CNT)
#define ENCODER_MSG_ENABLE_LEN          (4)
#define ENCODER_MSG_RESET_LEN           (4)

/// the message data access
#define ENCODER_MSG_BUF_PORT(LINK,CH,PHASE)     (*((uint32_t*)(LINK) + 6*CH + PHASE))
#define ENCODER_MSG_BUF_PIN(LINK,CH,PHASE)      (*((uint32_t*)(LINK) + 6*CH + PHASE + 3))

#define ENCODER_MSG_BUF_INVERTED(LINK)          (*((uint32_t*)(LINK)))
#define ENCODER_MSG_BUF_USING_B(LINK)           (*((uint32_t*)(LINK) + 1))
#define ENCODER_MSG_BUF_USING_Z(LINK)           (*((uint32_t*)(LINK) + 2))
#define ENCODER_MSG_BUF_EDGE(LINK)              (*((uint32_t*)(LINK) + 3))

#define ENCODER_MSG_BUF_COUNTS(LINK,CH)         (*((uint32_t*)(LINK) + CH))

#define ENCODER_MSG_BUF_ENABLE(LINK)            (*((uint32_t*)(LINK)))

#define ENCODER_MSG_BUF_RESET(LINK)             (*((uint32_t*)(LINK)))



// export public methods

void encoder_module_base_thread();

void encoder_pin_setup(uint8_t c, uint8_t phase, uint8_t port, uint8_t pin);

void encoder_setup(uint8_t c, uint8_t inverted, uint8_t using_B, uint8_t using_Z, uint8_t edge);
void encoder_state_set(uint8_t c, uint8_t state);
void encoder_counts_reset(uint8_t c);

uint8_t encoder_state_get(uint8_t c);
int32_t encoder_counts_get(uint8_t c);
int8_t volatile encoder_msg_recv(uint8_t type, uint8_t * msg, uint8_t length);




#endif
