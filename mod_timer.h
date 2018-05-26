/**
 * @file    mod_timer.h
 *
 * @brief   system timer control module header
 *
 * @note    timer frequency (TIMER_FREQUENCY) is same as CPU frequency
 *
 * This module implements an API to the system timer
 */

#ifndef _MOD_TIMER_H
#define _MOD_TIMER_H

#include <or1k-sprs.h>
#include <or1k-support.h>
#include <stdint.h>
#include "sys.h"




// public macros

/// the system timer frequency in Hz (same as CPU frequency)
#define TIMER_FREQUENCY CPU_FREQ




// public methods as macros

/// the fast version of timer_start()
#define TIMER_START() \
    or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, OR1K_SPR_TICK_TTMR_MODE_SET(0, OR1K_SPR_TICK_TTMR_MODE_CONTINUE))

/// the fast version of timer_stop()
#define TIMER_STOP() \
    or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, 0)

/// the fast version of timer_cnt_set()
#define TIMER_CNT_SET(CNT) \
    or1k_mtspr(OR1K_SPR_TICK_TTCR_ADDR, CNT)

/// the fast version of timer_cnt_get()
#define TIMER_CNT_GET() \
    or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR)




// export public methods

void timer_start();
void timer_stop();
void timer_cnt_set(uint32_t cnt);
uint32_t timer_cnt_get();




#endif
