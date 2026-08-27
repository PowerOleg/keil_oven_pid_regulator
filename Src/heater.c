#include "heater.h"
#include "common.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_tim.h"

#define PID_CS_LOW()    GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define PID_CS_HIGH()   GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define FILTER_SIZE 3


static int Max6675_read_raw(void)
{
    PID_CS_LOW();
    Delay_us(10);

    SPI_I2S_SendData(SPI1, 0x00);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
    uint8_t b1 = SPI_I2S_ReceiveData(SPI1);

    SPI_I2S_SendData(SPI1, 0x00);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
    uint8_t b2 = SPI_I2S_ReceiveData(SPI1);

    PID_CS_HIGH();
    Delay_us(10);

    return ((uint16_t)b1 << 8) | (uint16_t)b2;
}

float Max6675_get_temperature_c(void)
{
    uint16_t raw_temp = Max6675_read_raw();
		raw_temp = raw_temp >> 3;
		return (float)raw_temp * 0.25f;
/*	
		if (raw & 0x0004)
		{
        // Ѕит 2 установлен Ч ошибка (термопара не подключена)
        return -999.0f;
    }

    // ƒополнительно: провер€ем, что младшие 2 бита нулевые (должны быть по даташиту)
    if ((raw & 0x0003) != 0)
		{
        // Ќестандартное значение в зарезервированных битах Ч тоже ошибка
        return -999.0f;
    }

    return (float)(raw >> 3) * 0.25f;*/
}

static void Init_b9_gpio(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void Tim4_init_pwm(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef        TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_TimeBaseStructure.TIM_Prescaler     = 719;           // 72 ћ√ц / (71+1) = 1 ћ√ц //719 => 100 к√ц
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period        = 999;           // 1 ћ√ц / (49+1) = 20 к√ц //999 => 1 к√ц
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0;// начальна€ скважность 0
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);

    // ѕредзагрузка ARR и CCR Ц дл€ плавного изменени€
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_CCPreloadControl(TIM4, ENABLE);

    TIM_Cmd(TIM4, ENABLE);
}

void Heater_init(void)
{
		Init_b9_gpio();
		Tim4_init_pwm();
		Set_pwm_duty(0);
}

void Set_pwm_duty(uint8_t percent)
{
    if (percent > 100)
				percent = 100;
    uint16_t ccr_value = (uint16_t)((percent * (TIM4->ARR + 1)) / 100);
    TIM4->CCR4 = ccr_value;// CH4
}

void Heater_on(float setpoint, float temperature_c)
{
		Set_pwm_duty(100);
	
	
}

void Heater_average_filter(float *value)
{
		static float temp_buf[FILTER_SIZE] = {0};
		static uint8_t index = 0;
		static uint16_t count = 0;//важный параметр потому что показывает сколько значений брать в расчет среднего значени€
		
		temp_buf[index] = *value;
		index = (index + 1) % FILTER_SIZE;
		if (count < FILTER_SIZE)
				count++;
		
    float sum_temp = 0.0f;
    for (uint8_t i = 0; i < count; i++)
		{
        uint8_t idx = (index + i) % count;
        sum_temp += temp_buf[idx];
    }
    *value = sum_temp / count;
}
