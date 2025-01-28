#ifndef WIFI_H
#define WIFI_H

/******************************************************************************
 * @file    wifi.h
 * @brief   File containing definitions and function prototypes for the
 *          Web Server application hosted on the Raspberry Pi Pico W.
 *
 * @authors Gabriel Domingos de Medeiros
 * @date    February 2025
 * @version 1.0.0
 *
 * @note    This file includes the necessary definitions, constants, and function
 *          prototypes for the application.
 ******************************************************************************/

#include "pico/cyw43_arch.h"      // Library for using the connectivity module for raspberry pi pico w.
#include "lwip/tcp.h"             // TCP Function Library
#include "lwip/apps/httpd.h"      // Library of functions for HTTP protocol
#include "mqtt_utility.h"         // File containing useful functions for MQTT communication

#define BUTTON_A_PIN 5            // sets the pin number used for detecting the state of Button A.

char http_response[2048];         // The content of the HTTP response that will be sent to the client.

const char *button_state = "Button is not pressioned";
const char *last_state = "Button is not pressioned";
int start_wifi = 0;              // Integer that acts as a flag to indicate if Wi-Fi initialization started.
char * current_request = "none"; // This string holds the value of the current HTTP request being processed.

/**
 * @brief HTTP response template for the server.
 *
 * This define holds a preformatted HTTP response string, including headers and HTML content.
 * It is dynamically populated with the current button state (`%s`) before being sent to the client.
 *
 * @details:
 * - HTML/CSS styling for a simple web interface.
 * - Includes buttons for LED ON and OFF functionality.
 * - Displays the current button state dynamically in the response.
 * - Auto-refresh feature for real-time updates.
 *
 * @example:
 * When the button state is "Button is pressioned", the response dynamically updates
 * to reflect the status in the `<p>` tag of the HTML content.
 */
#define HTTP_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nRefresh: 1\r\n\r\n" \
                      "<!DOCTYPE html><html>" \
                      "<head>" \
                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">" \
                      "<meta http-equiv=\"refresh\" content=\"1\">" \
                      "<link rel=\"icon\" href=\"data:,\">" \
                      "<style>" \
                      "html { font-family: Arial, sans-serif; display: inline-block; margin: 0 auto; text-align: center; background-color: #f0f0f5; }" \
                      "body { margin: 0; padding: 0; }" \
                      "h1 { color: #333; margin-top: 20px; }" \
                      "p { font-size: 18px; color: #555; margin: 20px auto; }" \
                      ".container { width: 90%; max-width: 600px; margin: 20px auto; padding: 20px; border: 1px solid #ddd; border-radius: 10px; background: #fff; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1); }" \
                      ".buttonGreen { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; text-align: center; font-size: 16px; margin: 10px; border-radius: 5px; cursor: pointer; transition: background-color 0.3s ease; }" \
                      ".buttonGreen:hover { background-color: #45a049; }" \
                      ".buttonRed { background-color: #D11D53; border: none; color: white; padding: 15px 32px; text-align: center; font-size: 16px; margin: 10px; border-radius: 5px; cursor: pointer; transition: background-color 0.3s ease; }" \
                      ".buttonRed:hover { background-color: #c21845; }" \
                      "</style>" \
                      "</head>" \
                      "<body>" \
                      "<div class=\"container\">" \
                      "<h1>Control LED</h1>" \
                      "<form>" \
                      "<button class=\"buttonGreen\" name=\"led\" value=\"on\" type=\"submit\">LED ON</button>" \
                      "<button class=\"buttonRed\" name=\"led\" value=\"off\" type=\"submit\">LED OFF</button>" \
                      "</form>" \
                      "<p>Button State: %s</p>" \
                      "</div>" \
                      "</body></html>\r\n"


/* Functions */

// --------------------------- Http Callback Function ---------------------------

/**
 * @brief Processes incoming HTTP requests and handles LED control commands.
 *
 * @param arg A pointer to user-defined data passed to the callback (unused).
 * @param tpcb A pointer to the TCP Protocol Control Block (PCB) for the connection.
 * @param p A pointer to the buffer containing the HTTP request payload.
 * @param err The error status of the incoming data.
 * @return err_t Returns `ERR_OK` on success or an appropriate error code.
 *
 * This function processes HTTP requests received by the server. It checks for specific
 * commands in the request to control an LED (e.g., turning it ON or OFF). If a recognized
 * command is found, the function updates the LED's state accordingly. The response is then
 * generated dynamically based on the current button state and sent back to the client. 
 * Finally, the received buffer is freed, and the TCP connection is maintained.
 *
 * ### Behavior:
 * - If the client closes the connection (`p == NULL`), the server closes the TCP connection.
 * - Recognizes the following HTTP commands:
 *   - `"GET /?led=on"`: Turns the LED on with maximum brightness.
 *   - `"GET /?led=off"`: Turns the LED off.
 * - Replaces the `%s` placeholder in the HTTP response template with the current button state.
 * - Sends the generated HTTP response back to the client.
 *
 * @note This function is invoked automatically when new data is received by the server
 *   (callback registered in `connection_callback`).
 * @note Ensures proper memory management by freeing the received buffer after processing.
 */
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    
    // Checks if the connection was closed by the client
    if (p == NULL) {
        // Client closed the connection
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Process the HTTP request
    char *request = (char *)p->payload;

    // Checks if the request contains the command to turn on the LED.
    if (strstr(request, "GET /?led=on")) {
        current_request = "/?led=on";
        pwm_set_gpio_level(LED_BLUE_PIN, 255);  // Acende o LED azul com brilho máximo
    } else if (strstr(request, " /?led=off")) {
        current_request = "/?led=off";
        pwm_set_gpio_level(LED_BLUE_PIN, 0);  // Acende o LED azul com brilho máximo
    } else
        current_request = "none";

    // Replace %s in HTTP_RESPONSE
    snprintf(http_response, sizeof(http_response), HTTP_RESPONSE, button_state);

    // Send the response (example)
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);

    // Free the received buffer
    pbuf_free(p);

    return ERR_OK;
}


// --------------------------- Connection Callback Function ---------------------------

/**
 * @brief Handles new incoming TCP connections and sets up the HTTP callback.
 *
 * @param arg A pointer to user-defined data passed to the callback (unused).
 * @param newpcb A pointer to the new TCP Protocol Control Block (PCB) representing the connection.
 * @param err The error status of the incoming connection.
 * @return err_t Returns `ERR_OK` on success or an appropriate error code.
 *
 * This function is triggered whenever a new TCP connection is established with the server.
 * It associates the `http_callback` function to handle incoming data for the connection.
 * Optionally, it can register a polling function (e.g., `update_server`) for periodic server updates.
 *
 * ### Behavior:
 * - Registers the `http_callback` function to handle incoming HTTP requests on the connection.
 * - Optionally supports polling by enabling the `tcp_poll` function (commented in the code).
 * - Does not manage connection closing or cleanup directly; this is handled by other mechanisms.
 *
 * ### Notes:
 * @note The callback function `http_callback` is invoked when new data is received for the connection.
 * @note This function is designed to work in conjunction with the lwIP TCP/IP stack.
 */
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associates the HTTP callback
    //tcp_poll(newpcb, update_server, 100);
    return ERR_OK;
}


// --------------------------- Shutdown TCP Server Function ---------------------------

/**
 * @brief Shuts down the TCP server and releases resources.
 *
 * @param server_state A pointer to the `TCP_SERVER_T` structure representing the server state.
 *
 * This function gracefully shuts down the TCP server by closing all active connections,
 * releasing the occupied port, and freeing any dynamically allocated memory associated
 * with the server state.
 *
 * ### Behavior:
 * - Closes all active connections by invoking `tcp_server_close`.
 * - Frees the memory allocated for the `TCP_SERVER_T` structure.
 * - Prints a debug message indicating the server has been shut down and the port is released.
 *
 * @note Ensure that no ongoing operations depend on the server state before invoking this function.
 * @note The server state should be properly initialized before calling this function.
 */
void shutdown_tcp_server(TCP_SERVER_T *server_state) {
    if (server_state) {
        tcp_server_close(server_state);
        free(server_state);
    }
    DEBUG_printf("Servidor TCP encerrado e porta 80 liberada.\n");
}


// --------------------------- TCP Server Setup Function ---------------------------

/**
 * @brief Initializes and starts the HTTP server on port 80.
 *
 * This function sets up the TCP server by creating a Protocol Control Block (PCB),
 * binding it to port 80, and putting it in listening mode. It also associates the
 * `connection_callback` function to handle incoming client connections.
 *
 * ### Behavior:
 * - Creates a new TCP PCB using `tcp_new`.
 * - Binds the server to port 80 on all available network interfaces.
 * - Puts the PCB in listening mode to accept incoming connections.
 * - Associates the `connection_callback` function to manage new client connections.
 * - Prints status messages indicating success or errors during setup.
 *
 *
 * @note If PCB creation or port binding fails, the function prints an error message
 *   and exits without completing the server setup.
 * @note This function is designed for use with the lwIP TCP/IP stack.
 */
static void start_http_server(void) {

    // Creating a new TCP Protocol Control Structure (PCB)
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Error creating PCB\n");
        return;
    }

    // Connect the server to port 80
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Error connecting to server on port 80\n");
        return;
    }

    pcb = tcp_listen(pcb);  // Puts the PCB in listening mode
    tcp_accept(pcb, connection_callback);  // Associates the connection callback

    printf("HTTP server running on port 80...\n");
}

#endif /* WIFI_H */
