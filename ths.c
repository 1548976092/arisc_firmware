#include "io.h"
#include "clk.h"
#include "ths.h"

#define THS_BASE		0x01c25000

#define THS_H3_CTRL0		(THS_BASE + 0x00)
#define THS_H3_CTRL2		(THS_BASE + 0x40)
#define THS_H3_INT_CTRL		(THS_BASE + 0x44)
#define THS_H3_STAT		(THS_BASE + 0x48)
#define THS_H3_FILTER		(THS_BASE + 0x70)
#define THS_H3_CDATA		(THS_BASE + 0x74)
#define THS_H3_DATA		(THS_BASE + 0x80)

#define THS_H3_CTRL0_SENSOR_ACQ0(x)     (x)
#define THS_H3_CTRL2_SENSE_EN           BIT(0)
#define THS_H3_CTRL2_SENSOR_ACQ1(x)     ((x) << 16)
#define THS_H3_INT_CTRL_DATA_IRQ_EN     BIT(8)
#define THS_H3_INT_CTRL_THERMAL_PER(x)  ((x) << 12)
#define THS_H3_STAT_DATA_IRQ_STS        BIT(8)
#define THS_H3_FILTER_TYPE(x)           ((x) << 0)
#define THS_H3_FILTER_EN                BIT(2)

#define THS_H3_CLK_IN 40000000ull  /* Hz */
#define THS_H3_DATA_PERIOD 200  /* ms */

// n = clk / 4096 - 1

#define THS_H3_FILTER_TYPE_VALUE		1  /* average over 2^(n+1) samples */
#define THS_H3_FILTER_DIV 			(1 << (THS_H3_FILTER_TYPE_VALUE + 1))
#define THS_H3_INT_CTRL_THERMAL_PER_VALUE 	(THS_H3_DATA_PERIOD * THS_H3_CLK_IN / 4096 / 1000 / THS_H3_FILTER_DIV - 1)
#define THS_H3_CTRL0_SENSOR_ACQ0_VALUE		(16000 / 250) /* us */
#define THS_H3_CTRL2_SENSOR_ACQ1_VALUE		(16000 / 250) /* us */

// 250ns * ACQ0 + 3500ns

#define SID_BASE 		0x01c14000
#define SID_CAL_DATA 		(SID_BASE + 0x234)

void ths_init(void)
{
	clk_set_rate(CLK_THS, 4000000);
	clk_disable(CLK_THS);
	clk_enable(CLK_THS);

	uint32_t cal_data = readl(SID_CAL_DATA);
	cal_data &= 0xfff;
	if (cal_data)
		writel(cal_data, THS_H3_CDATA);

	writel(THS_H3_CTRL0_SENSOR_ACQ0(THS_H3_CTRL0_SENSOR_ACQ0_VALUE), THS_H3_CTRL0);
	writel(THS_H3_FILTER_EN | THS_H3_FILTER_TYPE(THS_H3_FILTER_TYPE_VALUE), THS_H3_FILTER);
	writel(THS_H3_CTRL2_SENSOR_ACQ1(THS_H3_CTRL2_SENSOR_ACQ1_VALUE) | THS_H3_CTRL2_SENSE_EN, THS_H3_CTRL2);
	writel(THS_H3_INT_CTRL_THERMAL_PER(THS_H3_INT_CTRL_THERMAL_PER_VALUE) | THS_H3_INT_CTRL_DATA_IRQ_EN, THS_H3_INT_CTRL);

//	writel(THS_H3_STAT_DATA_IRQ_STS, data->regs + THS_H3_STAT);
	//data->temp = ;
}

int ths_get_temp(void)
{
	uint32_t temp = readl(THS_H3_DATA);

	if (temp == 0)
		return 0;

	return 217000 - ((int)temp * 121);
}
