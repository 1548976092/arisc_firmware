#ifndef _STEPGEN_H
#define _STEPGEN_H




#include <or1k-support.h>
#include <or1k-sprs.h>
#include "string.h"
#include "timer.h"
#include "gpio.h"




#define CH_CNT          8
#define INTERVAL_LOSS   99 / 100




struct ch_t
{
    // STEP pin data
    uint8_t step_bank;
    uint8_t step_pin;
    uint8_t step_state;
    uint32_t step_port_addr;

    // channel data
    uint8_t enbl; // channel on/off = 0/1
    uint32_t freq; // frequency, Hz
    uint32_t interval; // interval, timer ticks
    uint32_t steps_todo; // number of steps to make
    uint32_t steps_task; // total number of steps to make
    uint32_t todo_tick; // timer tick to make pulses
    uint8_t todo_tick_ovrfl;
};

volatile struct ch_t ch[CH_CNT] = {{0}};




static void inline
ch_set_step_pin(uint8_t id, uint8_t bank, uint8_t pin)
{
    ch[id].step_bank = bank;
    ch[id].step_pin = pin;
    ch[id].step_port_addr = 16 + (bank == GPIO_BANK_L ? R_PIO_BASE :
        PIO_BASE + bank * BANK_SIZE);

    gpio_set_pincfg(bank, pin, GPIO_FUNC_OUTPUT);
    clr_bit(pin, ch[id].step_port_addr);
}

static uint8_t inline
ch_get_step_state(uint8_t id)
{
    return ch[id].step_state;
}




static void inline
ch_set_task(uint8_t id, uint32_t freq, uint32_t steps)
{
    ch[id].freq = freq;
    ch[id].interval = TIMER_FREQUENCY / freq / 2;
    ch[id].steps_task = steps;
    ch[id].steps_todo = steps;
    ch[id].step_state = 0;
}




static void inline
ch_enable(uint8_t id)
{
    uint32_t tick;

    tick = or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);
    ch[id].todo_tick = ch[id].interval + tick;
    if ( ch[id].todo_tick < tick ) ch[id].todo_tick_ovrfl = 1;

    ch[id].enbl = 1;
}

static uint8_t inline
ch_get_state(uint8_t id)
{
    return ch[id].enbl;
}

static void inline
ch_disable(uint8_t id)
{
    ch[id].enbl = 0;
}




static uint32_t inline
get_tick()
{
    return or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);
}




static uint8_t inline
stepgen_loop()
{
    uint8_t c = CH_CNT;
    uint32_t tick = 0, todo_tick = 0;
    uint8_t work_state = 0;

    tick = get_tick();

    for( c = CH_CNT, work_state = 0; c--; )
    {
        if ( ch[c].enbl )
        {
            if ( !ch[c].steps_todo )
            {
                ch[c].enbl = 0;
                continue;
            }

            work_state = 1;

            if
            (
                // TODO - check this condition properly!
                ( !ch[c].todo_tick_ovrfl && tick >= ch[c].todo_tick ) ||
                ( ch[c].todo_tick_ovrfl && (UINT32_MAX-tick) >= ch[c].todo_tick)
            )
            {
                if ( ch[c].step_state )
                {
                    clr_bit(ch[c].step_pin, ch[c].step_port_addr);
                    ch[c].step_state = 0;
                    --ch[c].steps_todo;
                }
                else
                {
                    set_bit(ch[c].step_pin, ch[c].step_port_addr);
                    ch[c].step_state = 1;
                }

                todo_tick = ch[c].todo_tick;
                ch[c].todo_tick += ch[c].interval;
                ch[c].todo_tick_ovrfl = ch[c].todo_tick < todo_tick ? 1 : 0;
            }
        }
    }

    return work_state;
}



#endif
