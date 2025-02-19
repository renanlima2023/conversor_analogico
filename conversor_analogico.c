#include "ssd1306.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h>

// Definição dos pinos
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12
#define JOYSTICK_X 27
#define JOYSTICK_Y 26
#define JOYSTICK_BTN 22
#define BUTTON_A 5
#define JOYSTICK_CENTER 2048
#define ADC_MAX 4095
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DEBOUNCE_DELAY 200

// Estado dos LEDs
bool led_green_state = false;
bool pwm_leds_enabled = true;

// Mapeamento de valores ADC para PWM (escala de 0 a 255)
int map_adc_to_pwm(int adc_value) {
    int pwm_value = (abs(adc_value - JOYSTICK_CENTER) * 255) / (ADC_MAX - JOYSTICK_CENTER);
    return pwm_value > 255 ? 255 : pwm_value;
}

// Configuração do PWM
void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice, 255);  // Definindo o limite máximo de PWM como 255
    pwm_set_chan_level(slice, pwm_gpio_to_channel(pin), 0);
    pwm_set_enabled(slice, true);
}

// Configuração do hardware
void setup() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    
    gpio_init(JOYSTICK_BTN);
    gpio_set_dir(JOYSTICK_BTN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    setup_pwm(LED_RED);
    setup_pwm(LED_BLUE);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();
    ssd1306_clear();
}

// Função para desenhar um quadrado no display
void draw_square(int x, int y) {
    int square_size = 8; // Tamanho do quadrado
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + square_size > 128) x = 128 - square_size;
    if (y + square_size > 64) y = 64 - square_size;
    
    // Desenha o quadrado pixel por pixel
    for (int x1 = x; x1 < x + square_size; x1++) {
        for (int y1 = y; y1 < y + square_size; y1++) {
            ssd1306_draw_pixel(x1, y1, true);
        }
    }
    ssd1306_display();  // Atualiza o display
}

// Função para atualizar a posição do quadrado
void update_square() {
    adc_select_input(0); // Eixo Y do joystick
    int y_val = adc_read();
    adc_select_input(1); // Eixo X do joystick
    int x_val = adc_read();
    
    // Mapeia os valores de ADC para a posição do quadrado no display
    int x = (x_val - JOYSTICK_CENTER) * (SSD1306_WIDTH - 8) / (ADC_MAX - JOYSTICK_CENTER);
    int y = (y_val - JOYSTICK_CENTER) * (SSD1306_HEIGHT - 8) / (ADC_MAX - JOYSTICK_CENTER);
    
    // Limita as posições para que o quadrado não ultrapasse os limites do display
    x = x < 0 ? 0 : (x > SSD1306_WIDTH - 8 ? SSD1306_WIDTH - 8 : x);
    y = y < 0 ? 0 : (y > SSD1306_HEIGHT - 8 ? SSD1306_HEIGHT - 8 : y);
    
    draw_square(x, y);
}


// Função para atualizar os LEDs
void update_leds() {
    if (!pwm_leds_enabled) {
        pwm_set_chan_level(pwm_gpio_to_slice_num(LED_BLUE), pwm_gpio_to_channel(LED_BLUE), 0);
        pwm_set_chan_level(pwm_gpio_to_slice_num(LED_RED), pwm_gpio_to_channel(LED_RED), 0);
        return;
    }

    adc_select_input(0); // Eixo Y controla LED azul
    int y_val = adc_read();
    adc_select_input(1); // Eixo X controla LED vermelho
    int x_val = adc_read();

    int red_intensity = map_adc_to_pwm(x_val);
    int blue_intensity = map_adc_to_pwm(y_val);

    // Desativa o LED se o joystick estiver próximo ao centro
    if (abs(x_val - JOYSTICK_CENTER) < 50) red_intensity = 0;
    if (abs(y_val - JOYSTICK_CENTER) < 50) blue_intensity = 0;
    
    pwm_set_chan_level(pwm_gpio_to_slice_num(LED_RED), pwm_gpio_to_channel(LED_RED), red_intensity);
    pwm_set_chan_level(pwm_gpio_to_slice_num(LED_BLUE), pwm_gpio_to_channel(LED_BLUE), blue_intensity);
}

// Alterna o estado do LED verde
void toggle_led_green() {
    led_green_state = !led_green_state;
    gpio_put(LED_GREEN, led_green_state);
}

// Função de interrupção para os botões
void button_callback(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if ((current_time - last_time) > DEBOUNCE_DELAY) {
        if (gpio == JOYSTICK_BTN && !gpio_get(JOYSTICK_BTN)) {
            toggle_led_green();
        } else if (gpio == BUTTON_A && !gpio_get(BUTTON_A)) {
            pwm_leds_enabled = !pwm_leds_enabled;
            update_leds();
        }
        last_time = current_time;
    }
}

int main() {
    setup();
    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL, true, &button_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_callback);
    
    while (1) {
        update_leds();
        update_square();
        sleep_ms(100);  // Adiciona um pequeno atraso para evitar excesso de processamento
    }
}
