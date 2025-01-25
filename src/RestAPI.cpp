#include "RestAPI.h"

#include <ArduinoJson.h>

static void parseSchema(std::map<String, Variant>& settings, const char* jsonSchema);
static void value2doc(const String& key, Variant& value, JsonDocument& doc);
static void doc2value(const String& key, Variant& value, JsonDocument& doc);
static void jsonVariant2value(JsonVariant& jsonValue, Variant& value);
static void settings2doc(std::map<String, Variant>& settings, JsonDocument& doc);
static void doc2settings(std::map<String, Variant>& settings, JsonDocument& doc);

RestAPI::RestAPI(const char* jsonSchema)
    : jsonSchema(jsonSchema) {
    parseSchema(settings, jsonSchema);
}

Variant& RestAPI::operator[](const char* key) {
    return settings[key];
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
    auto response = request->beginResponseStream("application/Json");

    JsonDocument responseDoc;
    settings2doc(settings, responseDoc);
    serializeJson(responseDoc, *response);

    request->send(response);
}

void RestAPI::elementGET(Variant& value, AsyncWebServerRequest* request) {
    auto response = request->beginResponseStream("application/Json");

    JsonDocument responseDoc;
    value2doc("value", value, responseDoc);
    serializeJson(responseDoc, *response);

    request->send(response);
}

void RestAPI::elementPATCH(Variant& value, AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t offset, size_t total) {
    JsonDocument requestDoc;
    deserializeJson(requestDoc, data, len);

    auto response = request->beginResponseStream("application/json");

    doc2value("value", value, requestDoc);

    JsonDocument responseDoc;
    value2doc("value", value, responseDoc);
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

    for (JsonPair kv : properties) {
        const char* key = kv.key().c_str();
        if (!properties[key]["cppType"]) continue;
        String cppType = properties[key]["cppType"] | "";

        JsonVariant jsonValue = kv.value()["value"];
        if (cppType == "String") settings[key] = jsonValue.as<const char*>();
        if (cppType == "int") settings[key] = jsonValue.as<int>();
        if (cppType == "float") settings[key] = jsonValue.as<float>();
        if (cppType == "double") settings[key] = jsonValue.as<double>();
        if (cppType == "bool") settings[key] = jsonValue.as<bool>();
        if (cppType == "int8_t") settings[key] = jsonValue.as<int8_t>();
        if (cppType == "uint8_t") settings[key] = jsonValue.as<uint8_t>();
        if (cppType == "int16_t") settings[key] = jsonValue.as<int16_t>();
        if (cppType == "uint16_t") settings[key] = jsonValue.as<uint16_t>();
        if (cppType == "int32_t") settings[key] = jsonValue.as<int32_t>();
        if (cppType == "uint32_t") settings[key] = jsonValue.as<uint32_t>();
        if (cppType == "int64_t") settings[key] = jsonValue.as<int64_t>();
        if (cppType == "uint64_t") settings[key] = jsonValue.as<uint64_t>();
    }
}

static void value2doc(const String& key, Variant& value, JsonDocument& doc) {
    if (value.isBool()) doc[key] = value.asBool();
    if (value.isDouble()) doc[key] = value.asDouble();
    if (value.isFloat()) doc[key] = value.asFloat();
    if (value.isString()) doc[key] = value.asString();
    if (value.isInt()) doc[key] = value.asInt();
    if (value.isI8()) doc[key] = value.asI8();
    if (value.isU8()) doc[key] = value.asU8();
    if (value.isI16()) doc[key] = value.asI16();
    if (value.isU16()) doc[key] = value.asU16();
    if (value.isI32()) doc[key] = value.asI32();
    if (value.isU32()) doc[key] = value.asU32();
    if (value.isI64()) doc[key] = value.asI64();
    if (value.isU64()) doc[key] = value.asU64();
}

static void doc2value(const String& key, Variant& value, JsonDocument& doc) {
    if (value.isBool()) value = doc[key].as<bool>();
    if (value.isDouble()) value = doc[key].as<double>();
    if (value.isFloat()) value = doc[key].as<float>();
    if (value.isString()) value = doc[key].as<const char*>();
    if (value.isInt()) value = doc[key].as<int>();
    if (value.isI8()) value = doc[key].as<int8_t>();
    if (value.isU8()) value = doc[key].as<uint8_t>();
    if (value.isI16()) value = doc[key].as<int16_t>();
    if (value.isU16()) value = doc[key].as<uint16_t>();
    if (value.isI32()) value = doc[key].as<int32_t>();
    if (value.isU32()) value = doc[key].as<uint32_t>();
    if (value.isI64()) value = doc[key].as<int64_t>();
    if (value.isU64()) value = doc[key].as<uint64_t>();
}

void jsonVariant2value(JsonVariant& jsonValue, Variant& value) {
    if (value.isString()) value = jsonValue.as<const char*>();
    if (value.isBool()) value = jsonValue.as<bool>();
    if (value.isInt()) value = jsonValue.as<int>();
    if (value.isFloat()) value = jsonValue.as<float>();
    if (value.isDouble()) value = jsonValue.as<double>();
    if (value.isI8()) value = jsonValue.as<int8_t>();
    if (value.isU8()) value = jsonValue.as<uint8_t>();
    if (value.isI16()) value = jsonValue.as<int16_t>();
    if (value.isU16()) value = jsonValue.as<uint16_t>();
    if (value.isI32()) value = jsonValue.as<int32_t>();
    if (value.isU32()) value = jsonValue.as<uint32_t>();
    if (value.isI64()) value = jsonValue.as<int64_t>();
    if (value.isU64()) value = jsonValue.as<uint64_t>();
}

static void settings2doc(std::map<String, Variant>& settings, JsonDocument& doc) {
    for (auto& [key, value] : settings) value2doc(key, value, doc);
}

static void doc2settings(std::map<String, Variant>& settings, JsonDocument& doc) {
    for (JsonPair kv : doc.as<JsonObject>()) {
        const char* key       = kv.key().c_str();
        auto        jsonValue = kv.value();
        Variant&    value     = settings[key];

        jsonVariant2value(jsonValue, value);
    }
}