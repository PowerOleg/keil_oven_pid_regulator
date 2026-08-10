#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"

extern volatile uint32_t timer2_cur_time_ms; 
void Tim2_count_mode_up(void);

extern volatile uint8_t tim3_1sec_flag;
void Tim3_init_1sec_timer(void);

void GPIO_Init_PB9_AF(void);
void Tim4_init_pwm(void);
void Set_pwm_duty(uint8_t percent);

#endif
