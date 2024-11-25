#include "SDReader.hpp"
#include "SD_MMC.h"
#include <dirent.h>

SDReader::SDReader()
{
    if (!SD_MMC.begin("", true)) {
        Serial.println("ERROR: SD card initialization failed");
        return;
    }
    Serial.println("SD card initialized");

    // Check if config.txt file exists in SD card
    if (!SD_MMC.exists("/config.txt")) {
        Serial.println("Config file not found");

        // Create config.txt file
        File file = SD_MMC.open("/config.txt", FILE_WRITE);
        if (!file) {
            Serial.println("Failed to create config file");
            return;
        }
        file.print(_defaultConfig.c_str());
        file.close();
        Serial.println("Config file created");
    }
}

WiFiConfig SDReader::readConfig()
{
    Serial.println("Reading config file...");

    // Read the entire file content
    String configFile = readFile("/config.txt");
    if (configFile.isEmpty()) {
        Serial.println("ERROR: Config file is empty or cannot be read.");
        return WiFiConfig(); // Return a default WiFiConfig object
    }

    WiFiConfig config;
    String lines[5]; // To store lines temporarily
    int index = 0;

    // Split the file content into lines
    while (configFile.length() > 0 && index < 5) {
        int endOfLine = configFile.indexOf('\n');
        if (endOfLine == -1) { // Last line
            lines[index++] = configFile;
            break;
        } else {
            lines[index++] = configFile.substring(0, endOfLine);
            configFile = configFile.substring(endOfLine + 1);
        }
    }

    // Parse the lines
    for (int i = 0; i < index; i++) {
        lines[i].trim(); // Remove any whitespace or newline characters

        if (lines[i].startsWith("SSID=")) {
            config.ssid = lines[i].substring(5).c_str();
        } else if (lines[i].startsWith("Password=")) {
            config.password = lines[i].substring(9).c_str();
        } else if (lines[i].startsWith("Gateway=")) {
            config.gateway = lines[i].substring(8).c_str();
        } else if (lines[i].startsWith("IP=")) {
            config.ip = lines[i].substring(3).c_str();
        } else if (lines[i].startsWith("MASK=")) {
            config.mask = lines[i].substring(5).c_str();
        }
    }

    Serial.println("Config file parsed successfully:");
    Serial.println("SSID: " + String(config.ssid.c_str()));
    Serial.println("Password: " + String(config.password.c_str()));
    Serial.println("Gateway: " + String(config.gateway.c_str()));
    Serial.println("IP: " + String(config.ip.c_str()));
    Serial.println("MASK: " + String(config.mask.c_str()));

    return config;
}

String SDReader::readFile(String path)
{
    File file = SD_MMC.open(path, FILE_READ);
    String reading;

    if (!file) {
        Serial.println("ERROR: Unable to open file");
        return "";
    }
    reading = file.readString();
    file.close();
    return reading;
}
