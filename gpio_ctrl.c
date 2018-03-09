#include "shmem.h"
#include "gpio_ctrl.h"




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

volatile uint32_t gpio_set_ctrl[GPIO_PORTS_CNT] = {0};
volatile uint32_t gpio_clr_ctrl[GPIO_PORTS_CNT] = {0};





void gpio_ctrl_base_thread()
{
    // port id
    static uint8_t p;
    // general mask
    static uint32_t todo;


    // if LinuxCNC's gpio control data is locked by the LinuxCNC
    if ( shm(gpio_ctrl_locked) )
    {
        // walk through all gpio ports
        for( p = GPIO_PORTS_CNT; p--; )
        {
            // if we need to set some pins
            if ( gpio_set_ctrl[p] )
            {
                // set pins
                *gpio_port_data[p] |= gpio_set_ctrl[p];
                // clear set control flags
                gpio_set_ctrl[p] = 0;
            }

            // if we need to reset some pins
            if ( gpio_clr_ctrl[p] )
            {
                // reset pins
                *gpio_port_data[p] &= ~(gpio_clr_ctrl[p]);
                // clear reset control flags
                gpio_clr_ctrl[p] = 0;
            }
        }
    }
    // if LinuxCNC's gpio control data is free
    // we can read which gpio pins LinuxCNC wants to set/reset
    else
    {
        // walk through all gpio ports
        for( p = GPIO_PORTS_CNT; p--; )
        {
            // clear general mask
            todo = 0;

            // if we need to set some pins
            if ( gpio_set_ctrl[p] )
            {
                // update general mask
                todo |= gpio_set_ctrl[p];
                // clear set control flags
                gpio_set_ctrl[p] = 0;
            }

            // if LinuxCNC wants to set some pins
            if ( shm_a(gpio_set_ctrl,p) )
            {
                // update general mask
                todo |= shm_a(gpio_set_ctrl,p);
                // clear set control flags
                shm_a(gpio_set_ctrl,p) = 0;
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
            if ( gpio_clr_ctrl[p] )
            {
                // update general mask
                todo |= gpio_clr_ctrl[p];
                // clear reset flags
                gpio_clr_ctrl[p] = 0;
            }

            // if LinuxCNC wants to reset some pins
            if ( shm_a(gpio_clr_ctrl,p) )
            {
                // update general mask
                todo |= shm_a(gpio_clr_ctrl,p);
                // clear set control flags
                shm_a(gpio_clr_ctrl,p) = 0;
            }

            // if we finally have some pins to reset
            if ( todo )
            {
                // reset pins
                *gpio_port_data[p] &= ~(todo);
            }
        }
    }
}
