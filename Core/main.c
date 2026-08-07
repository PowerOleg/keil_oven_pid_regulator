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
char buffer_uart[UART_BUFFER_SIZE] = {0};// Буфер для отправки лога на ПК

Led led_a8;
Led led_c13;


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
				Delay_us(1000000);
float temperature_c = Max6675_get_temperature_c();
				if (tim3_1sec_flag)
				{
						Led_toggle(&led_c13);
						tim3_1sec_flag = 0;
				}
					
				pressed_key = Check_keypad_pressed();
				if (pressed_key != NO_KEY)
						cur_action = pressed_key;

				switch (cur_action)
				{
						case TIME:

								break;
						case TEMPERATURE:
								OLED_ClearBuffer();
								OLED_PrintScaledSymbols(10, 0, font_table, temperature_indices, 8, 2);
//								OLED_PrintTemperature(10, 30, bmp280_result.temperature_c, 3, font_table);
								OLED_UpdateScreen();
								break;
						case HUMIDITY:

								break;
						case PRESSURE:

								break;
						case MIN_MAX_LOG:

								break;
						case PAGE_UP:

								break;
						case PAGE_DOWN:

								break;
						case DELETE_LOG:

								break;
						case SEND_DATA_TO_PC:
						{
								uint16_t buffer_size = 0;//Get_uart_buffer(buffer_uart);
								if (buffer_size > 0)
								{
										Uart2_send_string(buffer_uart, buffer_size);
										while(transmission_in_progress) {}
										OLED_ClearBuffer();
										OLED_PrintScaledSymbols(10, 0, font_table, sent_indices, 8, 2);
										OLED_UpdateScreen();
										Delay_us(1000000);
								}
								cur_action = previous_action;
								break;
						}
						case SET_TIME:
						{

								break;
						}
				}
				previous_action = cur_action;
		}
	
}
