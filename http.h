/******************************************************************************
 * @file    http.h
 * @brief   Arquivo contendo definições e protótipos de funções para a
 *          aplicação de Requisições HTTP no Raspberry Pi Pico W.
 *
 * @authors Gabriel Domingos de Medeiros
 * @date    Fevereiro 2025
 * @version 1.0.0
 *
 * @note    Este arquivo inclui as definições, constantes e protótipos de funções
 *          necessários para a aplicação.
 ******************************************************************************/

#ifndef HTTP_H
#define HTTP_H

#include "pico/cyw43_arch.h"      // Biblioteca para usar o módulo de conectividade para raspberry pi pico w.
#include "lwip/tcp.h"             // Biblioteca de Funções TCP
#include "lwip/apps/httpd.h"      // Biblioteca de funções para o protocolo HTTP
#include "lwip/dns.h"             // Biblioteca de Funções DNS
#include <time.h>                 // Biblioteca para manipulação de tempo
#include "defines_functions.h"    // Arquivo contendo definições e funções para o projeto.


// --------------------------- Função de Callback para Processar Respostas HTTP ---------------------------

/**
 * @brief Função de callback para processar respostas HTTP.
 *
 * @param arg Ponteiro para o argumento passado para o callback.
 * @param tpcb Ponteiro para o bloco de controle de protocolo TCP.
 * @param p Ponteiro para o buffer de pacotes contendo os dados recebidos.
 * @param err Código de erro indicando o status da operação de recebimento.
 * @return err_t Retorna ERR_OK em caso de sucesso, ou um código de erro apropriado em caso de falha.
 *
 * Esta função é chamada quando uma resposta HTTP é recebida do servidor.
 * Ela processa a resposta, imprime o conteúdo e libera o buffer recebido.
 *
 * ### Comportamento:
 * - Verifica se a conexão foi fechada pelo servidor.
 * - Processa a resposta HTTP e imprime o conteúdo.
 * - Libera o buffer recebido.
 *
 * @note O estado da conexão e o PCB são usados para gerenciar a conexão TCP.
 */
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


// --------------------------- Função de Callback para Processar Respostas do DNS ---------------------------

/**
 * @brief Função de callback para processar respostas do DNS.
 *
 * @param name O nome do domínio que foi resolvido.
 * @param ipaddr Ponteiro para o endereço IP resolvido.
 * @param callback_arg Argumento passado para o callback.
 *
 * Esta função é chamada quando uma resposta DNS é recebida.
 * Ela processa a resposta, imprime o endereço IP resolvido e tenta conectar ao servidor.
 *
 * ### Comportamento:
 * - Verifica se o endereço IP foi resolvido com sucesso.
 * - Imprime o endereço IP resolvido.
 * - Tenta conectar ao servidor usando o endereço IP resolvido.
 * - Associa o callback de recepção e envia a solicitação HTTP.
 *
 * @note O estado da conexão e o PCB são usados para gerenciar a conexão TCP.
 */
static void handle_dns_response(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
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


// --------------------------- Função para Iniciar uma Solicitação HTTP ---------------------------

/**
 * @brief Inicia uma solicitação HTTP.
 *
 * @param data Ponteiro para os dados da solicitação HTTP.
 * @param len Comprimento dos dados da solicitação HTTP.
 *
 * Esta função inicia uma solicitação HTTP criando um PCB, resolvendo o nome de domínio
 * e conectando ao servidor. Ela também associa o callback de recepção e envia a solicitação HTTP.
 *
 * ### Comportamento:
 * - Cria um novo PCB.
 * - Resolve o nome de domínio usando DNS.
 * - Conecta ao servidor usando o endereço IP resolvido.
 * - Associa o callback de recepção.
 * - Cria e envia a solicitação HTTP.
 *
 * @note O estado da conexão e o PCB são usados para gerenciar a conexão TCP.
 */
void star_http_request(const void *data, u16_t len) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        //return;
    }

    ip_addr_t server_ip;
    err_t err = dns_gethostbyname("api.thingspeak.com", &server_ip, handle_dns_response, (void *)data);
    if (err == ERR_OK) {
        // O endereço IP foi resolvido imediatamente
        handle_dns_response("api.thingspeak.com", &server_ip, (void *)data);
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


// --------------------------- Função para Criar a Solicitação HTTP POST com Dados JSON ---------------------------

/**
 * @brief Cria a solicitação HTTP POST com dados JSON.
 *
 * @param temperatura A temperatura a ser enviada na solicitação HTTP.
 *
 * Esta função cria uma solicitação HTTP POST com dados JSON, incluindo a temperatura,
 * latitude e longitude geradas aleatoriamente. Ela então inicia a solicitação HTTP.
 *
 * ### Comportamento:
 * - Gera coordenadas aleatórias para latitude e longitude.
 * - Formata a solicitação HTTP com os dados fornecidos.
 * - Inicia a solicitação HTTP chamando `star_http_request`.
 *
 * @note A função depende da função `generate_random_coordinates` para gerar coordenadas aleatórias.
 */
void build_http_request(float temperatura) {
    char http_request[256]; // Buffer para a requisição HTTP

    generate_random_coordinates(&lat, &lon);
    snprintf(http_request, sizeof(http_request),
             "GET /update?api_key=JWR3PN07O0NANG46&field1=%.2f&field2=%.6f&field3=%.6f HTTP/1.1\r\n"
             "Host: api.thingspeak.com\r\n"
             "Connection: close\r\n\r\n", temperatura, lat, lon);
    star_http_request(http_request, strlen(http_request));
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

#endif /*HTTP_H*/