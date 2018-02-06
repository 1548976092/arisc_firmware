#ifndef _STEPGEN_H
#define _STEPGEN_H




#include <or1k-sprs.h>
#include "timer.h"
#include "gpio.h"




#define CH_CNT 8




struct ch_t
{
    // STEP pin data
    uint8_t step_bank;
    uint8_t step_pin;
    uint8_t step_state;
    uint32_t *step_port;

    // channel data
    uint8_t enbl; // channel on/off = 0/1
    uint32_t freq; // frequency, Hz
    uint32_t interval; // interval, timer ticks
    uint32_t steps_todo; // number of steps to make
    uint32_t steps_task; // total number of steps to make
    uint32_t todo_tick; // timer tick to make pulses
};

volatile struct ch_t ch[CH_CNT] = {{0}};




void inline
ch_set_step_pin(uint8_t id, uint8_t bank, uint8_t pin)
{
    ch[id].step_bank = bank;
    ch[id].step_pin = pin;
    ch[id].step_port = (GPIO_BANK_L ? R_PIO_BASE : PIO_BASE + bank * BANK_SIZE)
                       + 4 * 4;

    // TODO - make it faster
    gpio_set_pincfg(bank, pin, GPIO_FUNC_OUTPUT);
    set_bit(pin, gpio_get_data_addr(bank));
}

uint8_t inline
ch_get_step_state(uint8_t id)
{
    return ch[id].step_state;
}




void inline
ch_set_task(uint8_t id, uint32_t freq, uint32_t steps)
{
    ch[id].freq = freq;
    ch[id].interval = TIMER_FREQUENCY / freq / 2;
    ch[id].steps_task = steps;
    ch[id].steps_todo = steps;
    ch[id].step_state = 0;
}




void inline
ch_enable(uint8_t id)
{
    ch[id].enbl = 1;
    ch[id].todo_tick = ch[id].interval + or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);
}

uint8_t inline
ch_get_state(uint8_t id)
{
    return ch[id].enbl;
}

void inline
ch_disable(uint8_t id)
{
    ch[id].enbl = 0;
}




uint32_t inline
get_tick()
{
    return or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);
}




#endif
