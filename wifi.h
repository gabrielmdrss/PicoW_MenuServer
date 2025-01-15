#ifndef WIFI_H
#define WIFI_H

#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/apps/httpd.h"

#define LED_PIN 12          // Sets the LED pin
#define BUTTON_A_PIN 5
#define WIFI_SSID "XXX"  // Replace with the name of your Wi-Fi network
#define WIFI_PASS "XXX" // Replace with your Wi-Fi network password

char http_response[2048];  // Buffering for HTTP responsev
const char *button_state = "Button is not pressioned";
const char *last_state = "Button is not pressioned";
int start_wifi = 0;
int connected = 0;
char * current_request = "none";

// Buffering for HTTP responses
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


// Callback function to process HTTP requests
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
        gpio_put(LED_PIN, 1);  // Turn on the LED
    } else if (strstr(request, " /?led=off")) {
        current_request = "/?led=off";
        gpio_put(LED_PIN, 0);  // Turn off the LED
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

static err_t update_server(void *arg, struct tcp_pcb *tpcb){
    
    if (!gpio_get(BUTTON_A_PIN))
        button_state = "1";
        //button_state = "Button is pressioned"; 
    else
        button_state = "0";
        //button_state = "Button is not pressioned"; 

    // Replace %s in HTTP_RESPONSE
    snprintf(http_response, sizeof(http_response), HTTP_RESPONSE, button_state);

    // Send the answer
    err_t write_err = tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);
    if (write_err != ERR_OK) {
        printf("Error sending HTTP response: %d\n", write_err);
        return write_err;
    }

    tcp_output(tpcb);  // Make sure data is sent

    // Close the connection
    tcp_close(tpcb);

    return ERR_OK;
}

// Connection callback: associates the http_callback with the connection
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associates the HTTP callback
    //tcp_poll(newpcb, update_server, 100);
    return ERR_OK;
}

// TCP Server Setup Function
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