# Ohmímetro com Reconhecimento Automático do Código de Cores de Resistores

Projeto desenvolvido para consolidar conhecimentos sobre componentes eletrônicos, leitura analógica e exibição gráfica utilizando a placa BitDogLab e o microcontrolador RP2040.

## 📝 Descrição

O **Ohmímetro** é um sistema embarcado capaz de medir automaticamente o valor de resistores conectados ao circuito e, além disso, identificar e exibir o respectivo código de cores tanto no display OLED SSD1306 quanto na matriz de leds. O sistema calcula a resistência com base na leitura analógica (ADC) e determina as faixas de cores conforme a série E24 (tolerância de 5%).

Este projeto visa consolidar conhecimentos sobre:
- Leitura de sinais analógicos utilizando o ADC do RP2040.
- Cálculo de valores de resistência a partir de medições elétricas.
- Identificação e associação de resistores comerciais padrão (E24).
- Exibição gráfica em displays OLED utilizando comunicação I2C.
- Organização de código em projetos embarcados.

## 🚀 Funcionalidades

- Leitura automática da resistência do componente conectado.
- Identificação do valor comercial padrão mais próximo (5% de tolerância).
- Exibição no display OLED:
  - Valor numérico da resistência (em ohms).
  - Código de cores do resistor (primeira, segunda faixa e multiplicador).
  - Representação gráfica opcional de um resistor estilizado com faixas.
- Utilização da matriz de LEDs para reforço visual das cores.

## 🔧 Como utilizar o projeto

### ✅ Requisitos

- Raspberry Pi Pico W
- Placa BitDogLab (ou equivalente)
- Display OLED SSD1306 conectado via I2C
- VS Code com extensão **Pico C SDK** configurada
- Drivers de comunicação serial USB para Raspberry Pi Pico

### 📦 Instalação e Compilação

1. Clone o repositório:
    ```bash
    git clone https://github.com/devthiagoribeiro/ohmimetroBitDogLab
    cd ohmimetroBitDogLab
    ```

2. Compile o projeto com o CMake:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

3. Envie o arquivo `.uf2` gerado para a Pico.

### ▶️ Execução

- Conecte um resistor à entrada de medição do circuito.
- A resistência será automaticamente medida e exibida.
- O valor em ohms e as cores correspondentes serão mostrados no display OLED e na matriz de leds.
- Veja também uma ilustração gráfica do resistor.

## 🎯 Demonstração

**🎥 [Clique para assistir o vídeo da demonstração](https://youtu.be/N8BdnTNSNUw)**
