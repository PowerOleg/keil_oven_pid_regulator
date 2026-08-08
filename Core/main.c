#include <string.h>
#include <stdio.h>
#include "common.h"
#include "hw_config.h"
#include "led.h"
#include "uart.h"
#include "oled_display.h"
#include "timer.h"
#include "keypad4x4.h"
#include "definitions.h"
#include "max6675.h"

volatile uint8_t cur_action = 0;
volatile uint8_t previous_action = 0;

volatile uint8_t pressed_key = 0;
char display_buffer[DISPLAY_LINE_SIZE] = {0};
char buffer_uart[UART_BUFFER_SIZE] = {0};// Буфер для отправки данных на ПК

Led led_a8;
Led led_c13;

float temperature_c_previous = 0.0f;

int main(void)
{
		if (Clock_config_72mhz() == ERROR)
				Error_handler();

		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1), ENABLE);
		RCC_APB1PeriphClockCmd((RCC_APB1Periph_USART2 | RCC_APB1Periph_PWR | RCC_APB1Periph_BKP), ENABLE);
		Led_init(&led_a8, GPIOA, GPIO_Pin_8);
		Led_init(&led_c13, GPIOC, GPIO_Pin_13);
		Tim3_init_1sec_timer();
		Init_systick_us();
		Tim2_count_mode_up();
		
		Keypad_init_gpio();
		Uart2_init();

	  SPI1_common_gpio_init();
		Delay_us(100000);
		SPI1_common_init();

		Delay_us(100000);
		Oled_gpio_init();
		Delay_us(100000);
		OLED_Init_SSD1306();
		Delay_us(100000);
    OLED_ClearBuffer();
	
		SPI_clear_rxne();
				
		Led_toggle(&led_a8);
		Delay_us(500000);
		Led_toggle(&led_c13);
		Led_toggle(&led_a8);

		while(1)
		{
				Delay_us(20000);

				if (tim3_1sec_flag)
				{
						Led_toggle(&led_c13);
						float temperature_c = Max6675_get_temperature_c();
						if (temperature_c != temperature_c_previous)
						{
								temperature_c_previous = temperature_c;
								OLED_ClearBuffer();
								OLED_PrintScaledSymbols(25, 8, font_table, temperature_full_indices, 11, 1);
								OLED_PrintScaledSymbols(0, 20, font_table, ustavka_indices, 8, 1);
								OLED_PrintTemperature(0, 30, temperature_c, 1, font_table);
								OLED_PrintScaledSymbols(0, 40, font_table, current_value_indices, 12, 1);
								OLED_PrintTemperature(0, 50, temperature_c, 1, font_table);
								OLED_UpdateScreen();
						}
					
						tim3_1sec_flag = 0;
				}
					
				pressed_key = Check_keypad_pressed();
				if (pressed_key != NO_KEY)
						cur_action = pressed_key;

				
				switch (cur_action)
				{
						case LEFT:

								break;
						case RIGHT:

								break;
						case START:

								break;
						case STOP:

								break;
						case SEND_DATA_TO_PC:
						{
								/*uint16_t buffer_size = 0;//Get_uart_buffer(buffer_uart);
								if (buffer_size > 0)
								{
										Uart2_send_string(buffer_uart, buffer_size);
										while(transmission_in_progress) {}
										OLED_ClearBuffer();
										OLED_PrintScaledSymbols(10, 0, font_table, sent_indices, 8, 2);
										OLED_UpdateScreen();
										Delay_us(1000000);
								}
								cur_action = previous_action;*/
								break;
						}

				}
				previous_action = cur_action;
		}
	
}
