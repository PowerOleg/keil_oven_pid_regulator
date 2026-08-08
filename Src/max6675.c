#include "max6675.h"
#include "common.h"
#include "stm32f10x_spi.h"

#define PID_CS_LOW()    GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define PID_CS_HIGH()   GPIO_SetBits(GPIOA, GPIO_Pin_4)

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
        // Бит 2 установлен — ошибка (термопара не подключена)
        return -999.0f;
    }

    // Дополнительно: проверяем, что младшие 2 бита нулевые (должны быть по даташиту)
    if ((raw & 0x0003) != 0)
		{
        // Нестандартное значение в зарезервированных битах — тоже ошибка
        return -999.0f;
    }

    return (float)(raw >> 3) * 0.25f;*/
}
