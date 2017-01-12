#include "io.h"
#include "gpio.h"
#include "regulator.h"

static uint32_t gpio_reg;

void regulator_init(void)
{
	gpio_init();
	gpio_set_pincfg(GPIO_BANK_L, 6, GPIO_FUNC_OUTPUT);
	gpio_reg = gpio_get_data_addr(GPIO_BANK_L);
	set_bit(6, gpio_reg);
}

int regulator_set_voltage(uint32_t voltage)
{
	if (voltage > 1100) {
		set_bit(6, gpio_reg);
	} else {
		clr_bit(6, gpio_reg);
	}
}

int regulator_get_voltage(uint32_t *voltage)
{
	return BIT(6) & readl(gpio_reg) ? 1300 : 1100;
}
