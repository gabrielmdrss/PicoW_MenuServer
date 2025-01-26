#ifndef AP_MODE_UTILITY_H
#define AP_MODE_UTILITY_H

#include "pico/cyw43_arch.h"
#include <string.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "dhcpserver.h"
#include "dnsserver.h"

#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
#define LED_TEST_BODY "<html><body><h1>Hello from Pico W.</h1><p>Led is %s</p><p><a href=\"?led=%d\">Turn led %s</a></body></html>"
#define LED_PARAM "led=%d"
#define LED_TEST "/ledtest"
#define CONFIG "/config"
#define LED_GPIO CYW43_WL_GPIO_LED_PIN
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s" CONFIG "\n\n"

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

#define SUCCESS_PAGE "<html><body style=\"display:flex;justify-content:center;align-items:center;height:100vh;background-color:#e3f2fd;\">\n\
<div style=\"text-align:center;max-width:400px;padding:20px;border-radius:10px;background-color:white;box-shadow:0 4px 8px rgba(0,0,0,0.2);\">\n\
<h1 style=\"color:#1976d2;\">Configuration saved successfully!</h1>\n\
<a href=\"/config\" style=\"display:inline-block;margin-top:20px;padding:10px 20px;background-color:#1976d2;color:white;text-decoration:none;border-radius:5px;font-size:16px;\">Back to Configuration</a>\n\
</div></body></html>"

#define FAILURE_PAGE "<html><body style=\"display:flex;justify-content:center;align-items:center;height:100vh;background-color:#e3f2fd;\">\n\
<div style=\"text-align:center;max-width:400px;padding:20px;border-radius:10px;background-color:white;box-shadow:0 4px 8px rgba(0,0,0,0.2);\">\n\
<h1 style=\"color:#1976d2;\">Error saving configuration</h1>\n\
<p style=\"color:#444;\">Por favor, tente novamente.</p>\n\
<a href=\"/config\" style=\"display:inline-block;margin-top:20px;padding:10px 20px;background-color:#1976d2;color:white;text-decoration:none;border-radius:5px;font-size:16px;\">Back to Configuration</a>\n\
</div></body></html>"

char ssid[32] = {0};
char password[64] = {0}; // Aumentado para aceitar senhas maiores
int id_pw_collected = 0;
int aux_connection = 1;


typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[1024];
    char result[1024];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;

void replace_plus_with_space(char *str) {
    while (*str) {
        if (*str == '+') {
            *str = ' ';
        }
        str++;
    }
}

int process_post_payload(const char *request, char *payload) {
    char id[32] = {0};
    char pw[64] = {0}; // Aumentado para aceitar senhas maiores

    // Extração do SSID
    char *ssid_start = strstr(payload, "ssid=");
    if (ssid_start) {
        ssid_start += strlen("ssid=");
        char *ssid_end = strchr(ssid_start, '&');
        size_t ssid_len = ssid_end ? (size_t)(ssid_end - ssid_start) : strlen(ssid_start);
        if (ssid_len < sizeof(id)) {
            strncpy(id, ssid_start, ssid_len);
            id[ssid_len] = '\0';
        }
    }

    // Extração da senha
    char *password_start = strstr(payload, "password=");
    if (password_start) {
        password_start += strlen("password=");
        char *password_end = strchr(password_start, '&'); // Procure o próximo delimitador
        size_t password_len = password_end ? (size_t)(password_end - password_start) : strlen(password_start);

        if (password_len < sizeof(pw)) {
            strncpy(pw, password_start, password_len);
            pw[password_len] = '\0'; // Garantir terminação
        }
    }

    // Debug
    DEBUG_printf("SSID extraído: %s\n", id);
    DEBUG_printf("Password extraído: %s\n", pw);

    strncpy(ssid, id, sizeof(ssid) - 1);
    strncpy(password, pw, sizeof(password) - 1);


    // Retorne sucesso ou erro
    return (strlen(id) > 0 && strlen(pw) > 0) ? 0 : -1;
}

static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_sent %u\n", len);
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("all done\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

static int test_server_content(const char *request, const char *params, char *result, size_t max_result_len) {
    int len = 0;
    if (strncmp(request, CONFIG, sizeof(CONFIG) - 1) == 0) {
        // Get the state of the led
        bool value;
        cyw43_gpio_get(&cyw43_state, LED_GPIO, &value);
        int led_state = value;

        // See if the user changed it
        if (params) {
            int led_param = sscanf(params, LED_PARAM, &led_state);
            if (led_param == 1) {
                if (led_state) {
                    // Turn led on
                    cyw43_gpio_set(&cyw43_state, 0, true);
                } else {
                    // Turn led off
                    cyw43_gpio_set(&cyw43_state, 0, false);
                }
            }
        }
        // Generate result
        if (led_state) {
            len = snprintf(result, max_result_len, WIFI_CONFIG_BODY, "ON", 0, "OFF");
        } else {
            len = snprintf(result, max_result_len, WIFI_CONFIG_BODY, "OFF", 1, "ON");
        }
    }
    return len;
}

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
        // Copy the request into the buffer
        pbuf_copy_partial(p, con_state->headers, p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len, 0);

        // Handle GET request
        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_GET); // + space
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
            
            // Generate content
            con_state->result_len = test_server_content(request, params, con_state->result, sizeof(con_state->result));
            DEBUG_printf("Request: %s?%s\n", request, params);
            DEBUG_printf("Result: %d\n", con_state->result_len);

            // Check we had enough buffer space
            if (con_state->result_len > sizeof(con_state->result) - 1) {
                DEBUG_printf("Too much result data %d\n", con_state->result_len);
                return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
            }

            // Generate web page
            if (con_state->result_len > 0) {
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                    200, con_state->result_len);
                if (con_state->header_len > sizeof(con_state->headers) - 1) {
                    DEBUG_printf("Too much header data %d\n", con_state->header_len);
                    return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
                }
            } else {
                // Send redirect
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT,
                    ipaddr_ntoa(con_state->gw));
                DEBUG_printf("Sending redirect %s", con_state->headers);
            }

            // Send the headers to the client
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("failed to write header data %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            // Send the body to the client
            if (con_state->result_len) {
                err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (err != ERR_OK) {
                    DEBUG_printf("failed to write result data %d\n", err);
                    return tcp_close_client_connection(con_state, pcb, err);
                }
            }
        }

        if (strncmp(HTTP_POST, con_state->headers, sizeof(HTTP_POST) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_POST); // + space
            char *space = strchr(request, ' ');
            if (space) {
                *space = 0; // Terminar a string no final do path
            }

            // Verifica se o marcador foi encontrado
            char *payload = strstr(con_state->headers, "\r\n\r\n");
            if (p->payload) {
                p->payload += 4; // Avança os 4 caracteres de "\r\n\r\n" para chegar no corpo

                // Garantir que o payload seja uma string terminada por '\0'
                ((char *)p->payload)[p->len - 4] = '\0';

                // Processar os dados recebidos no payload
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

            // Atualizar os headers para servir o HTML
            con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers),
                HTTP_RESPONSE_HEADERS, 200, strlen(con_state->result));

            // Enviar os headers para o cliente
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("Falha ao enviar os headers do POST: %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            // Enviar o corpo da resposta para o cliente
            err = tcp_write(pcb, con_state->result, strlen(con_state->result), 0);
            if (err != ERR_OK) {
                DEBUG_printf("Falha ao enviar os dados do corpo: %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }
        }

        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_poll_fn\n");
    return tcp_close_client_connection(con_state, pcb, ERR_OK); // Just disconnect clent?
}

static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err_fn %d\n", err);
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("failure in accept\n");
        return ERR_VAL;
    }
    DEBUG_printf("client connected\n");

    // Create the state for the connection
    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("failed to allocate connect state\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb; // for checking
    con_state->gw = &state->gw;

    // setup connection to client
    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

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
        DEBUG_printf("failed to bind to port %d\n");
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