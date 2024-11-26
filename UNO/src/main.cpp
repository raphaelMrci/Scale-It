#include "CommandHandler.hpp"
#include "HX711.h"
#include "config.h"

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <ctype.h>

// LCD configuration
#define I2C_ADDR 0x27  // I2C address of the LCD
#define LCD_COLUMNS 16 // Number of columns of your LCD
#define LCD_LINES 2    // Number of rows of your LCD

// HX711 configuration
#define DT_PIN 3  // Data pin of HX711
#define SCK_PIN 2 // Clock pin of HX711

// Tare button configuration
#define TARE_BUTTON_PIN 4 // Digital pin for the taring button

// Capture button
#define CAPTURE_BUTTON_PIN 5 // Digital pin for the capture button

#define ESP_RX 6
#define ESP_TX 7

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);
HX711 scale;

// SoftwareSerial for communication with ESP32
// SoftwareSerial espSerial(ESP_RX, ESP_TX);

// Instantiate CommandHandler for communication with ESP32
CommandHandler commandHandler(Serial);

status_t status = STATUS_BOOT;

void handleHello(const String &command)
{
    commandHandler.sendCommand("READY");
    commandHandler.sendCommand("INIT");

    if (status == STATUS_BOOT) {
        status = STATUS_SYNCED;
    }
}

void handleReady(const String &command)
{
    if (status == STATUS_BOOT) {
        status = STATUS_SYNCED;
        commandHandler.sendCommand("READY");
        commandHandler.sendCommand("INIT");
    }
}

void handleNoSDCard(const String &command)
{
    status = STATUS_NO_SDC;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("No SD card!");
}

void handleBadWiFiConfig(const String &command)
{
    status = STATUS_BAD_WIFI_CONF;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Bad WiFi config!");
}

void handleNoWiFiConnection(const String &command)
{
    status = STATUS_NO_WIFI_CONN;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Unable to connect");
    lcd.setCursor(1, 1);
    lcd.print("to WiFi...");
}

void handleCamInitFailed(const String &command)
{
    status = STATUS_CAM_INIT_FAIL;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Camera init");
    lcd.setCursor(1, 1);
    lcd.print("failed!");
}

void handleNoInternet(const String &command)
{
    status = STATUS_NO_INTERNET;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("No internet");
    lcd.setCursor(1, 1);
    lcd.print("connection...");
}

void handleInitSuccess(const String &command)
{
    if (status == STATUS_SYNCED) {
        status = STATUS_READY;
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Scale It!");
    }
}

// The ESP will respond with the name of the food item, and the calories per
// 100g
void handleFoodInfo(const String &command)
{
    // Parse the response
    int spaceIndex = command.indexOf(' ');
    String foodName = command.substring(0, spaceIndex);
    String calories = command.substring(spaceIndex + 1);

    // Display the food information on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(foodName);
    lcd.setCursor(0, 1);
    lcd.print(calories);
    lcd.print(" cal/100g");
    delay(5000); // Display for 5 seconds
}

void handleFoodNotRecognized(const String &command)
{
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Food not");
    lcd.setCursor(1, 1);
    lcd.print("recognized!");
    delay(2000); // Display for 2 seconds
}
void handleConfigFileNotCreated(const String &command)
{
    status = STATUS_ERROR;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Config file");
    lcd.setCursor(1, 1);
    lcd.print("not created...");
}

void statusHandler(const String &command)
{
    if (command.length() == 0) {
        return;
    }

    if (!isdigit(command[0])) {
        // Serial.println("Invalid status received: " + command);
        return;
    }

    status_t newStatus = static_cast<status_t>(command.toInt());

    switch (newStatus) {
    case STATUS_BOOT:
        handleHello("");
        break;
    case STATUS_SYNCED:
        handleReady("");
        break;
    case STATUS_INIT:
        break;
    case STATUS_READY:
        handleInitSuccess("");
        break;
    case STATUS_CAM_INIT_FAIL:
        handleCamInitFailed("");
        break;
    case STATUS_NO_WIFI_CONN:
        handleNoWiFiConnection("");
        break;
    case STATUS_NO_SDC:
        handleNoSDCard("");
        break;
    case STATUS_CONFIG_FILE_NOT_CREATED:
        handleConfigFileNotCreated("");
        break;
    case STATUS_BAD_WIFI_CONF:
        handleBadWiFiConfig("");
        break;
    case STATUS_NO_INTERNET:
        handleNoInternet("");
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    // espSerial.begin(115200); // Communication with ESP32
    delay(1000);

    while (!Serial) {
        ; // Wait for serial port to connect
    }

    delay(1000);
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Initializing...");

    // Register command handlers
    commandHandler.registerRoute("HELLO", handleHello);
    commandHandler.registerRoute("NO_SDC", handleNoSDCard);
    commandHandler.registerRoute("BAD_WIFI_CONF", handleBadWiFiConfig);
    commandHandler.registerRoute("NO_WIFI_CONN", handleNoWiFiConnection);
    commandHandler.registerRoute("CAM_INIT_FAIL", handleCamInitFailed);
    commandHandler.registerRoute("NO_INTERNET", handleNoInternet);
    commandHandler.registerRoute("INIT_SUCCESS", handleInitSuccess);
    commandHandler.registerRoute("FOOD_INFO", handleFoodInfo);
    commandHandler.registerRoute("FOOD_NOT_RECOG", handleFoodNotRecognized);
    commandHandler.registerRoute("CONFIG_FILE_NOT_CREATED",
                                 handleConfigFileNotCreated);
    commandHandler.registerRoute("READY", handleReady);
    commandHandler.registerRoute("STATUS", statusHandler);

    // Send HELLO
    commandHandler.sendCommand("HELLO");

    // Initialize HX711
    scale.begin(DT_PIN, SCK_PIN);

    // Set calibration factor (adjust based on calibration)
    scale.set_scale(-459.542);
    scale.tare(); // Reset the scale to 0

    // Initialize tare button
    pinMode(TARE_BUTTON_PIN,
            INPUT_PULLUP); // Configure as input with pull-up resistor
}

void loop()
{
    commandHandler.handleIncomingCommand(); // Handle serial commands

    if (status == STATUS_BOOT) {
        static unsigned long lastHello = 0;
        if (millis() - lastHello > 1000) { // Send HELLO every 1 second
            lastHello = millis();
            commandHandler.sendCommand("HELLO");
        }
        return;
    }

    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 1000) { // Send STATUS every 1 second
        lastStatus = millis();
        commandHandler.sendCommand("STATUS");
    }

    if (status != STATUS_READY) {
        return; // Skip processing if the system isn't ready
    }

    // Read weight
    float weight = scale.get_units(10); // Get average weight from 10 readings

    // Display weight on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weight:");
    lcd.setCursor(0, 1);
    lcd.print(weight, 2);
    lcd.print(" g");

    // Check if tare button is pressed
    if (digitalRead(TARE_BUTTON_PIN) == LOW) { // Button is pressed (active LOW)
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Taring...");
        scale.tare(); // Reset the scale to 0
        delay(1000);  // Give some time for taring
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tared!");
        delay(1000); // Display "Tared!" message
    }

    // Wait for 1 second before next reading
    delay(1000);
}
