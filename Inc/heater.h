#ifndef __HEATER_H
#define __HEATER_H

#include "stm32f10x.h"

float Max6675_get_temperature_c(void);
void Heater_init(void);
//void Heater_on(void);
//void Heater_off(void);
void Set_pwm_duty(uint8_t percent);

#endif
