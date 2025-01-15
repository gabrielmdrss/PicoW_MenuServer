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
#include "ssd1306/ssd1306_fonts.h"
#include "ssd1306/ssd1306.h"
#include "icons.h"
#include "wifi.h"


// ---------------------------- Button Definitions ----------------------------

/**
 * @brief GPIO pin definitions for the buttons.
 */
#define BUTTON_A 5  ///< Button for changing options/settings.
#define BUTTON_B 6   ///< Button for confirming/entering a selection.


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
				ssd1306_SetCursor(23, 25);
				ssd1306_WriteString("Connecting in: ", Font_6x8, White);
				ssd1306_SetCursor(7, 39);
				ssd1306_WriteString(WIFI_SSID, Font_6x8, 1);

				ssd1306_UpdateScreen();
				if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)){
					printf("Failed to connect to Wi-Fi\n");
					return;
				}

				uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
				printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);

				start_wifi = 1;
				ssd1306_SetCursor(30, 50);
				ssd1306_WriteString("CONNECTED", Font_7x10, 1);
				ssd1306_UpdateScreen();
				sleep_ms(2000);

				// Start the HTTP server
				start_http_server();

			} else {

				char buffer_string[7];
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
	
		} else if (item_selected == 1){
			// External function for using and sampling gyroscope functionality
			ssd1306_SetCursor(5, 30);
			ssd1306_WriteString("MATRIX", Font_11x18, White);

		}else if (item_selected == 2){
			// External function for using and sampling the kalman filter functionality
			ssd1306_SetCursor(5, 30);
			ssd1306_WriteString("BUZZER", Font_11x18, White);

		}else if (item_selected == 3){
			// External function for using and sampling the inertial sensor calibration functionality
			ssd1306_SetCursor(5, 30);
			ssd1306_WriteString("CALIB", Font_11x18, White);
		}
	}

	if (!(gpio_get(BUTTON_B)) && button_enter_clicked == 0) {

		button_enter_clicked = 1;

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
