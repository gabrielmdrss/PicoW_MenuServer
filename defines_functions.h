#ifndef DEFINES_FUNCTIONS_H
#define DEFINES_FUNCTIONS_H

#include <math.h>								// Biblioteca matemática para operações matemáticas.
#include <string.h>								// Biblioteca padrão para operações com strings.	
#include <stdlib.h>								// Biblioteca padrão para funções de propósito geral.
#include <ctype.h>								// Biblioteca padrão para funções de caracteres.
#include "pico/stdlib.h"						// Biblioteca padrão para Raspberry Pi Pico.
#include "pico/binary_info.h"					// Biblioteca para informações binárias.
#include "hardware/pwm.h"						// Biblioteca para operações com PWM (Modulação por Largura de Pulso).
#include "hardware/clocks.h"					// Biblioteca para configuração de clocks.
#include "ssd1306/ssd1306_fonts.h"				// Arquivo contendo fontes para o display SSD1306.
#include "ssd1306/ssd1306.h"					// Arquivo contendo funções para o display SSD1306.
#include "hardware/timer.h"                     // Biblioteca para operações com temporizadores.     
#include "defines_functions.h"                  // Arquivo contendo definições e funções para o projeto.
#include "lwip/tcpip.h"                         // Certifique-se de incluir a biblioteca LWIP


/*-------------------------------------- DEFINES ----------------------------------------*/

#define BUTTON_A 5                  // Botão para alterar opções/configurações.
#define BUTTON_B 6                  // Botão para confirmar/entrar em uma seleção.
#define BUZZER_PIN 21    			// Pino do buzzer
#define MIN_FREQUENCY 10 			// Frequência mínima do buzzer (Hz)
#define MAX_FREQUENCY 2000 			// Frequência máxima do buzzer (Hz)
#define ADC_UPPER_THRESHOLD 3500 	// Limite superior do ADC para incrementar a frequência
#define ADC_LOWER_THRESHOLD 850  	// Limite inferior do ADC para decrementar a frequência
#define STEP 20              		// Incremento ou decremento por iteração
#define TEMPERATURE_UNITS 'C'       // Unidade para medição de temperatura.

/*------------------------------------- VARIÁVEIS ---------------------------------------*/

float lat, lon;                         // Variáveis para armazenar latitude e longitude
float frequency = MIN_FREQUENCY;        // Frequência inicial do buzzer
int Limit_Buzzer = 0;                   // Limite do buzzer
uint8_t x_distance;                     // Distância no eixo X da barra de progresso
int start_wifi = 0;                     // Flag para indicar se o Wi-Fi está conectado
float temperature;                      // Variável para armazenar a temperatura
int percentual = 0;                     // Variável para armazenar o percentual da barra de progresso
char *ap_name = "PICO_W_AP";            // Nome da rede Wi-Fi no modo AP
char *ap_pw = "raspberry";              // Senha da rede Wi-Fi no modo AP
volatile bool timer_expired = false;    // Flag para indicar que o timer expirou
int inicialized = 0;                    // Flag para indicar se o sistema foi inicializado

/*--------------------------------------- FUNÇÕES ----------------------------------------*/

// --------------------------- Função de Geração de Coordenadas Aleatórias ---------------------------

/**
 * @brief Gera valores aleatórios dentro das faixas especificadas.
 *
 * @param lat Ponteiro para armazenar a latitude gerada.
 * @param lon Ponteiro para armazenar a longitude gerada.
 *
 * Esta função gera valores aleatórios para latitude e longitude dentro das faixas especificadas.
 *
 * ### Comportamento:
 * - Gera um valor aleatório para a latitude dentro da faixa especificada.
 * - Gera um valor aleatório para a longitude dentro da faixa especificada.
 *
 * @note Os valores gerados são armazenados nos parâmetros `lat` e `lon`.
 */
void generate_random_coordinates(float *lat, float *lon) {
    float lat_min = -5.813619388080041;
    float lat_max = -5.81018697695044;
    float lon_min = -35.20495186786747;
    float lon_max = -35.201481075005;

    *lat = lat_min + ((float)rand() / RAND_MAX) * (lat_max - lat_min);
    *lon = lon_min + ((float)rand() / RAND_MAX) * (lon_max - lon_min);
}


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


// --------------------------- Função de Callback do Timer ---------------------------

/**
 * @brief Função de callback para o timer.
 *
 * @param rt Ponteiro para a estrutura do timer repetitivo.
 * @return bool Retorna true para continuar chamando o callback.
 *
 * Esta função é chamada quando o timer expira. Ela define a flag `timer_expired` como true
 * para indicar que o timer expirou.
 *
 * ### Comportamento:
 * - Define a flag `timer_expired` como true.
 * - Retorna true para continuar chamando o callback.
 *
 * @note A função depende da variável externa `timer_expired`.
 */
bool timer_callback(repeating_timer_t *rt) {
    timer_expired = true;
    return true; // Retorna true para continuar chamando o callback
}

// --------------------------- Função para Iniciar o Timer ---------------------------

/**
 * @brief Inicia um timer repetitivo.
 *
 * Esta função cria e inicia um timer repetitivo que chama a função de callback `timer_callback`
 * a cada 2000 milissegundos.
 *
 * ### Comportamento:
 * - Cria um timer repetitivo.
 * - Inicia o timer com um intervalo de 2000 milissegundos.
 * - Associa a função de callback `timer_callback` ao timer.
 *
 * @note A função depende da função `add_repeating_timer_ms` para criar e iniciar o timer.
 */
void start_timer() {
    static repeating_timer_t timer;
    add_repeating_timer_ms(2000, timer_callback, NULL, &timer); // Timer de 2 segundos
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
    float alpha = 0.5;              // Fator de suavização (0.0 a 1.0)
    static uint filtered_value = 0; // Valor suavizado (preservado entre as chamadas)

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
    pwm_config_set_clkdiv(&config, 1.0f);   // Divisor inicial
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);             // Inicia com nível baixo
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
    pwm_set_wrap(slice_num, top - 1);   // Define o contador superior
    pwm_set_gpio_level(pin, top / 2);   // Ciclo de trabalho de 50%
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


// ---------------------------- Função de Inicialização Não Concluída ----------------------------

/**
 * @brief Exibe uma mensagem de inicialização não concluída no display SSD1306.
 *
 * Esta função exibe uma mensagem no display SSD1306 indicando que a inicialização
 * do BitDogLab não foi concluída. Ela também chama a função de escape para fornecer
 * feedback visual e transição entre diferentes telas de menu.
 *
 * ### Comportamento:
 * - Posiciona o cursor no display.
 * - Exibe a mensagem "Inicialize BitDogLab!".
 * - Exibe a mensagem "Pressione:".
 * - Exibe a mensagem "<System Setup>".
 * - Chama a função de escape para atualizar o display e alternar o estado da tela.
 *
 * @note A função depende das funções do driver SSD1306:
 *   - `ssd1306_SetCursor`: Posiciona o cursor em uma localização (x, y) especificada no display.
 *   - `ssd1306_WriteString`: Escreve uma string em uma posição especificada (x, y).
 *   - `scape_function`: Atualiza o display e alterna o estado da tela.
 */
void not_initialized(void) {
    ssd1306_SetCursor(4, 23);
    ssd1306_WriteString("Inicialize BitDogLab!", Font_6x8, White);

    ssd1306_SetCursor(37, 40);
    ssd1306_WriteString("Pressione:", Font_6x8, White);

    ssd1306_SetCursor(22, 50);
    ssd1306_WriteString("<System Setup>", Font_6x8, White);
    scape_function();
}


// ---------------------------- Função de Cabeçalho ----------------------------

/**
 * @brief Exibe um cabeçalho no display SSD1306.
 *
 * @param titulo O título a ser exibido no cabeçalho.
 * @param x A posição x do cursor no display.
 * @param y A posição y do cursor no display.
 *
 * Esta função exibe um cabeçalho no display SSD1306 com o título especificado.
 * Ela também desenha um retângulo ao redor do cabeçalho para destacá-lo.
 *
 * ### Comportamento:
 * - Posiciona o cursor no display.
 * - Exibe o título no cabeçalho.
 * - Desenha um retângulo ao redor do cabeçalho.
 *
 * @note A função depende das funções do driver SSD1306:
 *   - `ssd1306_SetCursor`: Posiciona o cursor em uma localização (x, y) especificada no display.
 *   - `ssd1306_WriteString`: Escreve uma string em uma posição especificada (x, y).
 *   - `ssd1306_FillRectangle`: Preenche um retângulo no display.
 *   - `ssd1306_DrawRectangle`: Desenha um retângulo no display.
 */
void cabecalho(char *titulo, int x, int y) {
    ssd1306_SetCursor(x, y);
    ssd1306_WriteString(titulo, Font_7x10, White);
    ssd1306_FillRectangle(1, 15, 128, 16, White);
    ssd1306_DrawRectangle(1, 20, 127, 63, White);
}

#endif  /*DEFINES_FUNCTIONS_H*/