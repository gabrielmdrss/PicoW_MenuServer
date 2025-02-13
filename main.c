#include <stdio.h>
#include "ap_mode/ap_mode_utility.h"
#include "menu/menu.h"
#include <time.h>

int main() {

    stdio_init_all();                   // Inicializa todas as funções de entrada e saída padrão

    srand(time(NULL));

/*------------------------------ Inicializando os pinos ---------------------------------*/

    gpio_init(BUTTON_A);                // Inicializa o pino do botão A
    gpio_init(BUTTON_B);                // Inicializa o pino do botão B
    gpio_set_dir(BUTTON_A, GPIO_IN);    // Define o pino do botão A como entrada
    gpio_set_dir(BUTTON_B, GPIO_IN);    // Define o pino do botão B como entrada
    gpio_pull_up(BUTTON_A);             // Habilita o pull-up interno no pino do botão A
    gpio_pull_up(BUTTON_B);             // Habilita o pull-up interno no pino do botão B

    adc_init();                         // Inicializa o ADC
    adc_set_temp_sensor_enabled(true);  // Habilita o sensor de temperatura interno
    adc_select_input(4);                // Seleciona o sensor de temperatura interno como entrada ADC
    adc_gpio_init(26);                  // Inicializa o pino GPIO 26 para ADC
    adc_gpio_init(27);                  // Inicializa o pino GPIO 27 para ADC
    pwm_init_buzzer(BUZZER_PIN);        // Inicializa o PWM para o buzzer

/*---------------------------------------------------------------------------------------*/


    ssd1306_Init();                     // Inicializa o display SSD1306
    

/*------------------------- Inicializando Setup para AP_MODE ----------------------------*/

    // TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T)); // Aloca memória para o estado do servidor TCP
    // if (!state) {
    //     DEBUG_printf("failed to allocate state\n");
    //     return 1;
    // }

    // // Inicializa o Wi-Fi
    // if (cyw43_arch_init()) {
    //     printf("Wi-Fi init failed");
    //     return 1;
    // }

    // // Habilita o modo AP (Access Point)
    // cyw43_arch_enable_ap_mode(ap_name, ap_pw, CYW43_AUTH_WPA2_AES_PSK);

    // ip4_addr_t mask;
    // IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1); // Configura o endereço IP do gateway
    // IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);    // Configura a máscara de sub-rede

    // // Inicia o servidor DHCP
    // dhcp_server_t dhcp_server;
    // dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // // Inicia o servidor DNS
    // dns_server_t dns_server;
    // dns_server_init(&dns_server, &state->gw);

    // // Abre o servidor TCP
    // if (!tcp_server_open(state)) {
    //     DEBUG_printf("failed to open server\n");
    //     return 1;
    // }

/*---------------------------------------------------------------------------------------*/

// Reinitialize o Wi-Fi no modo STA
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    printf("Habilitando modo STA...\n");
    cyw43_arch_enable_sta_mode();
    sleep_ms(500);

    // Tenta conectar ao Wi-Fi
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms("PROXXIMA273348-2.4 G", "31230618", CYW43_AUTH_WPA2_AES_PSK, 10000)) {
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

    aux_connection = 0;

    while (1)
    {
        if (aux_connection == 0){
            menu();     // Renderiza o menu principal
        }

        // else {
        //     menu_ap();  // Renderiza o menu do modo AP

        //     if (id_pw_collected == 1 && aux_connection == 1) {

        //         // Finaliza os serviços de rede relacionados ao servidor TCP
        //         dns_server_deinit(&dns_server);
        //         dhcp_server_deinit(&dhcp_server);

        //         shutdown_tcp_server(state);     // Encerra o servidor TCP

        //         cyw43_arch_disable_ap_mode();   // Desabilita o modo AP
        //         cyw43_arch_deinit();            // Libera recursos do Wi-Fi
        //         sleep_ms(500);

        //         // Reinitialize o Wi-Fi no modo STA (Station)
        //         if (cyw43_arch_init()) {
        //             printf("Wi-Fi init failed\n");
        //             return -1;
        //         }

        //         printf("Habilitando modo STA...\n");
        //         cyw43_arch_enable_sta_mode();   // Habilita o modo STA

        //         // Tenta conectar ao Wi-Fi
        //         printf("Conectando ao Wi-Fi...\n");

        //         if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 20000)) {
        //             printf("Erro: Falha ao conectar ao Wi-Fi.\n");
        //             ssd1306_SetCursor(21, 54);
        //             ssd1306_WriteString("NOT CONNECTED", Font_6x8, White);
        //             ssd1306_UpdateScreen();
        //             return -1;
        //         }
        //         ssd1306_SetCursor(35, 54);
        //         ssd1306_WriteString("CONNECTED", Font_6x8, White);
        //         ssd1306_UpdateScreen();
        //         sleep_ms(2000);
        //         printf("Conectado a %s\n", ssid);

        //         // Atualize flags para evitar reexecução da lógica
        //         aux_connection = 0;
        //     }
        // }
    }
    
    return 0;
}