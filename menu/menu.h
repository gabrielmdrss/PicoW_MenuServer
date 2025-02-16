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
#include "http.h"                               // Arquivo contendo funções para o protocolo HTTP.
#include "defines_functions.h"                  // Arquivo contendo definições e funções para o projeto.
#include "lwip/tcpip.h"                         // Certifique-se de incluir a biblioteca LWIP

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

    // Se a tela atual for a tela inicial
    if (current_screen == 0) {

        update_cursor();    // Atualiza o cursor com o joystick
        home_screen();      // Atualiza a Tela Inicial no Display OLED
    }

    // Se a tela atual for a tela específica
    if(current_screen) {

        // Limpa o Display
        ssd1306_Fill(Black);

        // Se o item selecionado for "Cloud"
        if (item_selected == 0){

            // Exibe o cabeçalho
            cabecalho("CLOUD:", 45, 1);

            char buffer_string[7];      // Buffer para armazenar valores formatados em string
            
            // Se o sistema estiver inicializado (Passado pela opção System Setup)
            if(inicialized){

                // Se a interrupção do timer já foi ativa (a cada 2 segundos)
                if(timer_expired){

                    // Lê a temperatura interna do sistema
                    temperature = read_onboard_temperature(TEMPERATURE_UNITS);

                    // Passará a temperatura para a função que construirá a requisição HTTP
                    build_http_request(temperature);
                    timer_expired = false;      // Reseta a flag de interrupção do timer
                }

                // Exibe a temperatura no display
                ssd1306_SetCursor(3, 24);
                sprintf(buffer_string, "- Temp: %.2f %c", temperature, TEMPERATURE_UNITS);
                ssd1306_WriteString(buffer_string, Font_6x8, White);
                sprintf(buffer_string, "- Latitude: %.4f", lat);
                ssd1306_DrawRectangle(1, 34, 127, 34, White);   // Separador Horizontal

                // Exibe a latitude no display
                ssd1306_SetCursor(3, 38);
                ssd1306_WriteString(buffer_string, Font_6x8, White);
                ssd1306_DrawRectangle(1, 48, 127, 48, White);   // Separador Horizontal

                // Exibe a longitude no display
                sprintf(buffer_string, "- Longitude: %.4f", lon);
                ssd1306_SetCursor(3, 52);
                ssd1306_WriteString(buffer_string, Font_6x8, White);

                cyw43_arch_poll();      // Polling do módulo CYW43 (Manter conexão Wi-Fi ativa)

              // Se o sistema não estiver inicializado  
            } else {

                // Função que printa na tela um aviso de não inicialização
                not_initialized();

            }
        } 

        // Se o item selecionado for "System Setup"
        else if (item_selected == 1){

            // Exibe o cabeçalho
            cabecalho("SYSTEM SETUP:", 20, 1);

            // Se for a primeira vez que o usuário acessa a opção
            if(inicialized == 0){

                char buffer_float[7];	// Buffer para armazenar valores formatados em string
            
                // Percentual de inicialização - Laço bloqueante para inicializar o sistema
                while(percentual < 100){

                    // Garantir a impressão do cabeçalho dentro do laço
                    cabecalho("SYSTEM SETUP:", 20, 1);
                    
                    ssd1306_SetCursor(11, 28);  
                    snprintf(buffer_float, sizeof(buffer_float), "%d\n", percentual);   // Armazenar valor do percentual em string
                    ssd1306_WriteString(buffer_float, Font_6x8, 1);                     // Escrever o valor no display
                    ssd1306_WriteString("%", Font_7x10, 1);


                    // Se o percentual for 20 = Inicialização do sensor de temperatura
                    if(percentual == 20){
                        ssd1306_Fill(Black);                                                            // Limpa o display
                        cabecalho("SYSTEM SETUP:", 20, 1);                                              // Exibe o cabeçalho
                        ssd1306_SetCursor(33, 28);
                        ssd1306_WriteString("- Temp INIT", Font_6x8, White);                            // Exibição do processo em andamento
                        ssd1306_DrawRectangle(11, 40, 117, 55, 1);                                      // Desenha a barra de progresso
                        ssd1306_FillRectangle(11, 40, (11 + (percentual * (117 - 11)) / 100), 55, 1);   // Preenche a barra de progresso com o percentual
                        ssd1306_SetCursor(11, 28);
                        snprintf(buffer_float, sizeof(buffer_float), "%d\n", percentual);               // Armazena o percentual em string
                        ssd1306_WriteString(buffer_float, Font_6x8, 1);                                 // Exibe o percentual no display
                        ssd1306_WriteString("%", Font_7x10, 1);
                        adc_set_temp_sensor_enabled(true);                                              // Habilita o sensor de temperatura interno
                        adc_select_input(4);                                                            // Seleciona o sensor de temperatura interno como entrada ADC
                        adc_gpio_init(26);                                                              // Inicializa o pino GPIO 26 para ADC
                        adc_gpio_init(27);                                                              // Inicializa o pino GPIO 27 para ADC


                    // Se o percentual for 50 = Inicialização do gerador de números aleatórios
                    } else if (percentual == 50) {
                        ssd1306_Fill(Black);                                                            // Limpa o display
                        cabecalho("SYSTEM SETUP:", 20, 1);                                              // Exibe o cabeçalho
                        ssd1306_SetCursor(33, 28);
                        ssd1306_WriteString("- Random INIT", Font_6x8, White);                          // Exibição do processo em andamento
                        ssd1306_DrawRectangle(11, 40, 117, 55, 1);                                      // Desenha a barra de progresso
                        ssd1306_FillRectangle(11, 40, (11 + (percentual * (117 - 11)) / 100), 55, 1);   // Preenche a barra de progresso com o percentual
                        ssd1306_SetCursor(11, 28);
                        snprintf(buffer_float, sizeof(buffer_float), "%d\n", percentual);               // Armazena o percentual em string
                        ssd1306_WriteString(buffer_float, Font_6x8, 1);                                 // Exibe o percentual no display
                        ssd1306_WriteString("%", Font_7x10, 1);                                         // Exibe o símbolo de porcentagem
                        srand(time(NULL));                                                              // Inicializa o gerador de números aleatórios

                    // Se o percentual for 75 = Inicialização do buzzer
                    } else if (percentual == 75) {
                        ssd1306_Fill(Black);                                                            // Limpa o display
                        cabecalho("SYSTEM SETUP:", 20, 1);                                              // Exibe o cabeçalho
                        ssd1306_SetCursor(33, 28);  
                        ssd1306_WriteString("- Buzzer INIT", Font_6x8, White);                          // Exibição do processo em andamento
                        ssd1306_DrawRectangle(11, 40, 117, 55, 1);                                      // Desenha a barra de progresso
                        ssd1306_FillRectangle(11, 40, (11 + (percentual * (117 - 11)) / 100), 55, 1);   // Preenche a barra de progresso com o percentual
                        ssd1306_SetCursor(11, 28);
                        snprintf(buffer_float, sizeof(buffer_float), "%d\n", percentual);               // Armazena o percentual em string
                        ssd1306_WriteString(buffer_float, Font_6x8, 1);                                 // Exibe o percentual no display
                        ssd1306_WriteString("%", Font_7x10, 1);
                        pwm_init_buzzer(BUZZER_PIN);                                                    // Inicializa o PWM para o buzzer

                    // Se o percentual for 90 = Inicialização do Wi-Fi
                    } else if (percentual == 90) {
                        ssd1306_Fill(Black);                                                            // Limpa o display
                        cabecalho("SYSTEM SETUP:", 20, 1);                                              // Exibe o cabeçalho
                        ssd1306_SetCursor(33, 28);
                        ssd1306_WriteString("- WiFi INIT...", Font_6x8, White);                         // Exibição do processo em andamento
                        ssd1306_DrawRectangle(11, 40, 117, 55, 1);                                      // Desenha a barra de progresso
                        ssd1306_FillRectangle(11, 40, (11 + (percentual * (117 - 11)) / 100), 55, 1);   // Preenche a barra de progresso com o percentual
                        ssd1306_SetCursor(11, 28);
                        snprintf(buffer_float, sizeof(buffer_float), "%d\n", percentual);               // Armazena o percentual em string
                        ssd1306_WriteString(buffer_float, Font_6x8, 1);                                 // Exibe o percentual no display
                        ssd1306_WriteString("%", Font_7x10, 1);
                        ssd1306_UpdateScreen();                                                         // Atualiza o display devido ação bloqueante do Wi-Fi

                        // Inicializa o Wi-Fi
                        if (cyw43_arch_init()) {
                            printf("Wi-Fi init failed\n");
                            return;
                        }

                        printf("Habilitando modo STA...\n");

                        // Habilita o modo STA
                        cyw43_arch_enable_sta_mode();   

                        // Tenta conectar ao Wi-Fi
                        printf("Conectando ao Wi-Fi...\n");

                        // Conecta ao Wi-Fi
                        if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
                            printf("Erro: Falha ao conectar ao Wi-Fi.\n");
                            break;  // Encerra o laço
                        }

                        printf("Conectado a %s\n", ssid);
                        
                        start_wifi = 1;     // Marca a flag de conexão Wi-Fi como ativa
                        start_timer();      // Inicializa o timer para a requisição HTTP
                    }
                    
                    // Atualização do percentual quando nenhum dos blocos IF é ativo
                    ssd1306_DrawRectangle(11, 40, 117, 55, 1);
			        ssd1306_FillRectangle(11, 40, (11 + (percentual * (117 - 11)) / 100), 55, 1);
                    sleep_ms(100);
                    percentual++;
                    ssd1306_UpdateScreen();
                }
                
                // Se o percentual não for 100 = Algo interrompeu a finalização do laço, exibir mensagem de erro, travar funcionamento.
                if(percentual != 100){
                    ssd1306_Fill(Black);                  
                    cabecalho("SYSTEM SETUP:", 20, 1);          
                    ssd1306_SetCursor(20, 24);
                    ssd1306_WriteString("Falha ao iniciar.", Font_6x8, White);
                    ssd1306_SetCursor(5, 34);
                    ssd1306_WriteString("Reinicie dispositivo", Font_6x8, White);
                    ssd1306_SetCursor(32, 44);
                    ssd1306_WriteString("e use novas", Font_6x8, White);
                    ssd1306_SetCursor(5, 54);
                    ssd1306_WriteString("credenciais de rede.", Font_6x8, White);
                    ssd1306_UpdateScreen();
                    while(1);

                // Se não houver interrupção, a inicialização foi bem sucedida
                } else {
                    inicialized = 1;    // Marca o sistema como inicializado
                }

            // Após a inicialização, se a opção for acessada, deverá constar como já inicializada
            } else {

                ssd1306_SetCursor(7, 33);
                ssd1306_WriteString("Ja esta inicializado", Font_6x8, White);

            }

        }
        
        // Se o item selecionado for "Buzzer PWM"
        else if (item_selected == 2){

            // Exibe o cabeçalho
            cabecalho("BUZZER PWM:", 25, 1);
            char buffer_string[7];	                                // Buffer para armazenar valores formatados em string
            

            // Se o sistema estiver inicializado (Passado pela opção System Setup)
            if(inicialized){
                
                adc_select_input(1);                                // Seleciona o pino GPIO 27 (Eixo X do Joystick) como entrada ADC
                uint adc_x_raw = adc_read();                        // Lê o valor bruto do ADC do eixo X do joystick
                uint filtered_read = low_pass_filter(adc_x_raw);    // Aplica um filtro passa-baixa ao valor bruto do ADC

                // Ajuste de frequência para indicar um passo no progresso da barra
                if (adc_x_raw > ADC_UPPER_THRESHOLD && frequency < MAX_FREQUENCY) {
                    frequency += STEP;
                } else if (adc_x_raw < ADC_LOWER_THRESHOLD && frequency > MIN_FREQUENCY) {
                    frequency -= STEP;
                }

                // Configura o buzzer com a nova frequência
                set_buzzer_frequency(BUZZER_PIN, frequency);

                // Atualiza a barra no display
                uint8_t x_distance = (uint8_t)(((frequency - MIN_FREQUENCY) * 128) / (MAX_FREQUENCY - MIN_FREQUENCY)) + 1;
                ssd1306_DrawRectangle(1, 48, 128, 63, 1);               // Desenha a barra de progresso
                ssd1306_FillRectangle(1, 48, x_distance, 63, 1);        // Preenche o progresso na barra

                // Mostra a frequência atual
                ssd1306_SetCursor(25, 30);
                sprintf(buffer_string, "FREQ: %u Hz", (uint)frequency);  // Formata a frequência em string
                ssd1306_WriteString(buffer_string, Font_7x10, 1);        // Exibe a frequência no display
            
            // Se o sistema não estiver inicializado
            } else {
                not_initialized();  // Função que printa na tela um aviso de não inicialização
            }
        }
        
        // Se o item selecionado for "Network Info"
        else if (item_selected == 3){

            // Exibe o cabeçalho
            cabecalho("NETWORK INFO:", 22, 1);

            // Se o sistema estiver inicializado (Passado pela opção System Setup)
            if(inicialized){

                ssd1306_DrawRectangle(32, 20, 32, 63, 1);	        // Separador Vertical

                char buffer_string[7];                              // Buffer para armazenar valores formatados em string
                uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
                sprintf(buffer_string, "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
                ssd1306_SetCursor(3, 23);
                ssd1306_WriteString("IP", Font_6x8, 1);             // Exibe o texto "IP"
                ssd1306_SetCursor(53, 23);
                ssd1306_WriteString(buffer_string, Font_6x8, 1);    // Exibe o endereço IP

                ssd1306_DrawRectangle(1, 34, 127, 34, 1);	        // Separador horizontal
                int32_t rssi;                                       // Variável para armazenar o RSSI
                cyw43_wifi_get_rssi(&cyw43_state, &rssi);           // Lê o RSSI do módulo CYW43
                sprintf(buffer_string, "%d dBm", rssi);             // Formata o RSSI em string
                ssd1306_SetCursor(3, 37);
                ssd1306_WriteString("RSSI", Font_6x8, 1);           // Exibe o texto "RSSI"
                ssd1306_SetCursor(81, 37);
                ssd1306_WriteString(buffer_string, Font_6x8, 1);    // Exibe o valor do RSSI
                ssd1306_DrawRectangle(1, 46, 127, 46, 1);	        // Separador horizontal

                ssd1306_SetCursor(3, 50);
                ssd1306_WriteString("WIFI", Font_6x8, 1);           // Exibe o texto "WIFI"

                // Se o Wi-Fi estiver conectado
                if(start_wifi){

                    ssd1306_SetCursor(70, 50);
                    ssd1306_WriteString("CONNECTED", Font_6x8, 1);  // Exibe o texto "CONNECTED"
                } else {
                    ssd1306_SetCursor(52, 50);
                    ssd1306_WriteString("DISCONNECTED", Font_6x8, 1);  // Exibe o texto "DISCONNECTED"
                }
            
            // Se o sistema não estiver inicializado
            } else {
                not_initialized();  // Função que printa na tela um aviso de não inicialização
            }
    }
    }

    // Função que analisa o estado do botão ENTER
    if (!(gpio_get(BUTTON_B)) && button_enter_clicked == 0) {

        button_enter_clicked = 1;           // Marca o botão ENTER como pressionado

        // Desliga o buzzer
        pwm_set_gpio_level(BUZZER_PIN, 0);  // Desativa o buzzer

        // Se o item selecionado for diferente de "Buzzer PWM"
        if(item_selected != 2){
        
        // Se a tela atual for a tela inicial
        if(current_screen)
            menu_enter_sound(BUZZER_PIN);   // Toca o som de entrada
        // Se a tela atual for a tela específica
        else
            menu_exit_sound(BUZZER_PIN);    // Toca o som de saída
        }

        // Alterna para o outro tipo de tela
        current_screen = !current_screen;
        
    }

    // Se o botão ENTER for liberado, a variável auxiliar retorna para baixo, dando chance de clicar novamente
    if ((gpio_get(BUTTON_B)) && button_enter_clicked == 1) {
        button_enter_clicked = 0;
    }

/*------------------------- Lógica para imprimir os itens corretos ----------------------------*/

    item_sel_previous = item_selected - 1;  // O item anterior é o item selecionado menos 1

    // Se o item anterior for menor que 0 = O item anterior estaria abaixo do primeiro = torná-lo o último
    if (item_sel_previous < 0) {
        item_sel_previous = NUM_ITEMS - 1;
    } 
    item_sel_next = item_selected + 1;      // O próximo item é o item selecionado mais 1

    // Se o próximo item for maior ou igual ao número total de itens = O próximo item estaria após o último = torná-lo o primeiro
    if (item_sel_next >= NUM_ITEMS) {
        item_sel_next = 0;
    }

/*---------------------------------------------------------------------------------------*/

    ssd1306_UpdateScreen(); // Atualiza o display
}


// ---------------------- Função de Renderização do Menu no AP Mode -----------------------

/**
 * @brief Renderiza o menu no modo AP (Access Point) em um display SSD1306.
 *
 * Esta função é responsável por desenhar a interface do modo AP em um display SSD1306
 * usando o protocolo I2C. Ela exibe o SSID, a senha e o endereço IP do ponto de acesso.
 *
 * ### Comportamento:
 * - Exibe o cabeçalho "AP-MODE:".
 * - Exibe o SSID da rede Wi-Fi.
 * - Exibe a senha da rede Wi-Fi.
 * - Exibe o endereço IP do ponto de acesso.
 * - Atualiza o display para refletir as mudanças.
 *
 * @note A função depende das funções do driver SSD1306:
 *   - `ssd1306_SetCursor`: Posiciona o cursor em uma localização (x, y) especificada no display.
 *   - `ssd1306_WriteString`: Escreve uma string em uma posição especificada (x, y).
 *   - `ssd1306_UpdateScreen`: Atualiza o display para refletir as mudanças.
 *
 * @note Assume que as variáveis `ap_name` e `ap_pw` estão definidas e contêm o SSID e a senha da rede Wi-Fi, respectivamente.
 */
void menu_ap (void){

    // Exibe o cabeçalho
    cabecalho("AP-MODE:", 40, 1);    
    char buffer_sg[7];                                      // Buffer para armazenar valores formatados em string   

    ssd1306_SetCursor(4, 22);   
    ssd1306_WriteString("ssid: ", Font_6x8, White);         // Exibe o texto "ssid"
    ssd1306_WriteString(ap_name, Font_6x8, White);          // Exibe o nome da rede Wi-Fi

    ssd1306_SetCursor(4, 33);
    ssd1306_WriteString("pw: ", Font_6x8, White);           // Exibe o texto "pw"
    ssd1306_WriteString(ap_pw, Font_6x8, White);            // Exibe a senha da rede Wi-Fi

    ssd1306_SetCursor(4, 44);
    ssd1306_WriteString("192.168.4.1", Font_6x8, White);    // Exibe o endereço IP do AP
    
    ssd1306_UpdateScreen();                                 // Atualiza o display
}

#endif /*MENU_H*/