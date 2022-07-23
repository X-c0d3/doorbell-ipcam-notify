/*

  # Author : Watchara Pongsri
  # [github/X-c0d3] https://github.com/X-c0d3/
  # Web Site: https://wwww.rockdevper.com

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SocketIoClient.h>
#include <SoftwareSerial.h>  //  ( NODEMCU ESP8266 )
#include <WiFiClientSecureAxTLS.h>
#include <arduino-timer.h>
#include <stdlib.h>

#include "configs.h"
#include "firmware.h"
#include "lineNotify.h"
#include "utility.h"

SocketIoClient webSocket;

unsigned long interval = 300;         // the time we need to wait
unsigned long previousMillis = 0;     // millis() returns an unsigned long.
auto timer = timer_create_default();  // create a timer with default settings
Timer<> default_timer;                // save as above

int senseDoorbell = 0;
unsigned int debounce = 1000;
unsigned long currentMillis = 0;
unsigned long prevRing = 0;

void event(const char* payload, size_t length) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(String(payload));
    String action = root["action"];
    if (action != "") {
        Serial.printf("=====>: %s\n", payload);
        // int delayTime = root["payload"]["delay"];
        String state = root["payload"]["state"];
        String messageInfo = root["payload"]["messageInfo"];
        // bool isAuto = root["payload"]["isAuto"];

        // if (action == "BOOTMPPT") {
        //     digitalWrite(D0, (state == "state:on") ? HIGH : LOW);

        //     String mpptStatus = String((state == "state:on") ? "ON" : "OFF");
        //     String msq = (messageInfo != "") ? messageInfo : "";
        //     msq += "\r\n===============\r\n Boot Mppt Status : " + mpptStatus;
        //     Line_Notify(msq);
        // }
    }
}

void takeSnapshot(String message) {
    digitalWrite(LED_BUILTIN, HIGH);
    HTTPClient http;
    http.begin("http://" + String(IPCAM_IP) + ":" + String(IPCAM_PORT) + "/snapshot.cgi?user=" + String(IPCAM_USERNAME) + "&pwd=" + String(IPCAM_PASSWORD));

    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK) {
            int len = http.getSize();
            WiFiClient* stream = http.getStreamPtr();
            Serial.println("Sending Snapshot from Ip Camera");
            if (Line_Notify_Picture(message, stream, len)) {
                Serial.println("The Snapshot sending success !!");
            }
        }
    }
    http.end();

    digitalWrite(LED_BUILTIN, LOW);
}

bool firmwareCheckUpdate(void*) {
    if (ENABLE_FIRMWARE_AUTOUPDATE)
        updateFirmware();

    return true;
}

bool statusCheck(void*) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    return true;
}

void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(CURRENT_SENSOR_PIN, INPUT);
    // Connect WIFI
    setup_Wifi();
    setupTimeZone();

    if (WiFi.status() == WL_CONNECTED) {
        webSocket.begin(SOCKETIO_HOST, SOCKETIO_PORT);
        webSocket.on(SOCKETIO_CHANNEL, event);

        timer.every(CHECK_FIRMWARE_INTERVAL, firmwareCheckUpdate);
        timer.every(2000, statusCheck);

        if (ENABLE_FIRMWARE_AUTOUPDATE)
            timer.every(CHECK_FIRMWARE_INTERVAL, firmwareCheckUpdate);

        Line_Notify(String(DEVICE_NAME) + " - Started...");
    }

    wdt_enable(10000);
}

void loop() {
    if (ENABLE_SOCKETIO && (WiFi.status() == WL_CONNECTED)) {
        wdt_reset();  // reset timer (feed watchdog)

        webSocket.loop();

        currentMillis = millis();
        if (currentMillis - prevRing >= debounce) {
            senseDoorbell = analogRead(CURRENT_SENSOR_PIN);
            // detecting doolbell from current sensor
            if (senseDoorbell > 54) {
                Serial.println("DingDong : Value is " + String(senseDoorbell));
                takeSnapshot("☃ มีผู้มาเยือน ☃ [" + String(senseDoorbell) + "]\r\n" + printLocalTime());
                prevRing = currentMillis;
            }
        }
    }

    timer.tick();
}