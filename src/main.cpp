#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#include "RestAPI.h"

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const int   WIFI_CHAN = 6;

AsyncWebServer server(80);

void setupWiFi() {
    Serial.print("Connecting Wifi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

const char* jsonSchema = R"json({
  "properties": {
    "name": {
      "type": "string",
      "cppType": "String"
    },
    "age": {
      "type": "number",
      "cppType": "uint8_t"
    },
    "temperature": {
      "type": "number", 
      "cppType": "float"
    },
    "admin" : {
      "type" : "checkbox",
      "cppType": "bool"
    }
  }
})json";

RestAPI api(jsonSchema);

void setupServer() {
    api.begin(&server, "/api", "/settings");
    server.begin();
}

void loadValues() {
    // kann später mit Preferences genutzt werden um Werte zu laden
    api["name"]        = "John";
    api["age"]         = 42;
    api["temperature"] = 36.6;

    // alternative Möglichkeit der Zuweisung:
    auto& admin = api["admin"];
    admin       = true;
}

void setup() {
    Serial.begin(115200);

    loadValues();

    setupWiFi();
    setupServer();
}

void loop() {
    Serial.println("alive");
    delay(1000);
}