#ifndef _SHMEM_H
#define _SHMEM_H




#include "gpio.h"
#include "stepgen.h"




#define SRAM_A2_SIZE        (48*1024)
#define SRAM_A2_ADDR        0x00000000 // for ARM cores - 0x00040000
#define ARISC_CONF_SIZE     2048
#define ARISC_CONF_ADDR     (SRAM_A2_ADDR + SRAM_A2_SIZE - ARISC_CONF_SIZE)

#define SHM_SIZE            2048
#define SHM_ADDR            (ARISC_CONF_ADDR - SHM_SIZE)




#define u32p    (volatile uint32_t*)




// ARM - Read/Write, ARISC - Read/Write
#define _shm_arisc_alive                   (u32p (SHM_ADDR + 0))   // 4
#define _shm_lcnc_alive                    (u32p (SHM_ADDR + 4))   // 4
#define _shm_stepgen_ch_setup              (u32p (SHM_ADDR + 8))   // 4
#define _shm_stepgen_task_new              (u32p (SHM_ADDR + 12))  // 4
#define _shm_stepgen_get_task_steps_done   (u32p (SHM_ADDR + 16))  // 4
//      _shm_<NAME>                        <TYPE><BASE>      <OFFSET>

// ARM - Read-only
#define _shm_stepgen_task_steps_done       (u32p (SHM_ADDR + 20))  // 64
//      _shm_<NAME>                        <TYPE><BASE>      <OFFSET>

// ARM - Read/Write
#define _shm_gpio_ctrl_locked              (u32p (SHM_ADDR + 84))  // 4
#define _shm_gpio_set_ctrl                 (u32p (SHM_ADDR + 88))  // 64
#define _shm_gpio_clr_ctrl                 (u32p (SHM_ADDR + 152)) // 64
//      _shm_<NAME>                        <TYPE><BASE>      <OFFSET>

// ARM - Read/Write
#define _shm_stepgen_step_port             (u32p (SHM_ADDR + 216)) // 64
#define _shm_stepgen_step_pin              (u32p (SHM_ADDR + 280)) // 64
#define _shm_stepgen_step_inv              (u32p (SHM_ADDR + 344)) // 4
#define _shm_stepgen_dir_port              (u32p (SHM_ADDR + 348)) // 64
#define _shm_stepgen_dir_pin               (u32p (SHM_ADDR + 412)) // 64
#define _shm_stepgen_dir_inv               (u32p (SHM_ADDR + 476)) // 4
#define _shm_stepgen_dirsetup              (u32p (SHM_ADDR + 480)) // 64
#define _shm_stepgen_dirhold               (u32p (SHM_ADDR + 544)) // 64
#define _shm_stepgen_task_dir              (u32p (SHM_ADDR + 608)) // 4
#define _shm_stepgen_task_steps            (u32p (SHM_ADDR + 612)) // 64
#define _shm_stepgen_task_time             (u32p (SHM_ADDR + 676)) // 64
#define _shm_stepgen_ch_enable             (u32p (SHM_ADDR + 740)) // 4
//      shm_<NAME>                        <TYPE><BASE>      <OFFSET>




#define shm(NAME)                   (*(_shm_##NAME))
#define shm_a(NAME,ID)              (*(_shm_##NAME + ID))
#define shm_bit(NAME,BIT)           (*(_shm_##NAME) & (1 << BIT))
#define shm_bit_val(NAME,BIT,BITS)  ((uint##BITS##_t)(*(_shm_##NAME) & (1 << BIT)) >> BIT)
#define shm_bit_set(NAME,BIT)       (*(_shm_##NAME) |= (1 << BIT))
#define shm_bit_clr(NAME,BIT)       (*(_shm_##NAME) &= ~(1 << BIT))




void shmem_clear_all();




#endif
