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

// Função para gerar valores aleatórios dentro das faixas especificadas
void generate_random_coordinates(float *lat, float *lon) {
    float lat_min = -5.813619388080041;
    float lat_max = -5.81018697695044;
    float lon_min = -35.20495186786747;
    float lon_max = -35.201481075005;

    *lat = lat_min + ((float)rand() / RAND_MAX) * (lat_max - lat_min);
    *lon = lon_min + ((float)rand() / RAND_MAX) * (lon_max - lon_min);
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

// Função para iniciar uma solicitação HTTP
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

// Função para criar a solicitação HTTP POST com dados JSON
void build_http_request(float temperatura) {
    char http_request[256]; // Buffer para a requisição HTTP
    float lat, lon;

    generate_random_coordinates(&lat, &lon);
    snprintf(http_request, sizeof(http_request),
             "GET /update?api_key=JWR3PN07O0NANG46&field1=%.2f&field2=%.6f&field3=%.6f HTTP/1.1\r\n"
             "Host: api.thingspeak.com\r\n"
             "Connection: close\r\n\r\n", temperatura, lat, lon);
    star_http_request(http_request, strlen(http_request));
}

#endif /*HTTP_H*/