#include "ssd1306.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h>

// Definição dos pinos usados no projeto
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
#define deadzone 300

// Estado dos LEDs
bool led_green_state = false;
bool pwm_leds_enabled = true;

// Estado da borda do display
int border_state = 0; // 0 = sem borda, 1 = fina, 2 = dupla, 3 = tracejada

// Mapeia os valores do ADC para os níveis de PWM (0 a 255)
int map_adc_to_pwm(int adc_value) {
    int pwm_value = (abs(adc_value - JOYSTICK_CENTER) * 255) / (ADC_MAX - JOYSTICK_CENTER);
    return pwm_value > 255 ? 255 : pwm_value;
}

// Configura o PWM para um determinado pino
void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice, 255);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(pin), 0);
    pwm_set_enabled(slice, true);
}

// Configuração inicial do hardware
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

// Desenha a borda na tela de acordo com o estado atual
void draw_border() {
    if (border_state == 0) return;
    
    for (int x = 0; x < 128; x++) {
        if (border_state == 3 && x % 4 > 1) continue;
        ssd1306_draw_pixel(x, 0, true);
        ssd1306_draw_pixel(x, 63, true);
    }
    for (int y = 0; y < 64; y++) {
        if (border_state == 3 && y % 4 > 1) continue;
        ssd1306_draw_pixel(0, y, true);
        ssd1306_draw_pixel(127, y, true);
    }

    if (border_state == 2) {
        for (int x = 2; x < 126; x++) {
            ssd1306_draw_pixel(x, 2, true);
            ssd1306_draw_pixel(x, 61, true);
        }
        for (int y = 2; y < 62; y++) {
            ssd1306_draw_pixel(2, y, true);
            ssd1306_draw_pixel(125, y, true);
        }
    }
    ssd1306_display();
}

// Desenha um quadrado na tela
void draw_square(int x, int y) {
    int square_size = 8;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + square_size > 128) x = 128 - square_size;
    if (y + square_size > 64) y = 64 - square_size;
    
    for (int x1 = x; x1 < x + square_size; x1++) {
        for (int y1 = y; y1 < y + square_size; y1++) {
            ssd1306_draw_pixel(x1, y1, true);
        }
    }
    ssd1306_display();
}

// Atualiza a posição do quadrado com base no joystick
void update_square() {
    adc_select_input(0);
    int y_val = adc_read();
    adc_select_input(1);
    int x_val = adc_read();
    
    int x = 64, y = 32;

    if (abs(x_val - JOYSTICK_CENTER) > deadzone) {
        x = (x_val - JOYSTICK_CENTER) * (SSD1306_WIDTH - 8) / (ADC_MAX - JOYSTICK_CENTER);
    }
    if (abs(y_val - JOYSTICK_CENTER) > deadzone) {
        y = (y_val - JOYSTICK_CENTER) * (SSD1306_HEIGHT - 8) / (ADC_MAX - JOYSTICK_CENTER);
    }
    
    ssd1306_clear();
    draw_border();
    draw_square(x, y);
    ssd1306_display();
}

// Atualiza os LEDs conforme a posição do joystick
void update_leds() {
    if (!pwm_leds_enabled) {
        pwm_set_chan_level(pwm_gpio_to_slice_num(LED_BLUE), pwm_gpio_to_channel(LED_BLUE), 0);
        pwm_set_chan_level(pwm_gpio_to_slice_num(LED_RED), pwm_gpio_to_channel(LED_RED), 0);
        return;
    }

    adc_select_input(0);
    int y_val = adc_read();
    adc_select_input(1);
    int x_val = adc_read();

    int red_intensity = map_adc_to_pwm(x_val);
    int blue_intensity = map_adc_to_pwm(y_val);
    
    if (abs(x_val - JOYSTICK_CENTER) < 50) red_intensity = 0;
    if (abs(y_val - JOYSTICK_CENTER) < 50) blue_intensity = 0;
    
    pwm_set_chan_level(pwm_gpio_to_slice_num(LED_RED), pwm_gpio_to_channel(LED_RED), red_intensity);
    pwm_set_chan_level(pwm_gpio_to_slice_num(LED_BLUE), pwm_gpio_to_channel(LED_BLUE), blue_intensity);
}

// Alterna o LED verde e o estado da borda
void toggle_led_green() {
    led_green_state = !led_green_state;
    gpio_put(LED_GREEN, led_green_state);
    border_state = (border_state + 1) % 4;
}

// Callback de interrupção para os botões
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
        sleep_ms(100);
    }
}
