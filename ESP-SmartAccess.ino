#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <TTP229.h>
#include <ErriezTTP229.h>

const char* ssid = "shree";
const char* password = "1234567890";

ESP8266WebServer server(80);

#if defined(ARDUINO_ARCH_AVR)
#define TTP229_SDO_PIN 2
#define TTP229_SCL_PIN 3
#elif defined(ARDUINO_ARCH_ESP8266)
#define TTP229_SDO_PIN D1
#define TTP229_SCL_PIN D2
#elif defined(ARDUINO_ARCH_ESP32)
#define TTP229_SDO_PIN 16
#define TTP229_SCL_PIN 4
#else
#error "May work, but not tested on this target"
#endif

ErriezTTP229 ttp229;

#define SS_PIN D8
#define RST_PIN D3
MFRC522 rfid(SS_PIN, RST_PIN);

#define GREEN_LED D0
#define RED_LED D4

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
ICACHE_RAM_ATTR
#endif
void keyChange()
{
    ttp229.keyChange = true;
}


#define MAX_PASSWORD_LENGTH 8
char currentPassword[MAX_PASSWORD_LENGTH + 1] = "1234";
char inputPassword[MAX_PASSWORD_LENGTH + 1] = "";
int inputIndex = 0;
bool resetMode = false;

byte masterTag[4] = {0xEA, 0x61, 0x27, 0xDF};

const char keyMap[16] = {
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '*', '0', '#', 'D'
};

void setup() {
  Serial.begin(9600);
  Serial.println("\nInitializing system...");
  
  Wire.begin();
  Serial.println("I2C initialized for TTP229 keypad");
  ttp229.begin(TTP229_SCL_PIN, TTP229_SDO_PIN);
  attachInterrupt(digitalPinToInterrupt(TTP229_SDO_PIN), keyChange, FALLING);
  
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  Serial.println("LED pins configured");
  
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID module initialized");
  
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.on("/keypad", handleKeypadInput);
  server.on("/reset", handleResetPage);
  server.begin();
  Serial.println("HTTP server started");
  
  Serial.print("Current password: ");
  Serial.println(currentPassword);
  Serial.print("Master RFID tag: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(masterTag[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  blinkLED(GREEN_LED, 3);
  Serial.println("System ready");
}

void loop() {
  server.handleClient();
  
  if (ttp229.keyChange) {
    int key=ttp229.GetKey16();
    delay(250);
    Serial.println(keyMap[key-1]);
    handleKeyPress(keyMap[key-1]);
    ttp229.keyChange = false;
  }

  if (!resetMode && rfid.PICC_IsNewCardPresent()) {
    Serial.println("RFID card detected");
    if (rfid.PICC_ReadCardSerial()) {
      Serial.print("Card UID: ");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      
      if (checkMasterTag(rfid.uid.uidByte)) {
        resetMode = true;
        inputIndex = 0;
        memset(inputPassword, 0, sizeof(inputPassword));
        Serial.println("Password reset mode activated. Enter new password:");
        blinkLED(GREEN_LED, 5); // Signal reset mode activation
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }
}

void handleKeyPress(char key) {
  Serial.print("Key pressed: ");
  Serial.println(key);
  
  if (key == '#') {
    if (resetMode) {
      Serial.println("Processing password reset...");
      if (inputIndex > 0) {
        inputPassword[inputIndex] = '\0';
        strcpy(currentPassword, inputPassword);
        resetMode = false;
        Serial.println("Password changed successfully!");
        Serial.print("New password: ");
        Serial.println(currentPassword);
        blinkLED(GREEN_LED, 3);
      } else {
        Serial.println("Empty password entered for reset");
        blinkLED(RED_LED, 2);
      }
    } else {
      Serial.println("Checking password...");
      inputPassword[inputIndex] = '\0';
      Serial.print("Input password: ");
      Serial.println(inputPassword);
      if (strcmp(inputPassword, currentPassword) == 0) {
        Serial.println("Password correct - Door unlocked!");
        digitalWrite(GREEN_LED, HIGH);
        delay(2000);
        digitalWrite(GREEN_LED, LOW);
      } else {
        Serial.println("Password incorrect!");
        blinkLED(RED_LED, 3);
      }
    }
    inputIndex = 0;
    memset(inputPassword, 0, sizeof(inputPassword));
  } 
  else if (key == '*') {
    Serial.println("Clearing input");
    inputIndex = 0;
    memset(inputPassword, 0, sizeof(inputPassword));
    blinkLED(RED_LED, 1);
  } 
  else if (inputIndex < MAX_PASSWORD_LENGTH) {
    Serial.print("Adding character to password buffer. Current length: ");
    Serial.println(inputIndex + 1);
    inputPassword[inputIndex++] = key;
    digitalWrite(GREEN_LED, HIGH);
    delay(100);
    digitalWrite(GREEN_LED, LOW);
  } else {
    Serial.println("Password input buffer full");
  }
}

bool checkMasterTag(byte* uid) {
  Serial.println("Checking RFID tag against master...");
  for (int i = 0; i < 4; i++) {
    if (uid[i] != masterTag[i]) {
      Serial.println("Tag is NOT master");
      Serial.println(uid[i]);
      return false;
    }
  }
  Serial.println("Tag is master");
  return true;
}

void blinkLED(int pin, int times) {
  Serial.print("Blinking LED on pin ");
  Serial.print(pin);
  Serial.print(" ");
  Serial.print(times);
  Serial.println(" times");
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(200);
    digitalWrite(pin, LOW);
    delay(200);
  }
}

void handleRoot() {
  Serial.println("Handling root page request");
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin: 0; padding: 20px; }";
  html += ".keypad { display: inline-block; margin: 20px auto; }";
  html += ".btn { width: 60px; height: 60px; margin: 5px; font-size: 24px; }";
  html += ".display { width: 220px; height: 40px; margin: 10px auto; padding: 5px; border: 1px solid #ccc; font-size: 20px; }";
  html += "</style></head><body>";
  html += "<h2>Smart Door Lock</h2>";
  html += "<div class='display' id='display'></div>";
  html += "<div class='keypad'>";
  html += "<div><button class='btn' onclick='addKey(1)'>1</button><button class='btn' onclick='addKey(2)'>2</button><button class='btn' onclick='addKey(3)'>3</button><button class='btn' onclick='addKey(\"A\")'>A</button></div>";
  html += "<div><button class='btn' onclick='addKey(4)'>4</button><button class='btn' onclick='addKey(5)'>5</button><button class='btn' onclick='addKey(6)'>6</button><button class='btn' onclick='addKey(\"B\")'>B</button></div>";
  html += "<div><button class='btn' onclick='addKey(7)'>7</button><button class='btn' onclick='addKey(8)'>8</button><button class='btn' onclick='addKey(9)'>9</button><button class='btn' onclick='addKey(\"C\")'>C</button></div>";
  html += "<div><button class='btn' onclick='clearInput()'>*</button><button class='btn' onclick='addKey(0)'>0</button><button class='btn' onclick='submitCode()'>#</button><button class='btn' onclick='addKey(\"D\")'>D</button></div>";
  html += "</div>";
  html += "<p id='message'></p>";
  html += "<p><a href='/reset'>Reset Password</a></p>";
  html += "<script>";
  html += "let code = '';";
  html += "function addKey(num) {";
  html += "  if(code.length < 4) {";
  html += "    code += num;";
  html += "    document.getElementById('display').innerText = '*'.repeat(code.length);";
  html += "  }";
  html += "}";
  html += "function clearInput() {";
  html += "  code = '';";
  html += "  document.getElementById('display').innerText = '';";
  html += "  document.getElementById('message').innerText = '';";
  html += "}";
  html += "function submitCode() {";
  html += "  fetch('/keypad?code=' + code)";
  html += "    .then(response => response.text())";
  html += "    .then(data => {";
  html += "      document.getElementById('message').innerText = data;";
  html += "      if(data.includes('Success')) {";
  html += "        document.getElementById('message').style.color = 'green';";
  html += "      } else {";
  html += "        document.getElementById('message').style.color = 'red';";
  html += "      }";
  html += "      code = '';";
  html += "      document.getElementById('display').innerText = '';";
  html += "    });";
  html += "}";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleKeypadInput() {
  Serial.println("Handling keypad input from web");
  if (server.hasArg("code")) {
    String code = server.arg("code");
    Serial.print("Received code from web: ");
    Serial.println(code);
    if (code == String(currentPassword)) {
      Serial.println("Web password correct");
      server.send(200, "text/plain", "Success! Door unlocked.");
      digitalWrite(GREEN_LED, HIGH);
      delay(2000);
      digitalWrite(GREEN_LED, LOW);
    } else {
      Serial.println("Web password incorrect");
      server.send(200, "text/plain", "Error! Incorrect code.");
      blinkLED(RED_LED, 3);
    }
  } else {
    Serial.println("Bad request to /keypad");
    server.send(400, "text/plain", "Bad Request");
  }
}

void handleResetPage() {
  Serial.println("Handling reset page request");
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin: 0; padding: 20px; }";
  html += "</style></head><body>";
  html += "<h2>Reset Password</h2>";
  html += "<p>Please scan your RFID tag to activate password reset mode.</p>";
  html += "<p>After scanning, enter a new password on the physical keypad and press # to confirm.</p>";
  html += "<p>Look for the LED indicators on the device to confirm success:</p>";
  html += "<ul style='list-style-type:none; padding:0;'>";
  html += "<li>Green LED blinking 5 times: Reset mode activated</li>";
  html += "<li>Green LED blinking 3 times: Password successfully changed</li>";
  html += "<li>Red LED blinking: Error or invalid input</li>";
  html += "</ul>";
  html += "<p><a href='/'>Back to main page</a></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}
