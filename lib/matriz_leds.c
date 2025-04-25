#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "matriz_leds.h"

// Arquivo .pio para controle da matriz
#include "pio_matrix.pio.h"

// Pino que realizará a comunicação do microcontrolador com a matriz
#define OUT_PIN 7

// Gera o binário que controla a cor de cada célula do LED
//rotina para definição da intensidade de cores do led
uint32_t gerar_binario_cor(double red, double green, double blue)
{
  unsigned char RED, GREEN, BLUE;
  RED = red * 255.0;
  GREEN = green * 255.0;
  BLUE = blue * 255.0;
  return (GREEN << 24) | (RED << 16) | (BLUE << 8);
}

uint configurar_matriz(PIO pio){
    bool ok;

    // Define o clock para 128 MHz, facilitando a divisão pelo clock
    ok = set_sys_clock_khz(128000, false);

    // Inicializa todos os códigos stdio padrão que estão ligados ao binário.
    stdio_init_all();

    printf("iniciando a transmissão PIO");
    if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

    //configurações da PIO
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    return sm;
}

void imprimir_desenho(Matriz_leds_config configuracao, PIO pio, uint sm){
    for (int contadorLinha = 4; contadorLinha >= 0; contadorLinha--){
        if(contadorLinha % 2){
            for (int contadorColuna = 0; contadorColuna < 5; contadorColuna ++){
                uint32_t valor_cor_binario = gerar_binario_cor(
                    configuracao[contadorLinha][contadorColuna].red,
                    configuracao[contadorLinha][contadorColuna].green,
                    configuracao[contadorLinha][contadorColuna].blue
                );

                pio_sm_put_blocking(pio, sm, valor_cor_binario);
            }
        }else{
            for (int contadorColuna = 4; contadorColuna >= 0; contadorColuna --){
                uint32_t valor_cor_binario = gerar_binario_cor(
                    configuracao[contadorLinha][contadorColuna].red,
                    configuracao[contadorLinha][contadorColuna].green,
                    configuracao[contadorLinha][contadorColuna].blue
                );

                pio_sm_put_blocking(pio, sm, valor_cor_binario);
            }
        }
    }
}

void clearMatriz(PIO pio, uint sm){
    Matriz_leds_config matriz;
    for (int linha = 0; linha < 5; linha++) {
        for (int coluna = 0; coluna < 5; coluna++) {
            matriz[linha][coluna] = (Led_config){0.0, 0.0, 0.0};
        }
    }
    // Imprime na matriz física
    imprimir_desenho(matriz, pio, sm);
}


void imprimirColuna(config Config, PIO pio, uint sm){
    // Tabela de cores predefinidas (0 a 9)
    RGB_cod cores_predefinidas[10] = {
        {0.0, 0.0, 0.0},        // Preto
        {0.08, 0.04, 0.0},      // Marrom
        {0.2, 0.0, 0.0},        // Vermelho
        {0.2, 0.1, 0.0},        // Laranja
        {0.2, 0.2, 0.0},        // Amarelo
        {0.0, 0.2, 0.0},        // Verde
        {0.0, 0.0, 0.2},        // Azul
        {0.1, 0.0, 0.1},        // Violeta
        {0.1, 0.1, 0.1},        // Cinza
        {0.2, 0.2, 0.2}         // Branco
    };

    Matriz_leds_config matriz;

    for(int i = 0; i < 5; i++){
        matriz[i][1] = cores_predefinidas[0];
        matriz[i][3] = cores_predefinidas[0];
    }

    // Preenche a coluna desejada com a cor escolhida
    for (int linha = 0; linha < 5; linha++) {
        for(int coluna = 0; coluna < 3; coluna++){
            matriz[linha][Config[coluna].faixa] = cores_predefinidas[Config[coluna].cor];
        }
    }

    // Imprime na matriz física
    imprimir_desenho(matriz, pio, sm);
}

RGB_cod obter_cor_por_parametro_RGB(int red, int green, int blue){
    RGB_cod cor_customizada = {red/255.0,green/255.0,blue/255.0};

    return cor_customizada;
}