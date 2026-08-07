#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

#include "stm32f10x.h"

void SPI1_gpio_init(void);
void SPI1_common_init(void);
void SPI1_init(void);
void SPI1_common_gpio_init(void);
void SPI_clear_rxne(void);

uint8_t Clock_config_72mhz(void);
void Init_systick_us(void);
void Init_pina9_button(void);
void Init_pinb10_button(void);
void Error_handler(void);

#endif
