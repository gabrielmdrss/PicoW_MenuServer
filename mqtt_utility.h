#ifndef MQTT_UTILITY_H
#define MQTT_UTILITY_H

#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "math.h"
#include "bsp/board.h"
#include "pico/binary_info.h"

#define LED_RED_PIN 13    // Pino do vermelho
#define LED_GREEN_PIN 11  // Pino do verde
#define LED_BLUE_PIN 12   // Pino do azul

#define MQTT_SERVER "10.220.0.83" //"broker.emqx.io" available at https://www.emqx.com/en/mqtt/public-mqtt5-broker
#define SUBS_STR_NAME "LED/COMMAND"
#define PUBLISH_STR_NAME "testy/temp"

/* Choose 'C' for Celsius or 'F' for Fahrenheit. */
#define TEMPERATURE_UNITS 'C'

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

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_request_cb(void *arg, err_t err);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

float read_onboard_temperature(const char unit);
void ftoa(float n, char* res, int afterpoint);
void ftoa(float n, char* res, int afterpoint);
int intToStr(int x, char str[], int d);

void reverse(char* str, int len) ;
char tempString[6];
int cont_envio;
int check_current_screen;

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Mensagem recebida: %.*s\n", len, data);

    // Converte a mensagem em string
    char message[32];
    strncpy(message, (char *)data, len);
    message[len] = '\0';

    // Variáveis para armazenar os valores R, G e B
    int red, green, blue;

    // Parse da mensagem no formato R/G/B
    if (sscanf(message, "%d/%d/%d", &red, &green, &blue) == 3) {
        // Garante que os valores estejam entre 0 e 255
        red = red < 0 ? 0 : (red > 255 ? 255 : red);
        green = green < 0 ? 0 : (green > 255 ? 255 : green);
        blue = blue < 0 ? 0 : (blue > 255 ? 255 : blue);

        // printf("R = %d, G = %d, B = %d\n", red, green, blue);

        // Converte os valores para o PWM (0-255 para 0-100% duty cycle)
        pwm_set_gpio_level(LED_RED_PIN, red);
        pwm_set_gpio_level(LED_GREEN_PIN, green);
        pwm_set_gpio_level(LED_BLUE_PIN, blue);

        printf("LED ajustado para: R=%d, G=%d, B=%d\n", red, green, blue);
    } else {
        printf("Mensagem inválida! Use o formato R/G/B.\n");
    }
}


static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Tópico recebido: %s\n", topic);
}

static void mqtt_request_cb(void *arg, err_t err) {
    printf("MQTT request callback: err %d\n", (int)err);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    printf("Status de conexão MQTT: %d\n", (int)status);

    if (status == MQTT_CONNECT_ACCEPTED) {
        err_t erro = mqtt_sub_unsub(client, SUBS_STR_NAME, 0, &mqtt_request_cb, NULL, 1);
        if (erro == ERR_OK) {
            printf("Inscrição no tópico '%s' realizada com sucesso!\n", SUBS_STR_NAME);
        } else {
            printf("Falha ao se inscrever no tópico '%s'!\n", SUBS_STR_NAME);
        }
    } else {
        printf("Conexão rejeitada!\n");
    }
}

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */
float read_onboard_temperature(const char unit) {
    
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
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

// Converts a floating-point/double number to a string. 
void ftoa(float n, char* res, int afterpoint) 
{ 
    // Extract integer part 
    int ipart = (int)n; 
 
    // Extract floating part 
    float fpart = n - (float)ipart; 
 
    // convert integer part to string 
    int i = intToStr(ipart, res, 0); 
 
    // check for display option after point 
    if (afterpoint != 0) { 
        res[i] = '.'; // add dot 
 
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter 
        // is needed to handle cases like 233.007 
        fpart = fpart * pow(10, afterpoint); 
 
        intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
}

// Converts a given integer x to string str[]. 
// d is the number of digits required in the output. 
// If d is more than the number of digits in x, 
// then 0s are added at the beginning. 
int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    } 
 
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d) 
        str[i++] = '0'; 
 
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
}

// Reverses a string 'str' of length 'len' 
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

void configure_pwm() {
    // Habilita PWM nos pinos do LED RGB
    gpio_set_function(LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE_PIN, GPIO_FUNC_PWM);

    // Obtém os slices PWM para cada pino
    uint slice_red = pwm_gpio_to_slice_num(LED_RED_PIN);
    uint slice_green = pwm_gpio_to_slice_num(LED_GREEN_PIN);
    uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE_PIN);

    // Configura o PWM para frequência de 1 kHz
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 2.0f); // Ajusta o divisor de clock

    // Inicializa PWM nos slices correspondentes
    pwm_init(slice_red, &config, true);
    pwm_init(slice_green, &config, true);
    pwm_init(slice_blue, &config, true);

    pwm_set_wrap(slice_red, 255);   // Ou aumente para 1023
    pwm_set_wrap(slice_green, 255);
    pwm_set_wrap(slice_blue, 255);


    // Inicializa todos os pinos com nível 0 (LED apagado)
    pwm_set_gpio_level(LED_RED_PIN, 0);
    pwm_set_gpio_level(LED_GREEN_PIN, 0);
    pwm_set_gpio_level(LED_BLUE_PIN, 0);
}

#endif /*MQTT_UTILITY_H*/