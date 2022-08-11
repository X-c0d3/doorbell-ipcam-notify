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
#include "makeResponse.h"
#include "utility.h"

SocketIoClient webSocket;

auto timer = timer_create_default();  // create a timer with default settings
Timer<> default_timer;                // save as above

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

void takeSnapshot() {
    digitalWrite(LED_PIN, HIGH);
    HTTPClient http;
    http.begin("http://" + String(IPCAM_IP) + ":" + String(IPCAM_PORT) + "/snapshot.cgi?user=" + String(IPCAM_USERNAME) + "&pwd=" + String(IPCAM_PASSWORD));
    // For Hi Resolution use : res=0
    // http.begin("http://" + String(IPCAM_IP) + ":" + String(IPCAM_PORT) + "/snapshot.cgi?user=" + String(IPCAM_USERNAME) + "&pwd=" + String(IPCAM_PASSWORD) + "&res=0");

    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK) {
            int len = http.getSize();
            WiFiClient* stream = http.getStreamPtr();
            Serial.println("Sending Snapshot from Ip Camera");
            if (Line_Notify_Picture("☃ มีผู้มาเยือน ☃ \r\n" + printLocalTime(), stream, len)) {
                Serial.println("The Snapshot sending success !!");
            }
        }
    }
    http.end();

    digitalWrite(LED_PIN, LOW);
}

bool firmwareCheckUpdate(void*) {
    if (ENABLE_FIRMWARE_AUTOUPDATE)
        updateFirmware();

    return true;
}

bool statusCheck(void*) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    return true;
}

void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);

    pinMode(VOLTAGE_ADAPTIVE_SENSOR, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(VOLTAGE_ADAPTIVE_SENSOR, HIGH);

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

    delay(1000);
}

unsigned int debounce = 1000;
unsigned long currentMillis = 0;
unsigned long prevRing = 0;

void loop() {
    timer.tick();
    if (ENABLE_SOCKETIO && (WiFi.status() == WL_CONNECTED)) {
        wdt_reset();  // reset timer (feed watchdog)
        webSocket.loop();
    }

    int switchStatus = digitalRead(VOLTAGE_ADAPTIVE_SENSOR);  // read status of switch
    if (switchStatus == LOW) {
        digitalWrite(LED_BUILTIN, LOW);

        currentMillis = millis();
        if (currentMillis - prevRing >= debounce) {
            // Mode 0 : Line Notify, 1: SocketIO
            Serial.println("DingDong " + String(switchStatus == LOW ? "ON" : "OFF") + " Time: " + printLocalTime());
            Serial.println("MODE: " + String(MODE));
            if (MODE == 0) {
                takeSnapshot();
            } else if (MODE == 1) {
                Serial.println("Send message to socketIO");
                // Send message to socketIO
                createResponse(webSocket, true);
            }

            Serial.println("#####################################");
            prevRing = currentMillis;
        }
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}