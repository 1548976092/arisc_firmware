/**
 * @file    gpio_ctrl.c
 *
 * @brief   GPIO control module
 *
 * This module implements an API to GPIO functionality for other modules
 */

#include "io.h"
#include "clk.h"
#include "gpio_ctrl.h"




// private vars

static uint32_t * gpio_port_data[GPIO_PORTS_CNT] =
{
    (uint32_t *) ( (GPIO_BASE + PA * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PB * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PC * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PD * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PE * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PF * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PG * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_R_BASE                    ) + 16 )
};

static uint32_t gpio_set_ctrl[GPIO_PORTS_CNT] = {0};
static uint32_t gpio_clr_ctrl[GPIO_PORTS_CNT] = {0};




// private methods

static inline void gpio_set_pincfg(uint32_t bank, uint32_t pin, uint32_t val)
{
    uint32_t offset = GPIO_CFG_OFFSET(pin);
    uint32_t addr = (bank == GPIO_BANK_L ? GPIO_R_BASE : GPIO_BASE + bank * GPIO_BANK_SIZE) + GPIO_CFG_INDEX(pin) * 4;
    uint32_t cfg;

    cfg = readl(addr);
    SET_BITS_AT(cfg, 3, offset, val);
    writel(cfg, addr);
}

static inline uint32_t gpio_get_pincfg(uint32_t bank, uint32_t pin)
{
    uint32_t offset = GPIO_CFG_OFFSET(pin);
    uint32_t addr = (bank == GPIO_BANK_L ? GPIO_R_BASE : GPIO_BASE + bank * GPIO_BANK_SIZE) + GPIO_CFG_INDEX(pin) * 4;

    return GET_BITS_AT(readl(addr), 3, offset);
}

static inline uint32_t gpio_get_data_addr(uint32_t bank)
{
    return (bank == GPIO_BANK_L ? GPIO_R_BASE : GPIO_BASE + bank * GPIO_BANK_SIZE) + 4 * 4;
}




// public methods

/**
 * @brief   module init
 * @note    call this function only once before gpio_ctrl_base_thread()
 * @retval  none
 */
void gpio_ctrl_init()
{
    clk_enable(CLK_R_PIO);
}

/**
 * @brief   module base thread
 * @note    call this function in the end of main loop
 * @retval  none
 */
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




/**
 * @brief   set pin mode to OUTPUT
 * @param   port    GPIO port number    (0 .. GPIO_PORTS_CNT)
 * @param   pin     GPIO pin number     (0 .. GPIO_PINS_CNT)
 * @retval  none
 */
void gpio_pin_setup_for_output(uint32_t port, uint32_t pin)
{
    gpio_set_pincfg(port, pin, GPIO_FUNC_OUTPUT);
}

/**
 * @brief   set pin mode to INPUT
 * @param   port    GPIO port number    (0 .. GPIO_PORTS_CNT)
 * @param   pin     GPIO pin number     (0 .. GPIO_PINS_CNT)
 * @retval  none
 */
void gpio_pin_setup_for_input(uint32_t port, uint32_t pin)
{
    gpio_set_pincfg(port, pin, GPIO_FUNC_INPUT);
}




/**
 * @brief   get pin state
 * @param   port    GPIO port number    (0 .. GPIO_PORTS_CNT)
 * @param   pin     GPIO pin number     (0 .. GPIO_PINS_CNT)
 * @retval  1 (HIGH)
 * @retval  0 (LOW)
 */
uint32_t gpio_pin_get(uint32_t port, uint32_t pin)
{
    return (*gpio_port_data[port] & (1 << pin)) ? HIGH : LOW;
}

/**
 * @brief   set pin state to HIGH (1)
 * @param   port    GPIO port number    (0 .. GPIO_PORTS_CNT)
 * @param   pin     GPIO pin number     (0 .. GPIO_PINS_CNT)
 * @retval  none
 */
void gpio_pin_set(uint32_t port, uint32_t pin)
{
    static uint32_t pin_mask;

    pin_mask = 1U << pin;
    gpio_set_ctrl[port] |= pin_mask;
    gpio_clr_ctrl[port] &= ~pin_mask;
}

/**
 * @brief   set pin state to LOW (0)
 * @param   port    GPIO port number    (0 .. GPIO_PORTS_CNT)
 * @param   pin     GPIO pin number     (0 .. GPIO_PINS_CNT)
 * @retval  none
 */
void gpio_pin_clear(uint32_t port, uint32_t pin)
{
    static uint32_t pin_mask;

    pin_mask = 1U << pin;
    gpio_set_ctrl[port] &= ~pin_mask;
    gpio_clr_ctrl[port] |= pin_mask;
}




/**
 * @brief   get port state
 * @param   port    GPIO port number (0 .. GPIO_PORTS_CNT)
 * @note    each bit value of returned value represents port pin state
 * @retval  0 .. 0xFFFFFFFF
 */
uint32_t gpio_port_get(uint32_t port)
{
    return *gpio_port_data[port];
}

/**
 * @brief   set port pins state by mask
 *
 * @param   port    GPIO port number        (0 .. GPIO_PORTS_CNT)
 * @param   mask    GPIO pins mask to set   (0 .. 0xFFFFFFFF) \n\n
 *                  mask examples: \n\n
 *                      mask = 0xFFFFFFFF (0b11111111111111111111111111111111) means <b>set all pins state to 1 (HIGH)</b> \n
 *                      mask = 0x00000001 (0b1) means <b>set pin 0 state to 1 (HIGH)</b> \n
 *                      mask = 0x0000000F (0b1111) means <b>set pins 0,1,2,3 states to 1 (HIGH)</b>
 *
 * @retval  none
 */
void gpio_port_set(uint32_t port, uint32_t mask)
{
    gpio_set_ctrl[port] |= mask;
    gpio_clr_ctrl[port] &= ~mask;
}

/**
 * @brief   clear port pins state by mask
 *
 * @param   port    GPIO port number        (0 .. GPIO_PORTS_CNT)
 * @param   mask    GPIO pins mask to clear (0 .. 0xFFFFFFFF) \n\n
 *                  mask examples: \n\n
 *                  mask = 0xFFFFFFFF (0b11111111111111111111111111111111) means <b>set all pins state to 0 (LOW)</b> \n
 *                  mask = 0x00000003 (0b11) means <b>set pins 0,1 states to 0 (LOW)</b> \n
 *                  mask = 0x00000008 (0b1000) means <b>set pin 3 state to 0 (LOW)</b>
 *
 * @retval  none
 */
void gpio_port_clear(uint32_t port, uint32_t mask)
{
    gpio_set_ctrl[port] &= ~mask;
    gpio_clr_ctrl[port] |= mask;
}




/**
    @example gpio_ctrl.c

    <b>Usage example 1</b>: single pin toggling:

    @code
        int main(void)
        {
            gpio_ctrl_init(); // module init

            gpio_pin_setup_for_output(PA,3); // configure pin PA3 as output

            for(;;) // main loop
            {
                // PA3 pin toggling
                if ( gpio_pin_get(PA,3) )   gpio_pin_clear(PA,3);
                else                        gpio_pin_set  (PA,3);

                gpio_ctrl_base_thread(); // real update of pin states
            }

            return 0;
        }
    @endcode

    <b>Usage example 2</b>: whole port toggling:

    @code
        int main(void)
        {
            gpio_ctrl_init(); // module init

            // configure whole port A as output
            uint8_t pin;
            for ( pin = 0; pin < GPIO_PINS_CNT; pin++ )
            {
                gpio_pin_setup_for_output(PA, pin);
            }

            for(;;) // main loop
            {
                // port A toggling
                if ( gpio_port_get(PA) )    gpio_port_clear(PA, 0xFFFFFFFF);
                else                        gpio_port_set  (PA, 0xFFFFFFFF);

                gpio_ctrl_base_thread(); // real update of pin states
            }

            return 0;
        }
    @endcode
*/
