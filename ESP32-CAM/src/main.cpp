#include "HX711.h"
#include "SDReader.hpp"
#include "WiFiConfig.hpp"

#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

// LCD configuration
#define I2C_ADDR 0x27  // I2C address of the LCD
#define LCD_COLUMNS 16 // Number of columns of your LCD
#define LCD_LINES 2    // Number of rows of your LCD
#define SDA_PIN 21     // Your custom SDA pin
#define SCL_PIN 22     // Your custom SCL pin

// HX711 configuration
#define LOADCELL_DOUT_PIN 15
#define LOADCELL_SCK_PIN 4

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);
HX711 scale;

// Calibration factor (adjust based on your load cell)
#define CALIBRATION_FACTOR 2280.0f

bool isStartupDone = false;

void connectToWiFi(const WiFiConfig &config)
{
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connection");

    Serial.println("Connecting to WiFi...");
    Serial.printf("SSID: %s\n", config.ssid.c_str());
    Serial.printf("Password: %s\n", config.password.c_str());

    if (config.ssid.empty() || config.password.empty()) {
        Serial.println(
            "ERROR: SSID or Password is missing. Cannot connect to WiFi.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Error: SSID");
        lcd.setCursor(0, 1);
        lcd.print("or Password");
        delay(3000);
        return;
    }

    // Check for static IP configuration
    if (!config.ip.empty() && !config.gateway.empty() && !config.mask.empty()) {
        IPAddress localIP, gateway, subnet;

        if (localIP.fromString(config.ip.c_str()) &&
            gateway.fromString(config.gateway.c_str()) &&
            subnet.fromString(config.mask.c_str())) {

            if (!WiFi.config(localIP, gateway, subnet)) {
                Serial.println("ERROR: Failed to configure static IP.");
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("WiFi Error: IP");
                delay(3000);
                return;
            }
            Serial.println("Static IP configuration applied.");
        } else {
            Serial.println("ERROR: Invalid IP, Gateway, or Mask format.");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("IP Config Error");
            delay(3000);
            return;
        }
    }

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    int retryCount = 20; // Retry up to 20 times
    int dotCount = 0;

    while (WiFi.status() != WL_CONNECTED && retryCount > 0) {
        delay(500);
        Serial.print(".");
        retryCount--;

        lcd.setCursor(0, 1);
        if (dotCount < 16) {
            lcd.print(".");
            dotCount++;
        } else {
            lcd.setCursor(0, 1);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            dotCount = 0;
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Connected!");
        delay(2000);
        lcd.clear();
    } else {
        Serial.println("\nERROR: Failed to connect to WiFi.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Connect");
        lcd.setCursor(0, 1);
        lcd.print("Failed!");
        delay(3000);
        return;
    }

    isStartupDone = true;
    lcd.clear();
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Initialize I2C communication
    Wire.begin(SDA_PIN, SCL_PIN);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Scale It!");
    delay(2000);
    lcd.clear();

    // Initialize HX711
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    if (!scale.is_ready()) {
        Serial.println("ERROR: HX711 not detected!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("HX711 Error!");
        delay(3000);
    } else {
        Serial.println("HX711 initialized.");
        scale.set_scale(CALIBRATION_FACTOR); // Set calibration factor
        scale.tare();                        // Reset scale to 0
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scale Ready");
        delay(2000);
        lcd.clear();
    }

    // Read WiFi configuration
    SDReader sdReader;
    WiFiConfig config = sdReader.readConfig();
    connectToWiFi(config);
}

void loop()
{
    // Measure weight
    if (scale.is_ready() && isStartupDone) {
        float weight = scale.get_units();
        Serial.printf("Weight: %.2f kg\n", weight);

        lcd.setCursor(0, 0);
        lcd.print("Weight:");
        lcd.setCursor(0, 1);
        lcd.printf("%.2f kg    ", weight); // Clear trailing characters
    } else {
        Serial.println("ERROR: HX711 not ready!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scale Error!");
    }

    delay(1000);
}
