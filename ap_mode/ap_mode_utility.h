/******************************************************************************
 * @file    ap_mode_utility.h
 * @brief   Arquivo contendo definições e protótipos de funções para a
 *          aplicação do modo Ponto de Acesso (AP) no Raspberry Pi Pico W.
 *
 * @autores Gabriel Domingos de Medeiros
 * @data    Fevereiro 2025
 * @versão  1.0.0
 *
 * @nota    Este arquivo inclui as definições, constantes e protótipos de funções
 *          necessários para a aplicação.
 ******************************************************************************/

#ifndef AP_MODE_UTILITY_H
#define AP_MODE_UTILITY_H

// ---------------------------------- Includes ---------------------------------

#include "pico/cyw43_arch.h"            // Biblioteca para usar o módulo de conectividade para Raspberry Pi Pico W.
#include <string.h>                     // Biblioteca padrão para funções de manipulação de strings.

#include "lwip/pbuf.h"                  // Biblioteca para gerenciamento de buffer de pacotes no LWIP.
#include "lwip/tcp.h"                   // Biblioteca de funções TCP.

#include "dhcpserver.h"                 // Biblioteca para funcionalidade de servidor DHCP.
#include "dnsserver.h"                  // Biblioteca para funcionalidade de servidor DNS.

// ----------------------------------- Defines ----------------------------------

#define TCP_PORT 80                     // Número da porta TCP para o servidor HTTP.
#define DEBUG_printf printf             // Macro para impressão de depuração.
#define POLL_TIME_S 5                   // Tempo de polling em segundos para operações do servidor.
#define HTTP_GET "GET"                  // String do método HTTP GET.
#define HTTP_POST "POST"                // String do método HTTP POST.
#define CONFIG "/config"                // Caminho da URL para a página de configuração.

/* Definições de Teste usando o LED Onboard do Raspberry Pi Pico */
#define LED_TEST_BODY "<html><body><h1>Olá do Pico W.</h1><p>Led está %s</p><p><a href=\"?led=%d\">Ligar led %s</a></body></html>"
#define LED_PARAM "led=%d"              // String de parâmetro para controle do LED na URL.
#define LED_TEST "/ledtest"             // Caminho da URL para a página de teste do LED.
#define CONFIG "/config"                // Caminho da URL para a página de configuração.
#define LED_GPIO CYW43_WL_GPIO_LED_PIN  // Número do pino GPIO para o LED no Raspberry Pi Pico W.


/**
 * @brief Modelo de cabeçalhos de resposta HTTP.
 *
 * Esta definição contém uma string de cabeçalhos de resposta HTTP pré-formatada, incluindo código de status e comprimento do conteúdo.
 * É dinamicamente populada com o código de status (`%d`) e comprimento do conteúdo (`%d`) antes de ser enviada ao cliente.
 */
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"

/**
 * @brief Resposta HTTP para redirecionamento.
 *
 * Esta definição contém uma string de resposta HTTP pré-formatada para redirecionamento.
 * É dinamicamente populada com a URL de destino (`%s`) antes de ser enviada ao cliente.
 */
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s" CONFIG "\n\n"

/**
 * @brief Modelo de corpo HTML para a página de configuração Wi-Fi.
 *
 * Esta definição contém uma string de corpo HTML pré-formatada para a página de configuração Wi-Fi.
 * Inclui um formulário para inserir SSID e senha, estilizado com CSS inline.
 */
#define WIFI_CONFIG_BODY "<html><body style=\"display:flex;justify-content:center;align-items:center;height:100vh;background-color:#e3f2fd;\">\n\
<div style=\"text-align:center;max-width:400px;padding:20px;border-radius:10px;background-color:white;box-shadow:0 4px 8px rgba(0,0,0,0.2);\">\n\
<h1 style=\"color:#1976d2;\">Wi-Fi Configuration</h1>\n\
<p style=\"color:#444;\">Enter your Wi-Fi credentials below:</p>\n\
<form method=\"POST\" action=\"/post\">\n\
<label for=\"ssid\" style=\"font-weight:bold;\">SSID:</label><br>\n\
<input type=\"text\" id=\"ssid\" name=\"ssid\" required style=\"width:100%;padding:10px;margin:10px 0;border:1px solid #ccc;border-radius:5px;\"><br>\n\
<label for=\"password\" style=\"font-weight:bold;\">PASSWORD:</label><br>\n\
<input type=\"password\" id=\"password\" name=\"password\" required style=\"width:100%;padding:10px;margin:10px 0;border:1px solid #ccc;border-radius:5px;\"><br>\n\
<button type=\"submit\" style=\"width:100%;padding:10px;background-color:#1976d2;color:white;border:none;border-radius:5px;font-size:16px;\">Salvar</button>\n\
</form>\n\
</div></body></html>"

/**
 * @brief Modelo de página HTML para configuração bem-sucedida.
 *
 * Esta definição contém uma string de página HTML pré-formatada para exibir uma mensagem de sucesso
 * quando a configuração é salva com sucesso. Nesta página, há um botão para 
 * retornar à página de configuração.
 */
#define SUCCESS_PAGE "<html><body style=\"display:flex;justify-content:center;align-items:center;height:100vh;background-color:#e3f2fd;\">\n\
<div style=\"text-align:center;max-width:400px;padding:20px;border-radius:10px;background-color:white;box-shadow:0 4px 8px rgba(0,0,0,0.2);\">\n\
<h1 style=\"color:#1976d2;\">Configuração salva com sucesso!</h1>\n\
<a href=\"/config\" style=\"display:inline-block;margin-top:20px;padding:10px 20px;background-color:#1976d2;color:white;text-decoration:none;border-radius:5px;font-size:16px;\">Voltar para Configuração</a>\n\
</div></body></html>"

/**
 * @brief Modelo de página HTML para configuração falhada.
 *
 * Esta definição contém uma string de página HTML pré-formatada para exibir uma mensagem de erro
 * quando há uma falha ao salvar a configuração. Nesta página, há um botão para 
 * retornar à página de configuração.
 */
#define FAILURE_PAGE "<html><body style=\"display:flex;justify-content:center;align-items:center;height:100vh;background-color:#e3f2fd;\">\n\
<div style=\"text-align:center;max-width:400px;padding:20px;border-radius:10px;background-color:white;box-shadow:0 4px 8px rgba(0,0,0,0.2);\">\n\
<h1 style=\"color:#1976d2;\">Error saving configuration</h1>\n\
<p style=\"color:#444;\">Por favor, tente novamente.</p>\n\
<a href=\"/config\" style=\"display:inline-block;margin-top:20px;padding:10px 20px;background-color:#1976d2;color:white;text-decoration:none;border-radius:5px;font-size:16px;\">Back to Configuration</a>\n\
</div></body></html>"

// ---------------------------------- Variáveis ---------------------------------

char ssid[32] = {0};              // Array para armazenar o SSID da rede Wi-Fi.
char password[64] = {0};          // Array para armazenar a senha da rede Wi-Fi. Aumentado para aceitar senhas mais longas.
int id_pw_collected = 0;          // Flag para indicar se o SSID e a senha foram coletados (1) ou não (0).
int aux_connection = 1;           // Variável auxiliar para indicar se o modo AP já foi desativado (0) ou não (1).


/**
 * @brief Estrutura para armazenar o estado do servidor TCP.
 *
 * Esta estrutura armazena o estado do servidor TCP, incluindo o PCB do servidor,
 * status de conclusão e endereço IP do gateway.
 */
typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;   // Ponteiro para o bloco de controle de protocolo TCP do servidor.
    bool complete;                // Flag para indicar se a operação do servidor está completa.
    ip_addr_t gw;                 // Endereço IP do gateway.
} TCP_SERVER_T;

/**
 * @brief Estrutura para armazenar o estado da conexão TCP.
 *
 * Esta estrutura armazena o estado de uma conexão TCP, incluindo o PCB da conexão,
 * comprimento dos dados enviados, cabeçalhos, resultado, comprimento dos cabeçalhos,
 * comprimento do resultado e endereço IP do gateway.
 */
typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;          // Ponteiro para o bloco de controle de protocolo TCP da conexão.
    int sent_len;                 // Comprimento dos dados enviados.
    char headers[1024];           // Buffer para armazenar os cabeçalhos HTTP.
    char result[1024];            // Buffer para armazenar o resultado HTTP.
    int header_len;               // Comprimento dos cabeçalhos HTTP.
    int result_len;               // Comprimento do resultado HTTP.
    ip_addr_t *gw;                // Ponteiro para o endereço IP do gateway.
} TCP_CONNECT_STATE_T;


// ---------------------------------- Funções ---------------------------------

// --------------------------- Função para Processar Payload POST ---------------------------

/**
 * @brief Processa o payload da requisição POST para extrair SSID e senha.
 *
 * Esta função recebe um payload codificado em URL de uma requisição HTTP POST,
 * decodifica-o, extrai os parâmetros SSID e senha, e os armazena
 * em variáveis globais para uso posterior.
 *
 * @param request A requisição HTTP completa (não usada nesta função, mas mantida para extensibilidade).
 * @param payload O corpo da requisição POST codificado em URL contendo SSID e senha.
 * 
 * @return 0 se o SSID e a senha forem extraídos com sucesso, -1 caso contrário.
 *
 * @note A função garante a terminação correta das strings para evitar estouro de buffer.
 *       Ela também lida com a codificação URL, substituindo sequências "%XX" por seus equivalentes ASCII
 *       e convertendo símbolos "+" em espaços.
 */
int process_post_payload(const char *request, char *payload) {
    if (!payload) return -1;            // Proteção contra ponteiro nulo

    // Garante que o payload esteja corretamente terminado com NULL
    size_t payload_len = strlen(payload);
    if (payload_len == 0) return -1;    // Verificação de payload vazio

    // Buffer para payload decodificado para evitar estouro de buffer
    char decoded_payload[1024] = {0}; 

    // Lógica de decodificação URL: Converte "%XX" para ASCII e substitui "+" por espaço
    char *src = payload;
    char *dest = decoded_payload;
    char hex[3] = {0};                  // Armazenamento temporário para conversão hexadecimal

    while (*src && (dest - decoded_payload) < sizeof(decoded_payload) - 1) {
        if (*src == '%' && isxdigit((unsigned char)src[1]) && isxdigit((unsigned char)src[2])) {
            hex[0] = src[1];
            hex[1] = src[2];
            *dest++ = (char)strtol(hex, NULL, 16);
            src += 3;                   // Pula "%XX"
        } else if (*src == '+') {
            *dest++ = ' ';              // Converte '+' para espaço
            src++;
        } else {
            *dest++ = *src++;           // Copia caracteres normais
        }
    }
    *dest = '\0';                       // Garante que a string esteja terminada com NULL

    // Buffers temporários para extração de SSID e senha
    char id[32] = {0};                  // SSID
    char pw[64] = {0};                  // Senha

    // Extrai SSID
    char *ssid_start = strstr(decoded_payload, "ssid=");
    if (ssid_start) {
        ssid_start += strlen("ssid=");
        char *ssid_end = strchr(ssid_start, '&');
        size_t ssid_len = ssid_end ? (size_t)(ssid_end - ssid_start) : strlen(ssid_start);
        if (ssid_len < sizeof(id)) {
            strncpy(id, ssid_start, ssid_len);
            id[ssid_len] = '\0';
        }
    }

    // Extrai senha
    char *password_start = strstr(decoded_payload, "password=");
    if (password_start) {
        password_start += strlen("password=");
        char *password_end = strchr(password_start, '&');// Encontra próximo parâmetro
        size_t password_len = password_end ? (size_t)(password_end - password_start) : strlen(password_start);

        if (password_len < sizeof(pw)) {
            strncpy(pw, password_start, password_len);
            pw[password_len] = '\0';                   // Garante que a string esteja terminada com NULL
        }
    }

    // Logs de depuração para credenciais extraídas
    DEBUG_printf("SSID Extraído: %s\n", id);
    DEBUG_printf("SENHA Extraída: %s\n", pw);

    // Armazena valores extraídos em variáveis globais
    strncpy(ssid, id, sizeof(ssid) - 1);
    strncpy(password, pw, sizeof(password) - 1);

    // Retorna sucesso apenas se SSID e senha foram extraídos com sucesso
    return (strlen(id) > 0 && strlen(pw) > 0) ? 0 : -1;
}


// --------------------------- Função para Fechar Conexão do Cliente ---------------------------

/**
 * @brief Fecha a conexão TCP do cliente e libera os recursos associados.
 *
 * @param con_state Ponteiro para a estrutura de estado da conexão TCP.
 * @param client_pcb Ponteiro para o bloco de controle de protocolo TCP do cliente.
 * @param close_err Código de erro a ser retornado se a operação de fechamento falhar.
 * @return err_t Retorna ERR_OK em caso de sucesso, ou um código de erro apropriado em caso de falha.
 *
 * Esta função fecha a conexão TCP do cliente, libera os recursos associados
 * e lida com quaisquer erros que ocorram durante a operação de fechamento.
 *
 * ### Comportamento:
 * - Verifica se o estado da conexão e o PCB do cliente são válidos.
 * - Limpa os callbacks TCP para o PCB do cliente.
 * - Tenta fechar a conexão TCP.
 * - Se a operação de fechamento falhar, aborta a conexão.
 * - Libera a estrutura de estado da conexão, se existir.
 */
static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        // Verifica se o estado da conexão e o PCB do cliente são válidos
        assert(con_state && con_state->pcb == client_pcb);

        // Limpa os callbacks TCP para o PCB do cliente
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);

        // Tenta fechar a conexão TCP
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            // Se o fechamento falhar, aborta a conexão
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }

        // Libera a estrutura de estado da conexão, se existir
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}


// --------------------------- Função para Fechar o Servidor TCP ---------------------------

/**
 * @brief Fecha a conexão do servidor TCP e libera os recursos associados.
 *
 * @param state Ponteiro para a estrutura de estado do servidor TCP.
 *
 * Esta função fecha a conexão do servidor TCP e libera os recursos associados.
 *
 * ### Comportamento:
 * - Verifica se o PCB do servidor é válido.
 * - Limpa o argumento TCP para o PCB do servidor.
 * - Fecha a conexão TCP para o PCB do servidor.
 * - Define o ponteiro do PCB do servidor como NULL.
 *
 * @note O PCB do servidor é definido como NULL após fechar a conexão.
 */
static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        // Limpa o argumento TCP para o PCB do servidor
        tcp_arg(state->server_pcb, NULL);

        // Fecha a conexão TCP para o PCB do servidor
        tcp_close(state->server_pcb);

        // Define o ponteiro do PCB do servidor como NULL
        state->server_pcb = NULL;
    }
}

// --------------------------- Função de Callback para Envio do Servidor TCP ---------------------------

/**
 * @brief Função de callback para quando os dados são enviados com sucesso pelo servidor TCP.
 *
 * @param arg Ponteiro para o argumento passado para o callback (estado da conexão).
 * @param pcb Ponteiro para o bloco de controle de protocolo TCP.
 * @param len Comprimento dos dados que foram enviados.
 * @return err_t Retorna ERR_OK em caso de sucesso, ou um código de erro apropriado em caso de falha.
 *
 * Esta função é chamada quando os dados são enviados com sucesso pelo servidor TCP.
 * Ela atualiza o comprimento enviado no estado da conexão e verifica se todos os dados foram enviados.
 * Se todos os dados foram enviados, ela fecha a conexão do cliente.
 *
 * ### Comportamento:
 * - Atualiza o comprimento enviado no estado da conexão.
 * - Verifica se o comprimento total enviado é maior ou igual à soma dos comprimentos do cabeçalho e do resultado.
 * - Se todos os dados foram enviados, ela fecha a conexão do cliente.
 *
 * @note O estado da conexão e o PCB são usados para gerenciar a conexão TCP.
 */
static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_sent %u\n", len);
    
    // Atualiza o comprimento enviado no estado da conexão
    con_state->sent_len += len;
    
    // Verifica se todos os dados foram enviados
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("all done\n");
        // Fecha a conexão do cliente
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}


// --------------------------- Função de Conteúdo do Servidor de Teste ---------------------------

/**
 * @brief Gera o conteúdo do servidor com base na requisição e nos parâmetros.
 *
 * @param request A string da requisição HTTP.
 * @param params A string de parâmetros da requisição HTTP.
 * @param result O buffer para armazenar o resultado gerado.
 * @param max_result_len O comprimento máximo do buffer de resultado.
 * @return int O comprimento do resultado gerado.
 *
 * Esta função gera o conteúdo do servidor com base na requisição e nos parâmetros fornecidos.
 * Ela verifica o estado do LED e o atualiza se necessário, depois gera o conteúdo HTML apropriado
 * para ser enviado de volta ao cliente.
 *
 * ### Comportamento:
 * - Verifica se a requisição corresponde ao caminho CONFIG.
 * - Recupera o estado atual do LED.
 * - Se os parâmetros forem fornecidos, atualiza o estado do LED com base nos parâmetros.
 * - Gera o conteúdo HTML com base no estado do LED.
 *
 * @note O resultado gerado é armazenado no parâmetro `result`.
 */
static int test_server_content(const char *request, const char *params, char *result, size_t max_result_len) {
    int len = 0;
    if (strncmp(request, CONFIG, sizeof(CONFIG) - 1) == 0) {
        // Obtém o estado do LED
        bool value;
        cyw43_gpio_get(&cyw43_state, LED_GPIO, &value);
        int led_state = value;

        // Verifica se o usuário mudou o estado
        if (params) {
            int led_param = sscanf(params, LED_PARAM, &led_state);
            if (led_param == 1) {
                if (led_state) {
                    // Liga o LED
                    cyw43_gpio_set(&cyw43_state, 0, true);
                } else {
                    // Desliga o LED
                    cyw43_gpio_set(&cyw43_state, 0, false);
                }
            }
        }
        // Gera o resultado
        if (led_state) {
            len = snprintf(result, max_result_len, WIFI_CONFIG_BODY, "ON", 0, "OFF");
        } else {
            len = snprintf(result, max_result_len, WIFI_CONFIG_BODY, "OFF", 1, "ON");
        }
    }
    return len;
}

// --------------------------- Função de Callback para Recebimento do Servidor TCP ---------------------------

/**
 * @brief Função de callback para receber dados do cliente TCP.
 *
 * @param arg Ponteiro para o argumento passado para o callback (estado da conexão).
 * @param pcb Ponteiro para o bloco de controle de protocolo TCP.
 * @param p Ponteiro para o buffer de pacotes contendo os dados recebidos.
 * @param err Código de erro indicando o status da operação de recebimento.
 * @return err_t Retorna ERR_OK em caso de sucesso, ou um código de erro apropriado em caso de falha.
 *
 * Esta função é chamada quando dados são recebidos do cliente TCP. Ela processa
 * os dados recebidos, lida com requisições HTTP GET e POST, e gera a resposta apropriada
 * para ser enviada de volta ao cliente.
 *
 * ### Comportamento:
 * - Verifica se a conexão está fechada.
 * - Asserta a validade do estado da conexão e do PCB.
 * - Copia a requisição recebida para o buffer.
 * - Lida com requisições HTTP GET e POST.
 * - Gera o conteúdo de resposta apropriado.
 * - Envia os cabeçalhos e o corpo da resposta para o cliente.
 * - Libera o buffer de pacotes e retorna o código de erro apropriado.
 *
 * @note O estado da conexão e o PCB são usados para gerenciar a conexão TCP.
 */
err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!p) {
        DEBUG_printf("connection closed\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);
    if (p->tot_len > 0) {
        DEBUG_printf("tcp_server_recv %d err %d\n", p->tot_len, err);
#if 0
        for (struct pbuf *q = p; q != NULL; q = q->next) {
            DEBUG_printf("in: %.*s\n", q->len, q->payload);
        }
#endif
        // Copia a requisição para o buffer
        pbuf_copy_partial(p, con_state->headers, p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len, 0);

        // Lida com requisição GET
        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_GET); // + espaço
            char *params = strchr(request, '?');
            if (params) {
                if (*params) {
                    char *space = strchr(request, ' ');
                    *params++ = 0;
                    if (space) {
                        *space = 0;
                    }
                } else {
                    params = NULL;
                }
            }
            
            // Gera conteúdo
            con_state->result_len = test_server_content(request, params, con_state->result, sizeof(con_state->result));
            DEBUG_printf("Request: %s?%s\n", request, params);
            DEBUG_printf("Result: %d\n", con_state->result_len);

            // Verifica se havia espaço suficiente no buffer
            if (con_state->result_len > sizeof(con_state->result) - 1) {
                DEBUG_printf("Too much result data %d\n", con_state->result_len);
                return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
            }

            // Gera página web
            if (con_state->result_len > 0) {
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                    200, con_state->result_len);
                if (con_state->header_len > sizeof(con_state->headers) - 1) {
                    DEBUG_printf("Too much header data %d\n", con_state->header_len);
                    return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
                }
            } else {
                // Envia redirecionamento
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT,
                    ipaddr_ntoa(con_state->gw));
                DEBUG_printf("Sending redirect %s", con_state->headers);
            }

            // Envia os cabeçalhos para o cliente
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("failed to write header data %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            // Envia o corpo para o cliente
            if (con_state->result_len) {
                err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (err != ERR_OK) {
                    DEBUG_printf("failed to write result data %d\n", err);
                    return tcp_close_client_connection(con_state, pcb, err);
                }
            }
        }

        if (strncmp(HTTP_POST, con_state->headers, sizeof(HTTP_POST) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_POST); // + espaço
            char *space = strchr(request, ' ');
            if (space) {
                *space = 0;     // Termina a string no final do caminho
            }

            // Verifica se o marcador foi encontrado
            char *payload = strstr(con_state->headers, "\r\n\r\n");
            if (p->payload) {
                p->payload += 4; // Avança 4 caracteres de "\r\n\r\n" para alcançar o corpo

                // Garante que o payload seja uma string terminada em NULL
                ((char *)p->payload)[p->len - 4] = '\0';

                // Processa os dados recebidos no payload
                int process_result = process_post_payload(request, p->payload);
                if (process_result >= 0) {
                    // Resposta de sucesso com página
                    id_pw_collected = 1;
                    snprintf(con_state->result, sizeof(con_state->result), SUCCESS_PAGE);
                } else {
                    // Resposta de falha com página
                    snprintf(con_state->result, sizeof(con_state->result), FAILURE_PAGE);
                }
            } else {
                // Resposta de falha com página
                snprintf(con_state->result, sizeof(con_state->result), FAILURE_PAGE);
            }

            // Atualiza cabeçalhos para servir o HTML
            con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers),
                HTTP_RESPONSE_HEADERS, 200, strlen(con_state->result));

            // Envia os cabeçalhos para o cliente
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("Failed to send POST headers: %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            // Envia o corpo da resposta para o cliente
            err = tcp_write(pcb, con_state->result, strlen(con_state->result), 0);
            if (err != ERR_OK) {
                DEBUG_printf("Failed to send body data: %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }
        }

        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}


// --------------------------- Função de Polling do Servidor ---------------------------

/**
 * @brief Faz polling no servidor TCP por atividade e lida com a desconexão do cliente.
 *
 * @param arg Ponteiro para o argumento passado para a função de polling, tipicamente o estado da conexão.
 * @param pcb Ponteiro para o bloco de controle de protocolo TCP do cliente.
 * @return err_t Retorna ERR_OK em caso de sucesso, ou um código de erro apropriado em caso de falha.
 *
 * Esta função é chamada periodicamente para fazer polling no servidor TCP por atividade.
 * Se um cliente estiver conectado, ela lida com a desconexão do cliente.
 *
 * ### Comportamento:
 * - Converte o argumento para a estrutura de estado da conexão.
 * - Registra a chamada da função de polling para fins de depuração.
 * - Chama a função para fechar a conexão do cliente.
 *
 * @note Esta função é tipicamente registrada como um callback para o evento de polling do servidor TCP.
 */
static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_poll_fn\n");
    return tcp_close_client_connection(con_state, pcb, ERR_OK); // Apenas desconecta o cliente?
}


// --------------------------- Função de Erro do Servidor ---------------------------

/**
 * @brief Lida com erros do servidor TCP e fecha a conexão do cliente.
 *
 * @param arg Ponteiro para o argumento passado para a função de erro, tipicamente o estado da conexão.
 * @param err O código de erro indicando o tipo de erro.
 *
 * Esta função é chamada quando ocorre um erro no servidor TCP.
 * Ela lida com o erro fechando a conexão do cliente se o erro não for ERR_ABRT.
 *
 * ### Comportamento:
 * - Converte o argumento para a estrutura de estado da conexão.
 * - Registra o erro para fins de depuração.
 * - Chama a função para fechar a conexão do cliente se o erro não for ERR_ABRT.
 *
 * @note Esta função é tipicamente registrada como um callback para o evento de erro do servidor TCP.
 */
static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        // Registra o erro para fins de depuração
        DEBUG_printf("tcp_client_err_fn %d\n", err);

        // Fecha a conexão do cliente
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}


// --------------------------- Função de Aceitação do Servidor ---------------------------

/**
 * @brief Aceita uma nova conexão de cliente TCP e configura o estado da conexão.
 *
 * @param arg Ponteiro para o argumento passado para a função de aceitação, tipicamente o estado do servidor.
 * @param client_pcb Ponteiro para o bloco de controle de protocolo TCP do cliente.
 * @param err O código de erro indicando o status da operação de aceitação.
 * @return err_t Retorna ERR_OK em caso de sucesso, ou um código de erro apropriado em caso de falha.
 *
 * Esta função é chamada quando uma nova conexão de cliente TCP é aceita.
 * Ela configura o estado da conexão e registra os callbacks necessários para o cliente.
 *
 * ### Comportamento:
 * - Verifica se há erros na operação de aceitação.
 * - Registra o status da conexão para fins de depuração.
 * - Aloca memória para a estrutura de estado da conexão.
 * - Configura o estado da conexão e registra os callbacks necessários.
 *
 * @note Esta função é tipicamente registrada como um callback para o evento de aceitação do servidor TCP.
 */
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("failure in accept\n");
        return ERR_VAL;
    }
    DEBUG_printf("client connected\n");

    // Cria o estado para a conexão
    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("failed to allocate connect state\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb; // para verificação
    con_state->gw = &state->gw;

    // Configura a conexão com o cliente
    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}


// --------------------------- Função para Abrir o Servidor ---------------------------

/**
 * @brief Abre um servidor TCP e começa a escutar por conexões de clientes.
 *
 * @param arg Ponteiro para o argumento passado para a função de abertura, tipicamente o estado do servidor.
 * @return bool Retorna true em caso de sucesso, false em caso de falha.
 *
 * Esta função inicializa e abre um servidor TCP, associa-o a uma porta e começa a escutar por conexões de clientes.
 *
 * ### Comportamento:
 * - Cria um novo bloco de controle de protocolo TCP (PCB).
 * - Associa o PCB à porta especificada.
 * - Começa a escutar por conexões de clientes com um backlog de 1.
 * - Registra os callbacks necessários para aceitar conexões de clientes.
 *
 * @note Esta função é tipicamente chamada para iniciar o servidor TCP.
 */
static bool tcp_server_open(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    DEBUG_printf("starting server on port %u\n", TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        DEBUG_printf("failed to bind to port %d\n", err);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}

#endif /*AP_MODE_UTILITY_H*/