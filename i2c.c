#include "gpio.h"
#include "clk.h"
#include "i2c.h"

#define I2C0_BASE 0x01c2ac00
#define I2C1_BASE 0x01c2b000
#define I2C2_BASE 0x01c2b400
#define I2C3_BASE 0x01f02400

void i2c_init(uint32_t bus, uint32_t clk_rate)
{
}

int i2c_read(uint32_t bus, uint32_t addr, uint32_t reg, uint32_t *data)
{
}

int i2c_write(uint32_t bus, uint32_t addr, uint32_t reg, uint32_t data)
{
}
