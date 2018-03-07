#include <or1k-support.h>
#include <or1k-sprs.h>
#include <stdint.h>
#include <string.h>

#include "cfg.h"
#include "io.h"
#include "gpio.h"
#include "debug.h"
#include "clk.h"
#include "timer.h"
#include "sys.h"
// TODO - test these modules
#include "shmem.h"
#include "gpio_ctrl.h"
#include "stepgen.h"




int main(void)
{
    // system settings
    enable_caches();
    gpio_init();
    uart0_init();
    clk_set_rate(CLK_CPUS, CPU_FREQ);
    timer_start();

    // init and clean up shared memory
    shmem_clear_all();




    // shm TEST
    uint8_t i = 0;

    shm(arisc_alive) = 1;
    shm(lcnc_alive) = 2;
    shm(stepgen_ch_setup) = 3;
    shm(stepgen_task_new) = 4;
    shm(stepgen_get_task_steps_done) = 5;

    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_task_steps_done,i) = 6;

    shm(gpio_ctrl_locked) = 7;
    for ( i = 0; i < GPIO_PORTS_CNT; i++ ) shm_a(gpio_set_ctrl,i) = 8;
    for ( i = 0; i < GPIO_PORTS_CNT; i++ ) shm_a(gpio_clr_ctrl,i) = 9;

    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_step_port,i) = 10;
    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_step_pin,i) = 11;
    shm(stepgen_step_inv) = 12;
    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_dir_port,i) = 13;
    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_dir_pin,i) = 14;
    shm(stepgen_dir_inv) = 15;
    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_dirsetup,i) = 16;
    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_dirhold,i) = 17;
    shm(stepgen_task_dir) = 18;
    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_task_steps,i) = 19;
    for ( i = 0; i < STEPGEN_CH_CNT; i++ ) shm_a(stepgen_task_time,i) = 20;
    shm(stepgen_ch_enable) = 21;




    // main loop
    for(;;)
    {
        // ARISC core is alive
//        if ( !shm->arisc_alive ) shm->arisc_alive = 1;

        // process stepgen thread
//        stepgen_base_thread();

        // process gpio control thread
//        gpio_ctrl_base_thread();
    }


    return 0;
}

