#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o Ohmímetro

int R_conhecido = 9870;   // Resistor de 10k ohm
float Rx = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
  reset_usb_boot(0, 0);
}

//Função para filtrar o valor bruto medido dentro dos possíveis valores comerciais 
float verificaRx(float Rx, float tolerancia) {
  int e24[] = {10, 11, 12, 13, 15, 16, 18, 20, 22, 24, 27, 30, 
               33, 36, 39, 43, 47, 51, 56, 62, 68, 75, 82, 91};

  for (int dec = 10; dec <= 10000; dec *= 10) {
    int i_start = 0;
    int i_end = 24;

    if (dec == 10) {
      i_start = 17; // começa em 51
    } else if (dec == 10000) {
      i_end = 1; // só o valor 10
    }

    for (int i = i_start; i < i_end; i++) {
      float valor = e24[i] * dec;
      float margem = valor * tolerancia;
      if (Rx >= valor - margem && Rx <= valor + margem) {
        return valor;
      }
    }
  }

  return -1; // Nenhum valor encontrado
}


int main()
{
  // Para ser utilizado o modo BOOTSEL com botão B
  gpio_init(botaoB);
  gpio_set_dir(botaoB, GPIO_IN);
  gpio_pull_up(botaoB);
  gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  // Aqui termina o trecho para modo BOOTSEL com botão B

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_t ssd;                                                // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  float tensao;
  char str_y[5]; // Buffer para armazenar a string

  bool cor = true;
  while (true)
  {
    adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 500.0f;

    // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
    Rx = (R_conhecido * media) / (ADC_RESOLUTION - media);
  
    sprintf(str_y, "%1.0f", verificaRx(Rx, 0.05));   // Converte o float em string

    // cor = !cor;
    //  Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    ssd1306_line(&ssd, 3, 15, 123, 15, cor);           // Desenha uma linha horinzotal
    ssd1306_draw_string(&ssd, "  Ohmimetro", 8, 6);    // Desenha uma string
    ssd1306_draw_string(&ssd, " Resistencia:", 8, 20); // Desenha uma string
    
    //Verificando se um valor válido de resistência foi encotrado para ser exibido no display,
    //caso contrário exibe uma mensagem de valor não identificado
    if(verificaRx(Rx, 0.05) > -1.0){
      ssd1306_draw_string(&ssd, str_y, 45, 30);           // Desenha uma string (Rx)
      ssd1306_line(&ssd, 8, 50, 45, 50, cor);             // Desenha uma linha horinzotal para formar o resistor
      ssd1306_line(&ssd, 83, 50, 119, 50, cor);           // Desenha uma linha horinzotal para formar o resistor
      ssd1306_rect(&ssd, 42, 45, 38, 17, cor, !cor);      // Desenha um retângulo para formar o resistor
    }else{
      ssd1306_draw_string(&ssd, "     Valor", 10, 32);    // Desenha uma string
      ssd1306_draw_string(&ssd, "      nao", 10, 42);     // Desenha uma string
      ssd1306_draw_string(&ssd, " identificado", 10, 52); // Desenha uma string
    }
    ssd1306_send_data(&ssd);                              // Atualiza o display
    sleep_ms(700);
  }
}