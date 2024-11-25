#include "SDReader.hpp"
#include "WiFiConfig.hpp"

#include "APIHandler.hpp"
#include "CommandHandler.hpp"

#include "config.h"

#include "esp_camera.h"

// Camera configuration
#define CAMERA_MODEL_WROVER_KIT // Has PSRAM

#include "camera_pins.h"

// Instantiate CommandHandler for communication with ESP32
CommandHandler commandHandler(Serial);

SDReader sdReader;
APIHandler apiHandler;

status_t status = STATUS_BOOT;

camera_config_t cameraConfig;

void camera_init()
{
    cameraConfig.ledc_channel = LEDC_CHANNEL_0;
    cameraConfig.ledc_timer = LEDC_TIMER_0;
    cameraConfig.pin_d0 = Y2_GPIO_NUM;
    cameraConfig.pin_d1 = Y3_GPIO_NUM;
    cameraConfig.pin_d2 = Y4_GPIO_NUM;
    cameraConfig.pin_d3 = Y5_GPIO_NUM;
    cameraConfig.pin_d4 = Y6_GPIO_NUM;
    cameraConfig.pin_d5 = Y7_GPIO_NUM;
    cameraConfig.pin_d6 = Y8_GPIO_NUM;
    cameraConfig.pin_d7 = Y9_GPIO_NUM;
    cameraConfig.pin_xclk = XCLK_GPIO_NUM;
    cameraConfig.pin_pclk = PCLK_GPIO_NUM;
    cameraConfig.pin_vsync = VSYNC_GPIO_NUM;
    cameraConfig.pin_href = HREF_GPIO_NUM;
    cameraConfig.pin_sccb_sda = SIOD_GPIO_NUM;
    cameraConfig.pin_sccb_scl = SIOC_GPIO_NUM;
    cameraConfig.pin_pwdn = PWDN_GPIO_NUM;
    cameraConfig.pin_reset = RESET_GPIO_NUM;
    cameraConfig.xclk_freq_hz = 20000000;
    cameraConfig.frame_size = FRAMESIZE_QVGA;
    cameraConfig.pixel_format = PIXFORMAT_JPEG; // for streaming
    // camera_config.pixel_format = PIXFORMAT_RGB565; // for face
    // detection/recognition
    cameraConfig.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    cameraConfig.fb_location = CAMERA_FB_IN_PSRAM;
    cameraConfig.jpeg_quality = 10;
    cameraConfig.fb_count = 2;
}

void handleHello(const String &command)
{
    if (status == STATUS_BOOT) {
        status = STATUS_SYNCED;
        commandHandler.sendCommand("HELLO");
    }
}

void handleInit(const String &command)
{
    if (status != STATUS_SYNCED) {
        return;
    }

    SDReader::err_sd_t err = sdReader.init();

    switch (err) {

    case SDReader::err_sd_t::SD_ERR_NO_SDC:
        commandHandler.sendCommand("NO_SDC");
        status = STATUS_ERROR;
        return;
    case SDReader::err_sd_t::SD_ERR_CONFIG_FILE_NOT_CREATED:
        commandHandler.sendCommand("CONFIG_FILE_NOT_CREATED");
        status = STATUS_ERROR;
        return;
    }

    // Read WiFi configuration
    WiFiConfig wifiConfig;

    SDReader::err_read_config_t readConfErr = sdReader.readConfig(wifiConfig);

    switch (readConfErr) {
    case SDReader::err_read_config_t::RC_BAD_WIFI_CONFIG:
        commandHandler.sendCommand("BAD_WIFI_CONF");
        status = STATUS_ERROR;
        return;
    }

    APIHandler::err_wifi_t api_err = apiHandler.init(wifiConfig);

    if (api_err == APIHandler::err_wifi_t::WIFI_ERR) {
        commandHandler.sendCommand("NO_WIFI_CONN");
        status = STATUS_ERROR;
        return;
    }

    camera_init();

    esp_err_t camErr = esp_camera_init(&cameraConfig);
    if (camErr != ESP_OK) {
        // Serial.printf("ERROR: Camera init failed with error 0x%x\n", camErr);
        commandHandler.sendCommand("CAM_INIT_FAIL");
        return;
    }

    APIHandler::api_response_code_t response = apiHandler.pingAPI();

    // Serial.println("API response: " + String(response));

    if (response != 200) {
        commandHandler.sendCommand("NO_INTERNET");
        status = STATUS_ERROR;
        return;
    }

    commandHandler.sendCommand("INIT_SUCCESS");
    status = STATUS_READY;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Register command handlers
    commandHandler.registerRoute("HELLO", handleHello);
    commandHandler.registerRoute("INIT", handleInit);

    commandHandler.sendCommand("HELLO");
}

void loop()
{
    commandHandler.handleIncomingCommand();
    delay(100);
}
