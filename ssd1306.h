#ifndef SSD1306_H
#define SSD1306_H

#include "pico/stdlib.h"

// Definições do display SSD1306
#define SSD1306_I2C_ADDR 0x3C
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_BUFFER_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

extern uint8_t ssd1306_buffer[SSD1306_BUFFER_SIZE];

// Funções do display SSD1306
void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_display(void);
void ssd1306_draw_pixel(int x, int y, bool color);
void ssd1306_set_position(int x, int y);
void ssd1306_send_command(uint8_t command);
void ssd1306_send_data(uint8_t data);

#endif
