#ifndef _STEPGEN_H
#define _STEPGEN_H




#define STEPGEN_CH_CNT 8




void stepgen_ch_set_step_pin(uint8_t id, uint8_t port, uint8_t pin);
void stepgen_ch_set_task(uint8_t id, uint32_t freq, uint32_t steps);
void stepgen_base_thread();




#endif
