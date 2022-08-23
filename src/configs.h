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
#define IPCAM_IP "192.168.137.109"
#define IPCAM_PORT 81
#define IPCAM_USERNAME "admin"
#define IPCAM_PASSWORD "123456789"

#define FIRMWARE_VERSION "1.3.2"
#define FIRMWARE_LASTUPDATE "2022-08-23"
#define FIRMWARE_SERVER "https://www.xxxxxxxx.com/firmware/firmware.json"

#define API_TIMEOUT 3000
#define ENABLE_DEBUG_MODE true
#define ENABLE_SOCKETIO true
#define DEFAULT_BAUD_RATE 115200
#define DEFAULT_INTERVAL 2000
#define CHECK_FIRMWARE_INTERVAL 60000
#define ENABLE_FIRMWARE_AUTOUPDATE true
#define ENABLE_LINE_NOTIFY true

#define VOLTAGE_ADAPTIVE_SENSOR D6
#define LED_PIN D5

// Mode 0 : Line Notify, 1: SocketIO
#define MODE 1

#endif