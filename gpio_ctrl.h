/*
 * GPIO control module
 */

#ifndef _GPIO_CTRL_H
#define _GPIO_CTRL_H

#define GPIO_BASE       0X01C20800
#define GPIO_R_BASE     0x01f02c00
#define GPIO_BANK_SIZE  0x24

#define GPIO_CFG_INDEX(pin)     (((pin) & 0x1F) >> 3)
#define GPIO_CFG_OFFSET(pin)    (((pin) & 0x7) << 2)

#define GPIO_PORTS_CNT  8 // max = 16

enum // port names
{
    PA, PB, PC, PD,
    PE, PF, PG, PL
};

// port bank names
#define GPIO_BANK_A     0
#define GPIO_BANK_B     1
#define GPIO_BANK_C     2
#define GPIO_BANK_D     3
#define GPIO_BANK_E     4
#define GPIO_BANK_F     5
#define GPIO_BANK_G     6
#define GPIO_BANK_L     7

// pin function
#define GPIO_FUNC_INPUT         0
#define GPIO_FUNC_OUTPUT        1
#define GPIO_FUNC_BANK_A_UART0  2
#define GPIO_FUNC_BANK_A_I2C0   2
#define GPIO_FUNC_BANK_A_I2C1   3
#define GPIO_FUNC_BANK_E_I2C2   3
#define GPIO_FUNC_BANK_L_I2C3   2

// pin states
enum
{
    LOW, HIGH
};




// export public methods

void gpio_ctrl_init(void);
void gpio_ctrl_base_thread();

void gpio_pin_setup_for_output(uint32_t port, uint32_t pin);
void gpio_pin_setup_for_input(uint32_t port, uint32_t pin);

uint32_t gpio_pin_get(uint32_t port, uint32_t pin);
void gpio_pin_set(uint32_t port, uint32_t pin);
void gpio_pin_clear(uint32_t port, uint32_t pin);

uint32_t gpio_port_get(uint32_t port);
void gpio_port_set(uint32_t port, uint32_t mask);
void gpio_port_clear(uint32_t port, uint32_t mask);




#endif
