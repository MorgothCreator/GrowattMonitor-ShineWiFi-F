#ifndef __DEF_H__
#define __DEF_H__

#include "credentials.h"

#define USE_INTERNAL_SPIFFS   false
#define USE_SPIFFS_LOG        false
#define USE_HTTPS             false

typedef struct {
  char *ssid;
  char *password;
}wiFiCredentials_t;

/*
If you have multiple wifi access points you can define multiple SSID's in below array.
You can define the WiFi credentials here or in a separate "credentials.h" file.
#define WIFI_CREDENTIALS { \
  { \
    "SSID 1", \
    "password_for_wofi_1" \
  }, \
  { \
    "SSID 2", \
    "password_for_wofi_2" \
  }, \
  { \
    "", \
    "" \
  } \
};
*/

#define CONNECT_RETRY_COUNT 20// Each take half a second

enum {
  DISCONNECTED,
  CONNECTING,
  CONNECTED
} WiFiState = DISCONNECTED;

#define LED1_BLUE     16// BLUE
#define LED5_RED      2 // RED
#define LED6_GREEN     0 // GREEN

#define LOG_BUFF_LEN    4096

#define OTA_NAME "OTA-GROWATT5000ES-1"
// OTA password "OTA_PASS" is defined in "credentials.h" file.

#endif