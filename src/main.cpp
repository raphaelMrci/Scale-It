#include "SDReader.hpp"
#include "WiFiConfig.hpp"

#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

#define I2C_ADDR 0x27  // I2C address of the LCD
#define LCD_COLUMNS 16 // Number of columns of your LCD
#define LCD_LINES 2    // Number of rows of your LCD
#define SDA_PIN 21     // Your custom SDA pin
#define SCL_PIN 22     // Your custom SCL pin

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

void connectToWiFi(const WiFiConfig &config)
{
    lcd.setCursor(0, 0);
    lcd.print("Connecting to WiFi");

    if (config.ssid.empty() || config.password.empty()) {
        Serial.println(
            "ERROR: SSID or Password is missing. Cannot connect to WiFi.");
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
                return;
            }
            Serial.println("Static IP configuration applied.");
        } else {
            Serial.println("ERROR: Invalid IP, Gateway, or Mask format.");
            return;
        }
    }

    // Connect to WiFi
    Serial.printf("Connecting to WiFi\n");
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
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C communication

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("ScaleDown");
    delay(2000);
    lcd.clear();

    SDReader sdReader;

    WiFiConfig config = sdReader.readConfig();

    connectToWiFi(config);
}

void loop()
{
}
