/*

  # Author : Watchara Pongsri
  # [github/X-c0d3] https://github.com/X-c0d3/
  # Web Site: https://wwww.rockdevper.com

*/

#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#include "ESP8266HTTPClient.h"
#include "ESP8266httpUpdate.h"
#include "configs.h"
#include "lineNotify.h"

int last = 0;
int firmwareUpgradeProgress = 0;
void OnProgress(int progress, int totalt) {
    firmwareUpgradeProgress = (100 * progress) / totalt;
    if (last != firmwareUpgradeProgress && firmwareUpgradeProgress % 10 == 0) {
        // print every 10%
        Serial.println("############## Current Version is " + String(FIRMWARE_VERSION) + " Updating =====>>> " + firmwareUpgradeProgress + "%");
    }
    last = firmwareUpgradeProgress;
}

void updateFirmware() {
    Serial.println("Checking for firmware updates.");
    Serial.println("Current Version: " + String(FIRMWARE_VERSION));

    HTTPClient http;
    BearSSL::WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(API_TIMEOUT);
    http.begin(client, FIRMWARE_SERVER);

    int httpCode = http.GET();
    if (httpCode == 200) {
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(http.getString());

        String newVersion = "";
        String firmwareUrl = "";
        String deviceName = "";
        for (int i = 0; i < (int)root["devices"].size(); i++) {
            if (DEVICE_NAME == root["devices"][i]["deviceName"]) {
                deviceName = root["devices"][i]["deviceName"].as<String>();
                newVersion = root["devices"][i]["version"].as<String>();
                firmwareUrl = root["devices"][i]["firmware"].as<String>();
                break;
            }
        }
        if (newVersion == "") return;
        Serial.println("Device Name: " + deviceName);
        Serial.println("Firmware URL: " + firmwareUrl);

        String currentVersion = String(FIRMWARE_VERSION);
        currentVersion.replace(".", "");  // replace version format from 0.0.1 to 001  for easy to compare
        String nextVersion = newVersion;
        nextVersion.replace(".", "");

        if (nextVersion.toDouble() > currentVersion.toDouble()) {
            Serial.println("##### Found new version: " + String(newVersion));
            Serial.println("##### Current version: " + String(FIRMWARE_VERSION));
            Line_Notify("Start Upgrading new firmware version: " + String(newVersion));
            delay(200);
            Update.onProgress(OnProgress);

            Serial.println("##### Start updating... ");
            t_httpUpdate_return resF = ESPhttpUpdate.update(client, firmwareUrl, String(FIRMWARE_VERSION));
            switch (resF) {
                case HTTP_UPDATE_FAILED:
                    Serial.printf("##### Update faild! (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                    break;
                case HTTP_UPDATE_NO_UPDATES:
                    Serial.println("##### No new update available");
                    break;
                // We can't see this, because of reset chip after update OK
                case HTTP_UPDATE_OK:
                    Serial.println("##### Update success");
                    Serial.println("##### Rebooting...");
                    break;

                default:
                    break;
            }
        }
    }
}

#endif