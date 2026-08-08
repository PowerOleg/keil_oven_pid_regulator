#ifndef __OLED_DISPLAY_H
#define __OLED_DISPLAY_H

#include "common.h"

#define DISPLAY_LINE_SIZE 40
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_BUFSIZE (OLED_WIDTH * OLED_HEIGHT / 8)
#define COL_PX 5
#define ROW_PX 7

#define OLED_CS_LOW()    			GPIO_ResetBits(GPIOA, GPIO_Pin_11)
#define OLED_CS_HIGH()   			GPIO_SetBits(GPIOA, GPIO_Pin_11)
#define OLED_DC_COMMAND()			GPIO_ResetBits(GPIOA, GPIO_Pin_9)
#define OLED_DC_DATA()				GPIO_SetBits(GPIOA, GPIO_Pin_9)
#define OLED_RES_HIGH()  			GPIO_SetBits(GPIOA, GPIO_Pin_10)
#define OLED_RES_LOW()   			GPIO_ResetBits(GPIOA, GPIO_Pin_10)


extern const uint8_t *font_table[];
	
//Битовые маски символов (5x7)
extern const uint8_t digit_font_5x7[10][COL_PX];// Цифры 0..9
extern const uint8_t colon_5x7[COL_PX];// Двоеточие
extern const uint8_t minus_5x7[COL_PX];// Минус
extern const uint8_t dot_5x7[COL_PX];// Точка
extern const uint8_t degree_5x7[COL_PX];// Градус (°)
extern const uint8_t letter_C_5x7[COL_PX];// Буква C
extern const uint8_t percent_5x7[COL_PX];
extern const uint8_t percent_15x21[15];
extern const uint8_t exclamation_5x7[COL_PX];

extern const uint8_t letter_V_cap_5x7[5];
extern const uint8_t letter_P_5x7[5];
extern const uint8_t letter_E_cap_5x7[5];
extern const uint8_t letter_m_5x7[5];
extern const uint8_t letter_M_cap_5x7[5];
extern const uint8_t letter_ru_Ya_cap_5x7[5];

extern const uint8_t space_5x7[COL_PX];
extern const uint8_t letter_ru_T_5x7[COL_PX];
extern const uint8_t letter_ru_P_cap_5x7[COL_PX];
extern const uint8_t letter_A_cap_5x7[COL_PX];
extern const uint8_t hyphen_5x7[COL_PX];

extern const uint8_t letter_L_cap_5x7[COL_PX];
extern const uint8_t letter_ru_Zh_cap_5x7[COL_PX];
extern const uint8_t letter_N_cap_5x7[COL_PX];
extern const uint8_t letter_O_cap_5x7[COL_PX];
extern const uint8_t letter_ru_SoftSign_5x7[COL_PX];
extern const uint8_t letter_D_cap_5x7[5];
extern const uint8_t letter_ru_I_cap_5x7[5];
extern const uint8_t letter_K_5x7[5];
extern const uint8_t letter_ru_U_cap_5x7[5];
extern const uint8_t letter_ru_F_cap_5x7[5];
extern const uint8_t letter_ru_Ch_cap_5x7[5];
extern const uint8_t slash_5x7[5];
extern const uint8_t letter_H_5x7[5];

extern const uint8_t temperature_full_indices[];
extern const uint8_t ustavka_indices[];
extern const uint8_t current_value_indices[];

void Oled_gpio_init(void);
void Oled_spi_init(void);
void OLED_Reset(void);
void OLED_WriteByte(uint8_t data, uint8_t dc);
void OLED_Init_SSD1306(void);

void OLED_ClearBuffer(void);
void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_UpdateScreen(void);
void OLED_DrawScaledChar(uint8_t x0, uint8_t y0, const uint8_t *bitmap, uint8_t scale);
void OLED_PrintScaledSymbols(uint8_t x0, uint8_t y0, const uint8_t **symbol_table,
                            const uint8_t *symbols, uint8_t count, uint8_t scale);
void OLED_fill_indices(uint8_t *indices, const uint8_t hours, const uint8_t minutes);
void OLED_PrintTemperature(uint8_t x, uint8_t y, float temperature, uint8_t scale, const uint8_t **symbol_table);
void OLED_PrintHumidity(uint8_t x, uint8_t y, float humidity, uint8_t scale, const uint8_t **font_table);
void OLED_DrawPercent15x21(uint8_t x, uint8_t y);
void OLED_DrawPercent15x21Buf(uint8_t x_px, uint8_t y_page);
void OLED_PrintPressure(uint8_t y, uint32_t pressure, uint8_t scale, const uint8_t **font_table);

#endif
