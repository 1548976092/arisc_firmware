#include <or1k-support.h>
#include <or1k-sprs.h>
#include <stdint.h>
#include <string.h>

#include "io.h"
#include "gpio.h"
#include "debug.h"
#include "clk.h"
#include "timer.h"




#define MAX_PULSE_FREQUENCY     200000 // Hz

#define BASE_CYCLES_CNT         1000000

#define TIMER_FREQUENCY         300000000 // Hz
#define TIMER_INTERVAL_LOSS     82 / 100
#define TIMER_INTERVAL          (TIMER_FREQUENCY / MAX_PULSE_FREQUENCY \
                                * TIMER_INTERVAL_LOSS / 2)

#define CH_CNT                  8




struct gpio_t
{
    int8_t bank;
    int8_t pin;
};

const struct gpio_t ch_pin[CH_CNT] = 
{
    /* 3*/{GPIO_BANK_A,12}, /* 5*/{GPIO_BANK_A,11}, 
    /* 7*/{GPIO_BANK_A, 6}, /* 8*/{GPIO_BANK_A,13},
    /*10*/{GPIO_BANK_A,14}, /*11*/{GPIO_BANK_A, 1}, 
    /*13*/{GPIO_BANK_A, 0}, /*15*/{GPIO_BANK_A, 3}
};




void enable_caches(void)
{
	for (unsigned addr = 0; addr < 16 * 1024 + 32 * 1024; addr += 16)
	{
		or1k_icache_flush(addr);
	}

	or1k_icache_enable();
}

void reset(void)
{
	asm("l.j _start");
	asm("l.nop");
}

void handle_exception(uint32_t type, uint32_t pc, uint32_t sp)
{
	if (type == 8) 
	{
		printf("interrupt\n");
	} 
	else if (type == 5) 
	{
		printf("timer\n");
	} 
	else 
	{
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


    uint32_t    ch_freq[CH_CNT]     = {200000,100000,50000,20000,
                                        10000,  5000, 2000,1000}; // Hz
    uint32_t    ch_cnt[CH_CNT]      = {0};
    uint32_t    ch_interv[CH_CNT]   = {0};
    uint8_t     ch_state[CH_CNT]    = {0};
    uint8_t     ch_enbl[CH_CNT]     = {0};
    int32_t     n                   = 0;
    int32_t     cnt                 = 0;
    uint32_t    ticks               = 0;


    // config output pins state and options
    for ( n = CH_CNT; n--; )
    {
        gpio_set_pincfg(ch_pin[n].bank, ch_pin[n].pin, GPIO_FUNC_OUTPUT);
        set_bit(ch_pin[n].pin, gpio_get_data_addr(ch_pin[n].bank));
        ch_interv[n] = MAX_PULSE_FREQUENCY / ch_freq[n] - 1;
        ch_enbl[n] = 1;
    }


    while(1)
    {
        // make MAX_PULSE_FREQUENCY base cycles
        for ( cnt = 2 * BASE_CYCLES_CNT; cnt--; )
        {
            // start catching of CPU ticks
            timer_start();
            
            // make pulses
            for ( n = CH_CNT; n--; )
            {
                if ( ch_enbl[n] )
                {
                    if ( ch_cnt[n] )
                    {
                        --ch_cnt[n];
                    }
                    else
                    {
                        if ( ch_state[n] )
                        {
                            clr_bit(ch_pin[n].pin, 
                                    gpio_get_data_addr(ch_pin[n].bank));
                            ch_state[n] = 0;
                        }
                        else
                        {
                            set_bit(ch_pin[n].pin, 
                                    gpio_get_data_addr(ch_pin[n].bank));
                            ch_state[n] = 1;
                        }

                        ch_cnt[n] = ch_interv[n];
                    }
                }
            }

            // we got the CPU ticks count for the `make pulses` code
            ticks = timer_stop();

            // wait a few ticks
            if ( ticks < TIMER_INTERVAL) delay_ticks(TIMER_INTERVAL - ticks);
        }

        // reset output pins state
        for ( n = CH_CNT; n--; )
        {
            set_bit(ch_pin[n].pin, gpio_get_data_addr(ch_pin[n].bank));
        }

        // wait a second
        delay_us(1000000);
    }


    return 0;
}

