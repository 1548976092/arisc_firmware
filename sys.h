#ifndef _SYS_H
#define _SYS_H

#include <stdint.h>




#define CPU_FREQ 450000000 // Hz




/* ccm */
#define CCM_BASE    0x01c20000
#define R_PRCM_BASE     0x01f01400

/* clock gating */
#define BUS_CLK_GATING_REG0         (CCM_BASE + 0x060)
#define BUS_CLK_GATING_REG1         (CCM_BASE + 0x064)
#define MSGBOX_GATING           BIT(21)
#define BUS_CLK_GATING_REG2         (CCM_BASE + 0x068)
#define PIO_GATING          BIT(5)
#define THS_GATING          BIT(8)
#define BUS_CLK_GATING_REG3         (CCM_BASE + 0x06c)
#define UART0_GATING            BIT(16)
#define I2C0_GATING             BIT(0)
#define I2C1_GATING             BIT(1)
#define I2C2_GATING             BIT(2)
#define BUS_CLK_GATING_REG4         (CCM_BASE + 0x070)

/* soft reset */
#define BUS_SOFT_RST_REG0       (CCM_BASE + 0x2c0)
#define BUS_SOFT_RST_REG1       (CCM_BASE + 0x2c4)
#define MSGBOX_RST          BIT(21)
#define BUS_SOFT_RST_REG2       (CCM_BASE + 0x2c8)
#define BUS_SOFT_RST_REG3       (CCM_BASE + 0x2d0)
#define THS_RST             BIT(8)
#define BUS_SOFT_RST_REG4       (CCM_BASE + 0x2d8)
#define UART0_RST           BIT(16)
#define I2C0_RST            BIT(0)
#define I2C1_RST            BIT(1)
#define I2C2_RST            BIT(2)

/* apb2 */
#define APB2_CFG_REG            (CCM_BASE + 0x058)
#define APB2_CLK_SRC_LOSC       (0x0 << 24)
#define APB2_CLK_SRC_OSC24M     (0x1 << 24)
#define APB2_CLK_SRC_PLL6       (0x2 << 24)
#define APB2_CLK_SRC_MASK       (0x3 << 24)
#define APB2_CLK_RATE_N_1       (0x0 << 16)
#define APB2_CLK_RATE_N_2       (0x1 << 16)
#define APB2_CLK_RATE_N_4       (0x2 << 16)
#define APB2_CLK_RATE_N_8       (0x3 << 16)
#define APB2_CLK_RATE_N_MASK        (3 << 16)
#define APB2_CLK_RATE_M(m)      (((m)-1) << 0)
#define APB2_CLK_RATE_M_MASK            (0x1f << 0)

/* pll6 */
#define PLL6_CTRL_REG           (CCM_BASE + 0x0028)
#define PLL6_CTRL_ENABLE        BIT(31)
#define PLL6_CTRL_LOCK          BIT(28)
#define PLL6_CTRL_BYPASS        BIT(25)
#define PLL6_CTRL_CLK_OUTEN     BIT(24)
#define PLL6_CTRL_24M_OUTEN     BIT(18)

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
#define AR100_CLKCFG_REG        (R_PRCM_BASE + 0x000)
#define AR100_CLKCFG_SRC_LOSC       (0 << 16)
#define AR100_CLKCFG_SRC_HOSC       (1 << 16)
#define AR100_CLKCFG_SRC_PLL6       (2 << 16)
#define AR100_CLKCFG_SRC_MASK       (0x3 << 16)
#define AR100_CLKCFG_POSTDIV(x)     (((x) & 0x1f) << 8)
#define AR100_CLKCFG_POSTDIV_MASK   (0x1f << 8)
#define AR100_CLKCFG_DIV(x)     (((x) & 0x3) << 4)
#define AR100_CLKCFG_DIV_MASK       (0x3 << 4)

#define R_PRCM_CLK_GATING_REG       (R_PRCM_BASE + 0x28) // ???
#define R_PIO_GATING            BIT(0)

// ARISC/CPUS and RTC power regulation
#define VDD_RTC_REG 0x01f00190




void enable_caches(void);
void reset(void);
void handle_exception(uint32_t type, uint32_t pc, uint32_t sp);
void clk_set_rate(uint32_t rate);




#endif
