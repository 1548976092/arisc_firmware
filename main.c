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

	// turn off leds
	gpio_set_pincfg(GPIO_BANK_A, 15, GPIO_FUNC_OUTPUT);
	gpio_set_pincfg(GPIO_BANK_L, 10, GPIO_FUNC_OUTPUT);
	clr_bit(15, gpio_get_data_addr(GPIO_BANK_A)); // RED led
	clr_bit(10, gpio_get_data_addr(GPIO_BANK_L)); // GREEN led

	// config test pin
	gpio_set_pincfg(GPIO_BANK_A, 12, GPIO_FUNC_OUTPUT);
	clr_bit(12, gpio_get_data_addr(GPIO_BANK_A));
        uint8_t pin_state = 0;

	puts("\nOpenRISC FW 1.0\n");

	while (1) 
	{
        	// indicate idle by the GREEN led
        	clr_bit(15, gpio_get_data_addr(GPIO_BANK_A)); // RED led
        	set_bit(10, gpio_get_data_addr(GPIO_BANK_L)); // GREEN led

		delay_us(1000000); // 1 sec of idle

        	// indicate busy state by the RED led
        	set_bit(15, gpio_get_data_addr(GPIO_BANK_A)); // RED led
        	clr_bit(10, gpio_get_data_addr(GPIO_BANK_L)); // GREEN led

		// toggling test pin
		for ( int32_t pulses = 10000000; pulses; )
		{
		        if ( pin_state ) 
		        {
			        clr_bit(12, gpio_get_data_addr(GPIO_BANK_A));
	                        pin_state = 0;
	                        --pulses;
	                }
		        else
		        {
			        set_bit(12, gpio_get_data_addr(GPIO_BANK_A));
	                        pin_state = 1;
	                }
                }
	}

	return 0;
}
