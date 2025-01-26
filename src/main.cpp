#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include "RestAPI.h"

const char* WIFI_SSID = "";
const char* WIFI_PASS = "";

AsyncWebServer server(80);
RestAPI        api(&server);
Preferences    prefs;

void loadPreferences() {
    prefs.begin("rest-api");
    api["name"]        = prefs.getString("name", "John");
    api["age"]         = prefs.getInt("age", 42);
    api["temperature"] = prefs.getFloat("temperature", 36.6f);
    api["admin"]       = prefs.getBool("admin", true);

    Serial.println("Values loaded");
    for (auto& [key, value] : api) Serial.printf("%s = %s\r\n", key.c_str(), value.as<String>().c_str());
}

void savePreferences() {
    prefs.putString("name", api["name"].as<String>());
    prefs.putInt("age", api["age"].as<int32_t>());
    prefs.putFloat("temperature", api["temperature"].as<float>());
    prefs.putBool("admin", api["admin"].as<bool>());
    Serial.println("Values saved");
}

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

void setupServer() {
    api.begin("/api", "/settings");
    server.on("/save", HTTP_GET, [](AsyncWebServerRequest* req) { savePreferences(); req->send(200, "text/plain", "Values stored"); });
    server.on("/load", HTTP_GET, [](AsyncWebServerRequest* req) { loadPreferences(); req->send(200, "text/plain", "Values loaded"); });
    server.begin();
}

void setup() {
    Serial.begin(115200);

    loadPreferences();
    setupWiFi();
    setupServer();
}

void loop() {
}