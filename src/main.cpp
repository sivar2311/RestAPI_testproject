#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>

#include "RestAPI.h"
#include "WebPage.h"

const char* WIFI_SSID = "";
const char* WIFI_PASS = "";

AsyncWebServer server(80);
RestAPI        api(server);
Preferences    prefs;

RestParameter parameters[] = {
    {"Username", "Your-Username", R"({"ui::focus" : true})"},
    {"Password", "Your-Password"},
};

void loadParameters() {
    prefs.begin("rest-api");

    for (auto& parameter : parameters) {
        parameter.load(prefs);
        Serial.printf("Parameter \"%s\" has been loaded\r\n", parameter.key.c_str());
    }
}

void addParameters(RestAPI& api) {
    for (auto& parameter : parameters) api.addParameter(parameter);
}

void handleParameterChange(RestParameter& parameter) {
    parameter.save(prefs);

    Serial.printf("Parameter \"%s\" changed to %s and has been saved.\r\n", parameter.key.c_str(), parameter.value.as<String>().c_str());
}

void setupWiFi() {
    Serial.print("Connecting Wifi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
        if (millis() - start > 10000) {
            WiFi.softAP("ESP32", "");
            break;
        }
    }
    Serial.println("connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void setupServer() {
    addParameters(api);

    api.onParameterChange(handleParameterChange);
    api.begin("/user", "Einstellungen", "Senden");

    server.begin();

    Serial.printf("Open your browser and navigate to: http://%s/user\r\n", WiFi.localIP().toString().c_str());
}

void setup() {
    Serial.begin(115200);

    loadParameters();
    setupWiFi();
    setupServer();
}

void loop() {
    Serial.printf("alive (%d)\r", millis() / 1000);
    delay(1000);
}