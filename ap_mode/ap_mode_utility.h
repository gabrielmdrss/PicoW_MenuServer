#ifndef AP_MODE_UTILITY_H
#define AP_MODE_UTILITY_H

/******************************************************************************
 * @file    ap_mode_utility.h
 * @brief   File containing definitions and function prototypes for the
 *          application of Access Point (AP) mode on the Raspberry Pi Pico W.
 *
 * @authors Gabriel Domingos de Medeiros
 * @date    February 2025
 * @version 1.0.0
 *
 * @note    This file includes the necessary definitions, constants, and function
 *          prototypes for the application.
 ******************************************************************************/

// ---------------------------------- Includes ---------------------------------

#include "pico/cyw43_arch.h"      // Library for using the connectivity module for Raspberry Pi Pico W.
#include <string.h>               // Standard library for string manipulation functions.

#include "lwip/pbuf.h"            // Library for packet buffer management in LWIP.
#include "lwip/tcp.h"             // TCP Function Library.

#include "dhcpserver.h"           // Library for DHCP server functionality.
#include "dnsserver.h"            // Library for DNS server functionality.

// ----------------------------------- Defines ----------------------------------

#define TCP_PORT 80               // TCP port number for the HTTP server.
#define DEBUG_printf printf       // Macro for debug printing.
#define POLL_TIME_S 5             // Polling time in seconds for server operations.
#define HTTP_GET "GET"            // HTTP GET method string.
#define HTTP_POST "POST"          // HTTP POST method string.
#define CONFIG "/config"          // URL path for the configuration page.

/* Defines of Test using the Raspberry Pi Pico Onboard LED*/
#define LED_TEST_BODY "<html><body><h1>Hello from Pico W.</h1><p>Led is %s</p><p><a href=\"?led=%d\">Turn led %s</a></body></html>"
#define LED_PARAM "led=%d"        // Parameter string for LED control in the URL.
#define LED_TEST "/ledtest"       // URL path for the LED test page.
#define CONFIG "/config"          // URL path for the configuration page.
#define LED_GPIO CYW43_WL_GPIO_LED_PIN // GPIO pin number for the LED on the Raspberry Pi Pico W.

/**
 * @brief HTTP response headers template.
 *
 * This define holds a preformatted HTTP response headers string, including status code and content length.
 * It is dynamically populated with the status code (`%d`) and content length (`%d`) before being sent to the client.
 */
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"

/**
 * @brief HTTP response for redirection.
 *
 * This define holds a preformatted HTTP response string for redirection.
 * It is dynamically populated with the target URL (`%s`) before being sent to the client.
 */
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s" CONFIG "\n\n"

/**
 * @brief HTML body template for Wi-Fi configuration page.
 *
 * This define holds a preformatted HTML body string for the Wi-Fi configuration page.
 * It includes a form for entering SSID and password, styled with inline CSS.
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
 * @brief HTML page template for successful configuration.
 *
 * This define holds a preformatted HTML page string for displaying a success message
 * when the configuration is saved successfully. On this page, there is a button to 
 * return to the configuration page.
 */
#define SUCCESS_PAGE "<html><body style=\"display:flex;justify-content:center;align-items:center;height:100vh;background-color:#e3f2fd;\">\n\
<div style=\"text-align:center;max-width:400px;padding:20px;border-radius:10px;background-color:white;box-shadow:0 4px 8px rgba(0,0,0,0.2);\">\n\
<h1 style=\"color:#1976d2;\">Configuration saved successfully!</h1>\n\
<a href=\"/config\" style=\"display:inline-block;margin-top:20px;padding:10px 20px;background-color:#1976d2;color:white;text-decoration:none;border-radius:5px;font-size:16px;\">Back to Configuration</a>\n\
</div></body></html>"

/**
 * @brief HTML page template for failed configuration.
 *
 * This define holds a preformatted HTML page string for displaying an error message
 * when there is a failure in saving the configuration. On this page, there is a button to 
 * return to the configuration page.
 */
#define FAILURE_PAGE "<html><body style=\"display:flex;justify-content:center;align-items:center;height:100vh;background-color:#e3f2fd;\">\n\
<div style=\"text-align:center;max-width:400px;padding:20px;border-radius:10px;background-color:white;box-shadow:0 4px 8px rgba(0,0,0,0.2);\">\n\
<h1 style=\"color:#1976d2;\">Error saving configuration</h1>\n\
<p style=\"color:#444;\">Por favor, tente novamente.</p>\n\
<a href=\"/config\" style=\"display:inline-block;margin-top:20px;padding:10px 20px;background-color:#1976d2;color:white;text-decoration:none;border-radius:5px;font-size:16px;\">Back to Configuration</a>\n\
</div></body></html>"

// ---------------------------------- Variables ---------------------------------

char ssid[32] = {0};              // Array to hold the SSID of the Wi-Fi network.
char password[64] = {0};          // Array to hold the password of the Wi-Fi network. Increased to accept longer passwords.
int id_pw_collected = 0;          // Flag to indicate if SSID and password have been collected(1) or not(0).
int aux_connection = 1;           // Auxiliary variable to indicate whether AP mode has already been disabled(0) or not(1).


/**
 * @brief Structure to hold TCP server state.
 *
 * This structure holds the state of the TCP server, including the server PCB,
 * completion status, and gateway IP address.
 */
typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;   // Pointer to the server's TCP protocol control block.
    bool complete;                // Flag to indicate if the server operation is complete.
    ip_addr_t gw;                 // Gateway IP address.
} TCP_SERVER_T;

/**
 * @brief Structure to hold TCP connection state.
 *
 * This structure holds the state of a TCP connection, including the connection PCB,
 * sent length, headers, result, header length, result length, and gateway IP address.
 */
typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;          // Pointer to the connection's TCP protocol control block.
    int sent_len;                 // Length of data sent.
    char headers[1024];           // Buffer to hold HTTP headers.
    char result[1024];            // Buffer to hold HTTP result.
    int header_len;               // Length of HTTP headers.
    int result_len;               // Length of HTTP result.
    ip_addr_t *gw;                // Pointer to the gateway IP address.
} TCP_CONNECT_STATE_T;


// ---------------------------------- Functions ---------------------------------

// --------------------------- Process POST Payload Function ---------------------------

/**
 * @brief Processes the POST request payload to extract SSID and password.
 *
 * This function takes a URL-encoded payload from an HTTP POST request,
 * decodes it, extracts the SSID and password parameters, and stores them
 * in global variables for further use.
 *
 * @param request The full HTTP request (not used in this function but kept for extensibility).
 * @param payload The URL-encoded POST request body containing SSID and password.
 * 
 * @return 0 if SSID and password are successfully extracted, -1 otherwise.
 *
 * @note The function ensures proper NULL-termination of strings to prevent buffer overflows.
 *       It also handles URL encoding, replacing "%XX" sequences with their ASCII equivalents
 *       and converting "+" symbols to spaces.
 */
int process_post_payload(const char *request, char *payload) {
    if (!payload) return -1; // Null pointer protection

    // Ensure payload is properly null-terminated
    size_t payload_len = strlen(payload);
    if (payload_len == 0) return -1; // Empty payload check

    // Buffer for decoded payload to prevent buffer overflow
    char decoded_payload[1024] = {0}; 

    // URL decoding logic: Converts "%XX" to ASCII and replaces "+" with space
    char *src = payload;
    char *dest = decoded_payload;
    char hex[3] = {0}; // Temporary storage for hex conversion

    while (*src && (dest - decoded_payload) < sizeof(decoded_payload) - 1) {
        if (*src == '%' && isxdigit((unsigned char)src[1]) && isxdigit((unsigned char)src[2])) {
            hex[0] = src[1];
            hex[1] = src[2];
            *dest++ = (char)strtol(hex, NULL, 16);
            src += 3; // Skip past "%XX"
        } else if (*src == '+') {
            *dest++ = ' '; // Convert '+' to space
            src++;
        } else {
            *dest++ = *src++; // Copy normal characters
        }
    }
    *dest = '\0'; // Ensure string is null-terminated

    // Temporary buffers for SSID and password extraction
    char id[32] = {0}; // SSID
    char pw[64] = {0}; // Password

    // Extract SSID
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

    // Extract password
    char *password_start = strstr(decoded_payload, "password=");
    if (password_start) {
        password_start += strlen("password=");
        char *password_end = strchr(password_start, '&'); // Find next parameter
        size_t password_len = password_end ? (size_t)(password_end - password_start) : strlen(password_start);

        if (password_len < sizeof(pw)) {
            strncpy(pw, password_start, password_len);
            pw[password_len] = '\0'; // Ensure string is null-terminated
        }
    }

    // Debugging logs for extracted credentials
    DEBUG_printf("Extracted SSID: %s\n", id);
    DEBUG_printf("Extracted PASSWORD: %s\n", pw);

    // Store extracted values in global variables
    strncpy(ssid, id, sizeof(ssid) - 1);
    strncpy(password, pw, sizeof(password) - 1);

    // Return success only if both SSID and password were extracted successfully
    return (strlen(id) > 0 && strlen(pw) > 0) ? 0 : -1;
}


// --------------------------- Close Client Connection Function ---------------------------

/**
 * @brief Closes the TCP client connection and frees associated resources.
 *
 * @param con_state Pointer to the TCP connection state structure.
 * @param client_pcb Pointer to the client's TCP protocol control block.
 * @param close_err Error code to return if the close operation fails.
 * @return err_t Returns ERR_OK on success, or an appropriate error code on failure.
 *
 * This function closes the TCP client connection, frees associated resources,
 * and handles any errors that occur during the close operation.
 *
 * ### Behavior:
 * - Asserts that the connection state and client PCB are valid.
 * - Clears the TCP callbacks for the client PCB.
 * - Attempts to close the TCP connection.
 * - If the close operation fails, it aborts the connection.
 * - Frees the connection state structure if it exists.
 */
static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        // Ensure the connection state and client PCB are valid
        assert(con_state && con_state->pcb == client_pcb);

        // Clear the TCP callbacks for the client PCB
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);

        // Attempt to close the TCP connection
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            // If closing fails, abort the connection
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }

        // Free the connection state structure if it exists
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}


// --------------------------- Close TCP Server Function ---------------------------

/**
 * @brief Closes the TCP server connection and frees associated resources.
 *
 * @param state Pointer to the TCP server state structure.
 *
 * This function closes the TCP server connection and frees associated resources.
 *
 * ### Behavior:
 * - Checks if the server PCB is valid.
 * - Clears the TCP argument for the server PCB.
 * - Closes the TCP connection for the server PCB.
 * - Sets the server PCB pointer to NULL.
 *
 * @note The server PCB is set to NULL after closing the connection.
 */
static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        // Clear the TCP argument for the server PCB
        tcp_arg(state->server_pcb, NULL);

        // Close the TCP connection for the server PCB
        tcp_close(state->server_pcb);

        // Set the server PCB pointer to NULL
        state->server_pcb = NULL;
    }
}


// --------------------------- TCP Server Sent Callback Function ---------------------------

/**
 * @brief Callback function for when data is successfully sent by the TCP server.
 *
 * @param arg Pointer to the argument passed to the callback (connection state).
 * @param pcb Pointer to the TCP protocol control block.
 * @param len Length of data that was sent.
 * @return err_t Returns ERR_OK on success, or an appropriate error code on failure.
 *
 * This function is called when data is successfully sent by the TCP server.
 * It updates the sent length in the connection state and checks if all data has been sent.
 * If all data has been sent, it closes the client connection.
 *
 * ### Behavior:
 * - Updates the sent length in the connection state.
 * - Checks if the total sent length is greater than or equal to the sum of header and result lengths.
 * - If all data has been sent, it closes the client connection.
 *
 * @note The connection state and PCB are used to manage the TCP connection.
 */
static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_sent %u\n", len);
    
    // Update the sent length in the connection state
    con_state->sent_len += len;
    
    // Check if all data has been sent
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("all done\n");
        // Close the client connection
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}


// --------------------------- Test Server Content Function ---------------------------

/**
 * @brief Generates the server content based on the request and parameters.
 *
 * @param request The HTTP request string.
 * @param params The parameters string from the HTTP request.
 * @param result The buffer to store the generated result.
 * @param max_result_len The maximum length of the result buffer.
 * @return int The length of the generated result.
 *
 * This function generates the server content based on the provided request and parameters.
 * It checks the state of the LED and updates it if necessary, then generates the appropriate
 * HTML content to be sent back to the client.
 *
 * ### Behavior:
 * - Checks if the request matches the CONFIG path.
 * - Retrieves the current state of the LED.
 * - If parameters are provided, updates the LED state based on the parameters.
 * - Generates the HTML content based on the LED state.
 *
 * @note The generated result is stored in the `result` parameter.
 */
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


// --------------------------- TCP Server Receive Callback Function ---------------------------

/**
 * @brief Callback function for receiving data from the TCP client.
 *
 * @param arg Pointer to the argument passed to the callback (connection state).
 * @param pcb Pointer to the TCP protocol control block.
 * @param p Pointer to the packet buffer containing the received data.
 * @param err Error code indicating the status of the receive operation.
 * @return err_t Returns ERR_OK on success, or an appropriate error code on failure.
 *
 * This function is called when data is received from the TCP client. It processes
 * the received data, handles HTTP GET and POST requests, and generates the appropriate
 * response to be sent back to the client.
 *
 * ### Behavior:
 * - Checks if the connection is closed.
 * - Asserts the validity of the connection state and PCB.
 * - Copies the received request into the buffer.
 * - Handles HTTP GET and POST requests.
 * - Generates the appropriate response content.
 * - Sends the response headers and body to the client.
 * - Frees the packet buffer and returns the appropriate error code.
 *
 * @note The connection state and PCB are used to manage the TCP connection.
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
                *space = 0; // Terminate the string at the end of the path
            }

            // Check if the marker was found
            char *payload = strstr(con_state->headers, "\r\n\r\n");
            if (p->payload) {
                p->payload += 4; // // Advance 4 characters of "\r\n\r\n" to reach the body

                // Ensure the payload is a null-terminated string
                ((char *)p->payload)[p->len - 4] = '\0';

                // Process the received data in the payload
                int process_result = process_post_payload(request, p->payload);
                if (process_result >= 0) {
                    // Success response with page
                    id_pw_collected = 1;
                    snprintf(con_state->result, sizeof(con_state->result), SUCCESS_PAGE);
                } else {
                    // Failure response with page
                    snprintf(con_state->result, sizeof(con_state->result), FAILURE_PAGE);
                }
            } else {
                // Failure response with page
                snprintf(con_state->result, sizeof(con_state->result), FAILURE_PAGE);
            }

            // Update headers to serve the HTML
            con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers),
                HTTP_RESPONSE_HEADERS, 200, strlen(con_state->result));

            // Send the headers to the client
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("Failed to send POST headers: %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            // Send the response body to the client
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


// --------------------------- Server Poll Function ---------------------------

/**
 * @brief Polls the TCP server for activity and handles client disconnection.
 *
 * @param arg Pointer to the argument passed to the poll function, typically the connection state.
 * @param pcb Pointer to the TCP protocol control block for the client.
 * @return err_t Returns ERR_OK on success, or an appropriate error code on failure.
 *
 * This function is called periodically to poll the TCP server for activity.
 * If a client is connected, it handles the disconnection of the client.
 *
 * ### Behavior:
 * - Casts the argument to the connection state structure.
 * - Logs the poll function call for debugging purposes.
 * - Calls the function to close the client connection.
 *
 * @note This function is typically registered as a callback for the TCP server poll event.
 */
static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_poll_fn\n");
    return tcp_close_client_connection(con_state, pcb, ERR_OK); // Just disconnect clent?
}


// --------------------------- Server Error Function ---------------------------

/**
 * @brief Handles TCP server errors and closes the client connection.
 *
 * @param arg Pointer to the argument passed to the error function, typically the connection state.
 * @param err The error code indicating the type of error.
 *
 * This function is called when an error occurs on the TCP server.
 * It handles the error by closing the client connection if the error is not ERR_ABRT.
 *
 * ### Behavior:
 * - Casts the argument to the connection state structure.
 * - Logs the error for debugging purposes.
 * - Calls the function to close the client connection if the error is not ERR_ABRT.
 *
 * @note This function is typically registered as a callback for the TCP server error event.
 */
static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        // Log the error for debugging purposes
        DEBUG_printf("tcp_client_err_fn %d\n", err);

        // Close the client connection
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}


// --------------------------- Server Accept Function ---------------------------

/**
 * @brief Accepts a new TCP client connection and sets up the connection state.
 *
 * @param arg Pointer to the argument passed to the accept function, typically the server state.
 * @param client_pcb Pointer to the client's TCP protocol control block.
 * @param err The error code indicating the status of the accept operation.
 * @return err_t Returns ERR_OK on success, or an appropriate error code on failure.
 *
 * This function is called when a new TCP client connection is accepted.
 * It sets up the connection state and registers the necessary callbacks for the client.
 *
 * ### Behavior:
 * - Checks for errors in the accept operation.
 * - Logs the connection status for debugging purposes.
 * - Allocates memory for the connection state structure.
 * - Sets up the connection state and registers the necessary callbacks.
 *
 * @note This function is typically registered as a callback for the TCP server accept event.
 */
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


// --------------------------- Server Open Function ---------------------------

/**
 * @brief Opens a TCP server and starts listening for client connections.
 *
 * @param arg Pointer to the argument passed to the open function, typically the server state.
 * @return bool Returns true on success, false on failure.
 *
 * This function initializes and opens a TCP server, binds it to a port, and starts listening for client connections.
 *
 * ### Behavior:
 * - Creates a new TCP protocol control block (PCB).
 * - Binds the PCB to the specified port.
 * - Starts listening for client connections with a backlog of 1.
 * - Registers the necessary callbacks for accepting client connections.
 *
 * @note This function is typically called to start the TCP server.
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