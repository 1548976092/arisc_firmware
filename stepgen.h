#ifndef _STEPGEN_H
#define _STEPGEN_H




#define STEPGEN_CH_CNT 8 // max = 16




struct arisc_stepgen_t
{
    uint8_t     step_state[STEPGEN_CH_CNT]; // 0/1
    uint8_t     dir_state[STEPGEN_CH_CNT]; // 0/1
    uint8_t     dir_setup[STEPGEN_CH_CNT]; // 2: dir setup, 1: dir hold, 0: step

    uint8_t     task[STEPGEN_CH_CNT]; // !0 = "we have a task"
    uint8_t     task_dir[STEPGEN_CH_CNT]; // DIR state to do
    uint32_t    task_steps[STEPGEN_CH_CNT]; // steps to do by task
    uint32_t    task_steps_todo[STEPGEN_CH_CNT]; // steps to do, 0 = do nothing

    uint32_t    step_ticks[STEPGEN_CH_CNT]; // arisc cpu ticks
    uint32_t    dirsetup_ticks[STEPGEN_CH_CNT]; // arisc cpu ticks
    uint32_t    dirhold_ticks[STEPGEN_CH_CNT]; // arisc cpu ticks

    uint32_t    todo_tick[STEPGEN_CH_CNT]; // timer tick to make pulses
    uint8_t     todo_tick_ovrfl[STEPGEN_CH_CNT]; // timer ticks overflow flag
};





void stepgen_ch_set_step_pin(uint8_t id, uint8_t port, uint8_t pin);
void stepgen_ch_set_task(uint8_t id, uint32_t freq, uint32_t steps);
void stepgen_base_thread();




#endif
