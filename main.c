/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "menu.h"
#include "wifi.h"

int main() {

    stdio_init_all();
    //sleep_ms(10000);

/*------------------------------ Inicializando os pinos ---------------------------------*/
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    pwm_init_buzzer(BUZZER_PIN); // Inicializa o PWM para o buzzer
/*---------------------------------------------------------------------------------------*/

    ssd1306_Init();

    // Starts HTTP Server
    if (cyw43_arch_init()) {
        printf("Error initializing Wi-Fi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");

    while (1)
    {
        menu();
    }
    
    return 0;
}
