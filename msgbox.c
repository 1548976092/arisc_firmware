#include "io.h"
#ifndef ARM_BOOT
#include "clk.h"
#endif
#include "msgbox.h"

#define MSGBOX_BASE 		0x01c17000

#define CTRL_REG0               (MSGBOX_BASE)
#define CTRL_REG1 		(MSGBOX_BASE + 0x04)
#define CTRL_USER0_RECEIVER(n)  (BIT(4) << ((n) * 8))
#define CTRL_USER0_SENDER(n)    (BIT(0) << ((n) * 8))

#define USER0_IRQ_EN_REG 	(MSGBOX_BASE + 0x40)
#define USER0_IRQ_STATUS_REG 	(MSGBOX_BASE + 0x50)
#define USER1_IRQ_EN_REG 	(MSGBOX_BASE + 0x60)
#define USER1_IRQ_STATUS_REG    (MSGBOX_BASE + 0x70)
#define TRANSMIT_MQn_IRQ_PEND(n) BIT(2 * (n) + 1)
#define RECEPTION_MQn_IRQ_PEND(n) BIT(2 * (n))

#define FIFO_STATUS_REG(n)   	(MSGBOX_BASE + 0x100 + (n) * 4)
#define MSG_STATUS_REG(n)   	(MSGBOX_BASE + 0x140 + (n) * 4)
#define MSG_REG(n)  	 	(MSGBOX_BASE + 0x180 + (n) * 4)

#define USER0 0  // arisc
#define USER1 1  // main cpu

// user0 = arisc, user1 = main cpu

void msgbox_init(void)
{
#ifndef ARM_BOOT
	clk_enable(CLK_MSGBOX);

	writel(CTRL_USER0_SENDER(0) | CTRL_USER0_RECEIVER(1), CTRL_REG0);
#endif
}

// return 1 if read, 0 if queue is empty

int msgbox_read(uint32_t queue, uint32_t* val)
{
	if ((readl(MSG_STATUS_REG(queue)) & 0x07) == 0) {
		return 0;
	}

        *val = readl(MSG_REG(queue));
	return 1;
}

int msgbox_write(uint32_t queue, uint32_t val)
{
	if ((readl(FIFO_STATUS_REG(queue)) & BIT(0))) {
		return 0;
	}

	writel(val, MSG_REG(queue));
	return 1;
}
