#include "io.h"
#include "clk.h"
#include "timer.h"
#include "debug.h"

/* ccm */
#define CCM_BASE	0x01c20000
#define R_PRCM_BASE     0x01f01400

/* clock gating */
#define BUS_CLK_GATING_REG0 		(CCM_BASE + 0x060)
#define BUS_CLK_GATING_REG1 		(CCM_BASE + 0x064)
#define MSGBOX_GATING 			BIT(21)
#define BUS_CLK_GATING_REG2 		(CCM_BASE + 0x068)
#define PIO_GATING 			BIT(5)
#define THS_GATING 			BIT(8)
#define BUS_CLK_GATING_REG3 		(CCM_BASE + 0x06c)
#define UART0_GATING 			BIT(16)
#define I2C0_GATING 			BIT(0)
#define I2C1_GATING 			BIT(1)
#define I2C2_GATING 			BIT(2)
#define BUS_CLK_GATING_REG4 		(CCM_BASE + 0x070)

/* soft reset */
#define BUS_SOFT_RST_REG0 		(CCM_BASE + 0x2c0)
#define BUS_SOFT_RST_REG1 		(CCM_BASE + 0x2c4)
#define MSGBOX_RST 			BIT(21)
#define BUS_SOFT_RST_REG2 		(CCM_BASE + 0x2c8)
#define BUS_SOFT_RST_REG3 		(CCM_BASE + 0x2d0)
#define THS_RST 			BIT(8)
#define BUS_SOFT_RST_REG4 		(CCM_BASE + 0x2d8)
#define UART0_RST 			BIT(16)
#define I2C0_RST 			BIT(0)
#define I2C1_RST 			BIT(1)
#define I2C2_RST 			BIT(2)

/* apb2 */
#define APB2_CFG_REG			(CCM_BASE + 0x058)
#define APB2_CLK_SRC_LOSC		(0x0 << 24)
#define APB2_CLK_SRC_OSC24M		(0x1 << 24)
#define APB2_CLK_SRC_PLL6		(0x2 << 24)
#define APB2_CLK_SRC_MASK		(0x3 << 24)
#define APB2_CLK_RATE_N_1		(0x0 << 16)
#define APB2_CLK_RATE_N_2		(0x1 << 16)
#define APB2_CLK_RATE_N_4		(0x2 << 16)
#define APB2_CLK_RATE_N_8		(0x3 << 16)
#define APB2_CLK_RATE_N_MASK		(3 << 16)
#define APB2_CLK_RATE_M(m)		(((m)-1) << 0)
#define APB2_CLK_RATE_M_MASK            (0x1f << 0)

/* pll6 */
#define PLL6_CTRL_REG			(CCM_BASE + 0x0028)
#define PLL6_CTRL_ENABLE		BIT(31)
#define PLL6_CTRL_LOCK			BIT(28)
#define PLL6_CTRL_BYPASS		BIT(25)
#define PLL6_CTRL_CLK_OUTEN		BIT(24)
#define PLL6_CTRL_24M_OUTEN		BIT(18)

/* ths */
#define THS_CLK_REG                     (CCM_BASE + 0x0074)
#define THS_CLK_SCLK_GATING             BIT(31)
#define THS_CLK_SRC_OSC24M              0
#define THS_CLK_DIV_RATIO_6             0x3
#define THS_CLK_DIV_RATIO_1             0x0

/* pll1 */
#define PLL_CPUX_CTRL_REG               (CCM_BASE)
#define PLL_CPUX_ENABLE                 BIT(31)
#define PLL_CPUX_LOCK                   BIT(28)
#define PLL_CPUX_P(v)                   ((v) << 16)
#define PLL_CPUX_N(v)                   ((v) << 8)
#define PLL_CPUX_K(v)                   ((v) << 4)
#define PLL_CPUX_M(v)                   ((v) << 0)

#define CPUX_AXI_CFG_REG                (CCM_BASE + 0x0050)

/*
 * AR100 clock configuration register:
 * [31:18] Reserved
 * [17:16] Clock source (00: LOSC, 01: HOSC, 10/11: PLL6/PDIV)
 * [15:13] Reserved
 * [12:8]  Post divide (00000: 1 - 11111: 32)
 * [7:6]   Reserved
 * [5:4]   Clock divide ratio (00: 1, 01: 2, 10: 4, 11: 8)
 * [3:0]   Reserved
 */
#define AR100_CLKCFG_REG		(R_PRCM_BASE + 0x000)
#define AR100_CLKCFG_SRC_LOSC		(0 << 16)
#define AR100_CLKCFG_SRC_HOSC		(1 << 16)
#define AR100_CLKCFG_SRC_PLL6		(2 << 16)
#define AR100_CLKCFG_SRC_MASK		(0x3 << 16)
#define AR100_CLKCFG_POSTDIV(x)		(((x) & 0x1f) << 8)
#define AR100_CLKCFG_POSTDIV_MASK	(0x1f << 8)
#define AR100_CLKCFG_DIV(x)		(((x) & 0x3) << 4)
#define AR100_CLKCFG_DIV_MASK		(0x3 << 4)

#define	R_PRCM_CLK_GATING_REG		(R_PRCM_BASE + 0x28) // ???
#define R_PIO_GATING 			BIT(0)

// ARISC/CPUS and RTC power regulation
#define VDD_RTC_REG 0x01f00190

void clk_enable(uint32_t clk)
{
	if (clk & CLK_UART0) {
		readl(BUS_SOFT_RST_REG4) |= UART0_RST;
		readl(BUS_CLK_GATING_REG3) |= UART0_GATING;
	}

	if (clk & CLK_R_PIO) {
		readl(R_PRCM_CLK_GATING_REG) |= R_PIO_GATING;
	}

	if (clk & CLK_THS) {
		readl(BUS_SOFT_RST_REG3) |= THS_RST;
		readl(BUS_CLK_GATING_REG2) |= THS_GATING;
	}

	if (clk & CLK_MSGBOX) {
		readl(BUS_SOFT_RST_REG1) |= MSGBOX_RST;
		readl(BUS_CLK_GATING_REG1) |= MSGBOX_GATING;
	}

	if (clk & CLK_I2C0) {
		readl(BUS_SOFT_RST_REG4) |= I2C0_RST;
		readl(BUS_CLK_GATING_REG3) |= I2C0_GATING;
	}

	if (clk & CLK_I2C1) {
		readl(BUS_SOFT_RST_REG4) |= I2C1_RST;
		readl(BUS_CLK_GATING_REG3) |= I2C1_GATING;
	}

	if (clk & CLK_I2C2) {
		readl(BUS_SOFT_RST_REG4) |= I2C2_RST;
		readl(BUS_CLK_GATING_REG3) |= I2C2_GATING;
	}

	if (clk & CLK_I2C3) {
		readl(R_PRCM_CLK_GATING_REG) |= BIT(6);
	}
}

void clk_disable(uint32_t clk)
{
	if (clk & CLK_UART0) {
		readl(BUS_SOFT_RST_REG4) &= ~UART0_RST;
		readl(BUS_CLK_GATING_REG3) &= ~UART0_GATING;
	}

	if (clk & CLK_THS) {
		readl(BUS_SOFT_RST_REG3) &= ~THS_RST;
		readl(BUS_CLK_GATING_REG2) &= ~THS_GATING;
	}

	if (clk & CLK_MSGBOX) {
		readl(BUS_SOFT_RST_REG1) &= ~MSGBOX_RST;
		readl(BUS_CLK_GATING_REG1) &= ~MSGBOX_GATING;
	}
}

struct pll1_factors {
	uint8_t n;
	uint8_t k;
	uint8_t m;
	uint8_t p;
};

const struct pll1_factors pll1_factors_freq_map[] = {
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 0 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 6 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 12 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 18 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 24 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 30 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 36 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 42 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 48 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 54 => 60 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 2 }, // 60 => 60 MHz
	{ .n = 10, .k = 0, .m = 0, .p = 2 }, // 66 => 66 MHz
	{ .n = 11, .k = 0, .m = 0, .p = 2 }, // 72 => 72 MHz
	{ .n = 12, .k = 0, .m = 0, .p = 2 }, // 78 => 78 MHz
	{ .n = 13, .k = 0, .m = 0, .p = 2 }, // 84 => 84 MHz
	{ .n = 14, .k = 0, .m = 0, .p = 2 }, // 90 => 90 MHz
	{ .n = 15, .k = 0, .m = 0, .p = 2 }, // 96 => 96 MHz
	{ .n = 16, .k = 0, .m = 0, .p = 2 }, // 102 => 102 MHz
	{ .n = 17, .k = 0, .m = 0, .p = 2 }, // 108 => 108 MHz
	{ .n = 18, .k = 0, .m = 0, .p = 2 }, // 114 => 114 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 1 }, // 120 => 120 MHz
	{ .n = 10, .k = 0, .m = 0, .p = 1 }, // 126 => 132 MHz
	{ .n = 10, .k = 0, .m = 0, .p = 1 }, // 132 => 132 MHz
	{ .n = 11, .k = 0, .m = 0, .p = 1 }, // 138 => 144 MHz
	{ .n = 11, .k = 0, .m = 0, .p = 1 }, // 144 => 144 MHz
	{ .n = 12, .k = 0, .m = 0, .p = 1 }, // 150 => 156 MHz
	{ .n = 12, .k = 0, .m = 0, .p = 1 }, // 156 => 156 MHz
	{ .n = 13, .k = 0, .m = 0, .p = 1 }, // 162 => 168 MHz
	{ .n = 13, .k = 0, .m = 0, .p = 1 }, // 168 => 168 MHz
	{ .n = 14, .k = 0, .m = 0, .p = 1 }, // 174 => 180 MHz
	{ .n = 14, .k = 0, .m = 0, .p = 1 }, // 180 => 180 MHz
	{ .n = 15, .k = 0, .m = 0, .p = 1 }, // 186 => 192 MHz
	{ .n = 15, .k = 0, .m = 0, .p = 1 }, // 192 => 192 MHz
	{ .n = 16, .k = 0, .m = 0, .p = 1 }, // 198 => 204 MHz
	{ .n = 16, .k = 0, .m = 0, .p = 1 }, // 204 => 204 MHz
	{ .n = 17, .k = 0, .m = 0, .p = 1 }, // 210 => 216 MHz
	{ .n = 17, .k = 0, .m = 0, .p = 1 }, // 216 => 216 MHz
	{ .n = 18, .k = 0, .m = 0, .p = 1 }, // 222 => 228 MHz
	{ .n = 18, .k = 0, .m = 0, .p = 1 }, // 228 => 228 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 0 }, // 234 => 240 MHz
	{ .n = 9, .k = 0, .m = 0, .p = 0 }, // 240 => 240 MHz
	{ .n = 10, .k = 0, .m = 0, .p = 0 }, // 246 => 264 MHz
	{ .n = 10, .k = 0, .m = 0, .p = 0 }, // 252 => 264 MHz
	{ .n = 10, .k = 0, .m = 0, .p = 0 }, // 258 => 264 MHz
	{ .n = 10, .k = 0, .m = 0, .p = 0 }, // 264 => 264 MHz
	{ .n = 11, .k = 0, .m = 0, .p = 0 }, // 270 => 288 MHz
	{ .n = 11, .k = 0, .m = 0, .p = 0 }, // 276 => 288 MHz
	{ .n = 11, .k = 0, .m = 0, .p = 0 }, // 282 => 288 MHz
	{ .n = 11, .k = 0, .m = 0, .p = 0 }, // 288 => 288 MHz
	{ .n = 12, .k = 0, .m = 0, .p = 0 }, // 294 => 312 MHz
	{ .n = 12, .k = 0, .m = 0, .p = 0 }, // 300 => 312 MHz
	{ .n = 12, .k = 0, .m = 0, .p = 0 }, // 306 => 312 MHz
	{ .n = 12, .k = 0, .m = 0, .p = 0 }, // 312 => 312 MHz
	{ .n = 13, .k = 0, .m = 0, .p = 0 }, // 318 => 336 MHz
	{ .n = 13, .k = 0, .m = 0, .p = 0 }, // 324 => 336 MHz
	{ .n = 13, .k = 0, .m = 0, .p = 0 }, // 330 => 336 MHz
	{ .n = 13, .k = 0, .m = 0, .p = 0 }, // 336 => 336 MHz
	{ .n = 14, .k = 0, .m = 0, .p = 0 }, // 342 => 360 MHz
	{ .n = 14, .k = 0, .m = 0, .p = 0 }, // 348 => 360 MHz
	{ .n = 14, .k = 0, .m = 0, .p = 0 }, // 354 => 360 MHz
	{ .n = 14, .k = 0, .m = 0, .p = 0 }, // 360 => 360 MHz
	{ .n = 15, .k = 0, .m = 0, .p = 0 }, // 366 => 384 MHz
	{ .n = 15, .k = 0, .m = 0, .p = 0 }, // 372 => 384 MHz
	{ .n = 15, .k = 0, .m = 0, .p = 0 }, // 378 => 384 MHz
	{ .n = 15, .k = 0, .m = 0, .p = 0 }, // 384 => 384 MHz
	{ .n = 16, .k = 0, .m = 0, .p = 0 }, // 390 => 408 MHz
	{ .n = 16, .k = 0, .m = 0, .p = 0 }, // 396 => 408 MHz
	{ .n = 16, .k = 0, .m = 0, .p = 0 }, // 402 => 408 MHz
	{ .n = 16, .k = 0, .m = 0, .p = 0 }, // 408 => 408 MHz
	{ .n = 17, .k = 0, .m = 0, .p = 0 }, // 414 => 432 MHz
	{ .n = 17, .k = 0, .m = 0, .p = 0 }, // 420 => 432 MHz
	{ .n = 17, .k = 0, .m = 0, .p = 0 }, // 426 => 432 MHz
	{ .n = 17, .k = 0, .m = 0, .p = 0 }, // 432 => 432 MHz
	{ .n = 18, .k = 0, .m = 0, .p = 0 }, // 438 => 456 MHz
	{ .n = 18, .k = 0, .m = 0, .p = 0 }, // 444 => 456 MHz
	{ .n = 18, .k = 0, .m = 0, .p = 0 }, // 450 => 456 MHz
	{ .n = 18, .k = 0, .m = 0, .p = 0 }, // 456 => 456 MHz
	{ .n = 19, .k = 0, .m = 0, .p = 0 }, // 462 => 480 MHz
	{ .n = 19, .k = 0, .m = 0, .p = 0 }, // 468 => 480 MHz
	{ .n = 19, .k = 0, .m = 0, .p = 0 }, // 474 => 480 MHz
	{ .n = 19, .k = 0, .m = 0, .p = 0 }, // 480 => 480 MHz
	{ .n = 20, .k = 0, .m = 0, .p = 0 }, // 486 => 504 MHz
	{ .n = 20, .k = 0, .m = 0, .p = 0 }, // 492 => 504 MHz
	{ .n = 20, .k = 0, .m = 0, .p = 0 }, // 498 => 504 MHz
	{ .n = 20, .k = 0, .m = 0, .p = 0 }, // 504 => 504 MHz
	{ .n = 21, .k = 0, .m = 0, .p = 0 }, // 510 => 528 MHz
	{ .n = 21, .k = 0, .m = 0, .p = 0 }, // 516 => 528 MHz
	{ .n = 21, .k = 0, .m = 0, .p = 0 }, // 522 => 528 MHz
	{ .n = 21, .k = 0, .m = 0, .p = 0 }, // 528 => 528 MHz
	{ .n = 22, .k = 0, .m = 0, .p = 0 }, // 534 => 552 MHz
	{ .n = 22, .k = 0, .m = 0, .p = 0 }, // 540 => 552 MHz
	{ .n = 22, .k = 0, .m = 0, .p = 0 }, // 546 => 552 MHz
	{ .n = 22, .k = 0, .m = 0, .p = 0 }, // 552 => 552 MHz
	{ .n = 23, .k = 0, .m = 0, .p = 0 }, // 558 => 576 MHz
	{ .n = 23, .k = 0, .m = 0, .p = 0 }, // 564 => 576 MHz
	{ .n = 23, .k = 0, .m = 0, .p = 0 }, // 570 => 576 MHz
	{ .n = 23, .k = 0, .m = 0, .p = 0 }, // 576 => 576 MHz
	{ .n = 24, .k = 0, .m = 0, .p = 0 }, // 582 => 600 MHz
	{ .n = 24, .k = 0, .m = 0, .p = 0 }, // 588 => 600 MHz
	{ .n = 24, .k = 0, .m = 0, .p = 0 }, // 594 => 600 MHz
	{ .n = 24, .k = 0, .m = 0, .p = 0 }, // 600 => 600 MHz
	{ .n = 25, .k = 0, .m = 0, .p = 0 }, // 606 => 624 MHz
	{ .n = 25, .k = 0, .m = 0, .p = 0 }, // 612 => 624 MHz
	{ .n = 25, .k = 0, .m = 0, .p = 0 }, // 618 => 624 MHz
	{ .n = 25, .k = 0, .m = 0, .p = 0 }, // 624 => 624 MHz
	{ .n = 26, .k = 0, .m = 0, .p = 0 }, // 630 => 648 MHz
	{ .n = 26, .k = 0, .m = 0, .p = 0 }, // 636 => 648 MHz
	{ .n = 26, .k = 0, .m = 0, .p = 0 }, // 642 => 648 MHz
	{ .n = 26, .k = 0, .m = 0, .p = 0 }, // 648 => 648 MHz
	{ .n = 27, .k = 0, .m = 0, .p = 0 }, // 654 => 672 MHz
	{ .n = 27, .k = 0, .m = 0, .p = 0 }, // 660 => 672 MHz
	{ .n = 27, .k = 0, .m = 0, .p = 0 }, // 666 => 672 MHz
	{ .n = 27, .k = 0, .m = 0, .p = 0 }, // 672 => 672 MHz
	{ .n = 28, .k = 0, .m = 0, .p = 0 }, // 678 => 696 MHz
	{ .n = 28, .k = 0, .m = 0, .p = 0 }, // 684 => 696 MHz
	{ .n = 28, .k = 0, .m = 0, .p = 0 }, // 690 => 696 MHz
	{ .n = 28, .k = 0, .m = 0, .p = 0 }, // 696 => 696 MHz
	{ .n = 29, .k = 0, .m = 0, .p = 0 }, // 702 => 720 MHz
	{ .n = 29, .k = 0, .m = 0, .p = 0 }, // 708 => 720 MHz
	{ .n = 29, .k = 0, .m = 0, .p = 0 }, // 714 => 720 MHz
	{ .n = 29, .k = 0, .m = 0, .p = 0 }, // 720 => 720 MHz
	{ .n = 15, .k = 1, .m = 0, .p = 0 }, // 726 => 768 MHz
	{ .n = 15, .k = 1, .m = 0, .p = 0 }, // 732 => 768 MHz
	{ .n = 15, .k = 1, .m = 0, .p = 0 }, // 738 => 768 MHz
	{ .n = 15, .k = 1, .m = 0, .p = 0 }, // 744 => 768 MHz
	{ .n = 15, .k = 1, .m = 0, .p = 0 }, // 750 => 768 MHz
	{ .n = 15, .k = 1, .m = 0, .p = 0 }, // 756 => 768 MHz
	{ .n = 15, .k = 1, .m = 0, .p = 0 }, // 762 => 768 MHz
	{ .n = 15, .k = 1, .m = 0, .p = 0 }, // 768 => 768 MHz
	{ .n = 10, .k = 2, .m = 0, .p = 0 }, // 774 => 792 MHz
	{ .n = 10, .k = 2, .m = 0, .p = 0 }, // 780 => 792 MHz
	{ .n = 10, .k = 2, .m = 0, .p = 0 }, // 786 => 792 MHz
	{ .n = 10, .k = 2, .m = 0, .p = 0 }, // 792 => 792 MHz
	{ .n = 16, .k = 1, .m = 0, .p = 0 }, // 798 => 816 MHz
	{ .n = 16, .k = 1, .m = 0, .p = 0 }, // 804 => 816 MHz
	{ .n = 16, .k = 1, .m = 0, .p = 0 }, // 810 => 816 MHz
	{ .n = 16, .k = 1, .m = 0, .p = 0 }, // 816 => 816 MHz
	{ .n = 17, .k = 1, .m = 0, .p = 0 }, // 822 => 864 MHz
	{ .n = 17, .k = 1, .m = 0, .p = 0 }, // 828 => 864 MHz
	{ .n = 17, .k = 1, .m = 0, .p = 0 }, // 834 => 864 MHz
	{ .n = 17, .k = 1, .m = 0, .p = 0 }, // 840 => 864 MHz
	{ .n = 17, .k = 1, .m = 0, .p = 0 }, // 846 => 864 MHz
	{ .n = 17, .k = 1, .m = 0, .p = 0 }, // 852 => 864 MHz
	{ .n = 17, .k = 1, .m = 0, .p = 0 }, // 858 => 864 MHz
	{ .n = 17, .k = 1, .m = 0, .p = 0 }, // 864 => 864 MHz
	{ .n = 18, .k = 1, .m = 0, .p = 0 }, // 870 => 912 MHz
	{ .n = 18, .k = 1, .m = 0, .p = 0 }, // 876 => 912 MHz
	{ .n = 18, .k = 1, .m = 0, .p = 0 }, // 882 => 912 MHz
	{ .n = 18, .k = 1, .m = 0, .p = 0 }, // 888 => 912 MHz
	{ .n = 18, .k = 1, .m = 0, .p = 0 }, // 894 => 912 MHz
	{ .n = 18, .k = 1, .m = 0, .p = 0 }, // 900 => 912 MHz
	{ .n = 18, .k = 1, .m = 0, .p = 0 }, // 906 => 912 MHz
	{ .n = 18, .k = 1, .m = 0, .p = 0 }, // 912 => 912 MHz
	{ .n = 12, .k = 2, .m = 0, .p = 0 }, // 918 => 936 MHz
	{ .n = 12, .k = 2, .m = 0, .p = 0 }, // 924 => 936 MHz
	{ .n = 12, .k = 2, .m = 0, .p = 0 }, // 930 => 936 MHz
	{ .n = 12, .k = 2, .m = 0, .p = 0 }, // 936 => 936 MHz
	{ .n = 19, .k = 1, .m = 0, .p = 0 }, // 942 => 960 MHz
	{ .n = 19, .k = 1, .m = 0, .p = 0 }, // 948 => 960 MHz
	{ .n = 19, .k = 1, .m = 0, .p = 0 }, // 954 => 960 MHz
	{ .n = 19, .k = 1, .m = 0, .p = 0 }, // 960 => 960 MHz
	{ .n = 20, .k = 1, .m = 0, .p = 0 }, // 966 => 1008 MHz
	{ .n = 20, .k = 1, .m = 0, .p = 0 }, // 972 => 1008 MHz
	{ .n = 20, .k = 1, .m = 0, .p = 0 }, // 978 => 1008 MHz
	{ .n = 20, .k = 1, .m = 0, .p = 0 }, // 984 => 1008 MHz
	{ .n = 20, .k = 1, .m = 0, .p = 0 }, // 990 => 1008 MHz
	{ .n = 20, .k = 1, .m = 0, .p = 0 }, // 996 => 1008 MHz
	{ .n = 20, .k = 1, .m = 0, .p = 0 }, // 1002 => 1008 MHz
	{ .n = 20, .k = 1, .m = 0, .p = 0 }, // 1008 => 1008 MHz
	{ .n = 21, .k = 1, .m = 0, .p = 0 }, // 1014 => 1056 MHz
	{ .n = 21, .k = 1, .m = 0, .p = 0 }, // 1020 => 1056 MHz
	{ .n = 21, .k = 1, .m = 0, .p = 0 }, // 1026 => 1056 MHz
	{ .n = 21, .k = 1, .m = 0, .p = 0 }, // 1032 => 1056 MHz
	{ .n = 21, .k = 1, .m = 0, .p = 0 }, // 1038 => 1056 MHz
	{ .n = 21, .k = 1, .m = 0, .p = 0 }, // 1044 => 1056 MHz
	{ .n = 21, .k = 1, .m = 0, .p = 0 }, // 1050 => 1056 MHz
	{ .n = 21, .k = 1, .m = 0, .p = 0 }, // 1056 => 1056 MHz
	{ .n = 14, .k = 2, .m = 0, .p = 0 }, // 1062 => 1080 MHz
	{ .n = 14, .k = 2, .m = 0, .p = 0 }, // 1068 => 1080 MHz
	{ .n = 14, .k = 2, .m = 0, .p = 0 }, // 1074 => 1080 MHz
	{ .n = 14, .k = 2, .m = 0, .p = 0 }, // 1080 => 1080 MHz
	{ .n = 22, .k = 1, .m = 0, .p = 0 }, // 1086 => 1104 MHz
	{ .n = 22, .k = 1, .m = 0, .p = 0 }, // 1092 => 1104 MHz
	{ .n = 22, .k = 1, .m = 0, .p = 0 }, // 1098 => 1104 MHz
	{ .n = 22, .k = 1, .m = 0, .p = 0 }, // 1104 => 1104 MHz
	{ .n = 23, .k = 1, .m = 0, .p = 0 }, // 1110 => 1152 MHz
	{ .n = 23, .k = 1, .m = 0, .p = 0 }, // 1116 => 1152 MHz
	{ .n = 23, .k = 1, .m = 0, .p = 0 }, // 1122 => 1152 MHz
	{ .n = 23, .k = 1, .m = 0, .p = 0 }, // 1128 => 1152 MHz
	{ .n = 23, .k = 1, .m = 0, .p = 0 }, // 1134 => 1152 MHz
	{ .n = 23, .k = 1, .m = 0, .p = 0 }, // 1140 => 1152 MHz
	{ .n = 23, .k = 1, .m = 0, .p = 0 }, // 1146 => 1152 MHz
	{ .n = 23, .k = 1, .m = 0, .p = 0 }, // 1152 => 1152 MHz
	{ .n = 24, .k = 1, .m = 0, .p = 0 }, // 1158 => 1200 MHz
	{ .n = 24, .k = 1, .m = 0, .p = 0 }, // 1164 => 1200 MHz
	{ .n = 24, .k = 1, .m = 0, .p = 0 }, // 1170 => 1200 MHz
	{ .n = 24, .k = 1, .m = 0, .p = 0 }, // 1176 => 1200 MHz
	{ .n = 24, .k = 1, .m = 0, .p = 0 }, // 1182 => 1200 MHz
	{ .n = 24, .k = 1, .m = 0, .p = 0 }, // 1188 => 1200 MHz
	{ .n = 24, .k = 1, .m = 0, .p = 0 }, // 1194 => 1200 MHz
	{ .n = 24, .k = 1, .m = 0, .p = 0 }, // 1200 => 1200 MHz
	{ .n = 16, .k = 2, .m = 0, .p = 0 }, // 1206 => 1224 MHz
	{ .n = 16, .k = 2, .m = 0, .p = 0 }, // 1212 => 1224 MHz
	{ .n = 16, .k = 2, .m = 0, .p = 0 }, // 1218 => 1224 MHz
	{ .n = 16, .k = 2, .m = 0, .p = 0 }, // 1224 => 1224 MHz
	{ .n = 25, .k = 1, .m = 0, .p = 0 }, // 1230 => 1248 MHz
	{ .n = 25, .k = 1, .m = 0, .p = 0 }, // 1236 => 1248 MHz
	{ .n = 25, .k = 1, .m = 0, .p = 0 }, // 1242 => 1248 MHz
	{ .n = 25, .k = 1, .m = 0, .p = 0 }, // 1248 => 1248 MHz
	{ .n = 26, .k = 1, .m = 0, .p = 0 }, // 1254 => 1296 MHz
	{ .n = 26, .k = 1, .m = 0, .p = 0 }, // 1260 => 1296 MHz
	{ .n = 26, .k = 1, .m = 0, .p = 0 }, // 1266 => 1296 MHz
	{ .n = 26, .k = 1, .m = 0, .p = 0 }, // 1272 => 1296 MHz
	{ .n = 26, .k = 1, .m = 0, .p = 0 }, // 1278 => 1296 MHz
	{ .n = 26, .k = 1, .m = 0, .p = 0 }, // 1284 => 1296 MHz
	{ .n = 26, .k = 1, .m = 0, .p = 0 }, // 1290 => 1296 MHz
	{ .n = 26, .k = 1, .m = 0, .p = 0 }, // 1296 => 1296 MHz
	{ .n = 27, .k = 1, .m = 0, .p = 0 }, // 1302 => 1344 MHz
	{ .n = 27, .k = 1, .m = 0, .p = 0 }, // 1308 => 1344 MHz
	{ .n = 27, .k = 1, .m = 0, .p = 0 }, // 1314 => 1344 MHz
	{ .n = 27, .k = 1, .m = 0, .p = 0 }, // 1320 => 1344 MHz
	{ .n = 27, .k = 1, .m = 0, .p = 0 }, // 1326 => 1344 MHz
	{ .n = 27, .k = 1, .m = 0, .p = 0 }, // 1332 => 1344 MHz
	{ .n = 27, .k = 1, .m = 0, .p = 0 }, // 1338 => 1344 MHz
	{ .n = 27, .k = 1, .m = 0, .p = 0 }, // 1344 => 1344 MHz
	{ .n = 18, .k = 2, .m = 0, .p = 0 }, // 1350 => 1368 MHz
	{ .n = 18, .k = 2, .m = 0, .p = 0 }, // 1356 => 1368 MHz
	{ .n = 18, .k = 2, .m = 0, .p = 0 }, // 1362 => 1368 MHz
	{ .n = 18, .k = 2, .m = 0, .p = 0 }, // 1368 => 1368 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1374 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1380 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1386 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1392 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1398 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1404 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1410 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1416 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1422 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1428 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1434 => 1440 MHz
	{ .n = 19, .k = 2, .m = 0, .p = 0 }, // 1440 => 1440 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1446 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1452 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1458 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1464 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1470 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1476 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1482 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1488 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1494 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1500 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1506 => 1512 MHz
	{ .n = 20, .k = 2, .m = 0, .p = 0 }, // 1512 => 1512 MHz
	{ .n = 15, .k = 3, .m = 0, .p = 0 }, // 1518 => 1536 MHz
	{ .n = 15, .k = 3, .m = 0, .p = 0 }, // 1524 => 1536 MHz
	{ .n = 15, .k = 3, .m = 0, .p = 0 }, // 1530 => 1536 MHz
	{ .n = 15, .k = 3, .m = 0, .p = 0 }, // 1536 => 1536 MHz
	{ .n = 21, .k = 2, .m = 0, .p = 0 }, // 1542 => 1584 MHz
	{ .n = 21, .k = 2, .m = 0, .p = 0 }, // 1548 => 1584 MHz
	{ .n = 21, .k = 2, .m = 0, .p = 0 }, // 1554 => 1584 MHz
	{ .n = 21, .k = 2, .m = 0, .p = 0 }, // 1560 => 1584 MHz
	{ .n = 21, .k = 2, .m = 0, .p = 0 }, // 1566 => 1584 MHz
	{ .n = 21, .k = 2, .m = 0, .p = 0 }, // 1572 => 1584 MHz
	{ .n = 21, .k = 2, .m = 0, .p = 0 }, // 1578 => 1584 MHz
	{ .n = 21, .k = 2, .m = 0, .p = 0 }, // 1584 => 1584 MHz
	{ .n = 16, .k = 3, .m = 0, .p = 0 }, // 1590 => 1632 MHz
	{ .n = 16, .k = 3, .m = 0, .p = 0 }, // 1596 => 1632 MHz
	{ .n = 16, .k = 3, .m = 0, .p = 0 }, // 1602 => 1632 MHz
	{ .n = 16, .k = 3, .m = 0, .p = 0 }, // 1608 => 1632 MHz
	{ .n = 16, .k = 3, .m = 0, .p = 0 }, // 1614 => 1632 MHz
	{ .n = 16, .k = 3, .m = 0, .p = 0 }, // 1620 => 1632 MHz
	{ .n = 16, .k = 3, .m = 0, .p = 0 }, // 1626 => 1632 MHz
	{ .n = 16, .k = 3, .m = 0, .p = 0 }, // 1632 => 1632 MHz
	{ .n = 22, .k = 2, .m = 0, .p = 0 }, // 1638 => 1656 MHz
	{ .n = 22, .k = 2, .m = 0, .p = 0 }, // 1644 => 1656 MHz
	{ .n = 22, .k = 2, .m = 0, .p = 0 }, // 1650 => 1656 MHz
	{ .n = 22, .k = 2, .m = 0, .p = 0 }, // 1656 => 1656 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1662 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1668 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1674 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1680 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1686 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1692 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1698 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1704 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1710 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1716 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1722 => 1728 MHz
	{ .n = 23, .k = 2, .m = 0, .p = 0 }, // 1728 => 1728 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1734 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1740 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1746 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1752 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1758 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1764 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1770 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1776 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1782 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1788 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1794 => 1800 MHz
	{ .n = 24, .k = 2, .m = 0, .p = 0 }, // 1800 => 1800 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1806 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1812 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1818 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1824 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1830 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1836 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1842 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1848 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1854 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1860 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1866 => 1872 MHz
	{ .n = 25, .k = 2, .m = 0, .p = 0 }, // 1872 => 1872 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1878 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1884 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1890 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1896 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1902 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1908 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1914 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1920 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1926 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1932 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1938 => 1944 MHz
	{ .n = 26, .k = 2, .m = 0, .p = 0 }, // 1944 => 1944 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1950 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1956 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1962 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1968 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1974 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1980 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1986 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1992 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 1998 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 2004 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 2010 => 2016 MHz
	{ .n = 27, .k = 2, .m = 0, .p = 0 }, // 2016 => 2016 MHz
};

#define N_ARRAY(a) (sizeof(a) / sizeof(*a))

static void get_cpu_clk_params_table(unsigned long freq, struct pll1_factors* factors)
{
	unsigned int idx = freq / 6000000;

	if (idx > N_ARRAY(pll1_factors_freq_map))
		idx = N_ARRAY(pll1_factors_freq_map) - 1;

	*factors = *(pll1_factors_freq_map + idx);
}

static void get_cpu_clk_params_calc_good_uboot(unsigned long freq, struct pll1_factors* factors)
{
	uint8_t p = 0;
	uint8_t k = 0;
	uint8_t m = 0;

	if (freq >= 1368000000) {
		k = 2;
	} else if (freq >= 768000000) {
		k = 1;
	}

	factors->n = freq / (24000000ul * (k + 1) / (m + 1)) - 1;
	factors->k = k;
	factors->m = m;
	factors->p = p;
}

static void get_cpu_clk_params_calc_wrong_uboot(unsigned long freq, struct pll1_factors* factors)
{
	uint8_t p = 0;
	uint8_t k = 0;
	uint8_t m = 0;

	if (freq > 1152000000) {
		k = 1;
	} else if (freq > 768000000) {
		k = 2;
		m = 1;
	}

	factors->n = freq / (24000000ul * (k + 1) / (m + 1)) - 1;
	factors->k = k;
	factors->m = m;
	factors->p = p;
}

static void get_cpu_clk_params_calc_wrong_kernel(unsigned long freq, struct pll1_factors* factors)
{
	unsigned long best_rate = 0;
	unsigned long best_n = 0, best_k = 0, best_m = 0, best_p = 0;
	unsigned long _n, _k, _m, _p;

	for (_k = 1; _k <= 4; _k++) {
		for (_n = 1; _n <= 32; _n++) {
			//for (_m = 1; _m <= 4; _m++) {
			for (_m = 1; _m <= 1; _m++) {
				for (_p = 0; _p <= (freq < 288000000 ? 2 : 0); _p++) {
					unsigned long tmp_rate;

					tmp_rate = 24000000 * _n * _k / (_m * (1 << _p));

					if (tmp_rate > freq)
						continue;

					if ((freq - tmp_rate) < (freq - best_rate)) {
						best_rate = tmp_rate;
						best_n = _n;
						best_k = _k;
						best_m = _m;
						best_p = _p;
					}
				}
			}
		}
	}

	factors->n = best_n - 1;
	factors->k = best_k - 1;
	factors->m = best_m - 1;
	factors->p = best_p;
}

void clk_set_rate(uint32_t clk, uint32_t rate)
{
	uint32_t reg;

	if (clk & CLK_CPUS) {
	    if (rate > 24000000) {
            // if rate <= 432 MHz, the VDD_CPUS/VDD_RTC can be set to 1.1V
            // if rate > 432 MHz, the VDD_CPUS/VDD_RTC must be set to 1.2-1.3V
	        if ( rate > 432000000 ) {
                uint32_t reg_vdd_rtc;
                reg_vdd_rtc = readl(VDD_RTC_REG);
                // 0b100 = 1.1V, 0b101 = 1.2V, 0b110 = 1.3V
                SET_BITS_AT(reg_vdd_rtc, 3, 0, 0b101);
                writel(reg_vdd_rtc, VDD_RTC_REG);
	        }

            uint8_t N,K,M,P;

            // the PLL6 output = 24 MHz * (N+1)*(K+1)/(M+1)/(1<<P)/2
            // N: 0..31, K: 0..3, M: 0..3
            if      (rate == 300000000) { N = 24; K = 1; M = 1; P = 0; }
            else if (rate == 312000000) { N = 25; K = 1; M = 1; P = 0; }
            else if (rate == 324000000) { N = 26; K = 1; M = 1; P = 0; }
            else if (rate == 336000000) { N = 27; K = 1; M = 1; P = 0; }
            else if (rate == 348000000) { N = 28; K = 1; M = 1; P = 0; }
            else if (rate == 360000000) { N = 29; K = 1; M = 1; P = 0; }
            else if (rate == 372000000) { N = 30; K = 1; M = 1; P = 0; }
            else if (rate == 384000000) { N = 31; K = 1; M = 1; P = 0; }
            else if (rate == 396000000) { N = 21; K = 2; M = 1; P = 0; }
            else if (rate == 414000000) { N = 22; K = 2; M = 1; P = 0; }
            else if (rate == 432000000) { N = 23; K = 2; M = 1; P = 0; }
            else if (rate == 450000000) { N = 24; K = 2; M = 1; P = 0; }
            else if (rate == 468000000) { N = 25; K = 2; M = 1; P = 0; }
            else if (rate == 486000000) { N = 26; K = 2; M = 1; P = 0; }
            else if (rate == 504000000) { N = 27; K = 2; M = 1; P = 0; }
            else if (rate == 522000000) { N = 28; K = 2; M = 1; P = 0; }
            else if (rate == 540000000) { N = 29; K = 2; M = 1; P = 0; }
            else if (rate == 558000000) { N = 30; K = 2; M = 1; P = 0; }
            else if (rate == 576000000) { N = 31; K = 2; M = 1; P = 0; }

            reg = readl(PLL6_CTRL_REG);
            SET_BITS_AT(reg, 2, 0, M);
            SET_BITS_AT(reg, 2, 4, K);
            SET_BITS_AT(reg, 5, 8, N);
            SET_BITS_AT(reg, 2, 16, P);
            reg |= PLL6_CTRL_ENABLE | PLL6_CTRL_CLK_OUTEN;
            writel(reg, PLL6_CTRL_REG);

            // Switch AR100 to PLL6
            reg = readl(AR100_CLKCFG_REG);
            reg &= ~AR100_CLKCFG_SRC_MASK;
            reg |= AR100_CLKCFG_SRC_PLL6;
            reg &= ~AR100_CLKCFG_POSTDIV_MASK;
            reg |= AR100_CLKCFG_POSTDIV(1);
            reg &= ~AR100_CLKCFG_DIV_MASK;
            reg |= AR100_CLKCFG_DIV(0);
            writel(reg, AR100_CLKCFG_REG);
		} else if (rate == 32768) {
			// Switch AR100 to LOSC
			reg = readl(AR100_CLKCFG_REG);
			reg &= ~AR100_CLKCFG_SRC_MASK;
			reg |= AR100_CLKCFG_SRC_LOSC;
			writel(reg, AR100_CLKCFG_REG);
		}
	} else if (clk & CLK_UART0) {
		if (rate == 24000000) {
			writel(APB2_CLK_SRC_OSC24M |
			       APB2_CLK_RATE_N_1 |
			       APB2_CLK_RATE_M(1),
			       APB2_CFG_REG);
		}
	} else if (clk & CLK_THS) {
		if (rate == 4000000) {
			writel(THS_CLK_SCLK_GATING | THS_CLK_SRC_OSC24M | THS_CLK_DIV_RATIO_6, THS_CLK_REG);
			writel(THS_CLK_SCLK_GATING | THS_CLK_SRC_OSC24M | THS_CLK_DIV_RATIO_1, THS_CLK_REG);
		}
	} else if (clk & CLK_CPUX_WRONG) {
		struct pll1_factors f;

		if (rate < 60000000) {
			rate = 60000000;
		}
		
		get_cpu_clk_params_calc_wrong_kernel(rate, &f);
		//get_cpu_clk_params_calc_wrong_uboot(rate, &f);
		//get_cpu_clk_params_calc_good_uboot(rate, &f);
		//get_cpu_clk_params_table(rate, &f);

		uint32_t p = f.p, m = f.m, k = f.k, n = f.n;
		uint32_t real_rate = 24000000;
		real_rate *= n + 1;
		real_rate *= k + 1;
		real_rate /= 1 << p;
		real_rate /= m + 1;
		printf("N=%u K=%u M=%u P=%u   rate=%u => %u MHz\n", n, k, m, p, rate / 1000000, real_rate / 1000000);

//#define BYPASS
#ifdef BYPASS
		reg = readl(CPUX_AXI_CFG_REG);
		SET_BITS_AT(reg, 2, 16, 1);
		writel(reg, CPUX_AXI_CFG_REG);

		delay_us(100);
#endif

		reg = readl(PLL_CPUX_CTRL_REG);

		SET_BITS_AT(reg, 2, 16, p);
		SET_BITS_AT(reg, 2, 0, m);
		SET_BITS_AT(reg, 5, 8, n);
		SET_BITS_AT(reg, 2, 4, k);

		writel(reg, PLL_CPUX_CTRL_REG);

		delay_us(1);

		while (!(readl(PLL_CPUX_CTRL_REG) & PLL_CPUX_LOCK));

#ifdef BYPASS
		delay_us(100);

		reg = readl(CPUX_AXI_CFG_REG);
		SET_BITS_AT(reg, 2, 16, 2);
		writel(reg, CPUX_AXI_CFG_REG);
#endif
	} else if (clk & CLK_CPUX) {
		struct pll1_factors f;

		if (rate < 60000000) {
			rate = 60000000;
		}

		get_cpu_clk_params_table(rate, &f);
		//get_cpu_clk_params_calc_wrong_kernel(rate, &f);

		uint32_t p = f.p, m = f.m, k = f.k, n = f.n;
		uint32_t real_rate = 24000000;
		real_rate *= n + 1;
		real_rate *= k + 1;
		real_rate /= 1 << p;
		real_rate /= m + 1;
		printf("N=%u K=%u M=%u P=%u   rate=%u => %u MHz\n", n, k, m, p, rate / 1000000, real_rate / 1000000);

		reg = readl(PLL_CPUX_CTRL_REG);

		if (GET_BITS_AT(reg, 2, 16) < p) {
			SET_BITS_AT(reg, 2, 16, p);

			writel(reg, PLL_CPUX_CTRL_REG);
			delay_us(200);
		}

		if (GET_BITS_AT(reg, 2, 0) < m) {
			SET_BITS_AT(reg, 2, 0, m);

			writel(reg, PLL_CPUX_CTRL_REG);
			delay_us(200);
		}

		SET_BITS_AT(reg, 5, 8, n);
		SET_BITS_AT(reg, 2, 4, k);

		writel(reg, PLL_CPUX_CTRL_REG);
		delay_us(100);

		while (!(readl(PLL_CPUX_CTRL_REG) & PLL_CPUX_LOCK));

		if (GET_BITS_AT(reg, 2, 0) > m) {
			SET_BITS_AT(reg, 2, 0, m);

			writel(reg, PLL_CPUX_CTRL_REG);
			delay_us(200);
		}

		if (GET_BITS_AT(reg, 2, 16) > p) {
			SET_BITS_AT(reg, 2, 16, p);

			writel(reg, PLL_CPUX_CTRL_REG);
			delay_us(200);
		}
	}
}

uint32_t clk_get_rate(uint32_t clk)
{
	uint32_t rate, reg;

	if (clk & CLK_CPUX) {
		reg = readl(PLL_CPUX_CTRL_REG);

		uint32_t p = GET_BITS_AT(reg, 2, 16),
			 m = GET_BITS_AT(reg, 2, 0),
			 n = GET_BITS_AT(reg, 5, 8),
                         k = GET_BITS_AT(reg, 2, 4);

		rate = 24000000;
		rate *= n + 1;
		rate *= k + 1;
		rate /= 1 << p;
		rate /= m + 1;

		return rate;
	}

	return 0;
}
