#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

Led led_a8;
Led led_c13;

volatile uint8_t is_stop = 1;
volatile uint8_t is_set_up_temperature = 1;
volatile uint8_t is_pc_connected = 0;

const uint8_t setpoint_size = 7;
uint8_t setpoint_array[setpoint_size] = {13, 0, 0, 16, 0, 17, 19};//000.0
volatile float setpoint = 0.0f;
volatile float temperature_c_previous = 0.0f;


float Convert_setpoint_to_float(const uint8_t* setpoint_array)
{
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

void Manage_from_mk(const uint8_t action)
{
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
						if (is_stop)
						{
								setpoint = Convert_setpoint_to_float(setpoint_array);
								if (setpoint > 0 && setpoint < 1000.0f)
								{
										is_stop = 0;
										is_set_up_temperature = 0;
										Heater_restart();
								}
						}
						break;
				case STOP:
						is_stop = 1;
						Set_pwm_duty(0);
						break;
		}
		is_pc_connected = 0;
}

void Manage_from_pc(const float temperature_c)
{
		is_pc_connected = 1;
		if (tim3_2sec_flag)
		{
				static char send_buf[UART_BUFFER_SIZE];
				snprintf(send_buf, sizeof(send_buf), "%.1f", temperature_c);
				Uart2_send(send_buf);
		}

		Uart2_receive_string();
		if (receive_flag)
		{
				if (uart_rx_buffer[0] == 0 || uart_rx_buffer[0] == 0x2D)
				{
						is_stop = 1;
						Set_pwm_duty(0);
				}
				else
				{
						setpoint = strtof((char *)uart_rx_buffer, NULL);
						if (setpoint > 0 && setpoint < 1000.0f)
						{
								is_stop = 0;
								is_set_up_temperature = 0;
								Heater_restart();
						}
				}
				receive_flag = 0;
		}
}

int main(void)
{
		if (Clock_config_72mhz() == ERROR)
				Error_handler();

		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1), ENABLE);
//		RCC_APB1PeriphClockCmd((RCC_APB1Periph_USART2 | RCC_APB1Periph_PWR | RCC_APB1Periph_BKP), ENABLE);
		Led_init(&led_a8, GPIOA, GPIO_Pin_8);
		Led_init(&led_c13, GPIOC, GPIO_Pin_13);
		Tim3_init_2sec_timer();
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
		float temperature_c = 0.0f;
		
		while(1)
		{
				Delay_us(20000);
				pressed_key = Check_keypad_pressed();
				if (pressed_key != NO_KEY)
						cur_action = pressed_key;
			
				if (is_set_up_temperature)
						Set_up_setpoint(cur_action);
			
				if (cur_action != CONNECT_TO_PC)
						Manage_from_mk(cur_action);
				else
						Manage_from_pc(temperature_c);
				
				if (tim3_2sec_flag)
				{
						Led_toggle(&led_c13);
						temperature_c = Max6675_get_temperature_c();
						if (temperature_c != temperature_c_previous)
						{
								Heater_average_filter(&temperature_c);
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
					
						if (is_pc_connected)
						{
								OLED_PrintScaledSymbols(110, 0, font_table, pc_indices, 2, 1);
								OLED_UpdateScreen();
						}
						
						if (!is_stop)
								Heater_on(setpoint, temperature_c);
						
						tim3_2sec_flag = 0;
				}
				
		}
}
