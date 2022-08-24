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

unsigned int debounce = 1000;
unsigned long currentMillis = 0;
unsigned long prevRing = 0;
void IRAM_ATTR Ext_INT1_ISR() {
    currentMillis = millis();
    if (currentMillis - prevRing >= debounce) {
        digitalWrite(LED_BUILTIN, LOW);
        // Mode 0 : Line Notify, 1: SocketIO
        Serial.println("DingDong Time: " + printLocalTime());
        Serial.println("MODE: " + String(MODE));
        if (MODE == 0) {
            takeSnapshot();
        } else if (MODE == 1) {
            Serial.println("Send message to socketIO");
            // Send message to socketIO
            createResponse(webSocket, true, 0);
        }

        Serial.println("#####################################");
        prevRing = currentMillis;
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(VOLTAGE_ADAPTIVE_SENSOR, HIGH);
        delay(1000);
    }
}

void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);

    // Note: Voltage adaptive sensor working on logic "LOW"
    pinMode(VOLTAGE_ADAPTIVE_SENSOR, INPUT);
    attachInterrupt(digitalPinToInterrupt(VOLTAGE_ADAPTIVE_SENSOR), Ext_INT1_ISR, FALLING);
    digitalWrite(VOLTAGE_ADAPTIVE_SENSOR, HIGH);
    // attachInterrupt(pin, ISR(callback function), interrupt type/mode)
    // Interrupt type/mode: It defines when the interrupt should be triggered.
    // CHANGE: Used to trigger the interrupt whenever pin value changes.
    // RISING: Used to trigger an interrupt when the pin goes from low to high.
    // FALLING: Used to trigger an interrupt when the pin goes from high to low.

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

        Line_Notify(String(DEVICE_NAME) + " v." + String(FIRMWARE_VERSION) + " - Started... ");
    }

    wdt_enable(10000);

    delay(1000);
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        wdt_reset();  // reset timer (feed watchdog)

        if (ENABLE_SOCKETIO)
            webSocket.loop();
    }
    timer.tick();
}