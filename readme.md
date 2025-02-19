# Controle de LEDs e Display OLED com Joystick - BitDogLab

## 📌 Descrição
Este projeto utiliza a placa BitDogLab para controlar LEDs RGB e exibir um quadrado móvel em um display OLED SSD1306, baseado na posição de um joystick analógico. Além disso, permite alternar o estado do LED verde e modificar a borda do display com botões físicos.

## 🚀 Funcionalidades
- Controle dos LEDs RGB via PWM com base nos valores do joystick.
- Exibição gráfica da posição do joystick no display OLED.
- Implementação de bordas no display (sem borda, fina, dupla e tracejada).
- Alternância do LED verde e mudança do estado da borda com o botão do joystick.
- Ativação/desativação dos LEDs PWM com o botão A.
- Implementação de interrupções para os botões com tratamento de bouncing via software.

## 🛠️ Hardware Utilizado
- Placa **BitDogLab**
- **Joystick Analógico** (Eixo X: GPIO 27, Eixo Y: GPIO 26, Botão: GPIO 22)
- **Display OLED SSD1306** (I2C: SDA GPIO 14, SCL GPIO 15)
- **LEDs RGB** (LED Vermelho: GPIO 13, LED Verde: GPIO 11, LED Azul: GPIO 12)
- **Botão A** (GPIO 5)

## 📌 Configuração do Hardware
- O joystick está conectado aos pinos ADC 26 e 27 para leitura dos eixos X e Y.
- Os LEDs utilizam PWM para controle da intensidade de cor.
- O display OLED se comunica via protocolo I2C.
- Os botões utilizam resistores pull-up internos para detecção de pressionamento.

## 📄 Estrutura do Código
- **setup()**: Inicializa os periféricos (ADC, PWM, I2C, GPIOs e display SSD1306).
- **map_adc_to_pwm()**: Converte valores do ADC para escala PWM (0-255).
- **setup_pwm()**: Configura os canais de PWM para os LEDs.
- **draw_square()**: Desenha o quadrado no display conforme a posição do joystick.
- **update_square()**: Atualiza a posição do quadrado no display.
- **update_leds()**: Ajusta a intensidade dos LEDs RGB com base no joystick.
- **toggle_led_green()**: Alterna o estado do LED verde e modifica a borda do display.
- **button_callback()**: Trata as interrupções dos botões com debounce.
- **main()**: Loop principal que atualiza LEDs e o display em intervalos regulares.

## 🔧 Como Compilar e Executar
1. Certifique-se de ter o ambiente de desenvolvimento para **Raspberry Pi Pico** configurado.
2. Compile o código com o SDK do **Pico**.
3. Faça o upload do binário para a placa.
4. Conecte os componentes conforme descrito e reinicie a placa.

## 📌 Melhorias Futuras
- Implementação de animações no display OLED.
- Suporte a novos tipos de bordas e efeitos visuais.
- Ajuste dinâmico da sensibilidade do joystick.

## 📜 Licença
Este projeto é open-source e pode ser utilizado livremente para fins educacionais e experimentais.

---
📩 **Dúvidas ou sugestões? Fique à vontade para entrar em contato!**

