#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>
#include "cfg.h"

#define TIMER_FREQUENCY CPU_FREQ

void delay_ticks(uint32_t ticks);
void timer_start(void);
uint32_t timer_stop(void);

static inline void delay_us(uint32_t us)
{
	delay_ticks(TIMER_FREQUENCY / 1000000 * us);
}

#endif
