/*

  # Author : Watchara Pongsri
  # [github/X-c0d3] https://github.com/X-c0d3/
  # Web Site: https://wwww.rockdevper.com

*/

#ifndef MAKE_RESPONSE_H
#define MAKE_RESPONSE_H

#include <Arduino.h>
// Required 5.13.x becuase compatible with FirebaseArduino
#include <ArduinoJson.h>
#include <SocketIoClient.h>

#include "configs.h"
#include "lineNotify.h"
#include "utility.h"

void createResponse(SocketIoClient& webSocket, bool status) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["deviceName"] = DEVICE_NAME;
    root["deviceId"] = getChipId();
    root["lastUpdated"] = NowString();
    root["ipAddress"] = WiFi.localIP().toString();
    root["wifiSignal"] = wifiSignal();

    JsonObject& data = root.createNestedObject("action");
    data["doorbellStatus"] = status;

    String output;
    root.prettyPrintTo(output);

    // Publish to socket.io server
    if (ENABLE_SOCKETIO)
        webSocket.emit("command", output.c_str());

    if (ENABLE_DEBUG_MODE)
        Serial.print(output);
}

#endif