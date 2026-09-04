#include <string.h>
#include "common.h"
#include "uart.h"
#include "misc.h"
#include "stm32f10x_usart.h"
#include "timer.h"

#define TIMEOUT200_MS 200

volatile uint16_t tx_remaining_bytes = 0;
volatile uint8_t *tx_buffer_ptr = NULL;
volatile uint8_t transmission_in_progress = 0;

uint8_t uart_rx_buffer[UART_BUFFER_SIZE] = {0};
volatile uint8_t receive_flag = 0;
uint16_t buffer_index = 0;
volatile uint32_t receive_count = 0;
volatile uint32_t lastByteReceivedTime = 0;

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        uint8_t receivedChar = USART_ReceiveData(USART2);
        
				if (buffer_index < UART_BUFFER_SIZE - 1) 
				{
						uart_rx_buffer[buffer_index++] = receivedChar;
				}
			
				lastByteReceivedTime = timer2_cur_time_ms;
				if (buffer_index >= UART_BUFFER_SIZE - 1)
						buffer_index = UART_BUFFER_SIZE - 1;
    }
}

void Uart2_receive_string(void)
{
    if (buffer_index > 0 && ((timer2_cur_time_ms - lastByteReceivedTime) >= TIMEOUT200_MS))
    {
				uint16_t messageLength = buffer_index;
        buffer_index = 0; // Очищаем индекс для нового приема
        uart_rx_buffer[messageLength] = '\0'; // Завершаем строку
				receive_flag = 1;
    }
}

void Uart2_init(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		USART_DeInit(USART2);
		
		GPIO_InitTypeDef gpio_init_struct;
		gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
		gpio_init_struct.GPIO_Pin = GPIO_Pin_2;//к RX USB TTL
		gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio_init_struct);

		gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpio_init_struct.GPIO_Pin = GPIO_Pin_3;//к TX USB-TTL
		GPIO_Init(GPIOA, &gpio_init_struct);
		
		USART_InitTypeDef uart_init = {0};
		uart_init.USART_BaudRate = 9600;
		uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		uart_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
		uart_init.USART_Parity = USART_Parity_No;
		uart_init.USART_StopBits = USART_StopBits_1;
		uart_init.USART_WordLength = USART_WordLength_8b;
		USART_Init(USART2, &uart_init);
		
		NVIC_InitTypeDef nvic_init = {0};
		nvic_init.NVIC_IRQChannel = USART2_IRQn;
		nvic_init.NVIC_IRQChannelPreemptionPriority = NVIC_UART_PRIORITY;
		nvic_init.NVIC_IRQChannelSubPriority = 0;
		nvic_init.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic_init);
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		USART_Cmd(USART2, ENABLE);
}

void Uart2_send(char *str)
{
		while (*str)
		{
			while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}
			USART_SendData(USART2, *str++);
		}
}
