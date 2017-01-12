#include <or1k-support.h>
#include <or1k-sprs.h>
#include "io.h"
#include "gpio.h"
#include "debug.h"
#include "clk.h"
#include "ths.h"
#include "timer.h"
#include "regulator.h"
#include "msgbox.h"

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

static unsigned int holdrand = 0;
static unsigned int rand(void) 
{
	 return (((holdrand = holdrand * 214013 + 2531011) >> 16) & 0x7fff);
}

int main(void)
{
	enable_caches();
	gpio_init();
	uart0_init();
	clk_set_rate(CLK_CPUS, 300000000);
	ths_init();
	regulator_init();
	msgbox_init();

	// turn on leds
	gpio_set_pincfg(GPIO_BANK_A, 15, GPIO_FUNC_OUTPUT);
	gpio_set_pincfg(GPIO_BANK_L, 10, GPIO_FUNC_OUTPUT);
	set_bit(15, gpio_get_data_addr(GPIO_BANK_A));
	set_bit(10, gpio_get_data_addr(GPIO_BANK_L));

	puts("\nOpenRISC FW 1.0\n");

	regulator_set_voltage(1300);

	// blink leds so that world knows we're alive
	uint32_t limiting = 0;
	uint32_t freq = 0;
	while (1) {
		delay_us(10000);

		uint32_t temp = ths_get_temp();
		if (temp > 60000)
			limiting = 1;
                else if (temp < 50000)
			limiting = 0;

		if (limiting) {
			clk_set_rate(CLK_CPUX, 60000000);
			//delay_us(10000);
			//regulator_set_voltage(1100);
		} else {
			// 464 432
#define F1 464
#define F2 816
			if (freq == F1)
				freq = F2;
			else 
				freq = F1;

			freq = rand() * 1024 / 32768;

			/*
			if (freq > 408) {
				regulator_set_voltage(1300);
				delay_us(10000);
			}
                          */

			clk_set_rate(CLK_CPUX_WRONG, freq * 1000000);

                        /*
			if (freq < 408) {
				delay_us(10000);
				regulator_set_voltage(1100);
			}
			*/
		}

		uint32_t msg = 0;
		if (msgbox_read(1, &msg)) {
			printf("got message %d\n", msg);
		}

		//printf("%f °C   %d MHz => %d MHz \n", temp, freq, clk_get_rate(CLK_CPUX) / 1000 / 1000);
	}
}
