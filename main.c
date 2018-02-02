#include <or1k-support.h>
#include <or1k-sprs.h>
#include "io.h"
#include "gpio.h"
#include "debug.h"
#include "clk.h"
#include "timer.h"

void enable_caches(void)
{
	unsigned addr;

	for (addr = 0; addr < 16 * 1024 + 32 * 1024; addr += 16)
		or1k_icache_flush(addr);

	or1k_icache_enable();
}

void reset(void)
{
	asm("l.j _start");
	asm("l.nop");
}

void handle_exception(uint32_t type, uint32_t pc, uint32_t sp)
{
	if (type == 8) {
		printf("interrupt\n");
	} else if (type == 5) {
		printf("timer\n");
	} else {
		printf("exception %u at pc=%x sp=%x\nrestarting...\n\n", type, pc, sp);
		reset();
	}
}

int main(void)
{
	enable_caches();
	gpio_init();
	uart0_init();
	clk_set_rate(CLK_CPUS, 300000000);

	// configure and turn off the leds
	gpio_set_pincfg(GPIO_BANK_A, 15, GPIO_FUNC_OUTPUT);
	gpio_set_pincfg(GPIO_BANK_L, 10, GPIO_FUNC_OUTPUT);
	clr_bit(15, gpio_get_data_addr(GPIO_BANK_A)); // RED led OFF
	clr_bit(10, gpio_get_data_addr(GPIO_BANK_L)); // GREEN led OFF

	// output to the UART0
	puts("ARISC test firmware by MX_Master.\n");
	puts("This firmware just blinking by onboard leds every second.\n");

	// simple blinking by leds
	while (1) 
	{
		clr_bit(15, gpio_get_data_addr(GPIO_BANK_A)); // RED led OFF
		set_bit(10, gpio_get_data_addr(GPIO_BANK_L)); // GREEN led ON

		delay_us(1000000); // 1 sec of idle

		set_bit(15, gpio_get_data_addr(GPIO_BANK_A)); // RED led ON
		clr_bit(10, gpio_get_data_addr(GPIO_BANK_L)); // GREEN led OFF

		delay_us(1000000); // 1 sec of idle
	}

	return 0;
}
