#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

#define UART_BUFFER_SIZE 10
extern uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
extern volatile uint8_t receive_flag;

void Uart2_init(void);
void Uart2_receive_string(void);
void Uart2_send(char *str);

#endif
