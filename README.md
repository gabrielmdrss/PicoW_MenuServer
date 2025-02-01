# PicoW_MenuServer

## Description
PicoW_MenuServer  is an IoT hub based on the BitDogLab development board, equipped with a Raspberry Pi Pico W, LEDs, pushbuttons, a 5x5 addressable LED matrix, and an SSD1306 display. The project allows initial configuration via Access Point and an interactive menu for feature control.

## Features
### 1. Network Configuration
- When powered on, the device enters **Access Point** mode.
- It creates a Wi-Fi network and hosts a web page.
- The page contains two fields for entering the **SSID** and **Password** of the desired network.
- After submission, AP mode is disabled, and the device attempts to connect to the new network.
- If the connection is successful, the device starts normal operation.

### 2. Interactive OLED Menu
- The menu is displayed on the **SSD1306** display.
- Navigation is done using the board's **joystick**.
- Option selection is performed via **push button B**.

### 3. Menu Features
#### Wi-Fi
*(Details to be added)*

#### MQTT
*(Details to be added)*

#### Buzzer
*(Details to be added)*

## Hardware Used
- **BitDogLab** (with Raspberry Pi Pico W)
- **5x5 Addressable LED Matrix**
- **OLED SSD1306 Display**
- **Pushbuttons**
- **Joystick**
- **Buzzer**

## Pins Used
- **LEDs**: On pin X
- **Pushbuttons**: On pin X
- **LED Matrix**: On pin X
- **OLED Display**: On pin X
- **Buzzer**: On pin X

## How to Use
1. Power on the device.
2. Connect to the Wi-Fi network created by AP mode.
3. Access the configuration page and enter SSID and password.
4. After a successful connection, use the menu to access features.

## License
This project is licensed under the MIT License.
