#pragma once

typedef enum status_e {
    STATUS_BOOT,
    STATUS_SYNCED,
    STATUS_READY,
    STATUS_ERROR = -1
} status_t;
