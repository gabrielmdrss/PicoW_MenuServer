#ifndef ICONS_H
#define ICONS_H

/******************************************************************************
 * @file    PNG.h
 * @brief   Header file containing definitions and arrays for the
 *          menu visualization.
 *
 * @authors  Gabriel Domingos de Medeiros, Jezreal Pereira Filgueiras
 * @date    December 2024
 * @version 1.0.0
 *
 * @note    This file includes the necessary definitions and constants
 *          for the diplay ssd1306 visualization.
 *
 * @copyright (c) 2024 VIRTUS--CC. All rights reserved.
 ******************************************************************************/


// --------------------- Menu and Item Selection Variables ---------------------

/**
 * @brief Defines for menu item configuration.
 *
 * These variables and constants manage the number of items in the menu, the item
 * names, and the selection process within the menu. The screen navigation and cursor
 * management are also handled through these variables.
 */
#define NUM_ITEMS  4          ///< Total number of items in the menu, also used for the number of screenshots and QR codes.
#define MAX_ITEM_LENGTH  20   ///< Maximum length of characters allowed for each item name.

int item_selected = 0;         ///< Current item selected in the menu.
int item_sel_previous;         ///< Previous item index, used to display the item before the selected one.
int item_sel_next;             ///< Next item index, used to display the item after the selected one.
int current_screen = 0;        ///< Indicates the current screen index being displayed.
int cursor = 0;                ///< Position of the cursor in the menu for navigation.
int button_change_clicked = 0; // only perform action when CHANGE button is clicked, and wait until another press
int button_enter_clicked = 0; // only perform action when ENTER button is clicked, and wait until another press
int up_clicked = 0; // only perform action when button is clicked, and wait until another press
int down_clicked = 0; // same as above


// ---------------------- Bitmap Icon Variables -----------------------

/**
 * @brief Bitmap data for the dashboard icon.
 *
 * This array contains the pixel data for the dashboard icon, represented in hexadecimal format.
 * The data will be used to display the icon on the screen in the application.
 */
const unsigned char bitmap_icon_dashboard [] = {
  0x07, 0xe0, 0x18, 0x18, 0x21, 0x24, 0x50, 0x02, 0x48, 0x0a, 0x84, 0x01, 0x83, 0x81, 0xa2, 0x45,
  0x82, 0x41, 0x81, 0x81, 0xa0, 0x05, 0x40, 0x02, 0x4b, 0xd2, 0x23, 0xc4, 0x18, 0x18, 0x07, 0xe0
};

/**
 * @brief Bitmap data for the park sensor icon.
 *
 * This array contains the pixel data for the park sensor icon, represented in hexadecimal format.
 * The data will be used to display the icon on the screen in the application.
 */
const unsigned char bitmap_icon_parksensor [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x44, 0x00, 0xA4, 0x00,
  0x9F, 0x00, 0x00, 0x81, 0x30, 0xA1, 0x48, 0xA9, 0x4B, 0xA9, 0x30, 0xA0,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

/**
 * @brief Bitmap data for the fireworks icon.
 *
 * This array contains the pixel data for the fireworks icon, represented in hexadecimal format.
 * The data will be used to display the icon on the screen in the application.
 */
const unsigned char bitmap_icon_fireworks [] = {
  0x00, 0x00, 0x00, 0x08, 0x00, 0x94, 0x10, 0x08, 0x10, 0x00, 0x6c, 0x00, 0x10, 0x10, 0x10, 0x10,
  0x00, 0x00, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x10, 0x04, 0x10, 0x0a, 0x00, 0x04, 0x00, 0x00, 0x00
};

/**
 * @brief Bitmap data for the battery icon.
 *
 * This array contains the pixel data for the battery icon, represented in hexadecimal format.
 * The data will be used to display the icon on the screen in the application.
 */
const unsigned char bitmap_icon_battery [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x40, 0x04, 0x5b, 0x66, 0x5b, 0x66,
  0x5b, 0x66, 0x40, 0x04, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * @brief Bitmap data for the scrollbar background icon.
 *
 * This array contains the pixel data for the scrollbar background icon, represented in hexadecimal format.
 * The data will be used to display the background of a scrollbar on the screen in the application.
 */
const unsigned char bitmap_scrollbar_background [] = {
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00
};

/**
 * @brief Array of pointers to bitmap icons.
 *
 * This array contains pointers to different bitmap icons. It is used to access the icon data for various
 * screen displays within the application.
 */
const unsigned char* bitmap_icons[4] = {
  bitmap_icon_dashboard,    ///< Pointer to the dashboard icon.
  bitmap_icon_parksensor,   ///< Pointer to the park sensor icon.
  bitmap_icon_fireworks,    ///< Pointer to the fireworks icon.
  bitmap_icon_battery       ///< Pointer to the battery icon.
};

/**
 * @brief Bitmap data for the box of the selected item
 *
 * This array contains pointers to different bitmap icons. It is used to access the icon data for various
 * screen displays within the application.
 */
const unsigned char bitmap_item_sel_outline [] = {
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0,
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0
};

/**
 * @brief Array of the bitmap icons titles.
 *
 * This array contains pointers to different bitmap icons. It is used to access the icon data for various
 * screen displays within the application.
 */
char menu_items [NUM_ITEMS] [MAX_ITEM_LENGTH] = {  // array with item names
  {"Wifi"},
  {"LED matrix"},
  {"Buzzer"},
  {"Calibration"}
 };

#endif /*ICONS_H*/