#ifndef MQTT_UTILITY_H
#define MQTT_UTILITY_H

/******************************************************************************
 * @file    mqtt_utility.h
 * @brief   Arquivo contendo definições e protótipos de funções para a
 *          aplicação de comunicação MQTT conectada a um broker local
 *          no Raspberry Pi Pico W.
 *
 * @authors Gabriel Domingos de Medeiros
 * @date    Fevereiro 2025
 * @version 1.0.0
 *
 * @note    Este arquivo inclui as definições, constantes e protótipos de funções
 *          necessários para a aplicação.
 ******************************************************************************/

// ---------------------------------- Includes ---------------------------------

#include <string.h>         // Biblioteca padrão para operações com strings.
#include "pico/stdlib.h"    // Biblioteca padrão para Raspberry Pi Pico.
#include "pico/cyw43_arch.h"// Biblioteca para usar o módulo de conectividade para Raspberry Pi Pico W.
#include "lwip/apps/mqtt.h" // Biblioteca MQTT para lidar com o protocolo MQTT.
#include "hardware/adc.h"   // Biblioteca para operações com ADC (Conversor Analógico-Digital).
#include "hardware/pwm.h"   // Biblioteca para operações com PWM (Modulação por Largura de Pulso).
#include "math.h"           // Biblioteca matemática para operações matemáticas.
#include "bsp/board.h"      // Pacote de suporte à placa para abstração de hardware.
#include "pico/binary_info.h"// Biblioteca para informações binárias.


// ----------------------------------- Defines ----------------------------------

#define LED_RED_PIN 13      // Número do pino para o LED vermelho.
#define LED_GREEN_PIN 11    // Número do pino para o LED verde.
#define LED_BLUE_PIN 12     // Número do pino para o LED azul.

#define MQTT_SERVER "10.220.0.83" // Endereço do servidor MQTT (broker local).
#define SUBS_STR_NAME "LED/TPC"   // Tópico MQTT para assinar mensagens de controle do LED.
#define PUBLISH_STR_NAME "TEMP/TPC" // Tópico MQTT para publicar dados de temperatura.

/* Escolha 'C' para Celsius ou 'F' para Fahrenheit. */
#define TEMPERATURE_UNITS 'C'     // Unidade para medição de temperatura.

struct mqtt_connect_client_info_t mqtt_client_info = {
    "<RA>/pico_w", /* client id */
    NULL,           /* user */
    NULL,           /* pass */
    0,              /* keep alive */
    NULL,           /* will_topic */
    NULL,           /* will_msg */
    0,              /* will_qos */
    0               /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    , NULL
#endif
};


// ---------------------------------- Variáveis ---------------------------------

char tempString[12];    // String contendo o valor convertido da temperatura interna
int cont_envio;         // Contador que ditará a velocidade com que a temperatura é enviada ao Broker
char last_led[32] = ""; // Declaração global


// ----------------------------------- Funções ----------------------------------

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_request_cb(void *arg, err_t err);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
float read_onboard_temperature(const char unit);
void ftoa(float n, char* res, int afterpoint);
void ftoa(float n, char* res, int afterpoint);
int intToStr(int x, char str[], int d);
void reverse(char* str, int len);


// -------------------------- Função de Callback de Dados Recebidos -------------------------

/**
 * @brief Função de callback para dados MQTT recebidos.
 *
 * @param arg Argumento definido pelo usuário (não utilizado).
 * @param data Ponteiro para os dados recebidos.
 * @param len Comprimento dos dados recebidos.
 * @param flags Flags indicando o status dos dados recebidos.
 *
 * Esta função é chamada quando dados são recebidos em um tópico MQTT assinado.
 * Ela processa os dados e ajusta as cores do LED de acordo.
 *
 * ### Comportamento:
 * - Converte os dados recebidos em uma string.
 * - Analisa a string para extrair valores RGB.
 * - Ajusta as cores do LED com base nos valores analisados.
 * - Garante que os valores RGB estejam dentro do intervalo válido (0-255).
 * - Atualiza a variável global `last_led` com a mensagem recebida.
 *
 * @note Os dados devem estar no formato "R/G/B", onde R, G e B são inteiros.
 * @note Se o formato dos dados for inválido, uma mensagem de erro é impressa.
 */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Mensagem recebida: %.*s\n", len, data);

    // Converte a mensagem para uma string
    char message[32];
    strncpy(message, (char *)data, len);
    message[len] = '\0';

    // Variáveis para armazenar os valores R, G e B
    int red, green, blue;

    // Analisa a mensagem no formato R/G/B
    if (sscanf(message, "%d/%d/%d", &red, &green, &blue) == 3) {
        // Garante que os valores estejam entre 0 e 255
        red = red < 0 ? 0 : (red > 255 ? 255 : red);
        green = green < 0 ? 0 : (green > 255 ? 255 : green);
        blue = blue < 0 ? 0 : (blue > 255 ? 255 : blue);

        // printf("R = %d, G = %d, B = %d\n", red, green, blue);

        // Converte os valores para PWM (0-255 para 0-100% ciclo de trabalho)
        pwm_set_gpio_level(LED_RED_PIN, red);
        pwm_set_gpio_level(LED_GREEN_PIN, green);
        pwm_set_gpio_level(LED_BLUE_PIN, blue);

        printf("LED ajustado para: R=%d, G=%d, B=%d\n", red, green, blue);

        strncpy(last_led, message, sizeof(last_led) - 1);
        last_led[sizeof(last_led) - 1] = '\0'; // Garante a terminação da string

        printf("Mensagem copiada para last_led: %s\n", message);

        printf("Valor de last_led: %s\n", last_led);

    } else {
        printf("Mensagem inválida! Use o formato R/G/B.\n");
    }
}


// --------------------------- Função de Callback de Publicação Recebida ---------------------------

/**
 * @brief Função de callback para mensagens de publicação MQTT recebidas.
 *
 * @param arg Argumento definido pelo usuário (não utilizado).
 * @param topic O tópico da mensagem recebida.
 * @param tot_len Comprimento total da mensagem recebida.
 *
 * Esta função é chamada quando uma mensagem é publicada em um tópico MQTT assinado.
 * Ela imprime o tópico recebido.
 *
 * ### Comportamento:
 * - Imprime o tópico da mensagem recebida.
 *
 * @note Esta função não processa o payload da mensagem.
 */
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Tópico recebido: %s\n", topic);
}


// --------------------------- Função de Callback de Requisição ---------------------------

/**
 * @brief Função de callback para requisições MQTT.
 *
 * @param arg Argumento definido pelo usuário (não utilizado).
 * @param err Status de erro da requisição.
 *
 * Esta função é chamada quando uma requisição MQTT é concluída.
 * Ela imprime o status da requisição.
 *
 * ### Comportamento:
 * - Imprime o status de erro da requisição concluída.
 *
 * @note Esta função é usada para lidar com a conclusão de requisições MQTT, como assinaturas.
 */
static void mqtt_request_cb(void *arg, err_t err) {
    printf("Callback de requisição MQTT: err %d\n", (int)err);
}


// --------------------------- Função de Callback de Conexão ---------------------------

/**
 * @brief Função de callback para o status da conexão MQTT.
 *
 * @param client Ponteiro para a estrutura do cliente MQTT.
 * @param arg Argumento definido pelo usuário (não utilizado).
 * @param status Status da conexão.
 *
 * Esta função é chamada quando o status da conexão MQTT muda.
 * Ela lida com a assinatura do tópico após a conexão ser aceita.
 *
 * ### Comportamento:
 * - Imprime o status da conexão.
 * - Assina o tópico especificado se a conexão for aceita.
 * - Imprime mensagem de sucesso ou falha para a assinatura.
 *
 * @note Esta função é invocada automaticamente quando o status da conexão muda.
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    printf("Status da conexão MQTT: %d\n", (int)status);

    if (status == MQTT_CONNECT_ACCEPTED) {
        err_t erro = mqtt_sub_unsub(client, SUBS_STR_NAME, 0, &mqtt_request_cb, NULL, 1);
        if (erro == ERR_OK) {
            printf("Assinatura do tópico '%s' bem-sucedida!\n", SUBS_STR_NAME);
        } else {
            printf("Falha ao assinar o tópico '%s'!\n", SUBS_STR_NAME);
        }
    } else {
        printf("Conexão rejeitada!\n");
    }
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


// --------------------------- Função de Configuração do PWM ---------------------------

/**
 * @brief Configura o PWM para o LED RGB.
 *
 * Esta função configura o PWM para os pinos do LED RGB e os inicializa como desligados.
 *
 * ### Comportamento:
 * - Define a função GPIO para os pinos do LED como PWM.
 * - Configura os slices PWM para cada pino do LED.
 * - Define a frequência PWM para 1 kHz.
 * - Inicializa os slices PWM.
 * - Define o valor de wrap do PWM para 255.
 * - Inicializa os pinos do LED como desligados.
 *
 * @note Esta função deve ser chamada durante a fase de configuração da aplicação.
 */
void configure_pwm() {
    // Habilita PWM nos pinos do LED RGB
    gpio_set_function(LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE_PIN, GPIO_FUNC_PWM);

    // Obtém os slices PWM para cada pino
    uint slice_red = pwm_gpio_to_slice_num(LED_RED_PIN);
    uint slice_green = pwm_gpio_to_slice_num(LED_GREEN_PIN);
    uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE_PIN);

    // Define PWM para frequência de 1 kHz
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 2.0f); // Ajusta o divisor de clock

    // Inicializa PWM nos slices correspondentes
    pwm_init(slice_red, &config, true);
    pwm_init(slice_green, &config, true);
    pwm_init(slice_blue, &config, true);

    pwm_set_wrap(slice_red, 255);
    pwm_set_wrap(slice_green, 255);
    pwm_set_wrap(slice_blue, 255);


    // Inicializa todos os pinos com 0 (LED desligado)
    pwm_set_gpio_level(LED_RED_PIN, 0);
    pwm_set_gpio_level(LED_GREEN_PIN, 0);
    pwm_set_gpio_level(LED_BLUE_PIN, 0);
}

#endif /*MQTT_UTILITY_H*/