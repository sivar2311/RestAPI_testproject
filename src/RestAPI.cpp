#include "RestAPI.h"

#include <ArduinoJson.h>

#include <variant>

static void                 value2doc(Variant& value, JsonDocument& doc, const String& key = "value");
static void                 doc2value(Variant& value, JsonDocument& doc, const String& key = "value");
static void                 jsonVariant2value(JsonVariant& jsonValue, Variant& value);
static void                 settings2doc(std::vector<std::pair<String, Variant>>& settings, JsonDocument& doc);
static void                 doc2settings(std::vector<std::pair<String, Variant>>& settings, JsonDocument& doc);
static AsyncResponseStream* beginJsonResponse(AsyncWebServerRequest* request);

RestAPI::RestAPI(AsyncWebServer* server)
    : server(server) {}

void RestAPI::begin(const String& baseRoute, const String& pageRoute) {
    if (!server) return;
    this->baseRoute = baseRoute;
    this->pageRoute = pageRoute;
    setupRoutes();
}

void RestAPI::pageGET(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "not yet implemented");
}

void RestAPI::apiGET(AsyncWebServerRequest* request) {
    auto response = beginJsonResponse(request);

    JsonDocument responseDoc;
    settings2doc(settings, responseDoc);
    serializeJson(responseDoc, *response);

    request->send(response);
}

void RestAPI::elementGET(Variant& value, AsyncWebServerRequest* request) {
    auto response = beginJsonResponse(request);

    JsonDocument responseDoc;
    value2doc(value, responseDoc);
    serializeJson(responseDoc, *response);

    request->send(response);
}

void RestAPI::elementPATCH(Variant& value, AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t offset, size_t total) {
    JsonDocument requestDoc;
    deserializeJson(requestDoc, data, len);

    auto response = beginJsonResponse(request);

    doc2value(value, requestDoc);

    JsonDocument responseDoc;
    value2doc(value, responseDoc);
    serializeJson(responseDoc, *response);
    request->send(response);
}

void RestAPI::apiPATCH(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t offset, size_t total) {
    JsonDocument requestDoc;
    deserializeJson(requestDoc, data, len);
    doc2settings(settings, requestDoc);

    apiGET(request);
}

void RestAPI::jsonSchemaGET(AsyncWebServerRequest* request) {
    JsonDocument doc;

    for (auto& [key, value] : settings) {
        String type = "unknown";

        if (value.is<bool>()) type = "boolean";
        if (value.is<int>() || value.is<float>() || value.is<double>() || value.is<int8_t>() || value.is<uint8_t>() || value.is<int16_t>() || value.is<uint16_t>() || value.is<int32_t>() || value.is<uint32_t>() || value.is<int64_t>() || value.is<uint64_t>()) type = "number";
        if (value.is<String>()) type = "string";

        doc["properties"][key]["type"] = type;
    }
    auto response = beginJsonResponse(request);
    serializeJson(doc, *response);
    request->send(response);
}

void RestAPI::setupRoutes() {
    if (!server) return;

    for (auto& [key, value] : settings) {
        String route = baseRoute + "/" + key;
        server->on(route.c_str(), HTTP_GET, std::bind(&RestAPI::elementGET, this, std::ref(value), std::placeholders::_1));
        server->on(route.c_str(), HTTP_PATCH, [](AsyncWebServerRequest*) {}, nullptr, std::bind(&RestAPI::elementPATCH, this, std::ref(value), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    }

    server->on(pageRoute.c_str(), HTTP_GET, std::bind(&RestAPI::pageGET, this, std::placeholders::_1));
    server->on(String(baseRoute + "/jsonSchema").c_str(), HTTP_GET, std::bind(&RestAPI::jsonSchemaGET, this, std::placeholders::_1));
    server->on(baseRoute.c_str(), HTTP_GET, std::bind(&RestAPI::apiGET, this, std::placeholders::_1));
    server->on(baseRoute.c_str(), HTTP_PATCH, [](AsyncWebServerRequest*) {}, nullptr, std::bind(&RestAPI::apiPATCH, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
}

template <typename... Types>
void value2docImpl(Variant& value, JsonDocument& doc, std::tuple<Types...>, const String& key) {
    auto assignValue = [&](auto type) {
        using T = decltype(type);
        if (value.is<T>()) {
            doc[key] = value.as<T>();
        }
    };
    (assignValue(Types{}), ...);
}

static void value2doc(Variant& value, JsonDocument& doc, const String& key) {
    value2docImpl(value, doc, std::tuple<bool, double, float, String, int, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>{}, key);
}

template <typename... Types>
void doc2valueImpl(Variant& value, JsonDocument& doc, std::tuple<Types...>, const String& key) {
    auto assignValue = [&](auto type) {
        using T = decltype(type);
        if (value.is<T>()) {
            value = doc[key].as<T>();
        }
    };
    (assignValue(Types{}), ...);
}

static void doc2value(Variant& value, JsonDocument& doc, const String& key) {
    doc2valueImpl(value, doc, std::tuple<bool, double, float, String, int, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>{}, key);
}

template <typename... Types>
void jsonVariant2valueImpl(JsonVariant& jsonValue, Variant& value, std::tuple<Types...>) {
    auto assignValue = [&](auto type) {
        using T = decltype(type);
        if (value.is<T>()) {
            value = jsonValue.as<T>();
        }
    };
    (assignValue(Types{}), ...);
}

void jsonVariant2value(JsonVariant& jsonValue, Variant& value) {
    jsonVariant2valueImpl(jsonValue, value, std::tuple<String, bool, int, float, double, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>{});
}

static void settings2doc(std::vector<std::pair<String, Variant>>& settings, JsonDocument& doc) {
    for (auto& [key, value] : settings) value2doc(value, doc, key);
}

static void doc2settings(std::vector<std::pair<String, Variant>>& settings, JsonDocument& doc) {
    for (JsonPair kv : doc.as<JsonObject>()) {
        const char* key = kv.key().c_str();
        JsonVariant jsonValue = kv.value();

        for (auto& setting : settings) {
            if (setting.first == key) {
                Variant& value = setting.second;
                jsonVariant2value(jsonValue, value);
            }
        }
    }
}

static AsyncResponseStream* beginJsonResponse(AsyncWebServerRequest* request) {
    return request->beginResponseStream("application/json");
};