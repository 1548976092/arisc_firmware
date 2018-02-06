#include "gpio.h"
#include "clk.h"

void gpio_init(void)
{
	clk_enable(CLK_R_PIO);
}

void gpio_set_pincfg(uint32_t bank, uint32_t pin, uint32_t val)
{
	uint32_t offset = CFG_OFFSET(pin);
	uint32_t addr = (bank == GPIO_BANK_L ? R_PIO_BASE : PIO_BASE + bank * BANK_SIZE) + CFG_INDEX(pin) * 4;
	uint32_t cfg;

	cfg = readl(addr);
	SET_BITS_AT(cfg, 3, offset, val);
	writel(cfg, addr);
}

uint32_t gpio_get_pincfg(uint32_t bank, uint32_t pin)
{
	uint32_t offset = CFG_OFFSET(pin);
	uint32_t addr = (bank == GPIO_BANK_L ? R_PIO_BASE : PIO_BASE + bank * BANK_SIZE) + CFG_INDEX(pin) * 4;

	return GET_BITS_AT(readl(addr), 3, offset);
}

uint32_t gpio_get_data_addr(uint32_t bank)
{
	return (bank == GPIO_BANK_L ? R_PIO_BASE : PIO_BASE + bank * BANK_SIZE) + 4 * 4;
}
