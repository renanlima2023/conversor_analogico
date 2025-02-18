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

// Mapeamento de valores
int map_adc_to_pwm(int adc_value) {
    int pwm_value = (abs(adc_value - JOYSTICK_CENTER) * 255) / (ADC_MAX - JOYSTICK_CENTER);
    
    // Limita o valor de PWM para o intervalo de 0 a 255
    if (pwm_value > 255) pwm_value = 255;

    return pwm_value;
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

// Desenha um quadrado no display OLED
void draw_square(int x_pos, int y_pos) {
    int square_size = 8;
    ssd1306_clear(); // Limpa o display antes de desenhar

    if (x_pos < 0) x_pos = 0;
    if (y_pos < 0) y_pos = 0;
    if (x_pos + square_size > 128) x_pos = 128 - square_size;
    if (y_pos + square_size > 64) y_pos = 64 - square_size;

    for (int x = x_pos; x < x_pos + square_size; x++) {
        for (int y = y_pos; y < y_pos + square_size; y++) {
            ssd1306_draw_pixel(x, y, true);
        }
    }

    ssd1306_display();
}

// Atualiza a posição do quadrado com base no joystick
void update_square_position() {
    adc_select_input(0); // Eixo Y (pino 26)
    int y_val = adc_read();
    
    adc_select_input(1); // Eixo X (pino 27)
    int x_val = adc_read();

    // Mapeamento da posição no display, centrando o quadrado
    int x_pos = (x_val - JOYSTICK_CENTER) * 128 / (ADC_MAX - JOYSTICK_CENTER);
    int y_pos = (y_val - JOYSTICK_CENTER) * 64 / (ADC_MAX - JOYSTICK_CENTER);

    draw_square(x_pos, y_pos);
}

// Atualiza a intensidade dos LEDs
void update_leds() {
    if (!pwm_leds_enabled) {
        pwm_set_chan_level(pwm_gpio_to_slice_num(LED_BLUE), pwm_gpio_to_channel(LED_BLUE), 0);
        pwm_set_chan_level(pwm_gpio_to_slice_num(LED_RED), pwm_gpio_to_channel(LED_RED), 0);
        return;
    }
    
    adc_select_input(0); // Agora o eixo Y controla o LED azul
    int y_val = adc_read();
    adc_select_input(1); // Agora o eixo X controla o LED vermelho
    int x_val = adc_read();
    
    int red_intensity = map_adc_to_pwm(x_val);
    int blue_intensity = map_adc_to_pwm(y_val);

    if (abs(x_val - JOYSTICK_CENTER) < 50) red_intensity = 0;
    if (abs(y_val - JOYSTICK_CENTER) < 50) blue_intensity = 0;
    
    pwm_set_chan_level(pwm_gpio_to_slice_num(LED_RED), pwm_gpio_to_channel(LED_RED), red_intensity);
    pwm_set_chan_level(pwm_gpio_to_slice_num(LED_BLUE), pwm_gpio_to_channel(LED_BLUE), blue_intensity);
}

// Alterna o LED verde
void toggle_led_green() {
    led_green_state = !led_green_state;
    gpio_put(LED_GREEN, led_green_state);
}

// Interrupções para os botões
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
        update_leds();           // Atualiza os LEDs
        update_square_position(); // Atualiza a posição do quadrado no display
        sleep_ms(100);
    }
}
