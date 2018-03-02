#include <or1k-support.h>
#include <or1k-sprs.h>

#include "timer.h"
#include "gpio.h"
#include "shmem.h"
#include "stepgen.h"




extern volatile struct global_shmem_t *shm;




#define asg                 shm->arisc.stepgen
#define lsg                 shm->lcnc.stepgen

#define set_pin(PORT,PIN)   (shm->arisc.gpio_set_ctrl[PORT] |= (1 << PIN))
#define clr_pin(PORT,PIN)   (shm->arisc.gpio_clr_ctrl[PORT] |= (1 << PIN))




// prepare channel
void stepgen_ch_setup(uint8_t c)
{
    // disable any tasks
    asg.task[c] = 0;
    asg.task_steps_todo[c] = 0;
    asg.dir_setup[c] = 0;

    // setup GPIO
    gpio_set_pincfg(lsg.step_port[c], lsg.step_pin[c], GPIO_FUNC_OUTPUT);
    gpio_set_pincfg(lsg.dir_port[c],  lsg.dir_pin[c],  GPIO_FUNC_OUTPUT);

    // set GPIO pin states
    asg.step_state[c] = 0;
    asg.dir_state[c]  = 0;

    // set step pin
    if ( asg.step_state[c] ^ lsg.step_inv[c] )
    {
        clr_pin(lsg.step_port[c], lsg.step_pin[c]);
    }
    else
    {
        set_pin(lsg.step_port[c], lsg.step_pin[c]);
    }

    // set DIR pin
    if ( asg.dir_state[c] ^ lsg.dir_inv[c] )
    {
        clr_pin(lsg.dir_port[c], lsg.dir_pin[c]);
    }
    else
    {
        set_pin(lsg.dir_port[c], lsg.dir_pin[c]);
    }

    // setup intervals
    asg.dirsetup_ticks[c]  = TIMER_FREQUENCY/(1000000000/lsg.dirsetup[c]);
    asg.dirhold_ticks[c]   = TIMER_FREQUENCY/(1000000000/lsg.dirhold[c]);

    // clear channel setup flag
    shm->stepgen_ch_setup[c] = 0;
}




// add new task for the channel
void stepgen_ch_new_task(uint8_t c)
{
    static uint32_t tick = 0;

    // do nothing if channel is disabled
    if ( !lsg.ch_enable[c] ) return;

    // get current arisc tick
    tick = or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);

    // set steps count
    asg.task_steps[c] = lsg.task_steps[c];
    asg.task_steps_todo[c] = lsg.task_steps[c];

    // if new DIR value is same as current
    if ( asg.dir_state[c] == lsg.task_dir[c] )
    {
        // calculate step length ticks
        asg.step_ticks[c] = TIMER_FREQUENCY
            / (1000000000/lsg.task_time[c])
            / asg.task_steps_todo[c]
            / 2;

        // set tick for the next step toggle
        asg.todo_tick[c] = tick;
        asg.todo_tick_ovrfl[c] = 0;
    }
    // if new DIR state != current DIR state
    else
    {
        // save new DIR value
        asg.task_dir[c] = lsg.task_dir[c];

        // calculate step length ticks
        asg.step_ticks[c] = TIMER_FREQUENCY
            / (1000000000 / (lsg.task_time[c] - lsg.dirsetup[c] - lsg.dirhold[c]))
            / asg.task_steps_todo[c]
            / 2;

        // set tick for the next DIR change
        asg.dir_setup[c] = 2; // 2 = dir setup, 1 = dir hold
        asg.todo_tick[c] = tick + asg.dirsetup_ticks[c];
        asg.todo_tick_ovrfl[c] = asg.todo_tick[c] < tick ? 1 : 0;
    }

    // clear channel's new task flag
    shm->stepgen_ch_task_new[c] = 0;
}




// base stepgen cycle
void stepgen_base_thread()
{
    static uint8_t c = STEPGEN_CH_CNT;
    static uint32_t tick = 0, todo_tick = 0;


    // get current arisc cpu tick
    tick = or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);


    // check all channels
    for( c = STEPGEN_CH_CNT; c--; )
    {
        // if channel setup is needed
        if ( shm->stepgen_ch_setup[c] ) stepgen_ch_setup(c);

        // run this code only when step is done
        if ( !asg.step_state[c] )
        {
            // if we must disable the channel
            if ( !lsg.ch_enable[c] && asg.task[c] ) asg.task[c] = 0;
            // if we have new task for the channel
            else if ( shm->stepgen_ch_task_new[c] ) stepgen_ch_new_task(c);
        }

        // if channel have no tasks - goto the next channel
        if ( !asg.task[c] ) continue;

        // if we are working with DIR
        if ( asg.dir_setup[c] )
        {
            // if it's time to make the DIR change
            if
            (
                ( !asg.todo_tick_ovrfl[c] && tick >= asg.todo_tick[c] ) ||
                ( asg.todo_tick_ovrfl[c] && (UINT32_MAX-tick) >= asg.todo_tick[c])
            )
            {
                // if it was DIR setup period
                if ( asg.dir_setup[c] > 1 )
                {
                    // change DIR state
                    asg.dir_state[c] = asg.task_dir[c];

                    // toggle DIR pin
                    if ( asg.dir_state[c] ^ lsg.dir_inv[c] )
                    {
                        clr_pin(lsg.dir_port[c], lsg.dir_pin[c]);
                    }
                    else
                    {
                        set_pin(lsg.dir_port[c], lsg.dir_pin[c]);
                    }

                    // set tick for the DIR hold period
                    todo_tick = asg.todo_tick[c];
                    asg.todo_tick[c] += asg.dirhold_ticks[c];
                    asg.todo_tick_ovrfl[c] = asg.todo_tick[c] < todo_tick ? 1 : 0;
                }
                // if it was DIR hold period and we have some steps to do
                else if ( asg.task_steps_todo[c] )
                {
                    // set tick for the next step toggle
                    asg.todo_tick[c] = tick;
                    asg.todo_tick_ovrfl[c] = 0;
                }

                // change DIR setup state
                --asg.dir_setup[c];
            }
        }
        // if we are working with steps
        else
        {
            // if we have no steps to do
            if ( !asg.task_steps_todo[c] )
            {
                // disable "we have task" flag
                asg.task[c] = 0;
                continue;
            }

            // if we must make the step change
            if
            (
                ( !asg.todo_tick_ovrfl[c] && tick >= asg.todo_tick[c] ) ||
                ( asg.todo_tick_ovrfl[c] && (UINT32_MAX-tick) >= asg.todo_tick[c])
            )
            {
                // if current step state is HIGH
                if ( asg.step_state[c] )
                {
                    // set step state to LOW
                    asg.step_state[c] = 0;

                    // decrease number of steps to do
                    --asg.task_steps_todo[c];
                }
                // if current step state is LOW
                else
                {
                    // set step state to HIGH
                    asg.step_state[c] = 1;
                }

                // toggle step pin
                if ( asg.step_state[c] ^ lsg.step_inv[c] )
                {
                    clr_pin(lsg.step_port[c], lsg.step_pin[c]);
                }
                else
                {
                    set_pin(lsg.step_port[c], lsg.step_pin[c]);
                }

                todo_tick = asg.todo_tick[c];
                asg.todo_tick[c] += asg.step_ticks[c];
                asg.todo_tick_ovrfl[c] = asg.todo_tick[c] < todo_tick ? 1 : 0;
            }
        }
    }
}




#undef asg
#undef lsg

#undef set_pin
#undef clr_pin
