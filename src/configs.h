#ifndef CONFIGS_H
#define CONFIGS_H

// Configuration
#define DEVICE_NAME "HomeSecurity"
#define WIFI_SSID "MY-WIFI"
#define WIFI_PASSWORD "123456789"
#define SOCKETIO_HOST "192.168.137.102"
#define SOCKETIO_PORT 4000
#define SOCKETIO_CHANNEL "ESP"

// Line config
#define LINE_TOKEN "XXXXXXXXXXXXXX"

// Setup IP Camera (Generic P2P IP Camera)
#define IPCAM_IP "192.168.137.108"
#define IPCAM_PORT 81
#define IPCAM_USERNAME "admin"
#define IPCAM_PASSWORD "1234567"

#define FIRMWARE_VERSION "0.1.2"
#define FIRMWARE_LASTUPDATE "2022-07-22"
#define FIRMWARE_SERVER "https://www.xxxxxx.com/firmware/firmware.json"

#define API_TIMEOUT 3000
#define ENABLE_DEBUG_MODE true
#define ENABLE_SOCKETIO true
#define DEFAULT_BAUD_RATE 115200
#define DEFAULT_INTERVAL 2000
#define CHECK_FIRMWARE_INTERVAL 60000
#define ENABLE_FIRMWARE_AUTOUPDATE false
#define ENABLE_LINE_NOTIFY true

#define CURRENT_SENSOR_PIN A0
#define LED_PIN D5

#endif