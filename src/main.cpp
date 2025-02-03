#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>

#include "RestAPI.h"
#include "WebPage.h"

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

AsyncWebServer server(80);
RestAPI        api(&server);
Preferences    prefs;

RestParameter parameters[] = {
    {"Name", "John"},
    {"Alter", 46},
    {"Temperatur", 33.6f},
    {"Admin", false}};

void loadSettings() {
    prefs.begin("rest-api");

    for (auto& parameter : parameters) parameter.load(prefs);

    Serial.println("Values loaded");
}

void saveSettings() {
    for (auto& parameter : parameters) parameter.save(prefs);

    Serial.println("Values saved");
}

void addParameters(RestAPI& api) {
    for (auto& parameter : parameters) api.addParameter(&parameter);
}

void setupWiFi() {
    Serial.print("Connecting Wifi");
    WiFi.begin(WIFI_SSID, WIFI_PASS, 6);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void setupServer() {
    api.begin("/api/v1", "/settings", "Einstellungen", "Senden");

    addParameters(api);

    api.onParameterChange([](RestParameter& param) {
        Serial.printf("%s changed to %s\r\n", param.key.c_str(), param.value.as<String>().c_str());
        param.save(prefs);
    });

    server.begin();
}

void setup() {
    Serial.begin(115200);

    loadSettings();
    setupWiFi();
    setupServer();
}

void loop() {
    Serial.printf("%d alive\r\n", millis());
    delay(1000);
}