#ifndef MENU_H
#define MENU_H

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "ssd1306/ssd1306_fonts.h"
#include "ssd1306/ssd1306.h"
#include "wifi.h"
#include "lwip/apps/mqtt.h"
#include "icons.h"
#include "mqtt_utility.h"


// ---------------------------- Button Definitions ----------------------------

/**
 * @brief GPIO pin definitions for the buttons.
 */
#define BUTTON_A 5  ///< Button for changing options/settings.
#define BUTTON_B 6   ///< Button for confirming/entering a selection.

//-----------------------------Buzzer Definitions-------------------------------

#define BUZZER_PIN 21    // Pino do buzzer
#define MIN_FREQUENCY 10 // Frequência mínima do buzzer (Hz)
#define MAX_FREQUENCY 2000 // Frequência máxima do buzzer (Hz)
#define ADC_UPPER_THRESHOLD 3500 // Limite superior do ADC para incrementar a frequência
#define ADC_LOWER_THRESHOLD 850  // Limite inferior do ADC para decrementar a frequência
#define STEP 20              // Incremento ou decremento por iteração
float frequency = MIN_FREQUENCY;
int Limit_Buzzer = 0;
uint8_t x_distance;

//-------------------------------MQTT Variables---------------------------------

ip_addr_t addr;
mqtt_client_t *cliente_mqtt;
int connected_mqtt;

// --------------------------- Low Pass Filter Function ---------------------------

/**
 * @brief Applies a low-pass filter to smooth input data.
 *
 * @param new_value The new input value to be filtered.
 * @return uint16_t The filtered output value.
 *
 * This function applies a smoothing algorithm using an exponential
 * moving average. The smoothing factor `alpha` determines the influence
 * of the new value relative to the previous filtered value.
 */
uint low_pass_filter(uint new_value) {
    float alpha = 0.5;            ///< Fator de suavização (0.0 a 1.0)
    static uint filtered_value = 0; ///< Valor suavizado (preservado entre as chamadas)

    filtered_value = (alpha * new_value) + ((1 - alpha) * filtered_value);
    return filtered_value;
}

// ------------------------------- Buzzer Functions -------------------------------

// Inicializa o PWM para o buzzer
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f); // Divisor inicial
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0); // Inicia com nível baixo
}

// Define a frequência do buzzer
void set_buzzer_frequency(uint pin, float frequency) {
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Calcula o valor de "top" e ajusta o divisor
    uint32_t source_hz = clock_get_hz(clk_sys);
    float divisor = (float)source_hz / (frequency * 4096.0f);
    uint32_t top = source_hz / (frequency * divisor);

    pwm_set_clkdiv(slice_num, divisor); // Configura o divisor
    pwm_set_wrap(slice_num, top - 1);  // Configura o contador superior
    pwm_set_gpio_level(pin, top / 2);  // Duty cycle de 50%
}

void menu_enter_sound(uint pin) {
    
	uint frequencies[3];

    if (frequency <= 210) {
        frequencies[0] = frequency + 300;
        frequencies[1] = 10;
        frequencies[2] = 110; // Tons crescentes
    } else if (frequency >= 1710) {
        frequencies[0] = 2010;
        frequencies[1] = frequency - 200;
        frequencies[2] = frequency - 100; // Tons decrescentes
    } else {
        frequencies[0] = frequency + 300;
        frequencies[1] = frequency - 200;
        frequencies[2] = frequency - 100; // Combinação de tons
    }

    uint duration_ms = 75; // Duração de cada tom

    for (int i = 0; i < 3; i++) {
        set_buzzer_frequency(pin, frequencies[i]); 	// Configura o tom
        pwm_set_gpio_level(pin, 2048);            	// Ativa o buzzer
        sleep_ms(duration_ms);                   	// Duração do tom
    }
    pwm_set_gpio_level(pin, 0); // Desativa o buzzer
}

void menu_exit_sound(uint pin) {

    // Frequências para o som de saída  
	uint frequencies[3];

    if (frequency <= 310) {
        frequencies[0] = 10;
        frequencies[1] = frequency + 200;
        frequencies[2] = frequency + 100; // Tons crescentes
    } else if (frequency >= 1810) {
        frequencies[0] = frequency - 300;
        frequencies[1] = 2010;
        frequencies[2] = 1910; // Tons decrescentes
    } else {
        frequencies[0] = frequency - 300;
        frequencies[1] = frequency + 200;
        frequencies[2] = frequency + 100; // Combinação de tons
    }

	uint duration_ms = 75; // Duração de cada tom

    for (int i = 0; i < 3; i++) {
        set_buzzer_frequency(pin, frequencies[i]); // Configura o tom
        pwm_set_gpio_level(pin, 2048);            // Ativa o buzzer
        sleep_ms(duration_ms);                   // Duração do tom
    }
    pwm_set_gpio_level(pin, 0); // Desativa o buzzer
}

void scape_function(void){
	ssd1306_UpdateScreen();
	sleep_ms(2000);
	current_screen = !current_screen;
}

// ---------------------------- Home Screen Rendering Function ----------------------------

/**
 * @brief Renders the menu on an SSD1306 display.
 *
 * This function is responsible for drawing a home screen interface on an SSD1306 display
 * using the I2C protocol. It displays icons and corresponding option names for the
 * current, previous, and next menu items. It also includes a selection outline and
 * a scrollbar for visual navigation feedback.
 *
 * Functionality:
 * - Clears the display by setting all pixels to white.
 * - Writes the names and icons of the previous, current, and next home screen items.
 * - Draws a rectangular outline to highlight the selected menu item.
 * - Renders a scrollbar on the right side of the display, indicating the current
 *   position in the menu.
 *
 * Dependencies:
 * - The function relies on the SSD1306 driver functions:
 *   - `ssd1306_Fill(0)`: Clears the display by setting all pixels to a specific color.
 *   - `ssd1306_SetCursor`: Positions the cursor at a specified (x, y) location on the display.
 *   - `ssd1306_WriteString`: Writes a string at a specified position (x, y).
 *   - `ssd1306_DrawBitmap`: Renders a bitmap image (icon) at a specified position (x, y).
 *   - `ssd1306_DrawRectangle`: Draws a rectangle at the specified coordinates.
 *
 * @note Assumes that `menu_items` and `bitmap_icons` arrays are defined and populated
 *       with the corresponding strings and bitmap data for the home screen options.
 */
void home_screen(void) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(25, 5);
	ssd1306_WriteString(menu_items[item_sel_previous], Font_7x10, White);
	ssd1306_DrawBitmap(4, 2, bitmap_icons[item_sel_previous], 16, 16, White);
	ssd1306_SetCursor(25, 5 + 20 + 2);
	ssd1306_WriteString(menu_items[item_selected], Font_7x10, White);
	ssd1306_DrawBitmap(4, 24, bitmap_icons[item_selected], 16, 16, White);
	ssd1306_SetCursor(25, 5 + 20 + 20 + 2 + 2);
	ssd1306_WriteString(menu_items[item_sel_next], Font_7x10, White);
	ssd1306_DrawBitmap(4, 46, bitmap_icons[item_sel_next], 16, 16, White);

	ssd1306_DrawBitmap(0, 22, bitmap_item_sel_outline, 128, 21, White);
	ssd1306_DrawBitmap(128 - 8, 0, bitmap_scrollbar_background, 8, 64, White);
	ssd1306_DrawRectangle(125, 64 / NUM_ITEMS * item_selected, 128,
			(64 / NUM_ITEMS * item_selected + (64 / NUM_ITEMS)), White);
	ssd1306_DrawRectangle(126, 64 / NUM_ITEMS * item_selected, 127,
			(64 / NUM_ITEMS * item_selected + (64 / NUM_ITEMS)), White);
}

void update_cursor(void){

	adc_select_input(0);
	uint adc_y_raw = adc_read();
	uint filtered_read = low_pass_filter(adc_y_raw);

	// Verificar estado do analógico-UP
	if ((filtered_read > 3000) && up_clicked == 0) {
		up_clicked = 1; // Marcar como pressionado
		cursor--;
		if (cursor == -1)
			cursor = 3;
		item_selected--;
		if (item_selected < 0)
			item_selected = NUM_ITEMS - 1;
	}
	// Liberar o botão UP
	if (filtered_read <= 3000) {
		up_clicked = 0;
	}

	// Verificar estado do analógico-DOWN
	if ((filtered_read < 1100) && down_clicked == 0) {
		down_clicked = 1; // Marcar como pressionado
		cursor++;
		if (cursor == 4)
			cursor = 0;
		item_selected++;
		if (item_selected >= NUM_ITEMS)
			item_selected = 0;
	}
	// Liberar o botão DOWN
	if (filtered_read >= 1100) {
		down_clicked = 0;
	}
}

void menu(void) {

	if (current_screen == 0) {

        update_cursor();    // Atualizar cursor com o joystick
        home_screen();      // Atualizar tela inicial no diplay OLED

	}

	if(current_screen) {

		// Fills the display with a specific color
		ssd1306_Fill(Black);

		// WIFI OPTION
		if (item_selected == 0){
			// External function for utilizing and sampling ultrasonic sensor functionality

			// Display header
			ssd1306_SetCursor(45, 1);
			ssd1306_WriteString("WI-FI", Font_7x10, White);
			ssd1306_FillRectangle(1, 15, 128, 16, White);
			ssd1306_DrawRectangle(1, 20, 127, 63, White);

			if(!start_wifi){

				// Writing de name of the WI-FI SSID
				ssd1306_SetCursor(7, 25);
				ssd1306_WriteString("Connecting in: ", Font_6x8, White);
				ssd1306_SetCursor(7, 39);
				ssd1306_WriteString(WIFI_SSID, Font_6x8, 1);

				ssd1306_UpdateScreen();
				if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)){
				ssd1306_SetCursor(18, 50);
				ssd1306_WriteString("NOT CONNECTED", Font_7x10, 1);
				scape_function();
					return;
				}

				uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
				//printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);

				start_wifi = 1;
				ssd1306_SetCursor(30, 50);
				ssd1306_WriteString("CONNECTED", Font_7x10, 1);
				ssd1306_UpdateScreen();
				sleep_ms(2000);

				// Start the HTTP server
				start_http_server();

			} else {

				char buffer_string[7];  // Buffer to hold formatted string values
				uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
				sprintf(buffer_string, "IP %d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
				ssd1306_SetCursor(18, 25);
				ssd1306_WriteString(buffer_string, Font_6x8, White);

				ssd1306_SetCursor(23, 45);
				ssd1306_WriteString("HTTP REQUEST: ", Font_6x8, White);
				ssd1306_SetCursor(45, 55);
				if (current_request == "none")
					ssd1306_SetCursor(45, 55);
				else
					ssd1306_SetCursor(36, 55);
				ssd1306_WriteString(current_request, Font_6x8, White);


				if (!gpio_get(BUTTON_A_PIN))
					button_state = "1";
					//button_state = "Button is pressioned"; 
				else
					button_state = "0";
					//button_state = "Button is not pressioned"; 
			}

			cyw43_arch_poll();  // Required to keep Wi-Fi active
		} 

		// MQTT Connection OPTION
		else if (item_selected == 1){
			// External function for using and sampling gyroscope functionality
			ssd1306_SetCursor(10, 1);
			ssd1306_WriteString("MQTT Connection: ", Font_7x10, 1);
			ssd1306_FillRectangle(1, 15, 128, 16, 1);	// Draw header rectangle
			ssd1306_DrawRectangle(1, 20, 127, 63, 1);	// Draw main display rectangle

			if(!start_wifi){

				ssd1306_SetCursor(15, 38);
				ssd1306_WriteString("CONNECT TO WIFI!!", Font_6x8, 1);
				scape_function();

			} else {

				if(!connected_mqtt){

				ssd1306_SetCursor(10, 20);
				ssd1306_WriteString("CONNECTING TO BROKER!!", Font_6x8, 1);
				ssd1306_UpdateScreen();
				
				if (!ip4addr_aton(MQTT_SERVER, &addr)) {
					ssd1306_SetCursor(20, 38);
					ssd1306_WriteString("IP ERROR!!", Font_6x8, 1);
					scape_function();
				}

				cliente_mqtt = mqtt_client_new();
				mqtt_set_inpub_callback(cliente_mqtt, &mqtt_incoming_publish_cb, &mqtt_incoming_data_cb, NULL);
				err_t erro = mqtt_client_connect(cliente_mqtt, &addr, 1883, &mqtt_connection_cb, NULL, &mqtt_client_info);

				connected_mqtt = 1;

				if (erro != ERR_OK) {
					ssd1306_SetCursor(15, 38);
					ssd1306_WriteString("CONNECTION ERROR!!", Font_6x8, 1);
					scape_function();
				}

				ssd1306_SetCursor(30, 50);
				ssd1306_WriteString("CONNECTED", Font_7x10, 1);
				ssd1306_UpdateScreen();
				sleep_ms(2000);
				//connected_mqtt = 1;

				}
				
				cont_envio++;
				sleep_ms(1);

				ssd1306_SetCursor(5, 20);
                ssd1306_WriteString("PUB topic testy/temp = ", Font_6x8, White);
				if(cont_envio >= 1000){
					cont_envio = 0;
                    float temperature = read_onboard_temperature(TEMPERATURE_UNITS);


                    //float to string
                    ftoa(temperature, tempString, 2);

                    ssd1306_WriteString(tempString, Font_6x8, White);
                    mqtt_publish(cliente_mqtt, PUBLISH_STR_NAME, tempString, 5, 0, false, &mqtt_request_cb, NULL);
				}
			}
		}
		
		// BUZZER OPTION
		else if (item_selected == 2){
			// External function for using and sampling the kalman filter functionality

			char buffer_string[7];	// Buffer to hold formatted string values
			ssd1306_SetCursor(25, 1);
			ssd1306_WriteString("BUZZER PWM: ", Font_7x10, 1);
			ssd1306_FillRectangle(1, 15, 128, 16, 1);	// Draw header rectangle
			ssd1306_DrawRectangle(1, 20, 127, 63, 1);	// Draw main display rectangle

			// Lê o valor ADC e filtra
			adc_select_input(1);
			uint adc_x_raw = adc_read();
			uint filtered_read = low_pass_filter(adc_x_raw);

			// Ajuste da frequência
			if (adc_x_raw > ADC_UPPER_THRESHOLD && frequency < MAX_FREQUENCY) {
				frequency += STEP;
			} else if (adc_x_raw < ADC_LOWER_THRESHOLD && frequency > MIN_FREQUENCY) {
				frequency -= STEP;
			}

			// Configura o buzzer com a nova frequência
			set_buzzer_frequency(BUZZER_PIN, frequency);

			// Atualiza a barra no display
			uint8_t x_distance = (uint8_t)(((frequency - MIN_FREQUENCY) * 128) / (MAX_FREQUENCY - MIN_FREQUENCY)) + 1;
			ssd1306_DrawRectangle(1, 48, 128, 63, 1);
			ssd1306_FillRectangle(1, 48, x_distance, 63, 1); // Barra horizontal

			// Mostra a frequência atual
			ssd1306_SetCursor(25, 30);
			sprintf(buffer_string, "FREQ: %u Hz", (uint)frequency);
			ssd1306_WriteString(buffer_string, Font_7x10, 1);

		}
		
		else if (item_selected == 3){
			// External function for using and sampling the inertial sensor calibration functionality
			ssd1306_SetCursor(5, 30);
			ssd1306_WriteString("CALIB", Font_11x18, White);
		}
	}

	if (!(gpio_get(BUTTON_B)) && button_enter_clicked == 0) {

		button_enter_clicked = 1;

		// Turning off the buzzer
		pwm_set_gpio_level(BUZZER_PIN, 0);

		if(item_selected != 2){
		
		if(current_screen)
			menu_enter_sound(BUZZER_PIN);
		else
			menu_exit_sound(BUZZER_PIN);
		}

		// Switching to the other screen type
		current_screen = !current_screen;
		
	}

	// If the ENTER button was release, the auxiliar variable returns to low
	if ((gpio_get(BUTTON_B)) && button_enter_clicked == 1) {
		button_enter_clicked = 0;
	}

/*---------------------- Lógica para imprimir os itens corretos -------------------------*/

	item_sel_previous = item_selected - 1;
	if (item_sel_previous < 0) {
		item_sel_previous = NUM_ITEMS - 1;
	} // previous item would be below first = make it the last
	item_sel_next = item_selected + 1;
	if (item_sel_next >= NUM_ITEMS) {
		item_sel_next = 0;
	} // next item would be after last = make it the first

/*---------------------------------------------------------------------------------------*/

    ssd1306_UpdateScreen();
}

#endif /*MENU_H*/
