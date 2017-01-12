#ifndef __REGULATOR_H__
#define __REGULATOR_H__

void regulator_init(void);
int regulator_set_voltage(uint32_t voltage);
int regulator_get_voltage(uint32_t *voltage);

#endif
