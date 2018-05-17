/*
 * GPIO control module
 */

#include "io.h"
#include "clk.h"
#include "gpio_ctrl.h"




static uint32_t * gpio_port_data[GPIO_PORTS_CNT] =
{
    (uint32_t *) ( (GPIO_BASE + PA * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PB * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PC * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PD * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PE * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PF * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PG * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *)                ( (GPIO_R_BASE) + 16 )
};

static uint32_t gpio_set_ctrl[GPIO_PORTS_CNT] = {0};
static uint32_t gpio_clr_ctrl[GPIO_PORTS_CNT] = {0};




void gpio_ctrl_init(void)
{
    clk_enable(CLK_R_PIO);
}

void gpio_ctrl_base_thread()
{
    // port id
    static uint8_t p;

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




static void gpio_set_pincfg(uint32_t bank, uint32_t pin, uint32_t val)
{
    uint32_t offset = GPIO_CFG_OFFSET(pin);
    uint32_t addr = (bank == GPIO_BANK_L ? GPIO_R_BASE : GPIO_BASE + bank * GPIO_BANK_SIZE) + GPIO_CFG_INDEX(pin) * 4;
    uint32_t cfg;

    cfg = readl(addr);
    SET_BITS_AT(cfg, 3, offset, val);
    writel(cfg, addr);
}

static uint32_t gpio_get_pincfg(uint32_t bank, uint32_t pin)
{
    uint32_t offset = GPIO_CFG_OFFSET(pin);
    uint32_t addr = (bank == GPIO_BANK_L ? GPIO_R_BASE : GPIO_BASE + bank * GPIO_BANK_SIZE) + GPIO_CFG_INDEX(pin) * 4;

    return GET_BITS_AT(readl(addr), 3, offset);
}

static uint32_t gpio_get_data_addr(uint32_t bank)
{
    return (bank == GPIO_BANK_L ? GPIO_R_BASE : GPIO_BASE + bank * GPIO_BANK_SIZE) + 4 * 4;
}
