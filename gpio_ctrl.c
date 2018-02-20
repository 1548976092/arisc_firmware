#include "shmem.h"
#include "gpio_ctrl.h"




extern volatile struct global_shmem_t *shm;

volatile uint32_t * gpio_port_data[GPIO_PORTS_CNT] =
{
    (volatile uint32_t *) ( (PIO_BASE + PA * BANK_SIZE) + 16 ),
    (volatile uint32_t *) ( (PIO_BASE + PB * BANK_SIZE) + 16 ),
    (volatile uint32_t *) ( (PIO_BASE + PC * BANK_SIZE) + 16 ),
    (volatile uint32_t *) ( (PIO_BASE + PD * BANK_SIZE) + 16 ),
    (volatile uint32_t *) ( (PIO_BASE + PE * BANK_SIZE) + 16 ),
    (volatile uint32_t *) ( (PIO_BASE + PF * BANK_SIZE) + 16 ),
    (volatile uint32_t *) ( (PIO_BASE + PG * BANK_SIZE) + 16 ),
    (volatile uint32_t *)                ( (R_PIO_BASE) + 16 )
};




void gpio_ctrl_base_thread()
{
    uint8_t p;

    // if LinuxCNC's gpio control data is locked by the LinuxCNC
    if ( shm->lcnc.gpio_ctrl_locked )
    {
        // walk through all gpio ports
        for( p = GPIO_PORTS_CNT; p--; )
        {
            // if we need to set some pins
            if ( shm->arisc.gpio_set_ctrl[p] )
            {
                // set pins
                *gpio_port_data[p] |= shm->arisc.gpio_set_ctrl[p];
                // clear set control flags
                shm->arisc.gpio_set_ctrl[p] = 0;
            }

            // if we need to reset some pins
            if ( shm->arisc.gpio_clr_ctrl[p] )
            {
                // reset pins
                *gpio_port_data[p] &= ~(shm->arisc.gpio_clr_ctrl[p]);
                // clear reset control flags
                shm->arisc.gpio_clr_ctrl[p] = 0;
            }
        }
    }
    // if LinuxCNC's gpio control data is free
    // we can read which gpio pins LinuxCNC wants to set/reset
    else
    {
        // general mask
        uint32_t todo;

        // walk through all gpio ports
        for( p = GPIO_PORTS_CNT; p--; )
        {
            // clear general mask
            todo = 0;

            // if we need to set some pins
            if ( shm->arisc.gpio_set_ctrl[p] )
            {
                // update general mask
                todo |= shm->arisc.gpio_set_ctrl[p];
                // clear set control flags
                shm->arisc.gpio_set_ctrl[p] = 0;
            }

            // if LinuxCNC wants to set some pins
            if ( shm->lcnc.gpio_set_ctrl[p] )
            {
                // update general mask
                todo |= shm->lcnc.gpio_set_ctrl[p];
                // clear set control flags
                shm->lcnc.gpio_set_ctrl[p] = 0;
            }

            // if we finally have some pins to set
            if ( todo )
            {
                // set pins
                *gpio_port_data[p] |= (todo);
                // clear general mask
                todo = 0;
            }

            // if we need to reset some pins
            if ( shm->arisc.gpio_clr_ctrl[p] )
            {
                // update general mask
                todo |= shm->arisc.gpio_clr_ctrl[p];
                // clear reset flags
                shm->arisc.gpio_clr_ctrl[p] = 0;
            }

            // if LinuxCNC wants to reset some pins
            if ( shm->lcnc.gpio_clr_ctrl[p] )
            {
                // update general mask
                todo |= shm->lcnc.gpio_clr_ctrl[p];
                // clear set control flags
                shm->lcnc.gpio_clr_ctrl[p] = 0;
            }

            // if we finally have some pins to set
            if ( todo )
            {
                // reset pins
                *gpio_port_data[p] &= ~(todo);
            }
        }
    }
}
