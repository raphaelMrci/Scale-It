#include "SDReader.hpp"
#include "WiFiConfig.hpp"

#include <WiFi.h>

void connectToWiFi(const WiFiConfig &config)
{
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
    while (WiFi.status() != WL_CONNECTED && retryCount > 0) {
        delay(500);
        Serial.print(".");
        retryCount--;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nERROR: Failed to connect to WiFi.");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    SDReader sdReader;

    WiFiConfig config = sdReader.readConfig();

    connectToWiFi(config);
}

void loop()
{
}
