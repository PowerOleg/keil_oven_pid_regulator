#include <string.h>
#include "common.h"
#include "hw_config.h"
#include "stm32f10x_flash.h"
#include "misc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_i2c.h"


uint8_t Clock_config_72mhz(void)
{
		ErrorStatus HSEStartUpStatus;
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
	
		if (HSEStartUpStatus == SUCCESS)
		{
				FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
				FLASH_SetLatency(FLASH_Latency_2);
					
				RCC_HCLKConfig(RCC_SYSCLK_Div1);
				RCC_PCLK1Config(RCC_HCLK_Div2); // APB1: 36MHz (для TIM2-4)
				RCC_PCLK2Config(RCC_HCLK_Div1); // APB2: 72MHz (для GPIO, USART1, TIM1)
				 
				RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // 8MHz * 9 = 72MHz
				RCC_PLLCmd(ENABLE);
				while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
				RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
				while(RCC_GetSYSCLKSource() != 0x08); // Ждем, пока PLL станет источником SYSCLK
			
				return SUCCESS;
		}
		return ERROR;
}

void SPI1_common_init(void)
{
    // P.S. Тактирование уже должно быть включено
    SPI_InitTypeDef spi;
    SPI_StructInit(&spi);
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_Low;          // полярность CK = 0, когда CS=1
    spi.SPI_CPHA = SPI_CPHA_1Edge;        // захват по первому фронту
    spi.SPI_NSS = SPI_NSS_Soft;           // управление CS программное
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // частота <=10 МГц
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &spi);
    SPI_Cmd(SPI1, ENABLE);
}


void SPI1_common_gpio_init(void)
{
    // Настройка GPIO для SPI1
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    // SCK (PA5), MOSI (PA7) - выход push-pull alternate function
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);

    // MISO (PA6) - вход floating (если используете прием)
    gpio.GPIO_Pin = GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);
	
    gpio.GPIO_Pin = GPIO_Pin_4;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
}

//to clear any transmitted data of last transmission
void SPI_clear_rxne(void)
{
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == SET)
    SPI_I2S_ReceiveData(SPI1);
		SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_OVR);
}

void SPI1_init(void)
{
    // Инициализация SPI1
    SPI_InitTypeDef spi;
    SPI_StructInit(&spi);
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_Low;          // полярность CK = 0, когда CS=1
    spi.SPI_CPHA = SPI_CPHA_1Edge;        // захват по первому фронту
    spi.SPI_NSS = SPI_NSS_Soft;           // управление CS программное
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; // частота <=10 МГц//SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // Частота около 18 MHz
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &spi);
    SPI_Cmd(SPI1, ENABLE);
}

void SPI1_gpio_init(void)
{
    // Настройка GPIO для SPI1
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    // SCK (PA5), MOSI (PA7) - выход push-pull alternate function
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);

    // MISO (PA6) - вход floating
    gpio.GPIO_Pin = GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    // CSB (PA4) - выход push-pull, начально высокий уровень
    gpio.GPIO_Pin = GPIO_Pin_4;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &gpio);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);  // CS = 1 (неактивен)
}

void Init_systick_us(void)
{
	SysTick->LOAD = 72 - 1;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	NVIC_SetPriority(SysTick_IRQn, NVIC_SYSTICK_PRIORITY); 
  NVIC_EnableIRQ(SysTick_IRQn);
}
/*
void Init_pina9_button(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
		GPIO_InitTypeDef pina9 = {0};
		pina9.GPIO_Pin = GPIO_Pin_9;
		pina9.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOA, &pina9); 
		
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource9);// Подключаем линию EXTI9 к порту PA9
		
		EXTI_InitTypeDef exti_line9;
			exti_line9.EXTI_Line = EXTI_Line9; // Указываем конкретную линию
			exti_line9.EXTI_Mode = EXTI_Mode_Interrupt;
			exti_line9.EXTI_Trigger = EXTI_Trigger_Falling; // Срабатываем по нажатию (0V)
			exti_line9.EXTI_LineCmd = ENABLE;
			EXTI_Init(&exti_line9);
		
		NVIC_InitTypeDef nvic_button_9;
			nvic_button_9.NVIC_IRQChannel = EXTI9_5_IRQn;
			nvic_button_9.NVIC_IRQChannelPreemptionPriority = NVIC_BUTTON_PRIORITY;
			nvic_button_9.NVIC_IRQChannelSubPriority = 0;
			nvic_button_9.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&nvic_button_9);
}*/

void Init_pinb10_button(void)
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
		GPIO_InitTypeDef pinb10 = {0};
		pinb10.GPIO_Pin = GPIO_Pin_10;
		pinb10.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOB, &pinb10); 
		
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
		
		EXTI_InitTypeDef exti_line10;
			exti_line10.EXTI_Line = EXTI_Line10;
			exti_line10.EXTI_Mode = EXTI_Mode_Interrupt;
			exti_line10.EXTI_Trigger = EXTI_Trigger_Falling;
			exti_line10.EXTI_LineCmd = ENABLE;
			EXTI_Init(&exti_line10);
		
		NVIC_InitTypeDef nvic_button_10;
			nvic_button_10.NVIC_IRQChannel = EXTI15_10_IRQn;
			nvic_button_10.NVIC_IRQChannelPreemptionPriority = NVIC_BUTTON_PRIORITY;
			nvic_button_10.NVIC_IRQChannelSubPriority = 0;
			nvic_button_10.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&nvic_button_10);
}

/* Инициализация GPIOA (PA0 как AF output для TIM2_CH1) */
void Init_pina0_pwm_gpio(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* PA0: Alternate Function Output, Push-Pull, 50 MHz */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void Error_handler(void)
{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
		while(1) {}
}
