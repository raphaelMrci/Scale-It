#pragma once

#include <Arduino.h>

typedef enum status_e {
    STATUS_BOOT,
    STATUS_SYNCED,
    STATUS_READY,
    STATUS_ERROR = -1
} status_t;

const String API_URL("http://15.237.127.113:8000");
