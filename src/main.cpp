#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>

#include "RestAPI.h"
#include "WebPage.h"

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

AsyncWebServer server(80);
RestAPI        api(server);
Preferences    prefs;

RestParameter parameters[] = {
    {"Username", "Your-Username"},
    {"Password", "Your-Password", RestParameter::Password},
    {"Range", 45, {-2.2f, 62}},
    {"Number", 32},
};

auto& [username, password, range, number] = parameters;


void loadParameters() {
    prefs.begin("rest-api");
    for (auto& parameter : parameters) parameter.load(prefs);
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
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void setupServer() {
    addParameters(api);

    api.onParameterChange(handleParameterChange);
    api.begin("/user", "User", "save");

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