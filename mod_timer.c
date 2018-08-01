/**
 * @file    mod_timer.c
 *
 * @brief   system timer control module
 *
 * @note    timer frequency (TIMER_FREQUENCY) is same as CPU frequency
 *
 * This module implements an API to the system timer
 */

#include <or1k-sprs.h>
#include <or1k-support.h>
#include "mod_timer.h"




// private vars

static uint32_t cnt_curr = 0;
static uint32_t cnt_prev = 0;
static uint32_t cnt_ovfl = 0;




// public methods

/**
 * @brief   start system timer in continues mode
 * @note    timer counter value is not affected by this function
 * @retval  none
 */
void timer_start()
{
    // set system timer mode to CONTINUES
    or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, OR1K_SPR_TICK_TTMR_MODE_SET(0, OR1K_SPR_TICK_TTMR_MODE_CONTINUE));
}

/**
 * @brief   stop system timer
 * @note    timer counter value is not affected by this function
 * @retval  none
 */
void timer_stop()
{
    // disable system timer
    or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, 0);
}

/**
 * @brief   set system timer counter value
 * @param   cnt     new value (in ticks) for the system timer counter
 * @retval  none
 */
void timer_cnt_set(uint32_t cnt)
{
    // set system timer ticks counter value
    or1k_mtspr(OR1K_SPR_TICK_TTCR_ADDR, cnt);
}

/**
 * @brief   get system timer counter value
 * @retval  0..0xFFFFFFFF (ticks)
 */
uint32_t timer_cnt_get()
{
    // get system timer ticks counter value
    return or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);
}

/**
 * @brief   get system timer counter total value
 * @retval  0 .. 2^64-1 (ticks)
 */
uint64_t timer_cnt_get_64()
{
    // get system timer ticks counter value
    __asm__ __volatile__ ("l.mfspr\t\t%0,%1,0" : "=r" (cnt_curr) : "r" (OR1K_SPR_TICK_TTCR_ADDR));

    if ( cnt_curr < cnt_prev ) ++cnt_ovfl;

    cnt_prev = cnt_curr;

    return ((uint64_t)cnt_ovfl) * ((uint64_t)4294967296UL) + ((uint64_t)cnt_curr);
}




/**
    @example mod_timer.c

    <b>Usage example 1</b>: get code execution time in CPU ticks

    @code
        #include <stdint.h>
        #include "mod_timer.h"

        int main(void)
        {
            uint32_t i, n, ticks;

            timer_start(); // start timer
            timer_cnt_set(0); // reset timer counter

            // your code
            for ( i = 0, n = 0; i < 123456; i++ ) n = i*i;

            ticks = timer_cnt_get(); // get execution time of your code (in CPU ticks)
            timer_stop(); // stop timer

            return 0;
        }
    @endcode

    <b>Usage example 2</b>: using fast macros to get code execution time in CPU ticks

    @code
        #include <or1k-sprs.h>
        #include <or1k-support.h>
        #include <stdint.h>
        #include "mod_timer.h"

        int main(void)
        {
            uint32_t i, n, ticks;

            TIMER_START(); // start timer
            TIMER_CNT_SET(0); // reset timer counter

            // your code
            for ( i = 0, n = 0; i < 123456; i++ ) n = i*i;

            ticks = TIMER_CNT_SET(); // get execution time of your code (in CPU ticks)
            TIMER_STOP(); // stop timer

            return 0;
        }
    @endcode
*/
