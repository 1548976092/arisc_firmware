#ifndef _SHMEM_H
#define _SHMEM_H




#include "gpio.h"
#include "stepgen.h"




#define SRAM_A2_SIZE        (48*1024)
#define SRAM_A2_ADDR        0x00040000
#define ARISC_CONF_SIZE     2048
#define ARISC_CONF_ADDR     (SRAM_A2_ADDR + SRAM_A2_SIZE - ARISC_CONF_SIZE)
#define ARISC_SHMEM_SIZE    1024
#define ARISC_SHMEM_ADDR    (ARISC_CONF_ADDR - ARISC_SHMEM_SIZE)




// ARM - Read-only, ARISC - Read/Write
struct arisc_stepgen_shmem_t
{
    uint8_t     step_state[STEPGEN_CH_CNT]; // 0/1
    uint8_t     dir_state[STEPGEN_CH_CNT]; // 0/1
    uint8_t     dir_setup[STEPGEN_CH_CNT]; // 2: dir setup, 1: dir hold, 0: step

    uint8_t     task[STEPGEN_CH_CNT]; // !0 = "we have a task"
    uint8_t     task_dir[STEPGEN_CH_CNT]; // DIR state to do
    uint32_t    task_steps[STEPGEN_CH_CNT]; // steps to do by task
    uint32_t    task_steps_todo[STEPGEN_CH_CNT]; // steps to do, 0 = do nothing

//    uint32_t    steplen_ticks[STEPGEN_CH_CNT]; // arisc cpu ticks
//    uint32_t    stepspace_ticks[STEPGEN_CH_CNT]; // arisc cpu ticks
    uint32_t    step_ticks[STEPGEN_CH_CNT]; // arisc cpu ticks
    uint32_t    dirsetup_ticks[STEPGEN_CH_CNT]; // arisc cpu ticks
    uint32_t    dirhold_ticks[STEPGEN_CH_CNT]; // arisc cpu ticks

    uint32_t    todo_tick[STEPGEN_CH_CNT]; // timer tick to make pulses
    uint8_t     todo_tick_ovrfl[STEPGEN_CH_CNT]; // timer ticks overflow flag
};

// ARM - Read/Write, ARISC - Read-only
struct lcnc_stepgen_shmem_t
{
    uint8_t     step_port[STEPGEN_CH_CNT]; // step pin port ID
    uint8_t     step_pin[STEPGEN_CH_CNT]; // step pin ID
    uint8_t     step_inv[STEPGEN_CH_CNT]; // step pin inverted output flag

    uint8_t     dir_port[STEPGEN_CH_CNT];
    uint8_t     dir_pin[STEPGEN_CH_CNT];
    uint8_t     dir_inv[STEPGEN_CH_CNT]; // inverted flag

//    uint32_t    steplen[STEPGEN_CH_CNT]; // ns
//    uint32_t    stepspace[STEPGEN_CH_CNT]; // ns
    uint32_t    dirsetup[STEPGEN_CH_CNT]; // ns
    uint32_t    dirhold[STEPGEN_CH_CNT]; // ns
//    uint32_t    maxaccel[STEPGEN_CH_CNT]; // steps/s*s

    uint8_t     task_dir[STEPGEN_CH_CNT];
    uint32_t    task_steps[STEPGEN_CH_CNT];
    uint32_t    task_time[STEPGEN_CH_CNT]; // ns

    uint8_t     ch_enable[STEPGEN_CH_CNT];
};

// ARM - Read-only, ARISC - Read/Write
struct arisc_shmem_t
{
    uint8_t lcnc_alive_fails;

    uint32_t gpio_set_ctrl[GPIO_PORTS_CNT];
    uint32_t gpio_clr_ctrl[GPIO_PORTS_CNT];

    struct arisc_stepgen_shmem_t stepgen;
};

// ARM - Read/Write, ARISC - Read-only
struct lcnc_shmem_t
{
    uint8_t arisc_alive_fails;

    uint8_t gpio_ctrl_locked;
    uint32_t gpio_set_ctrl[GPIO_PORTS_CNT];
    uint32_t gpio_clr_ctrl[GPIO_PORTS_CNT];

    struct lcnc_stepgen_shmem_t stepgen;
};

struct global_shmem_t
{
    // ARM - Read/Write, ARISC - Read/Write
    uint8_t arisc_alive;
    uint8_t lcnc_alive;
    uint8_t stepgen_ch_setup[STEPGEN_CH_CNT];
    uint8_t stepgen_ch_task_new[STEPGEN_CH_CNT];

    // ARM - Read-only, ARISC - Read/Write
    struct arisc_shmem_t arisc;

    // ARM - Read/Write, ARISC - Read-only
    struct lcnc_shmem_t lcnc;
};




void shmem_clear_all();




#endif
