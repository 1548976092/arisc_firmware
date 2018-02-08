#ifndef _SYS_H
#define _SYS_H




#include <or1k-support.h>
#include <or1k-sprs.h>
#include "debug.h"




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




#endif
