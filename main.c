/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "ap_mode/ap_mode_utility.h"
#include "menu/menu.h"


int main() {

    stdio_init_all();
    //sleep_ms(10000);

/*------------------------------ Inicializando os pinos ---------------------------------*/
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);

    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // Selecione o sensor de temperatura interno
    adc_gpio_init(26);
    adc_gpio_init(27);
    pwm_init_buzzer(BUZZER_PIN); // Inicializa o PWM para o buzzer
    configure_pwm();
/*---------------------------------------------------------------------------------------*/

    ssd1306_Init();

/*------------------------------ Starting AP_MODE Setup ---------------------------------*/
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return 1;
    }

    //cyw43_arch_deinit();

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return 1;
    }

    cyw43_arch_enable_ap_mode(ap_name, ap_pw, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the dns server
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    if (!tcp_server_open(state)) {
        DEBUG_printf("failed to open server\n");
        return 1;
    }
/*---------------------------------------------------------------------------------------*/

    while (1)
    {

        if (aux_connection == 0){
            menu();

        } else {

            menu_ap();
            // printf("id_pw_collected = %d,  aux_connection = %d\n", id_pw_collected, aux_connection);
            // sleep_ms(1000);

            if (id_pw_collected == 1 && aux_connection == 1) {

                // Finalize os serviços de rede relacionados ao servidor TCP
                dns_server_deinit(&dns_server);
                dhcp_server_deinit(&dhcp_server);

                shutdown_tcp_server(state);

                cyw43_arch_disable_ap_mode(); // Desabilita o modo AP
                cyw43_arch_deinit();          // Libera recursos do Wi-Fi
                sleep_ms(500);

                // Reinitialize o Wi-Fi no modo STA
                if (cyw43_arch_init()) {
                    printf("Wi-Fi init failed\n");
                    return -1;
                }

                printf("Habilitando modo STA...\n");
                cyw43_arch_enable_sta_mode();

                // Tenta conectar ao Wi-Fi
                printf("Conectando ao Wi-Fi...\n");

                if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 20000)) {
                    printf("Erro: Falha ao conectar ao Wi-Fi.\n");
                    ssd1306_SetCursor(21, 54);
                    ssd1306_WriteString("NOT CONNECTED", Font_6x8, White);
                    ssd1306_UpdateScreen();
                    return -1;
                }
                ssd1306_SetCursor(35, 54);
                ssd1306_WriteString("CONNECTED", Font_6x8, White);
                ssd1306_UpdateScreen();
                sleep_ms(2000);
                printf("Conectado a %s\n", ssid);

                // Atualize flags para evitar reexecução da lógica
                aux_connection = 0;

            }
        }
    }
    
    return 0;
}
