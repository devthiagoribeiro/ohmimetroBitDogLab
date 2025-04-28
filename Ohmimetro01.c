#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "hardware/pio.h"
#include "lib/matriz_leds.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o Ohmímetro

int R_conhecido = 9870;   // Resistor de 10k ohm
float Rx = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
char *faixas[3];          //Armazena a sequência de faixas correspondente ao resistor que estamos medindo
PIO pio;                  //variável para configuração da matriz de leds
uint sm;                  //variável para configuração da matriz de leds

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

//Função para designar a tabela de cores do resistor que está sendo medido
void tabelaDeCores(uint resistencia){
  char *cores[] = {
    "black",   // 0
    "brown",   // 1
    "red",     // 2
    "orange",  // 3
    "yellow",  // 4
    "green",   // 5
    "blue",    // 6
    "violet",  // 7
    "gray",    // 8
    "white",   // 9
  };

  int primeira, segunda, multiplicador;

  // Divide até que reste um número de dois dígitos
  int base = resistencia;
  multiplicador = 0;

  while (base >= 100) {
      base /= 10;
      multiplicador++;
  }

  primeira = base / 10;
  segunda = base % 10;

  // Atribui as cores com base nos índices
  faixas[0] = cores[primeira];
  faixas[1] = cores[segunda];
  faixas[2] = cores[multiplicador];
  //Acende as faixas de led correspondentes na matriz de leds
  imprimirColuna((config){
    {0, primeira},
    {2, segunda},
    {4, multiplicador}
  }, pio, sm);
}

int main()
{
  stdio_init_all();
  //inicialização da matriz de leds
  pio = pio0;
  sm = configurar_matriz(pio);

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

  // Limpa a matriz de leds
  clearMatriz(pio, sm);

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

    //  Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    ssd1306_line(&ssd, 3, 15, 123, 15, cor);           // Desenha uma linha horinzotal
    ssd1306_draw_string(&ssd, "  Ohmimetro", 8, 6, 1);    // Desenha uma string
    ssd1306_draw_string(&ssd, " Resistencia:", 22, 18, 0.8); // Desenha uma string
    
    //Verificando se um valor válido de resistência foi encotrado para ser exibido no display,
    //caso contrário exibe uma mensagem de valor não identificado
    if(verificaRx(Rx, 0.05) > -1.0){
      tabelaDeCores(verificaRx(Rx, 0.05)); //Armazena a sequência de cores correspondente a rx em faixas
      ssd1306_draw_string(&ssd, str_y, 45, 26, 1);           // Desenha uma string (Rx)
      //representação da tabela de cores
      ssd1306_draw_string(&ssd, faixas[0], 6, 37, 0.8);     // Desenha uma string 
      ssd1306_draw_string(&ssd, faixas[1], 45, 37, 0.8);   // Desenha uma string
      ssd1306_draw_string(&ssd, faixas[2], 85, 37, 0.8);  // Desenha uma string 
      ssd1306_rect(&ssd, 35, 3, 41, 10, cor, !cor);      // Desenha um retângulo
      ssd1306_rect(&ssd, 35, 43, 41, 10, cor, !cor);      // Desenha um retângulo
      ssd1306_rect(&ssd, 35, 83, 42, 10, cor, !cor);      // Desenha um retângulo
      //desenho do resistor
      ssd1306_line(&ssd, 8, 53, 45, 53, cor);             // Desenha uma linha horinzotal para formar o resistor
      ssd1306_line(&ssd, 83, 53, 119, 53, cor);           // Desenha uma linha horinzotal para formar o resistor
      ssd1306_rect(&ssd, 47, 45, 38, 14, cor, !cor);      // Desenha um retângulo para formar o resistor
      ssd1306_rect(&ssd, 47, 50, 5, 14, cor, cor);      // Desenha um retângulo para formar o resistor
      ssd1306_rect(&ssd, 47, 60, 5, 14, cor, cor);      // Desenha um retângulo para formar o resistor
      ssd1306_rect(&ssd, 47, 70, 5, 14, cor, cor);      // Desenha um retângulo para formar o resistor
    }else{
      clearMatriz(pio, sm);                                  // Limpa a matriz de leds
      ssd1306_draw_string(&ssd, "     Valor", 10, 32, 1);    // Desenha uma string
      ssd1306_draw_string(&ssd, "      nao", 10, 42, 1);     // Desenha uma string
      ssd1306_draw_string(&ssd, " identificado", 10, 52, 1); // Desenha uma string
    }
    ssd1306_send_data(&ssd);                              // Atualiza o display
    sleep_ms(700);
  }
}