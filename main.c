#include <stdio.h>
#include "ap_mode/ap_mode_utility.h"
#include "menu/menu.h"
#include <time.h>

/*
    Alguns pontos importantes a serem destacados:
    1 - Nessa etapa do projeto, apenas dispositivos Android conseguem enviar a senha para o Raspberry Pi Pico W, em iOS e notebooks não foi possível devido a diferentes requisições HTTP geradas
    2 - Link do ThingSpeak: https://thingspeak.mathworks.com/channels/2838403
    3 - Link do vídeo de demonstração: https://youtube.com/shorts/M_ZbwZbM4-g?si=VA5z2ySlKUF2r0dz
    4 - A aplicação iniciará com o AP Mode habilitado e apenas entrará no Menu principal quando alguém clicar no botão de enviar e for retornado Sucesso no envio das credenciais
    5 - A navegação dentro do menu é dada pelo joystick e botão B (ENTER).
    4 - Se o ssid ou senha do wifi for escrito incorretamente, só será visível quando já no menu, o usuário clicar em <System Setup> e imprimir o erro e necessidade de reiniciar a placa para enviar novamente
*/


int main() {

    stdio_init_all();                   // Inicializa todas as funções de entrada e saída padrão

/*-------------------------- Inicializando os pinos do menu -----------------------------*/

    gpio_init(BUTTON_B);                // Inicializa o pino do botão B
    gpio_set_dir(BUTTON_B, GPIO_IN);    // Define o pino do botão B como entrada
    gpio_pull_up(BUTTON_B);             // Habilita o pull-up interno no pino do botão B

    adc_init();                         // Inicializa o ADC

/*---------------------------------------------------------------------------------------*/


    ssd1306_Init();                     // Inicializa o display SSD1306
    

/*------------------------- Inicializando Setup para AP_MODE ----------------------------*/

    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T)); // Aloca memória para o estado do servidor TCP
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return 1;
    }

    // Inicializa o Wi-Fi
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return 1;
    }

    // Habilita o modo AP (Access Point)
    cyw43_arch_enable_ap_mode(ap_name, ap_pw, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1); // Configura o endereço IP do gateway
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);    // Configura a máscara de sub-rede

    // Inicia o servidor DHCP
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Inicia o servidor DNS
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    // Abre o servidor TCP
    if (!tcp_server_open(state)) {
        DEBUG_printf("failed to open server\n");
        return 1;
    }

/*---------------------------------------------------------------------------------------*/

    //aux_connection = 0;

    while (1)
    {
        if (aux_connection == 0){
            menu();     // Renderiza o menu principal
        }

        else {
            menu_ap();  // Renderiza o menu do modo AP

            if (id_pw_collected == 1 && aux_connection == 1) {

                // Finaliza os serviços de rede relacionados ao servidor TCP
                dns_server_deinit(&dns_server);
                dhcp_server_deinit(&dhcp_server);

                shutdown_tcp_server(state);     // Encerra o servidor TCP

                cyw43_arch_disable_ap_mode();   // Desabilita o modo AP
                cyw43_arch_deinit();            // Libera recursos do Wi-Fi
                sleep_ms(500);

                ssd1306_SetCursor(40, 54);
                ssd1306_WriteString("RECEIVED", Font_6x8, White);
                ssd1306_UpdateScreen();
                sleep_ms(2000);

                aux_connection = 0;
            }
        }
    }
    
    return 0;
}