/******************************************************************************
 * @file    menu.h
 * @brief   Arquivo contendo definições e protótipos de funções para a
 *          aplicação de menu no display SSD1306 hospedada no Raspberry Pi Pico W.
 *
 * @authors Gabriel Domingos
 * @date    Fevereiro 2025
 * @version 1.0.0
 *
 * @note    Este arquivo inclui as definições, constantes e protótipos de funções
 *          necessários para a aplicação.
 ******************************************************************************/

#ifndef MENU_H
#define MENU_H

#include <math.h>								// Biblioteca matemática para operações matemáticas.
#include <string.h>								// Biblioteca padrão para operações com strings.	
#include <stdlib.h>								// Biblioteca padrão para funções de propósito geral.
#include <ctype.h>								// Biblioteca padrão para funções de caracteres.
#include "pico/stdlib.h"						// Biblioteca padrão para Raspberry Pi Pico.
#include "pico/binary_info.h"					// Biblioteca para informações binárias.
#include "hardware/i2c.h"						// Biblioteca para operações com I2C.
#include "hardware/adc.h"						// Biblioteca para operações com ADC (Conversor Analógico-Digital).
#include "hardware/pwm.h"						// Biblioteca para operações com PWM (Modulação por Largura de Pulso).
#include "hardware/clocks.h"					// Biblioteca para configuração de clocks.
#include "ssd1306/ssd1306_fonts.h"				// Arquivo contendo fontes para o display SSD1306.
#include "ssd1306/ssd1306.h"					// Arquivo contendo funções para o display SSD1306.
#include "lwip/apps/mqtt.h"						// Biblioteca MQTT para lidar com o protocolo MQTT.
#include "icons.h"                              // Arquivo contendo ícones para o display SSD1306.
#include "hardware/timer.h"                     // Biblioteca para operações com temporizadores.     
#include "http.h"


// ---------------------------- Definições de Botões ----------------------------

/**
 * @brief Definições de pinos GPIO para os botões.
 */
#define BUTTON_A 5  ///< Botão para alterar opções/configurações.
#define BUTTON_B 6   ///< Botão para confirmar/entrar em uma seleção.


//----------------------------- Definições do Buzzer -------------------------------

#define BUZZER_PIN 21    			// Pino do buzzer
#define MIN_FREQUENCY 10 			// Frequência mínima do buzzer (Hz)
#define MAX_FREQUENCY 2000 			// Frequência máxima do buzzer (Hz)
#define ADC_UPPER_THRESHOLD 3500 	// Limite superior do ADC para incrementar a frequência
#define ADC_LOWER_THRESHOLD 850  	// Limite inferior do ADC para decrementar a frequência
#define STEP 20              		// Incremento ou decremento por iteração
float frequency = MIN_FREQUENCY;
int Limit_Buzzer = 0;
uint8_t x_distance;

//--------------------------------Variáveis provisórias--------------------------------------------

#define TEMPERATURE_UNITS 'C'     // Unidade para medição de temperatura.
int start_wifi = 0;
int connected_mqtt = 0;


// --------------------------- Função de Leitura da Temperatura Interna ---------------------------

/**
 * @brief Lê o sensor de temperatura interno.
 *
 * @param unit A unidade de temperatura ('C' para Celsius, 'F' para Fahrenheit).
 * @return float A temperatura na unidade especificada.
 *
 * Esta função lê o sensor de temperatura interno e retorna a temperatura
 * na unidade especificada.
 *
 * ### Comportamento:
 * - Habilita o sensor de temperatura interno.
 * - Seleciona a entrada ADC para o sensor de temperatura.
 * - Lê o valor do ADC e converte para temperatura.
 * - Retorna a temperatura na unidade especificada.
 *
 * @note A função assume um ADC de 12 bits com uma tensão de referência de 3.3V.
 */
float read_onboard_temperature(const char unit) {
    
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // Seleciona o sensor de temperatura interno

    /* Conversão de 12 bits, assume valor máximo == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (unit == 'C') {
        return tempC;
    } else if (unit == 'F') {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}

// --------------------------- Função de Inversão de String ---------------------------

/**
 * @brief Inverte uma string.
 *
 * @param str A string a ser invertida.
 * @param len O comprimento da string.
 *
 * Esta função inverte a string fornecida.
 *
 * ### Comportamento:
 * - Troca caracteres do início e do fim da string.
 * - Continua trocando até que o meio da string seja alcançado.
 *
 * @note A string invertida é armazenada no parâmetro `str`.
 */
void reverse(char* str, int len) 
{ 
    int i = 0, j = len - 1, temp; 
    while (i < j) { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; 
        j--; 
    } 
} 


// --------------------------- Função de Conversão de Inteiro para String ---------------------------

/**
 * @brief Converte um inteiro para uma string.
 *
 * @param x O inteiro a ser convertido.
 * @param str A string resultante.
 * @param d Número de dígitos necessários na saída.
 * @return int O comprimento da string resultante.
 *
 * Esta função converte um inteiro para uma string com o número especificado de dígitos.
 * Se o número de dígitos for maior que o número de dígitos no inteiro, zeros
 * são adicionados no início.
 *
 * ### Comportamento:
 * - Converte o inteiro para uma string.
 * - Adiciona zeros à esquerda se necessário.
 * - Inverte a string para obter a ordem correta.
 *
 * @note A string resultante é armazenada no parâmetro `str`.
 */
int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    if (x == 0) {
    str[i++] = '0'; // Adiciona o dígito 0
    }
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    } 
 
    // Se o número de dígitos necessário for maior, então 
    // adiciona zeros no início 
    while (i < d) 
        str[i++] = '0'; 
 
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
}


// --------------------------- Função de Conversão de Float para String ---------------------------

/**
 * @brief Converte um número de ponto flutuante para uma string.
 *
 * @param n O número de ponto flutuante a ser convertido.
 * @param res A string resultante.
 * @param afterpoint Número de dígitos após o ponto decimal.
 *
 * Esta função converte um número de ponto flutuante para uma string com o número
 * especificado de dígitos após o ponto decimal.
 *
 * ### Comportamento:
 * - Extrai a parte inteira do número.
 * - Extrai a parte fracionária do número.
 * - Converte a parte inteira para uma string.
 * - Adiciona um ponto decimal se necessário.
 * - Converte a parte fracionária para uma string.
 *
 * @note A string resultante é armazenada no parâmetro `res`.
 */
void ftoa(float n, char* res, int afterpoint) 
{ 
    // Extrai a parte inteira 
    int ipart = (int)n; 
 
    // Extrai a parte fracionária 
    float fpart = n - (float)ipart; 
 
    // Converte a parte inteira para string 
    int i = intToStr(ipart, res, 0); 
 
    // Verifica se é necessário exibir a parte fracionária 
    if (afterpoint != 0) { 
        res[i] = '.'; // Adiciona o ponto decimal 
 
        // Obtém o valor da parte fracionária até o número de 
        // pontos após o ponto decimal especificado. O terceiro parâmetro 
        // é necessário para lidar com casos como 233.007 
        fpart = fpart * pow(10, afterpoint); 
 
        intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
}


//------------------------------ Variáveis do Modo AP -------------------------------

char *ap_name = "PICO_W_AP";
char *ap_pw = "raspberry";

//-----------------------------------------------------------------------------------

volatile bool timer_expired = false;

//-----------------------------------------------------------------------------------

bool timer_callback(repeating_timer_t *rt) {
    timer_expired = true;
    return true; // Retorna true para continuar chamando o callback
}

void start_timer() {
    static repeating_timer_t timer;
    add_repeating_timer_ms(1000, timer_callback, NULL, &timer); // Timer de 1 segundo
}
// -------------------------- Função de Filtro Passa-Baixa --------------------------

/**
 * @brief Aplica um filtro passa-baixa para suavizar dados de entrada.
 *
 * @param new_value O novo valor de entrada a ser filtrado.
 * @return uint16_t O valor de saída filtrado.
 *
 * Esta função aplica um algoritmo de suavização usando uma média móvel exponencial.
 * O fator de suavização `alpha` determina a influência do novo valor em relação ao valor filtrado anterior.
 */
uint low_pass_filter(uint new_value) {
    float alpha = 0.5;            ///< Fator de suavização (0.0 a 1.0)
    static uint filtered_value = 0; ///< Valor suavizado (preservado entre as chamadas)

    filtered_value = (alpha * new_value) + ((1 - alpha) * filtered_value);
    return filtered_value;
}


// ------------------------------ Funções do Buzzer ------------------------------

/**
 * @brief Inicializa o PWM para o buzzer.
 *
 * Esta função configura a configuração do PWM para o buzzer conectado ao pino GPIO especificado.
 * Ela configura o pino GPIO para funcionalidade PWM, inicializa o slice PWM e define o nível inicial
 * do PWM para baixo.
 *
 * @param pin O pino GPIO conectado ao buzzer.
 *
 * @note A função realiza as seguintes operações:
 * - Configura o pino GPIO para funcionalidade PWM.
 * - Recupera o número do slice PWM para o pino GPIO especificado.
 * - Define a configuração padrão do PWM.
 * - Inicializa o slice PWM com a configuração.
 * - Define o nível inicial do PWM para baixo.
 *
 * @note A função depende das seguintes funções:
 *   - `gpio_set_function(pin, GPIO_FUNC_PWM)`: Configura o pino GPIO para funcionalidade PWM.
 *   - `pwm_gpio_to_slice_num(pin)`: Recupera o número do slice PWM para o pino GPIO especificado.
 *   - `pwm_get_default_config()`: Recupera a configuração padrão do PWM.
 *   - `pwm_config_set_clkdiv(&config, divisor)`: Define o divisor de clock para a configuração do PWM.
 *   - `pwm_init(slice_num, &config, true)`: Inicializa o slice PWM com a configuração.
 *   - `pwm_set_gpio_level(pin, level)`: Define o nível do PWM para o pino GPIO especificado.
 */
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f); // Divisor inicial
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0); // Inicia com nível baixo
}


/**
 * @brief Define a frequência do buzzer.
 *
 * Esta função configura as configurações do PWM para gerar uma frequência específica
 * no buzzer conectado ao pino GPIO especificado. Ela calcula o divisor de clock apropriado
 * e o valor máximo para o PWM com base na frequência desejada.
 *
 * @param pin O pino GPIO conectado ao buzzer.
 * @param frequency A frequência desejada para o buzzer em Hertz.
 *
 * @note A função realiza as seguintes operações:
 * - Calcula o divisor de clock e o valor máximo para o PWM com base na frequência desejada.
 * - Configura as configurações do PWM com os valores calculados.
 * - Define o ciclo de trabalho do PWM para 50% para gerar o tom.
 *
 * @note A função depende das seguintes funções:
 *   - `clock_get_hz(clk_sys)`: Recupera a frequência do clock do sistema.
 *   - `pwm_gpio_to_slice_num(pin)`: Recupera o número do slice PWM para o pino GPIO especificado.
 *   - `pwm_set_clkdiv(slice_num, divisor)`: Define o divisor de clock para o slice PWM.
 *   - `pwm_set_wrap(slice_num, top)`: Define o valor máximo para o slice PWM.
 *   - `pwm_set_gpio_level(pin, level)`: Define o ciclo de trabalho do PWM para o pino GPIO especificado.
 */
void set_buzzer_frequency(uint pin, float frequency) {
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Calcula o valor de "top" e ajusta o divisor
    uint32_t source_hz = clock_get_hz(clk_sys);
    float divisor = (float)source_hz / (frequency * 4096.0f);
    uint32_t top = source_hz / (frequency * divisor);

    pwm_set_clkdiv(slice_num, divisor); // Define o divisor
    pwm_set_wrap(slice_num, top - 1);  // Define o contador superior
    pwm_set_gpio_level(pin, top / 2);  // Ciclo de trabalho de 50%
}


/**
 * @brief Toca uma sequência de sons ao entrar no menu.
 *
 * Esta função gera uma sequência de tons usando um buzzer para indicar
 * que o menu está sendo acessado. Os tons variam com base na frequência atual
 * e podem ser crescentes, decrescentes ou uma combinação de ambos.
 *
 * @param pin O pino GPIO conectado ao buzzer.
 *
 * @note A função realiza as seguintes operações:
 * - Determina a sequência de frequências com base na frequência atual.
 * - Define a frequência do buzzer e ativa o buzzer para cada tom na sequência.
 * - Introduz um atraso entre cada tom.
 * - Desativa o buzzer após a sequência ser concluída.
 *
 * @note A função depende das seguintes variáveis externas:
 *   - `frequency`: A frequência atual usada para determinar a sequência de tons.
 */
void menu_enter_sound(uint pin) {
    
    uint frequencies[3];

    if (frequency <= 210) {
        frequencies[0] = frequency + 300;
        frequencies[1] = 10;
        frequencies[2] = 110; // Tons crescentes
    } else if (frequency >= 1710) {
        frequencies[0] = 2010;
        frequencies[1] = frequency - 200;
        frequencies[2] = frequency - 100; // Tons decrescentes
    } else {
        frequencies[0] = frequency + 300;
        frequencies[1] = frequency - 200;
        frequencies[2] = frequency - 100; // Combinação de tons
    }

    uint duration_ms = 75; // Duração de cada tom

    for (int i = 0; i < 3; i++) {
        set_buzzer_frequency(pin, frequencies[i]); 	// Configura o tom
        pwm_set_gpio_level(pin, 2048);            	// Ativa o buzzer
        sleep_ms(duration_ms);                   	// Duração do tom
    }
    pwm_set_gpio_level(pin, 0); // Desativa o buzzer
}


/**
 * @brief Toca uma sequência de sons ao sair do menu.
 *
 * Esta função gera uma sequência de tons usando um buzzer para indicar
 * que o menu está sendo fechado. Os tons variam com base na frequência atual
 * e podem ser crescentes, decrescentes ou uma combinação de ambos.
 *
 * @param pin O pino GPIO conectado ao buzzer.
 *
 * @note A função realiza as seguintes operações:
 * - Determina a sequência de frequências com base na frequência atual.
 * - Define a frequência do buzzer e ativa o buzzer para cada tom na sequência.
 * - Introduz um atraso entre cada tom.
 * - Desativa o buzzer após a sequência ser concluída.
 *
 * @note A função depende das seguintes variáveis externas:
 *   - `frequency`: A frequência atual usada para determinar a sequência de tons.
 */
void menu_exit_sound(uint pin) {

    // Frequências para o som de saída  
    uint frequencies[3];

    if (frequency <= 310) {
        frequencies[0] = 10;
        frequencies[1] = frequency + 200;
        frequencies[2] = frequency + 100; // Tons crescentes
    } else if (frequency >= 1810) {
        frequencies[0] = frequency - 300;
        frequencies[1] = 2010;
        frequencies[2] = 1910; // Tons decrescentes
    } else {
        frequencies[0] = frequency - 300;
        frequencies[1] = frequency + 200;
        frequencies[2] = frequency + 100; // Combinação de tons
    }

    uint duration_ms = 75; // Duração de cada tom

    for (int i = 0; i < 3; i++) {
        set_buzzer_frequency(pin, frequencies[i]); // Configura o tom
        pwm_set_gpio_level(pin, 2048);            // Ativa o buzzer
        sleep_ms(duration_ms);                   // Duração do tom
    }
    pwm_set_gpio_level(pin, 0); // Desativa o buzzer
}


// ---------------------------- Função de Escape ----------------------------

/**
 * @brief Lida com a funcionalidade de escape para a tela inicial.
 *
 * Esta função atualiza o display SSD1306, introduz um atraso e alterna
 * o estado da tela atual. Ela é tipicamente usada para fornecer feedback visual
 * e transição entre diferentes telas de menu.
 *
 * @note A função depende das seguintes variáveis externas:
 *   - `current_screen`: Uma flag indicando o estado da tela atual.
 */
void scape_function(void){
    ssd1306_UpdateScreen();
    sleep_ms(2000);
    current_screen = !current_screen;
}


// ---------------------------- Função de Renderização da Tela Inicial ----------------------------

/**
 * @brief Renderiza o menu em um display SSD1306.
 *
 * Esta função é responsável por desenhar uma interface de tela inicial em um display SSD1306
 * usando o protocolo I2C. Ela exibe ícones e nomes de opções correspondentes para os
 * itens de menu atual, anterior e próximo. Também inclui um contorno de seleção e
 * uma barra de rolagem para feedback visual de navegação.
 *
 * @note A função realiza as seguintes operações:
 * - Limpa o display definindo todos os pixels para preto.
 * - Escreve os nomes e ícones dos itens de menu anterior, atual e próximo.
 * - Desenha um contorno retangular para destacar o item de menu selecionado.
 * - Renderiza uma barra de rolagem no lado direito do display, indicando a posição atual
 *   no menu.
 *
 * @note A função depende das funções do driver SSD1306:
 *   - `ssd1306_Fill(Black)`: Limpa o display definindo todos os pixels para preto.
 *   - `ssd1306_SetCursor`: Posiciona o cursor em uma localização (x, y) especificada no display.
 *   - `ssd1306_WriteString`: Escreve uma string em uma posição (x, y) especificada.
 *   - `ssd1306_DrawBitmap`: Renderiza uma imagem bitmap (ícone) em uma posição (x, y) especificada.
 *   - `ssd1306_DrawRectangle`: Desenha um retângulo nas coordenadas especificadas.
 *
 * @note Assume que os arrays `menu_items` e `bitmap_icons` estão definidos e populados
 *       com as strings e dados bitmap correspondentes para as opções da tela inicial.
 */
void home_screen(void) {
    // Limpa o display
    ssd1306_Fill(Black);

    // Exibe o item de menu anterior
    ssd1306_SetCursor(25, 5);
    ssd1306_WriteString(menu_items[item_sel_previous], Font_7x10, White);
    ssd1306_DrawBitmap(4, 2, bitmap_icons[item_sel_previous], 16, 16, White);

    // Exibe o item de menu atual
    ssd1306_SetCursor(25, 5 + 20 + 2);
    ssd1306_WriteString(menu_items[item_selected], Font_7x10, White);
    ssd1306_DrawBitmap(4, 24, bitmap_icons[item_selected], 16, 16, White);

    // Exibe o item de menu próximo
    ssd1306_SetCursor(25, 5 + 20 + 20 + 2 + 2);
    ssd1306_WriteString(menu_items[item_sel_next], Font_7x10, White);
    ssd1306_DrawBitmap(4, 46, bitmap_icons[item_sel_next], 16, 16, White);

    // Desenha o contorno de seleção
    ssd1306_DrawBitmap(0, 22, bitmap_item_sel_outline, 128, 21, White);

    // Desenha o fundo da barra de rolagem
    ssd1306_DrawBitmap(128 - 8, 0, bitmap_scrollbar_background, 8, 64, White);

    // Desenha a barra de rolagem indicando a posição atual
    ssd1306_DrawRectangle(125, 64 / NUM_ITEMS * item_selected, 128,
            (64 / NUM_ITEMS * item_selected + (64 / NUM_ITEMS)), White);
    ssd1306_DrawRectangle(126, 64 / NUM_ITEMS * item_selected, 127,
            (64 / NUM_ITEMS * item_selected + (64 / NUM_ITEMS)), White);
}


// ---------------------------- Função de Atualização da Posição do Cursor ----------------------------

/**
 * @brief Atualiza a posição do cursor com base na entrada do joystick.
 *
 * Esta função lê o valor do eixo Y de um joystick analógico e atualiza a posição do cursor
 * no menu de acordo. Ela usa um filtro passa-baixa para suavizar a entrada do joystick
 * e verifica limites específicos para determinar se o cursor deve se mover para cima
 * ou para baixo. A função também lida com a rotação dos itens do menu quando o cursor
 * atinge o topo ou o fundo.
 *
 * @note A função realiza as seguintes operações:
 * - Lê o valor bruto do ADC do eixo Y do joystick.
 * - Aplica um filtro passa-baixa ao valor bruto do ADC.
 * - Verifica se o joystick foi movido para cima ou para baixo com base nos limites filtrados do ADC.
 * - Atualiza a posição do cursor e o índice do item de menu selecionado.
 * - Lida com a rotação do cursor e do índice do item de menu ao atingir os limites.
 *
 * @note A função depende das seguintes variáveis externas:
 *   - `up_clicked`: Uma flag indicando se o movimento para cima do joystick foi detectado.
 *   - `down_clicked`: Uma flag indicando se o movimento para baixo do joystick foi detectado.
 *   - `cursor`: A posição atual do cursor no menu.
 *   - `item_selected`: O índice do item de menu atualmente selecionado.
 *   - `NUM_ITEMS`: O número total de itens do menu.
 *
 * @note A função assume que o ADC está devidamente inicializado e configurado.
 */
void update_cursor(void){

    adc_select_input(0);
    uint adc_y_raw = adc_read();
    uint filtered_read = low_pass_filter(adc_y_raw);

    // Verifica o estado do joystick para cima
    if ((filtered_read > 3000) && up_clicked == 0) {
        up_clicked = 1; // Marca como pressionado
        cursor--;
        if (cursor == -1)
            cursor = 3;
        item_selected--;
        if (item_selected < 0)
            item_selected = NUM_ITEMS - 1;
    }
    // Libera o botão para cima
    if (filtered_read <= 3000) {
        up_clicked = 0;
    }

    // Verifica o estado do joystick para baixo
    if ((filtered_read < 1100) && down_clicked == 0) {
        down_clicked = 1; // Marca como pressionado
        cursor++;
        if (cursor == 4)
            cursor = 0;
        item_selected++;
        if (item_selected >= NUM_ITEMS)
            item_selected = 0;
    }
    // Libera o botão para baixo
    if (filtered_read >= 1100) {
        down_clicked = 0;
    }
}


// ---------------------------- Função de Renderização do Menu ----------------------------

/**
 * @brief Renderiza o menu em um display SSD1306.
 *
 * Esta função é responsável por desenhar uma interface de tela inicial em um display SSD1306
 * usando o protocolo I2C. Ela exibe ícones e nomes de opções correspondentes para os
 * itens de menu atual, anterior e próximo. Também inclui um contorno de seleção e
 * uma barra de rolagem para feedback visual de navegação.
 *
 * @note A função realiza as seguintes operações:
 * - Limpa o display definindo todos os pixels para branco.
 * - Escreve os nomes e ícones dos itens de menu anterior, atual e próximo.
 * - Desenha um contorno retangular para destacar o item de menu selecionado.
 * - Renderiza uma barra de rolagem no lado direito do display, indicando a posição atual
 *   no menu.
 *
 * @note A função depende das funções do driver SSD1306:
 *   - `ssd1306_Fill(0)`: Limpa o display definindo todos os pixels para uma cor específica.
 *   - `ssd1306_SetCursor`: Posiciona o cursor em uma localização (x, y) especificada no display.
 *   - `ssd1306_WriteString`: Escreve uma string em uma posição especificada (x, y).
 *   - `ssd1306_DrawBitmap`: Renderiza uma imagem bitmap (ícone) em uma posição especificada (x, y).
 *   - `ssd1306_DrawRectangle`: Desenha um retângulo nas coordenadas especificadas.
 *
 * @note Assume que os arrays `menu_items` e `bitmap_icons` estão definidos e populados
 *       com as strings e dados bitmap correspondentes para as opções da tela inicial.
 */
void menu(void) {

    if (current_screen == 0) {

        update_cursor();    // Atualiza o cursor com o joystick
        home_screen();      // Atualiza a Tela Inicial no Display OLED
    }

    if(current_screen) {

        // Preenche o display com uma cor específica
        ssd1306_Fill(Black);

        // OPÇÃO Servidor Web
        if (item_selected == 0){
            // Função externa para utilizar e amostrar a funcionalidade do sensor ultrassônico

            // Exibe o cabeçalho
            ssd1306_SetCursor(45, 1);
            ssd1306_WriteString("CLOUD:", Font_7x10, White);
            ssd1306_FillRectangle(1, 15, 128, 16, White);
            ssd1306_DrawRectangle(1, 20, 127, 63, White);

            if(!start_wifi){

            //start_http_server();
            start_timer();
            start_wifi = 1;

            } else {


                if(timer_expired){
                    float temperature = read_onboard_temperature(TEMPERATURE_UNITS);

                    build_http_request(temperature);
                    timer_expired = false;
                }

            }

            cyw43_arch_poll();  // Necessário para manter o Wi-Fi ativo
        } 

        // OPÇÃO Conexão MQTT
        else if (item_selected == 1){
            // Função externa para usar e amostrar a funcionalidade do giroscópio
            ssd1306_SetCursor(20, 1);
            ssd1306_WriteString("SYSTEM SETUP: ", Font_7x10, 1);
            ssd1306_FillRectangle(1, 15, 128, 16, 1);	// Desenha o retângulo do cabeçalho
            ssd1306_DrawRectangle(1, 20, 127, 63, 1);	// Desenha o retângulo principal do display

        }
        
        // OPÇÃO BUZZER
        else if (item_selected == 2){
            // Função externa para usar e amostrar a funcionalidade do filtro de Kalman

            char buffer_string[7];	// Buffer para armazenar valores formatados em string
            ssd1306_SetCursor(25, 1);
            ssd1306_WriteString("BUZZER PWM: ", Font_7x10, 1);
            ssd1306_FillRectangle(1, 15, 128, 16, 1);	// Desenha o retângulo do cabeçalho
            ssd1306_DrawRectangle(1, 20, 127, 63, 1);	// Desenha o retângulo principal do display

            // Lê o valor ADC e filtra
            adc_select_input(1);
            uint adc_x_raw = adc_read();
            uint filtered_read = low_pass_filter(adc_x_raw);

            // Ajuste da frequência
            if (adc_x_raw > ADC_UPPER_THRESHOLD && frequency < MAX_FREQUENCY) {
                frequency += STEP;
            } else if (adc_x_raw < ADC_LOWER_THRESHOLD && frequency > MIN_FREQUENCY) {
                frequency -= STEP;
            }

            // Configura o buzzer com a nova frequência
            set_buzzer_frequency(BUZZER_PIN, frequency);

            // Atualiza a barra no display
            uint8_t x_distance = (uint8_t)(((frequency - MIN_FREQUENCY) * 128) / (MAX_FREQUENCY - MIN_FREQUENCY)) + 1;
            ssd1306_DrawRectangle(1, 48, 128, 63, 1);
            ssd1306_FillRectangle(1, 48, x_distance, 63, 1); // Barra horizontal

            // Mostra a frequência atual
            ssd1306_SetCursor(25, 30);
            sprintf(buffer_string, "FREQ: %u Hz", (uint)frequency);
            ssd1306_WriteString(buffer_string, Font_7x10, 1);

        }
        
        // OPÇÃO Informações da Rede
        else if (item_selected == 3){
            // Função externa para usar e amostrar a funcionalidade de calibração do sensor inercial
            ssd1306_SetCursor(22, 1);
            ssd1306_WriteString("NETWORK INFO: ", Font_7x10, 1);
            ssd1306_FillRectangle(1, 15, 128, 16, 1);	// Desenha o retângulo do cabeçalho
            ssd1306_DrawRectangle(1, 20, 127, 63, 1);	// Desenha o retângulo principal do display
            ssd1306_DrawRectangle(28, 20, 28, 63, 1);	// Separador Vertical

            char buffer_string[7];                      // Buffer para armazenar valores formatados em string
            uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
            sprintf(buffer_string, "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
            ssd1306_SetCursor(3, 23);
            ssd1306_WriteString("IP", Font_6x8, 1);
            ssd1306_SetCursor(53, 23);
            ssd1306_WriteString(buffer_string, Font_6x8, 1);
            ssd1306_DrawRectangle(1, 31, 127, 31, 1);	// Separador horizontal
            
            int32_t rssi;
            cyw43_wifi_get_rssi(&cyw43_state, &rssi);
            sprintf(buffer_string, "%d dBm", rssi);
            ssd1306_SetCursor(3, 33);
            ssd1306_WriteString("RSSI", Font_6x8, 1);
            ssd1306_SetCursor(80, 33);
            ssd1306_WriteString(buffer_string, Font_6x8, 1);
            ssd1306_DrawRectangle(1, 41, 127, 41, 1);	// Separador horizontal

            ssd1306_SetCursor(3, 43);
            ssd1306_WriteString("MQTT", Font_6x8, 1);
            ssd1306_DrawRectangle(1, 51, 127, 41, 1);	// Separador horizontal

            if(connected_mqtt){
                ssd1306_SetCursor(70, 43);
                ssd1306_WriteString("CONNECTED", Font_6x8, 1);
            } else {
                ssd1306_SetCursor(52, 43);
                ssd1306_WriteString("DISCONNECTED", Font_6x8, 1);
            }

            ssd1306_SetCursor(3, 53);
            ssd1306_WriteString("WEB", Font_6x8, 1);

            if(start_wifi){

                ssd1306_SetCursor(70, 54);
                ssd1306_WriteString("CONNECTED", Font_6x8, 1);
            } else {
                ssd1306_SetCursor(52, 54);
                ssd1306_WriteString("DISCONNECTED", Font_6x8, 1);
            }
        }
    }

    if (!(gpio_get(BUTTON_B)) && button_enter_clicked == 0) {

        button_enter_clicked = 1;

        // Desliga o buzzer
        pwm_set_gpio_level(BUZZER_PIN, 0);

        if(item_selected != 2){
        
        if(current_screen)
            menu_enter_sound(BUZZER_PIN);
        else
            menu_exit_sound(BUZZER_PIN);
        }

        // Alterna para o outro tipo de tela
        current_screen = !current_screen;
        
    }

    // Se o botão ENTER foi liberado, a variável auxiliar retorna para baixo
    if ((gpio_get(BUTTON_B)) && button_enter_clicked == 1) {
        button_enter_clicked = 0;
    }

/*------------------------- Lógica para imprimir os itens corretos ----------------------------*/

    item_sel_previous = item_selected - 1;
    if (item_sel_previous < 0) {
        item_sel_previous = NUM_ITEMS - 1;
    } // o item anterior estaria abaixo do primeiro = torná-lo o último
    item_sel_next = item_selected + 1;
    if (item_sel_next >= NUM_ITEMS) {
        item_sel_next = 0;
    } // o próximo item estaria após o último = torná-lo o primeiro

/*---------------------------------------------------------------------------------------*/

    ssd1306_UpdateScreen();
}

void menu_ap (void){

    char buffer_sg[7];
    ssd1306_SetCursor(40, 1);
    ssd1306_WriteString("AP-MODE", Font_7x10, White);
    ssd1306_FillRectangle(1, 15, 128, 16, White);
    ssd1306_DrawRectangle(1, 20, 127, 63, White);
    ssd1306_SetCursor(4, 22);
    ssd1306_WriteString("ssid: ", Font_6x8, White);
    ssd1306_WriteString(ap_name, Font_6x8, White);
    ssd1306_SetCursor(4, 33);
    ssd1306_WriteString("pw: ", Font_6x8, White);
    ssd1306_WriteString(ap_pw, Font_6x8, White);
    ssd1306_SetCursor(4, 44);
    ssd1306_WriteString("192.168.4.1", Font_6x8, White);
    ssd1306_UpdateScreen();

}
#endif /*MENU_H*/