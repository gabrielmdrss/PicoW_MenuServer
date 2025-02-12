#ifndef WIFI_H
#define WIFI_H

/******************************************************************************
 * @file    wifi.h
 * @brief   Arquivo contendo definições e protótipos de funções para a
 *          aplicação de Servidor Web hospedada no Raspberry Pi Pico W.
 *
 * @authors Gabriel Domingos de Medeiros
 * @date    Fevereiro 2025
 * @version 1.0.0
 *
 * @note    Este arquivo inclui as definições, constantes e protótipos de funções
 *          necessários para a aplicação.
 ******************************************************************************/

// ---------------------------------- Includes ---------------------------------

#include "pico/cyw43_arch.h"      // Biblioteca para usar o módulo de conectividade para raspberry pi pico w.
#include "lwip/tcp.h"             // Biblioteca de Funções TCP
#include "lwip/apps/httpd.h"      // Biblioteca de funções para o protocolo HTTP
#include "mqtt_utility.h"         // Arquivo contendo funções úteis para comunicação MQTT


// ----------------------------------- Defines ----------------------------------

#define BUTTON_A_PIN 5            // Número do pino usado no Botão A.

/**
 * @brief Modelo de resposta HTTP para o servidor.
 *
 * Este define contém uma string de resposta HTTP pré-formatada, incluindo cabeçalhos e conteúdo HTML.
 * Ela é dinamicamente populada com o estado atual do botão (`%s`) antes de ser enviada ao cliente.
 *
 * @details:
 * - Estilização HTML/CSS para uma interface web simples.
 * - Inclui botões para funcionalidade de LIGAR e DESLIGAR LED.
 * - Exibe o estado atual do botão dinamicamente na resposta.
 * - Recurso de atualização automática para atualizações em tempo real.
 *
 * @example:
 * Quando o estado do botão é "Button is pressioned", a resposta é atualizada dinamicamente
 * para refletir o status na tag `<p>` do conteúdo HTML.
 */
#define HTTP_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" \
                      "<!DOCTYPE html><html>" \
                      "<head>" \
                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">" \
                      "<link rel=\"icon\" href=\"data:,\">" \
                      "<style>" \
                      "html { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f5; }" \
                      "body { margin: 0; padding: 0; }" \
                      "h1 { color: #333; margin-top: 20px; }" \
                      "p { font-size: 18px; color: #555; margin: 20px auto; }" \
                      ".container { width: 90%; max-width: 600px; margin: 20px auto; padding: 20px; border: 1px solid #ddd; border-radius: 10px; background: #fff; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1); }" \
                      ".button { border: none; color: white; padding: 15px 32px; font-size: 16px; margin: 10px; border-radius: 5px; cursor: pointer; transition: background-color 0.3s ease; display: inline-block; }" \
                      ".red { background-color: #D11D53; } .red:hover { background-color: #c21845; }" \
                      ".green { background-color: #4CAF50; } .green:hover { background-color: #45a049; }" \
                      ".blue { background-color: #007BFF; } .blue:hover { background-color: #006FE6; }" \
                      ".purple { background-color:rgb(122, 3, 201); } .purple:hover { background-color:rgb(122, 3, 201); }" \
                      ".off { background-color: #555; } .off:hover { background-color: #333; }" \
                      ".brightness { background-color: #FFA500; } .brightness:hover { background-color: #e59400; }" \
                      "input[type='range'] { width: 80%; margin: 20px 0; }" \
                      "</style>" \
                      "</head>" \
                      "<body>" \
                      "<div class=\"container\">" \
                      "<h1>Controlar LED</h1>" \
                      "<form>" \
                      "<button class=\"button red\" name=\"led\" value=\"red\" type=\"submit\">Vermelho</button>" \
                      "<button class=\"button green\" name=\"led\" value=\"green\" type=\"submit\">Verde</button>" \
                      "<button class=\"button blue\" name=\"led\" value=\"blue\" type=\"submit\">Azul</button>" \
                      "<button class=\"button purple\" name=\"led\" value=\"purple\" type=\"submit\">Roxo</button>" \
                      "<button class=\"button off\" name=\"led\" value=\"off\" type=\"submit\">Desligar</button>" \
                      "</form>" \
                      "<p>Brilho:</p>" \
                      "<form>" \
                      "<input type='range' min='0' max='255' value='%s' name='brightness'>" \
                      "<br><button class=\"button brightness\" type='submit'>Definir Brilho</button>" \
                      "</form>" \
                      "</div>" \
                      "</body></html>\r\n"


// ---------------------------------- Variáveis ---------------------------------

char http_response[2048];         // O conteúdo da resposta HTTP que será enviada ao cliente.
int start_wifi = 0;              // Inteiro que atua como uma flag para indicar se a inicialização do Wi-Fi começou.
char * current_request = "none"; // Esta string contém o valor da solicitação HTTP atual sendo processada.
static int brightness = 128;     // O valor inicial de brilho para o LED.
static int last_red = 0, last_green = 0, last_blue = 0;  // Armazena a última cor


// ---------------------------------- Funções ---------------------------------

// --------------------------- Função de Callback HTTP ---------------------------

/**
 * @brief Processa solicitações HTTP recebidas e lida com comandos de controle do LED.
 *
 * @param arg Um ponteiro para dados definidos pelo usuário passados para o callback (não utilizado).
 * @param tpcb Um ponteiro para o Bloco de Controle de Protocolo TCP (PCB) para a conexão.
 * @param p Um ponteiro para o buffer contendo o payload da solicitação HTTP.
 * @param err O status de erro dos dados recebidos.
 * @return err_t Retorna `ERR_OK` em caso de sucesso ou um código de erro apropriado.
 *
 * Esta função processa solicitações HTTP recebidas pelo servidor. Ela verifica comandos específicos
 * na solicitação para controlar um LED (por exemplo, ligá-lo ou desligá-lo). Se um comando reconhecido
 * for encontrado, a função atualiza o estado do LED de acordo. A resposta é então gerada dinamicamente
 * com base no estado atual do botão e enviada de volta ao cliente. Finalmente, o buffer recebido é liberado,
 * e a conexão TCP é mantida.
 *
 * ### Comportamento:
 * - Se o cliente fechar a conexão (`p == NULL`), o servidor fecha a conexão TCP.
 * - Reconhece os seguintes comandos HTTP:
 *   - `"GET /?led=on"`: Liga o LED com brilho máximo.
 *   - `"GET /?led=off"`: Desliga o LED.
 * - Substitui o espaço reservado `%s` no modelo de resposta HTTP com o estado atual do botão.
 * - Envia a resposta HTTP gerada de volta ao cliente.
 *
 * @note Esta função é invocada automaticamente quando novos dados são recebidos pelo servidor
 *   (callback registrado em `connection_callback`).
 * @note Garante o gerenciamento adequado da memória liberando o buffer recebido após o processamento.
 */
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *request = (char *)p->payload;
    int red = last_red, green = last_green, blue = last_blue;  // Mantém a última cor

    // Processar mudança de cor do LED
    if (strstr(request, "GET /?led=red")) {
        red = 255; green = 0; blue = 0;
        current_request = "led=red";
    } 
    else if (strstr(request, "GET /?led=green")) {
        red = 0; green = 255; blue = 0;
        current_request = "led=green";
    }
    else if (strstr(request, "GET /?led=blue")) {
        red = 0; green = 0; blue = 255;
        current_request = "led=blue";
    }
    else if (strstr(request, "GET /?led=purple")) {
        red = 255; green = 0; blue = 255;
        current_request = "led=purple";
    }
    else if (strstr(request, "GET /?led=off")) {
        current_request = "led=off";
        red = 0; green = 0; blue = 0;
    }

    // Atualizar cor armazenada somente se não for um "off"
    if (!(red == 0 && green == 0 && blue == 0)) {
        last_red = red;
        last_green = green;
        last_blue = blue;
    }

    // Processar ajuste de brilho
    char *brightness_param = strstr(request, "brightness=");
    
    if (brightness_param) {
        char *brightness_value = brightness_param + 10;
        
        while (*brightness_value && !isdigit(*brightness_value)) {
            brightness_value++;  
        }

        brightness = strtol(brightness_value, NULL, 10);
    }

    printf("Brilho: %d\n", brightness);

    // Aplicar brilho à cor do LED
    pwm_set_gpio_level(LED_RED_PIN, (red * brightness) / 255);
    pwm_set_gpio_level(LED_GREEN_PIN, (green * brightness) / 255);
    pwm_set_gpio_level(LED_BLUE_PIN, (blue * brightness) / 255);

    // Atualizar resposta HTTP
    snprintf(http_response, sizeof(http_response), HTTP_RESPONSE, brightness);

    // Enviar resposta
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);

    // Liberar buffer recebido
    pbuf_free(p);

    return ERR_OK;
}


// --------------------------- Função de Callback de Conexão ---------------------------

/**
 * @brief Lida com novas conexões TCP recebidas e configura o callback HTTP.
 *
 * @param arg Um ponteiro para dados definidos pelo usuário passados para o callback (não utilizado).
 * @param newpcb Um ponteiro para o novo Bloco de Controle de Protocolo TCP (PCB) representando a conexão.
 * @param err O status de erro da conexão recebida.
 * @return err_t Retorna `ERR_OK` em caso de sucesso ou um código de erro apropriado.
 *
 * Esta função é acionada sempre que uma nova conexão TCP é estabelecida com o servidor.
 * Ela associa a função `http_callback` para lidar com dados recebidos na conexão.
 * Opcionalmente, pode registrar uma função de polling (por exemplo, `update_server`) para atualizações periódicas do servidor.
 *
 * ### Comportamento:
 * - Registra a função `http_callback` para lidar com solicitações HTTP recebidas na conexão.
 * - Opcionalmente, suporta polling habilitando a função `tcp_poll` (comentada no código).
 * - Não gerencia diretamente o fechamento ou limpeza da conexão; isso é tratado por outros mecanismos.
 *
 * ### Notas:
 * @note A função de callback `http_callback` é invocada quando novos dados são recebidos para a conexão.
 * @note Esta função é projetada para funcionar em conjunto com a pilha TCP/IP lwIP.
 */
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associa o callback HTTP
    //tcp_poll(newpcb, update_server, 100);
    return ERR_OK;
}


// --------------------------- Função de Desligamento do Servidor TCP ---------------------------

/**
 * @brief Desliga o servidor TCP e libera recursos.
 *
 * @param server_state Um ponteiro para a estrutura `TCP_SERVER_T` representando o estado do servidor.
 *
 * Esta função desliga graciosamente o servidor TCP fechando todas as conexões ativas,
 * liberando a porta ocupada e liberando qualquer memória dinamicamente alocada associada
 * ao estado do servidor.
 *
 * ### Comportamento:
 * - Fecha todas as conexões ativas invocando `tcp_server_close`.
 * - Libera a memória alocada para a estrutura `TCP_SERVER_T`.
 * - Imprime uma mensagem de depuração indicando que o servidor foi desligado e a porta foi liberada.
 *
 * @note Certifique-se de que nenhuma operação em andamento dependa do estado do servidor antes de invocar esta função.
 * @note O estado do servidor deve ser devidamente inicializado antes de chamar esta função.
 */
void shutdown_tcp_server(TCP_SERVER_T *server_state) {
    if (server_state) {
        tcp_server_close(server_state);
        free(server_state);
    }
    DEBUG_printf("Servidor TCP fechado e porta 80 liberada.\n");
}


// --------------------------- Função de Configuração do Servidor TCP ---------------------------

/**
 * @brief Inicializa e inicia o servidor HTTP na porta 80.
 *
 * Esta função configura o servidor TCP criando um Bloco de Controle de Protocolo (PCB),
 * vinculando-o à porta 80 e colocando-o em modo de escuta. Ela também associa a função
 * `connection_callback` para lidar com conexões de clientes recebidas.
 *
 * ### Comportamento:
 * - Cria um novo PCB TCP usando `tcp_new`.
 * - Vincula o servidor à porta 80 em todas as interfaces de rede disponíveis.
 * - Coloca o PCB em modo de escuta para aceitar conexões recebidas.
 * - Associa a função `connection_callback` para gerenciar novas conexões de clientes.
 * - Imprime mensagens de status indicando sucesso ou erros durante a configuração.
 *
 * @note Se a criação do PCB ou a vinculação da porta falhar, a função imprime uma mensagem de erro
 *   e sai sem concluir a configuração do servidor.
 * @note Esta função é projetada para uso com a pilha TCP/IP lwIP.
 */
static void start_http_server(void) {

    // Criando uma nova Estrutura de Controle de Protocolo TCP (PCB)
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    // Conectar o servidor à porta 80
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao conectar ao servidor na porta 80\n");
        return;
    }

    pcb = tcp_listen(pcb);  // Coloca o PCB em modo de escuta
    tcp_accept(pcb, connection_callback);  // Associa o callback de conexão

    printf("Servidor HTTP em execução na porta 80...\n");
}

// Função para criar a solicitação HTTP POST com dados JSON
void create_http_request(float media,float corrente) {
    char json_data[64]; // Buffer para o JSON
    char http_request[256]; // Buffer para a requisição HTTP
    
    snprintf(http_request, sizeof(http_request),
             "GET /update?api_key=JWR3PN07O0NANG46&field1=10 HTTP/1.1\r\n"
             "Host: api.thingspeak.com\r\n"
             "Connection: close\r\n\r\n");
    send_http_request(http_request, strlen(http_request));
}

// Função para iniciar uma solicitação HTTP
void send_http_request(const void *data, u16_t len) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        //return;
    }

    ip_addr_t server_ip;
    err_t err = dns_gethostbyname("api.thingspeak.com", &server_ip, dns_callback, (void *)data);
    if (err == ERR_OK) {
        // O endereço IP foi resolvido imediatamente
        dns_callback("api.thingspeak.com", &server_ip, (void *)data);
    } else if (err == ERR_INPROGRESS) {
        // A resolução do DNS está em andamento, o callback será chamado quando terminar
        printf("Resolução do DNS em andamento...\n");
    } else {
        printf("Erro ao iniciar a resolução do DNS\n");
        return;
    }

    if (tcp_connect(pcb, &server_ip, 80, NULL) != ERR_OK) {
        printf("Erro ao conectar ao servidor\n");
        //return;
    }

    // Associa o callback de recepção
    tcp_recv(pcb, http_client_callback);

    // Cria e envia a solicitação HTTP
    tcp_write(pcb, data, len, TCP_WRITE_FLAG_COPY);
}
// Função de callback para processar respostas HTTP
static err_t http_client_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        // Conexão fechada pelo servidor
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Processa a resposta HTTP
    char *response = (char *)p->payload;
    printf("Resposta HTTP:\n%s\n", response);

    // Libera o buffer recebido
    pbuf_free(p);

    return ERR_OK;
}
// Função de callback para processar respostas do DNS
static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr == NULL) {
        printf("Erro ao resolver o nome de domínio: %s\n", name);
        return;
    }

    printf("Nome de domínio resolvido: %s -> %s\n", name, ipaddr_ntoa(ipaddr));

    // Conecte ao servidor usando o endereço IP resolvido
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    if (tcp_connect(pcb, ipaddr, 80, NULL) != ERR_OK) {
        printf("Erro ao conectar ao servidor\n");
        return;
    }

    // Associa o callback de recepção
    tcp_recv(pcb, http_client_callback);

    // Cria e envia a solicitação HTTP
    const char *data = (const char *)callback_arg;
    tcp_write(pcb, data, strlen(data), TCP_WRITE_FLAG_COPY);
}

#endif /* WIFI_H */
