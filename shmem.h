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




struct stepgen_shmem_t
{
    uint8_t step_port[STEPGEN_CH_CNT];
    uint8_t step_pin[STEPGEN_CH_CNT];
    uint8_t step_state[STEPGEN_CH_CNT];

    uint8_t dir_port[STEPGEN_CH_CNT];
    uint8_t dir_pin[STEPGEN_CH_CNT];
    uint8_t dir_state[STEPGEN_CH_CNT];

    uint32_t task_freq[STEPGEN_CH_CNT];
    uint32_t task_steps[STEPGEN_CH_CNT];
    uint32_t task_steps_todo[STEPGEN_CH_CNT];

    uint32_t interval[STEPGEN_CH_CNT];
    uint32_t todo_tick[STEPGEN_CH_CNT]; // timer tick to make pulses
    uint8_t todo_tick_ovrfl[STEPGEN_CH_CNT];

    uint8_t ch_enable[STEPGEN_CH_CNT];
};

struct arisc_shmem_t
{
    uint8_t lcnc_alive_fails;

    uint32_t gpio_set_ctrl[GPIO_PORTS_CNT];
    uint32_t gpio_clr_ctrl[GPIO_PORTS_CNT];

    struct stepgen_shmem_t stepgen;
};

struct lcnc_shmem_t
{
    uint8_t arisc_alive_fails;

    uint8_t gpio_ctrl_locked;
    uint32_t gpio_set_ctrl[GPIO_PORTS_CNT];
    uint32_t gpio_clr_ctrl[GPIO_PORTS_CNT];
};

struct global_shmem_t
{
    // ARM - Read/Write, ARISC - Read/Write
    uint8_t arisc_alive;
    uint8_t lcnc_alive;

    // ARM - Read-only, ARISC - Read/Write
    struct arisc_shmem_t arisc;

    // ARM - Read/Write, ARISC - Read-only
    struct lcnc_shmem_t lcnc;
};




#endif
