#include "RestAPI.h"

#include <ArduinoJson.h>

static void parseSchema(std::map<String, Variant>& settings, const char* jsonSchema);
static void value2doc(Variant& value, JsonDocument& doc, const String& key = "value");
static void doc2value(Variant& value, JsonDocument& doc, const String& key = "value");
static void jsonVariant2value(JsonVariant& jsonValue, Variant& value);
static void settings2doc(std::map<String, Variant>& settings, JsonDocument& doc);
static void doc2settings(std::map<String, Variant>& settings, JsonDocument& doc);
static AsyncResponseStream* beginJsonResponse(AsyncWebServerRequest* request);

RestAPI::RestAPI(const char* jsonSchema)
    : jsonSchema(jsonSchema) {
    parseSchema(settings, jsonSchema);
}

void RestAPI::begin(AsyncWebServer* server, const String& baseRoute, const String& pageRoute) {
    if (!server) return;
    this->server    = server;
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
    request->send(200, "application/Json", jsonSchema);
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

static void parseSchema(std::map<String, Variant>& settings, const char* jsonSchema) {
    JsonDocument doc;
    deserializeJson(doc, jsonSchema);

    JsonObject properties = doc["properties"];

    std::map<String, std::function<void(const JsonVariant&, Variant&)>> typeMap = {
        {"String", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<const char*>(); }},
        {"int", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<int>(); }},
        {"float", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<float>(); }},
        {"double", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<double>(); }},
        {"bool", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<bool>(); }},
        {"int8_t", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<int8_t>(); }},
        {"uint8_t", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<uint8_t>(); }},
        {"int16_t", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<int16_t>(); }},
        {"uint16_t", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<uint16_t>(); }},
        {"int32_t", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<int32_t>(); }},
        {"uint32_t", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<uint32_t>(); }},
        {"int64_t", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<int64_t>(); }},
        {"uint64_t", [](const JsonVariant& jsonValue, Variant& value) { value = jsonValue.as<uint64_t>(); }},
    };

    for (JsonPair kv : properties) {
        const char* key = kv.key().c_str();
        if (!properties[key]["cppType"]) continue;
        String cppType = properties[key]["cppType"] | "";

        JsonVariant jsonValue = kv.value()["value"];
        if (typeMap.find(cppType) != typeMap.end()) {
            typeMap[cppType](jsonValue, settings[key]);
        }
    }
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
    value2docImpl(value, doc, std::tuple<
        bool, double, float, String, int, 
        int8_t, uint8_t, int16_t, uint16_t, 
        int32_t, uint32_t, int64_t, uint64_t
    >{}, key);
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
    doc2valueImpl(value, doc, std::tuple<
        bool, double, float, String, int, 
        int8_t, uint8_t, int16_t, uint16_t, 
        int32_t, uint32_t, int64_t, uint64_t
    >{}, key);
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
    jsonVariant2valueImpl(jsonValue, value, std::tuple<
        String, bool, int, float, double, 
        int8_t, uint8_t, int16_t, uint16_t, 
        int32_t, uint32_t, int64_t, uint64_t
    >{});
}

static void settings2doc(std::map<String, Variant>& settings, JsonDocument& doc) {
    for (auto& [key, value] : settings) value2doc(value, doc, key);
}

static void doc2settings(std::map<String, Variant>& settings, JsonDocument& doc) {
    for (JsonPair kv : doc.as<JsonObject>()) {
        const char* key       = kv.key().c_str();
        auto        jsonValue = kv.value();
        Variant&    value     = settings[key];

        jsonVariant2value(jsonValue, value);
    }
}

static AsyncResponseStream* beginJsonResponse(AsyncWebServerRequest* request) {
    return request->beginResponseStream("application/json");
};