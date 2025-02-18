#include "ssd1306.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

// Definir o buffer de pixels para o display
uint8_t ssd1306_buffer[SSD1306_BUFFER_SIZE];

// Função para inicializar o display SSD1306
void ssd1306_init(void) {
    // Comandos de inicialização do display SSD1306
    ssd1306_send_command(0xAE); // Desliga o display
    ssd1306_send_command(0xD5); // Define o clock do display
    ssd1306_send_command(0x80); // Configuração do clock
    ssd1306_send_command(0xA8); // Define a altura do display
    ssd1306_send_command(0x3F); // 64 linhas
    ssd1306_send_command(0xD3); // Deslocamento para o início
    ssd1306_send_command(0x00); // Deslocamento inicial
    ssd1306_send_command(0x40); // Definir o início da linha
    ssd1306_send_command(0x8D); // Ativar a alimentação externa
    ssd1306_send_command(0x14); // Acionar a alimentação externa
    ssd1306_send_command(0x20); // Modo de endereçamento
    ssd1306_send_command(0x00); // Modo de endereçamento horizontal
    ssd1306_send_command(0xA1); // Inverter o mapeamento dos pinos
    ssd1306_send_command(0xC8); // Configurar o mapeamento de linha
    ssd1306_send_command(0xDA); // Configurar os pinos de segmento
    ssd1306_send_command(0x12); // Configuração de resistência
    ssd1306_send_command(0x81); // Configurar o contraste
    ssd1306_send_command(0x7F); // Valor de contraste
    ssd1306_send_command(0xD9); // Frequência do pre-charge
    ssd1306_send_command(0xF1); // Frequência do pre-charge
    ssd1306_send_command(0xDB); // Configuração de resistência de descarga
    ssd1306_send_command(0x40); // Configuração de resistência de descarga
    ssd1306_send_command(0xA4); // Desabilitar a exibição de inversão
    ssd1306_send_command(0xA6); // Desativar o modo de inversão
    ssd1306_send_command(0xAF); // Ativar o display
    ssd1306_clear();
}

// Função para limpar o display SSD1306
void ssd1306_clear(void) {
    for (int i = 0; i < SSD1306_BUFFER_SIZE; i++) {
        ssd1306_buffer[i] = 0x00;
    }
    ssd1306_display();
}

// Função para atualizar o display com os dados do buffer
void ssd1306_display(void) {
    for (int i = 0; i < 8; i++) {
        ssd1306_send_command(0xB0 + i); // Linha de endereçamento
        ssd1306_send_command(0x00); // Coluna baixa
        ssd1306_send_command(0x10); // Coluna alta
        for (int j = 0; j < SSD1306_WIDTH; j++) {
            ssd1306_send_data(ssd1306_buffer[i * SSD1306_WIDTH + j]);
        }
    }
}

// Função para desenhar um pixel no display
void ssd1306_draw_pixel(int x, int y, bool color) {
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;

    if (color) {
        ssd1306_buffer[(y / 8) * SSD1306_WIDTH + x] |= (1 << (y % 8));
    } else {
        ssd1306_buffer[(y / 8) * SSD1306_WIDTH + x] &= ~(1 << (y % 8));
    }
}

// Função para enviar um comando ao display
void ssd1306_send_command(uint8_t command) {
    uint8_t data[2] = {0x00, command};
    i2c_write_blocking(i2c1, SSD1306_I2C_ADDR, data, 2, false);
}

// Função para enviar dados ao display
void ssd1306_send_data(uint8_t data) {
    uint8_t data_buffer[2] = {0x40, data};
    i2c_write_blocking(i2c1, SSD1306_I2C_ADDR, data_buffer, 2, false);
}
