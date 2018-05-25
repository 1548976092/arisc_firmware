#include <or1k-support.h>
#include <or1k-sprs.h>

#include "timer.h"
#include "mod_gpio.h"
#include "shmem.h"
#include "stepgen.h"




extern volatile uint32_t gpio_set_ctrl[GPIO_PORTS_CNT];
extern volatile uint32_t gpio_clr_ctrl[GPIO_PORTS_CNT];

uint8_t lcnc_alive_fails = 0;

volatile struct arisc_stepgen_t stepgen = {0};




#define set_pin(PORT,PIN)   (gpio_set_ctrl[PORT] |= (1 << PIN))
#define clr_pin(PORT,PIN)   (gpio_clr_ctrl[PORT] |= (1 << PIN))




// prepare channel
void stepgen_ch_setup(uint8_t c)
{
    // disable any tasks
    stepgen.task[c] = 0;
    stepgen.task_steps_todo[c] = 0;
    stepgen.dir_setup[c] = 0;

    // setup GPIO
    gpio_set_pincfg(shm_a(stepgen_step_port,c), shm_a(stepgen_step_pin,c), GPIO_FUNC_OUTPUT);
    gpio_set_pincfg(shm_a(stepgen_dir_port,c),  shm_a(stepgen_dir_pin,c),  GPIO_FUNC_OUTPUT);

    // set GPIO pin states
    stepgen.step_state[c] = 0;
    stepgen.dir_state[c]  = 0;

    // set step pin
    if ( stepgen.step_state[c] ^ shm_bit_val(stepgen_step_inv,c,8) ) {
        clr_pin(shm_a(stepgen_step_port,c), shm_a(stepgen_step_pin,c));
    } else {
        set_pin(shm_a(stepgen_step_port,c), shm_a(stepgen_step_pin,c));
    }

    // set DIR pin
    if ( stepgen.dir_state[c] ^ shm_bit_val(stepgen_dir_inv,c,8) ) {
        clr_pin(shm_a(stepgen_dir_port,c), shm_a(stepgen_dir_pin,c));
    } else {
        set_pin(shm_a(stepgen_dir_port,c), shm_a(stepgen_dir_pin,c));
    }

    // setup intervals
    stepgen.dirsetup_ticks[c]  = TIMER_FREQUENCY/(1000000000/shm_a(stepgen_dirsetup,c));
    stepgen.dirhold_ticks[c]   = TIMER_FREQUENCY/(1000000000/shm_a(stepgen_dirhold,c));

    // clear channel setup flag
    shm_bit_clr(stepgen_ch_setup,c);
}




// add new task for the channel
void stepgen_ch_new_task(uint8_t c)
{
    static uint32_t tick = 0;

    // do nothing if channel is disabled
    if ( !shm_bit(stepgen_ch_enable,c) ) return;

    // get current arisc tick
    tick = or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR);

    // set steps count
    stepgen.task_steps[c] = shm_a(stepgen_task_steps,c);
    stepgen.task_steps_todo[c] = stepgen.task_steps[c];

    // if new DIR value is same as current
    if ( stepgen.dir_state[c] == shm_bit_val(stepgen_task_dir,c,8) )
    {
        // calculate step length ticks
        stepgen.step_ticks[c] = TIMER_FREQUENCY
            / (1000000000/shm_a(stepgen_task_time,c))
            / stepgen.task_steps_todo[c]
            / 2;

        // set tick for the next step toggle
        stepgen.todo_tick[c] = tick;
        stepgen.todo_tick_ovrfl[c] = 0;
    }
    // if new DIR state != current DIR state
    else
    {
        // save new DIR value
        stepgen.task_dir[c] = shm_bit(stepgen_task_dir,c) ? 1 : 0;

        // calculate step length ticks
        stepgen.step_ticks[c] = TIMER_FREQUENCY
            / (1000000000 / (shm_a(stepgen_task_time,c) - shm_a(stepgen_dirsetup,c) - shm_a(stepgen_dirhold,c)))
            / stepgen.task_steps_todo[c]
            / 2;

        // set tick for the next DIR change
        stepgen.dir_setup[c] = 2; // 2 = dir setup, 1 = dir hold
        stepgen.todo_tick[c] = tick + stepgen.dirsetup_ticks[c];
        stepgen.todo_tick_ovrfl[c] = stepgen.todo_tick[c] < tick ? 1 : 0;
    }

    // clear channel's new task flag
    shm_bit_clr(stepgen_task_new,c);
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
        if ( shm_bit(stepgen_ch_setup,c) ) stepgen_ch_setup(c);

        // run this code only when step is done
        if ( !stepgen.step_state[c] )
        {
            // if we must disable the channel
            if ( !shm_bit(stepgen_ch_enable,c) && stepgen.task[c] ) stepgen.task[c] = 0;
            // if we have new task for the channel
            else if ( shm_bit(stepgen_task_new,c) ) stepgen_ch_new_task(c);
        }

        // if channel have no tasks - goto the next channel
        if ( !stepgen.task[c] ) continue;

        // if we are working with DIR
        if ( stepgen.dir_setup[c] )
        {
            // if it's time to make the DIR change
            if
            (
                ( !stepgen.todo_tick_ovrfl[c] && tick >= stepgen.todo_tick[c] ) ||
                ( stepgen.todo_tick_ovrfl[c] && (UINT32_MAX-tick) >= stepgen.todo_tick[c])
            )
            {
                // if it was DIR setup period
                if ( stepgen.dir_setup[c] > 1 )
                {
                    // change DIR state
                    stepgen.dir_state[c] = stepgen.task_dir[c];

                    // toggle DIR pin
                    if ( stepgen.dir_state[c] ^ shm_bit_val(stepgen_dir_inv,c,8) )
                    {
                        clr_pin(shm_a(stepgen_dir_port,c), shm_a(stepgen_dir_pin,c));
                    }
                    else
                    {
                        set_pin(shm_a(stepgen_dir_port,c), shm_a(stepgen_dir_pin,c));
                    }

                    // set tick for the DIR hold period
                    todo_tick = stepgen.todo_tick[c];
                    stepgen.todo_tick[c] += stepgen.dirhold_ticks[c];
                    stepgen.todo_tick_ovrfl[c] = stepgen.todo_tick[c] < todo_tick ? 1 : 0;
                }
                // if it was DIR hold period and we have some steps to do
                else if ( stepgen.task_steps_todo[c] )
                {
                    // set tick for the next step toggle
                    stepgen.todo_tick[c] = tick;
                    stepgen.todo_tick_ovrfl[c] = 0;
                }

                // change DIR setup state
                --stepgen.dir_setup[c];
            }
        }
        // if we are working with steps
        else
        {
            // if we have no steps to do
            if ( !stepgen.task_steps_todo[c] )
            {
                // disable "we have task" flag
                stepgen.task[c] = 0;
                continue;
            }

            // if we must make the step change
            if
            (
                ( !stepgen.todo_tick_ovrfl[c] && tick >= stepgen.todo_tick[c] ) ||
                ( stepgen.todo_tick_ovrfl[c] && (UINT32_MAX-tick) >= stepgen.todo_tick[c])
            )
            {
                // if current step state is HIGH
                if ( stepgen.step_state[c] )
                {
                    // set step state to LOW
                    stepgen.step_state[c] = 0;

                    // decrease number of steps to do
                    --stepgen.task_steps_todo[c];
                }
                // if current step state is LOW
                else
                {
                    // set step state to HIGH
                    stepgen.step_state[c] = 1;
                }

                // toggle step pin
                if ( stepgen.step_state[c] ^ shm_bit_val(stepgen_step_inv,c,8) )
                {
                    clr_pin(shm_a(stepgen_step_port,c), shm_a(stepgen_step_pin,c));
                }
                else
                {
                    set_pin(shm_a(stepgen_step_port,c), shm_a(stepgen_step_pin,c));
                }

                todo_tick = stepgen.todo_tick[c];
                stepgen.todo_tick[c] += stepgen.step_ticks[c];
                stepgen.todo_tick_ovrfl[c] = stepgen.todo_tick[c] < todo_tick ? 1 : 0;
            }
        }
    }
}
