/**
 * @file    mod_gpio.h
 *
 * @brief   GPIO control module header
 *
 * This module implements an API to GPIO functionality for other modules
 */


#ifndef _MOD_GPIO_H
#define _MOD_GPIO_H

#include <stdint.h>
#include "mod_msg.h"




#define GPIO_BASE       0X01C20800 ///< GPIO registers block start address
#define GPIO_R_BASE     0x01f02c00 ///< GPIO R registers block start address
#define GPIO_BANK_SIZE  0x24

#define GPIO_CFG_INDEX(pin)     (((pin) & 0x1F) >> 3)
#define GPIO_CFG_OFFSET(pin)    (((pin) & 0x7) << 2)

#define GPIO_PORTS_CNT  8   ///< number of GPIO ports
#define GPIO_PINS_CNT   32  ///< number of GPIO port pins

/// a GPIO port names
enum { PA, PB, PC, PD, PE, PF, PG, PL };

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

/// the GPIO pin states
enum { LOW, HIGH };

/// the message types
enum
{
    GPIO_MSG_SETUP_FOR_OUTPUT = 0x10,
    GPIO_MSG_SETUP_FOR_INPUT,

    GPIO_MSG_PIN_GET,
    GPIO_MSG_PIN_SET,
    GPIO_MSG_PIN_CLEAR,

    GPIO_MSG_PORT_GET,
    GPIO_MSG_PORT_SET,
    GPIO_MSG_PORT_CLEAR
};

/// the message data sizes
#define GPIO_MSG_BUF_LEN MSG_LEN

/// the message data access
struct gpio_msg_port_t      { uint32_t port; };
struct gpio_msg_port_pin_t  { uint32_t port; uint32_t pin;  };
struct gpio_msg_port_mask_t { uint32_t port; uint32_t mask; };
struct gpio_msg_state_t     { uint32_t state; };




// public methods as macros

/// set pin state = 1
#define GPIO_PIN_SET(PORT,PIN_MASK) \
    *gpio_port_data[PORT] |= PIN_MASK

/// set pin state = 0
#define GPIO_PIN_CLEAR(PORT,PIN_MASK_NOT) \
    *gpio_port_data[PORT] &= PIN_MASK_NOT

/// get pin state
#define GPIO_PIN_GET(PORT,PIN_MASK) \
    (*gpio_port_data[PORT] & PIN_MASK)




// export public methods

void gpio_module_init();

void gpio_pin_setup_for_output(uint32_t port, uint32_t pin);
void gpio_pin_setup_for_input(uint32_t port, uint32_t pin);

uint32_t gpio_pin_get(uint32_t port, uint32_t pin);
void gpio_pin_set(uint32_t port, uint32_t pin);
void gpio_pin_clear(uint32_t port, uint32_t pin);

uint32_t gpio_port_get(uint32_t port);
void gpio_port_set(uint32_t port, uint32_t mask);
void gpio_port_clear(uint32_t port, uint32_t mask);

int8_t volatile gpio_msg_recv(uint8_t type, uint8_t * msg, uint8_t length);




#endif
