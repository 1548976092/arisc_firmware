#include <or1k-support.h>
#include <or1k-sprs.h>

#include "timer.h"
#include "gpio.h"
#include "shmem.h"
#include "stepgen.h"




extern volatile struct global_shmem_t *shm;




#define sg                  shm->arisc.stepgen

#define set_pin(PORT,PIN)   (shm->arisc.gpio_set_ctrl[PORT] |= (1 << PIN))
#define clr_pin(PORT,PIN)   (shm->arisc.gpio_clr_ctrl[PORT] |= (1 << PIN))




void stepgen_ch_set_step_pin(uint8_t id, uint8_t port, uint8_t pin)
{
    sg.step_port[id] = port;
    sg.step_pin[id] = pin;

    gpio_set_pincfg(port, pin, GPIO_FUNC_OUTPUT);
    clr_pin(port, pin);
}




void stepgen_ch_set_task(uint8_t id, uint32_t freq, uint32_t steps)
{
    sg.task_freq[id] = freq;
    sg.interval[id] = TIMER_FREQUENCY / freq / 2;
    sg.task_steps[id] = steps;
    sg.task_steps_todo[id] = steps;
    sg.step_state[id] = 0;
}




void stepgen_ch_enable(uint8_t id)
{
    uint32_t tick;

    tick = or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);
    sg.todo_tick[id] = sg.interval[id] + tick;
    if ( sg.todo_tick[id] < tick ) sg.todo_tick_ovrfl[id] = 1;

    sg.ch_enable[id] = 1;
}

void stepgen_ch_disable(uint8_t id)
{
    sg.ch_enable[id] = 0;
}

uint8_t stepgen_ch_get_state(uint8_t id)
{
    return sg.ch_enable[id];
}





void stepgen_base_thread()
{
    uint8_t c = STEPGEN_CH_CNT;
    uint32_t tick = 0, todo_tick = 0;

    tick = or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);

    for( c = STEPGEN_CH_CNT; c--; )
    {
        if ( sg.ch_enable[c] )
        {
            if ( !sg.task_steps_todo[c] )
            {
                sg.ch_enable[c] = 0;
                continue;
            }

            if
            (
                ( !sg.todo_tick_ovrfl[c] && tick >= sg.todo_tick[c] ) ||
                ( sg.todo_tick_ovrfl[c] && (UINT32_MAX-tick) >= sg.todo_tick[c])
            )
            {
                if ( sg.step_state[c] )
                {
                    clr_pin(sg.step_port[c], sg.step_pin[c]);
                    sg.step_state[c] = 0;
                    --sg.task_steps_todo[c];
                }
                else
                {
                    set_pin(sg.step_port[c], sg.step_pin[c]);
                    sg.step_state[c] = 1;
                }

                todo_tick = sg.todo_tick[c];
                sg.todo_tick[c] += sg.interval[c];
                sg.todo_tick_ovrfl[c] = sg.todo_tick[c] < todo_tick ? 1 : 0;
            }
        }
    }
}




#undef sg

#undef set_pin
#undef clr_pin
