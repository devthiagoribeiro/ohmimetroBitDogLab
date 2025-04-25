// Definição de tipo da estrutura que irá controlar a cor dos LED's
typedef struct {
    double red;
    double green;
    double blue;
}Led_config;

typedef struct{
    uint faixa;
    uint cor;
}Colunas;

typedef Colunas config[3];

typedef Led_config RGB_cod;

// Definição de tipo da matriz de leds
typedef Led_config Matriz_leds_config[5][5]; 

uint32_t gerar_binario_cor(double red, double green, double blue);

uint configurar_matriz(PIO pio);

void imprimir_desenho(Matriz_leds_config configuracao, PIO pio, uint sm);
void imprimirColuna(config Config, PIO pio, uint sm);
void clearMatriz(PIO pio, uint sm);

RGB_cod obter_cor_por_parametro_RGB(int red, int green, int blue);