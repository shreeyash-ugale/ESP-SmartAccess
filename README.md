# ESP-SmartAccess

## Overview
ESP-SmartAccess is an ESP8266-based smart access control system integrating an RFID reader, a 16-key TTP229 touch keypad, and a web interface for password management. This project enhances security by allowing access through an RFID master card or a password-protected keypad. The system also supports remote access via a web-based keypad.

## Features
- **RFID Authentication**: Unlock using an RFID master card.
- **Keypad Authentication**: Enter a numeric password for access.
- **Web-based Interface**: Remotely enter passwords via a web portal.
- **Password Reset Mode**: Change passwords using the master RFID card.
- **LED Status Indicators**: Visual feedback for authentication success/failure.
- **ESP8266 Web Server**: Handles HTTP requests for remote access.

## Hardware Requirements
- **ESP8266 (e.g., NodeMCU, Wemos D1 Mini)**
- **MFRC522 RFID Reader Module**
- **TTP229 16-Key Touch Keypad Module**
- **LED Indicators (Green & Red)**
- **Power Supply (Micro USB or 5V adapter)**

## Wiring Diagram
| Component     | ESP8266 Pin |
|--------------|------------|
| RFID SS      | D8         |
| RFID RST     | D3         |
| TTP229 SDO   | D1         |
| TTP229 SCL   | D2         |
| Green LED    | D0         |
| Red LED      | D4         |

## Installation
### 1. Install Required Libraries
Ensure the following libraries are installed in the Arduino IDE:
- **ESP8266WiFi**
- **ESP8266WebServer**
- **SPI**
- **MFRC522**
- **Wire**
- **ErriezTTP229**

### 2. Upload the Code
1. Open `ESP-SmartAccess.ino` in Arduino IDE.
2. Configure your WiFi credentials:
   ```cpp
   const char* ssid = "your_SSID";
   const char* password = "your_PASSWORD";
   ```
3. Upload the code to the ESP8266 board.

### 3. Setup and Usage
- Upon boot, the ESP8266 connects to WiFi and starts a web server.
- The RFID master tag can activate password reset mode.
- Use the TTP229 keypad or the web interface to enter the password.
- Successful authentication triggers the green LED, while failure triggers the red LED.
- Access the web interface via the ESP8266's IP address.

## Web Interface
### **Features:**
- Virtual keypad for entering passwords.
- Reset page for password change instructions.
- Authentication feedback messages.

### **Accessing the Web Interface:**
1. After connecting to WiFi, find the ESP8266's IP address in the Serial Monitor.
2. Open a web browser and enter the IP address.
3. Use the on-screen keypad to enter the password and authenticate.

## Future Improvements
- Integration with IoT platforms for remote access logs.
- Mobile app support.
- Enhanced encryption for password storage.
- MongoDB integration for logs, password storage, etc.

