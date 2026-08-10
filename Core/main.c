#include <string.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "hw_config.h"
#include "led.h"
#include "uart.h"
#include "oled_display.h"
#include "timer.h"
#include "keypad4x4.h"
#include "definitions.h"
#include "heater.h"

volatile uint8_t cur_action = 0;
volatile uint8_t previous_action = 0;

volatile uint8_t pressed_key = 0;
char display_buffer[DISPLAY_LINE_SIZE] = {0};
char buffer_uart[UART_BUFFER_SIZE] = {0};// Буфер для отправки данных на ПК

Led led_a8;
Led led_c13;

uint8_t is_stop = 1;
uint8_t is_set_up_temperature = 1;

const uint8_t setpoint_size = 7;
uint8_t setpoint_array[setpoint_size] = {13, 0, 0, 16, 0, 17, 19};//000.0
float setpoint = 0.0f;
float temperature_c_previous = 0.0f;



float Convert_setpoint_to_float(const uint8_t* setpoint_array)
{
/*		float result = 0.0f;
		for (uint8_t i = 0, j = 2; i < 5; i++, j--)
		{
				if (setpoint_array[i] == 13 || setpoint_array[i] == 16)//убераем подчеркивание и точку из расчетов
						continue;
				if (i == 4)
				{
						result += setpoint_array[i] * 0.1f;
						continue;
				}
				uint16_t n = (uint16_t)(pow(10, j) + 0.5);
				result += setpoint_array[i] * n;
		}
*/
		return ((setpoint_array[0] * 100) + (setpoint_array[1] * 10) + setpoint_array[2] + (setpoint_array[4] * 0.1f));
}
	
void Set_up_setpoint(const uint8_t key)
{
		if (key == 0x00 || key == 0xFF || key == START || key == STOP || key == SET_UP_TEMPERATURE || key == CONNECT_TO_PC)
				return;

		setpoint_array[symbol_index++] = key - '0';
		if (symbol_index == 3)
				symbol_index++;
		cur_action = 0xFF;
		if (symbol_index >= 5)
		{
				is_set_up_temperature = 0;
				return;
		}
		setpoint_array[symbol_index] = 13;
}

int main(void)
{
		if (Clock_config_72mhz() == ERROR)
				Error_handler();

		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1), ENABLE);
//		RCC_APB1PeriphClockCmd((RCC_APB1Periph_USART2 | RCC_APB1Periph_PWR | RCC_APB1Periph_BKP), ENABLE);
		Led_init(&led_a8, GPIOA, GPIO_Pin_8);
		Led_init(&led_c13, GPIOC, GPIO_Pin_13);
		Tim3_init_1sec_timer();
		Init_systick_us();
		
		Tim2_count_mode_up();
		Keypad_init_gpio();//Tim2
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
		Heater_init();
		Led_toggle(&led_a8);
		Delay_us(500000);
		Led_toggle(&led_c13);
		Led_toggle(&led_a8);
		

		while(1)
		{
				Delay_us(20000);
				pressed_key = Check_keypad_pressed();
				if (pressed_key != NO_KEY)
						cur_action = pressed_key;
			
				if (is_set_up_temperature)
						Set_up_setpoint(cur_action);
			
				if (tim3_1sec_flag)
				{
						Led_toggle(&led_c13);
						float temperature_c = Max6675_get_temperature_c();
						if (temperature_c != temperature_c_previous)
						{
								temperature_c_previous = temperature_c;
								OLED_ClearBuffer();
								OLED_PrintScaledSymbols(25, 0, font_table, temperature_full_indices, 11, 1);
								OLED_PrintScaledSymbols(0, 10, font_table, ustavka_indices, 8, 1);
								OLED_PrintScaledSymbols(0, 20, font_table, setpoint_array, setpoint_size, 1);
								OLED_PrintScaledSymbols(0, 30, font_table, current_value_indices, 12, 1);
								OLED_PrintTemperature(0, 40, temperature_c, 1, font_table);
								if (is_stop)
										OLED_PrintScaledSymbols(0, 54, font_table, stop_indices, 18, 1);
								else
										OLED_PrintScaledSymbols(0, 54, font_table, start_indices, 12, 1);
								OLED_UpdateScreen();
						}
					
						tim3_1sec_flag = 0;
				}
				
				switch (cur_action)
				{
						case SET_UP_TEMPERATURE:
								is_stop = 1;
								is_set_up_temperature = 1;
								symbol_index = 0;
								setpoint_array[symbol_index] = 13;
								cur_action = STOP;
								break;
						case START:
								is_stop = 0;
								is_set_up_temperature = 0;
								setpoint = Convert_setpoint_to_float(setpoint_array);
								Set_pwm_duty(100);
								break;
						case STOP:
								is_stop = 1;
								Set_pwm_duty(0);
								break;
						case CONNECT_TO_PC:
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
//				previous_action = cur_action;
		}
	
}
