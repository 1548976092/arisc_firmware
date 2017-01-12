#include "msgbox.h"

#define CPUCFG_BASE	0x01f01c00

#define readl(addr)	(*((volatile unsigned long  *)(addr)))
#define writel(v, addr)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

int main(void)
{
	volatile unsigned long cpu_cfg;
	
	msgbox_init();

	/* assert reset to ar100 */
	cpu_cfg = readl(CPUCFG_BASE);
	cpu_cfg &= ~(1uL);
	writel(cpu_cfg, CPUCFG_BASE);
	delay(1000);
	/* deassert reset to ar100 */
	cpu_cfg |= 1;
	writel(cpu_cfg, CPUCFG_BASE);
	
	uint32_t a = 0;
	while (1) {
		msgbox_write(1, a);
		//delay(10000);
		a++;
	}

	return 0;
}
