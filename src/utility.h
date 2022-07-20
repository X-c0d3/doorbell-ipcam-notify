/*

  # Author : Watchara Pongsri
  # [github/X-c0d3] https://github.com/X-c0d3/
  # Web Site: https://wwww.rockdevper.com

*/

#ifndef UTILITY_H
#define UTILITY_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureAxTLS.h>

#include "configs.h"
#include "time.h"
char ntp_server1[20] = "pool.ntp.org";
char ntp_server2[20] = "time.nist.gov";
char ntp_server3[20] = "time.uni.net.th";

int timezone = 7 * 3600;  // timezone for Thailand is +7
int dst = 0;
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
String printLocalTime() {
    time_t now = time(nullptr);
    struct tm* ptm = localtime(&now);

    String tmpNow = weekDays[ptm->tm_wday];
    tmpNow += ", ";
    tmpNow += String(ptm->tm_mday);
    tmpNow += "/";
    // tmpNow += String(ptm->tm_mon + 1);
    tmpNow += String(months[ptm->tm_mon]);
    tmpNow += "/";
    tmpNow += String(ptm->tm_year + 1900);
    tmpNow += " ";
    tmpNow += String(ptm->tm_hour);
    tmpNow += ":";
    tmpNow += String(ptm->tm_min);
    tmpNow += ":";
    tmpNow += String(ptm->tm_sec);
    return tmpNow;
}

void printMessage(String message, bool isPrintLn) {
    if (isPrintLn)
        Serial.println(message);
    else
        Serial.print(message);
}

void setup_Wifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    if (WiFi.getMode() & WIFI_AP) {
        WiFi.softAPdisconnect(true);
    }

    Serial.println();
    printMessage("WIFI Connecting...", true);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // setup_IpAddress();

    Serial.println();
    Serial.print("WIFI Connected ");
    String ip = WiFi.localIP().toString();
    Serial.println(ip.c_str());
    Serial.println("Socket.io Server: " + String(SOCKETIO_HOST));
    Serial.println();
}

String NowString() {
    time_t now = time(nullptr);
    struct tm* newtime = localtime(&now);

    String tmpNow = "";
    tmpNow += String(newtime->tm_hour);
    tmpNow += ":";
    tmpNow += String(newtime->tm_min);
    tmpNow += ":";
    tmpNow += String(newtime->tm_sec);
    return tmpNow;
}

void setupTimeZone() {
    for (int i = 0; i <= 10; i++) {
        configTime(timezone, dst, ntp_server1, ntp_server2, ntp_server3);
        Serial.println("Waiting for time => " + String(i));
        while (!time(nullptr)) {
            Serial.print(".");
            delay(1000);
        }
        time_t now = time(nullptr);
        if (i == 10) {
            Serial.println();
            Serial.println(ctime(&now));
        }
        delay(1000);
    }
}

String getSplitValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }

    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String getChipId() {
    String ChipIdHex = String((uint32_t)(ESP.getChipId() >> 32), HEX);
    ChipIdHex += String((uint32_t)ESP.getChipId(), HEX);
    return ChipIdHex;
}

long rssi;
long wifiSignal() {
    // WifiSignalStrength
    // Convert to scale -48 to 0 eg. map(rssi, -100, 0, 0, -48);
    // I used -100 instead of -120 because <= -95 is unusable
    // Negative number so we can draw n pixels from the bottom in black
    rssi = WiFi.RSSI();  // eg. -63
    if (rssi < -99) {
        rssi = -99;
    }
    // Serial.println("WifiSignal: " + String(rssi) + "db");
    return rssi;
}

int digits(int x) {
    return ((bool)x * (int)log10(abs(x)) + 1);
}

#endif