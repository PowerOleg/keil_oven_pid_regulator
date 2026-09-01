#ifndef __HEATER_H
#define __HEATER_H

#include "stm32f10x.h"

float Max6675_get_temperature_c(void);
void Heater_init(void);
void Heater_on(float setpoint, float temperature_c);
void Heater_average_filter(float *result_measure);
void Set_pwm_duty(uint8_t percent);
void Heater_restart(void);

#endif
