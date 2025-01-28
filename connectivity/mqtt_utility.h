#ifndef MQTT_UTILITY_H
#define MQTT_UTILITY_H

/******************************************************************************
 * @file    mqtt_utility.h
 * @brief   File containing definitions and function prototypes for the
 *          application of  a MQTT communication connected to a local broker
 *          on the Raspberry Pi Pico W.
 *
 * @authors Gabriel Domingos de Medeiros
 * @date    February 2025
 * @version 1.0.0
 *
 * @note    This file includes the necessary definitions, constants, and function
 *          prototypes for the application.
 ******************************************************************************/

// ---------------------------------- Includes ---------------------------------

#include <string.h>         // Standard library for string operations.
#include "pico/stdlib.h"    // Standard library for Raspberry Pi Pico.
#include "pico/cyw43_arch.h"// Library for using the connectivity module for Raspberry Pi Pico W.
#include "lwip/apps/mqtt.h" // MQTT library for handling MQTT protocol.
#include "hardware/adc.h"   // Library for ADC (Analog-to-Digital Converter) operations.
#include "hardware/pwm.h"   // Library for PWM (Pulse Width Modulation) operations.
#include "math.h"           // Math library for mathematical operations.
#include "bsp/board.h"      // Board support package for hardware abstraction.
#include "pico/binary_info.h"// Library for binary information.


// ----------------------------------- Defines ----------------------------------

#define LED_RED_PIN 13      // Pin number for the red LED.
#define LED_GREEN_PIN 11    // Pin number for the green LED.
#define LED_BLUE_PIN 12     // Pin number for the blue LED.

#define MQTT_SERVER "10.220.0.83" // MQTT server address (local broker).
#define SUBS_STR_NAME "LED/TPC"   // MQTT topic for subscribing to LED control messages.
#define PUBLISH_STR_NAME "TEMP/TPC" // MQTT topic for publishing temperature data.

/* Choose 'C' for Celsius or 'F' for Fahrenheit. */
#define TEMPERATURE_UNITS 'C'     // Unit for temperature measurement.

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


// ---------------------------------- Variables ---------------------------------

char tempString[12];    // String containing the converted value of the internal temperature
int cont_envio;         // Counter that will dictate the speed at which the temperature is sent to the Broker
char last_led[32] = ""; // Global declaration


// ---------------------------------- Functions ---------------------------------

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_request_cb(void *arg, err_t err);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
float read_onboard_temperature(const char unit);
void ftoa(float n, char* res, int afterpoint);
void ftoa(float n, char* res, int afterpoint);
int intToStr(int x, char str[], int d);
void reverse(char* str, int len);


// -------------------------- Callback Incoming Data Function -------------------------

/**
 * @brief Callback function for incoming MQTT data.
 *
 * @param arg User-defined argument (unused).
 * @param data Pointer to the incoming data.
 * @param len Length of the incoming data.
 * @param flags Flags indicating the status of the incoming data.
 *
 * This function is called when data is received on a subscribed MQTT topic.
 * It processes the data and adjusts the LED colors accordingly.
 *
 * ### Behavior:
 * - Converts the incoming data to a string.
 * - Parses the string to extract RGB values.
 * - Adjusts the LED colors based on the parsed values.
 * - Ensures the RGB values are within the valid range (0-255).
 * - Updates the global `last_led` variable with the received message.
 *
 * @note The data should be in the format "R/G/B" where R, G, and B are integers.
 * @note If the data format is invalid, an error message is printed.
 */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Message received: %.*s\n", len, data);

    // Converts the message to a string
    char message[32];
    strncpy(message, (char *)data, len);
    message[len] = '\0';

    // Variables to store R, G and B values
    int red, green, blue;

    // Parse message in R/G/B format
    if (sscanf(message, "%d/%d/%d", &red, &green, &blue) == 3) {
        // Ensures values ​​are between 0 and 255
        red = red < 0 ? 0 : (red > 255 ? 255 : red);
        green = green < 0 ? 0 : (green > 255 ? 255 : green);
        blue = blue < 0 ? 0 : (blue > 255 ? 255 : blue);

        // printf("R = %d, G = %d, B = %d\n", red, green, blue);

        // Converts values ​​to PWM (0-255 to 0-100% duty cycle)
        pwm_set_gpio_level(LED_RED_PIN, red);
        pwm_set_gpio_level(LED_GREEN_PIN, green);
        pwm_set_gpio_level(LED_BLUE_PIN, blue);

        printf("LED adjusted for: R=%d, G=%d, B=%d\n", red, green, blue);

        strncpy(last_led, message, sizeof(last_led) - 1);
        last_led[sizeof(last_led) - 1] = '\0'; // Ensures string termination

        printf("Message copied to last_led: %s\n", message);

        printf("Value of last_led: %s\n", last_led);


    } else {
        printf("Invalid message! Use the format R/G/B.\n");
    }
}


// --------------------------- Callback Incoming Publish Function ---------------------------

/**
 * @brief Callback function for incoming MQTT publish messages.
 *
 * @param arg User-defined argument (unused).
 * @param topic The topic of the incoming message.
 * @param tot_len Total length of the incoming message.
 *
 * This function is called when a message is published to a subscribed MQTT topic.
 * It prints the received topic.
 *
 * ### Behavior:
 * - Prints the topic of the received message.
 *
 * @note This function does not process the message payload.
 */
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Topic received: %s\n", topic);
}

// --------------------------- Callback Request Function ---------------------------

/**
 * @brief Callback function for MQTT requests.
 *
 * @param arg User-defined argument (unused).
 * @param err Error status of the request.
 *
 * This function is called when an MQTT request completes.
 * It prints the status of the request.
 *
 * ### Behavior:
 * - Prints the error status of the completed request.
 *
 * @note This function is used for handling the completion of MQTT requests such as subscriptions.
 */
static void mqtt_request_cb(void *arg, err_t err) {
    printf("MQTT request callback: err %d\n", (int)err);
}


// --------------------------- Callback Connection Function ---------------------------

/**
 * @brief Callback function for MQTT connection status.
 *
 * @param client Pointer to the MQTT client structure.
 * @param arg User-defined argument (unused).
 * @param status Connection status.
 *
 * This function is called when the MQTT connection status changes.
 * It handles subscription to the topic upon successful connection.
 *
 * ### Behavior:
 * - Prints the connection status.
 * - Subscribes to the specified topic if the connection is accepted.
 * - Prints success or failure message for the subscription.
 *
 * @note This function is invoked automatically when the connection status changes.
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    printf("MQTT connection status: %d\n", (int)status);

    if (status == MQTT_CONNECT_ACCEPTED) {
        err_t erro = mqtt_sub_unsub(client, SUBS_STR_NAME, 0, &mqtt_request_cb, NULL, 1);
        if (erro == ERR_OK) {
            printf("Subscription to topic '%s' successful!\n", SUBS_STR_NAME);
        } else {
            printf("Failed to subscribe to topic '%s'!\n", SUBS_STR_NAME);
        }
    } else {
        printf("Connection rejected!\n");
    }
}


// --------------------------- Read Onboard Temperature Function ---------------------------

/**
 * @brief Reads the onboard temperature sensor.
 *
 * @param unit The unit of temperature ('C' for Celsius, 'F' for Fahrenheit).
 * @return float The temperature in the specified unit.
 *
 * This function reads the onboard temperature sensor and returns the temperature
 * in the specified unit.
 *
 * ### Behavior:
 * - Enables the onboard temperature sensor.
 * - Selects the ADC input for the temperature sensor.
 * - Reads the ADC value and converts it to temperature.
 * - Returns the temperature in the specified unit.
 *
 * @note The function assumes a 12-bit ADC with a reference voltage of 3.3V.
 */
float read_onboard_temperature(const char unit) {
    
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // Select the internal temperature sensor

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


// --------------------------- Float to String Function ---------------------------

/**
 * @brief Converts a floating-point number to a string.
 *
 * @param n The floating-point number to convert.
 * @param res The resulting string.
 * @param afterpoint Number of digits after the decimal point.
 *
 * This function converts a floating-point number to a string with the specified
 * number of digits after the decimal point.
 *
 * ### Behavior:
 * - Extracts the integer part of the number.
 * - Extracts the fractional part of the number.
 * - Converts the integer part to a string.
 * - Adds a decimal point if required.
 * - Converts the fractional part to a string.
 *
 * @note The resulting string is stored in the `res` parameter.
 */
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


// --------------------------- Integer to String Function ---------------------------

/**
 * @brief Converts an integer to a string.
 *
 * @param x The integer to convert.
 * @param str The resulting string.
 * @param d Number of digits required in the output.
 * @return int The length of the resulting string.
 *
 * This function converts an integer to a string with the specified number of digits.
 * If the number of digits is more than the number of digits in the integer, zeros
 * are added at the beginning.
 *
 * ### Behavior:
 * - Converts the integer to a string.
 * - Adds leading zeros if required.
 * - Reverses the string to get the correct order.
 *
 * @note The resulting string is stored in the `str` parameter.
 */
int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    if (x == 0) {
    str[i++] = '0'; // Add the digit 0
    }
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


// --------------------------- Reverse String Function ---------------------------

/**
 * @brief Reverses a string.
 *
 * @param str The string to reverse.
 * @param len The length of the string.
 *
 * This function reverses the given string.
 *
 * ### Behavior:
 * - Swaps characters from the beginning and end of the string.
 * - Continues swapping until the middle of the string is reached.
 *
 * @note The reversed string is stored in the `str` parameter.
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


// --------------------------- Configure PWM Function ---------------------------

/**
 * @brief Configures the PWM for the RGB LED.
 *
 * This function sets up the PWM for the RGB LED pins and initializes them to off.
 *
 * ### Behavior:
 * - Sets the GPIO function for the LED pins to PWM.
 * - Configures the PWM slices for each LED pin.
 * - Sets the PWM frequency to 1 kHz.
 * - Initializes the PWM slices.
 * - Sets the PWM wrap value to 255.
 * - Initializes the LED pins to off.
 *
 * @note This function should be called during the setup phase of the application.
 */
void configure_pwm() {
    // Enables PWM on RGB LED pins
    gpio_set_function(LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE_PIN, GPIO_FUNC_PWM);

    // Get the PWM slices for each pin
    uint slice_red = pwm_gpio_to_slice_num(LED_RED_PIN);
    uint slice_green = pwm_gpio_to_slice_num(LED_GREEN_PIN);
    uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE_PIN);

    // Sets PWM to 1 kHz frequency
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 2.0f); // Adjust the clock divider

    // Initialize PWM on corresponding slices
    pwm_init(slice_red, &config, true);
    pwm_init(slice_green, &config, true);
    pwm_init(slice_blue, &config, true);

    pwm_set_wrap(slice_red, 255);
    pwm_set_wrap(slice_green, 255);
    pwm_set_wrap(slice_blue, 255);


    // Initialize all pins to 0 (LED off)
    pwm_set_gpio_level(LED_RED_PIN, 0);
    pwm_set_gpio_level(LED_GREEN_PIN, 0);
    pwm_set_gpio_level(LED_BLUE_PIN, 0);
}

#endif /*MQTT_UTILITY_H*/
