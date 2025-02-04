#include "RestAPI.h"

#include <ArduinoJson.h>

#include "WebPage.h"

static AsyncResponseStream* beginJsonResponse(AsyncWebServerRequest* request);
static std::vector<String>  splitPath(const String& basePath, AsyncWebServerRequest* req, const char* delimiter = "/");
static void                 doc2value(const String& key, JsonDocument& doc, ArduinoVariant& value);
static void                 value2doc(const String& key, JsonDocument& doc, ArduinoVariant& value);
static void                 NullHandler(AsyncWebServerRequest* request);
static RestParameter*       findParameter(std::vector<RestParameter*>& parameters, const String& name);

RestAPI::RestAPI(AsyncWebServer* server)
    : server(server) {}

RestAPI::RestAPI(AsyncWebServer& server)
    : server(&server) {}

void RestAPI::begin(const String& pageRoute, const String& pageTitle, const String& buttonText, const String& baseRoute) {
    if (!server) return;
    this->pageRoute  = pageRoute;
    this->pageTitle  = pageTitle;
    this->buttonText = buttonText;
    this->baseRoute  = baseRoute.length() ? baseRoute : pageRoute + "/api";
    setupRoutes();
}

void RestAPI::addParameter(RestParameter& parameter) {
    parameters.push_back(&parameter);
}

void RestAPI::addParameter(RestParameter* parameter) {
    parameters.push_back(parameter);
}

void RestAPI::onParameterChange(ParameterChangeHandler handler) { parameterChangeHandler = handler; }

void RestAPI::handlePage(AsyncWebServerRequest* request) {
    request->send(200, "text/html", webPage, [&](const String& key) -> String {
        if (key == "API_ROUTE") return baseRoute;
        if (key == "JSON_SCHEMA_ROUTE") return pageRoute + "/jsonSchema";
        if (key == "PAGE_TITLE") return pageTitle;
        if (key == "BUTTON_TEXT") return buttonText;
        return "";
    });
}

void RestAPI::handleJsonSchema(AsyncWebServerRequest* request) {
    JsonDocument doc;

    for (auto& parameter : parameters) doc["properties"][parameter->key]["type"] = parameter->type();

    auto response = beginJsonResponse(request);
    serializeJson(doc, *response);
    request->send(response);
}

void RestAPI::handleUISchema(AsyncWebServerRequest* request) {
    JsonDocument responseDoc;

    for (auto parameter : parameters) {
        JsonDocument uiSchema;
        if (deserializeJson(uiSchema, parameter->uiSchema) == DeserializationError::Ok) responseDoc[parameter->key]["schema"] = uiSchema.as<JsonObject>();
    }

    auto response = beginJsonResponse(request);
    serializeJson(responseDoc, *response);
    request->send(response);
}

static void setErrorKeyNotFound(JsonDocument& responseDoc, AsyncResponseStream* response, const String& key) {
    response->setCode(404);
    responseDoc["error"] = "'" + key + "' not found";
}

void RestAPI::handleRestGET(AsyncWebServerRequest* req) {
    auto pathElements = splitPath(baseRoute, req);
    auto response     = beginJsonResponse(req);

    JsonDocument responseDoc;

    if (pathElements.size()) {
        const String& key = pathElements[0];

        auto parameter = findParameter(parameters, key);

        if (parameter)
            value2doc("value", responseDoc, parameter->value);
        else
            setErrorKeyNotFound(responseDoc, response, key);
    } else {
        for (auto parameter : parameters) {
            const String& key = parameter->key;
            value2doc(key, responseDoc, parameter->value);
        }
    }

    serializeJson(responseDoc, *response);
    req->send(response);
}

void RestAPI::handleRestPATCH(AsyncWebServerRequest* req, uint8_t* data, size_t size, size_t offset, size_t total) {
    auto pathElements = splitPath(baseRoute, req);

    JsonDocument requestDoc;
    auto         jsonError = deserializeJson(requestDoc, data, size);

    JsonDocument responseDoc;
    auto         response = beginJsonResponse(req);

    if (jsonError) {
        responseDoc["error"] = jsonError.c_str();
        serializeJson(responseDoc, *response);
        req->send(response);
        return;
    }

    if (pathElements.size()) {
        const String& key = pathElements[0];

        auto parameter = findParameter(parameters, key);

        if (parameter) {
            doc2value("value", requestDoc, parameter->value);
            value2doc("value", responseDoc, parameter->value);
            if (parameterChangeHandler) parameterChangeHandler(*parameter);
        } else
            setErrorKeyNotFound(responseDoc, response, key);
    } else {
        for (auto jsonPair : requestDoc.as<JsonObject>()) {
            auto key       = jsonPair.key().c_str();
            auto parameter = findParameter(parameters, key);
            if (parameter) {
                doc2value(key, requestDoc, parameter->value);
            }
            if (parameterChangeHandler) parameterChangeHandler(*parameter);
        }
    }

    serializeJson(responseDoc, *response);
    req->send(response);
}

void RestAPI::handleRestDELETE(AsyncWebServerRequest* req) {
    auto pathElements = splitPath(baseRoute, req);

    JsonDocument responseDoc;
    auto         response = beginJsonResponse(req);

    if (pathElements.size()) {
        const String& key       = pathElements[0];
        auto          parameter = findParameter(parameters, key);

        if (parameter) {
            parameter->value.clear();
            value2doc(key, responseDoc, parameter->value);
            if (parameterChangeHandler) parameterChangeHandler(*parameter);
        } else {
            setErrorKeyNotFound(responseDoc, response, key);
        }
    } else {
        for (auto parameter : parameters) {
            parameter->value.clear();
            value2doc(parameter->key, responseDoc, parameter->value);
            if (parameterChangeHandler) parameterChangeHandler(*parameter);
        }
    }

    serializeJson(responseDoc, *response);
    req->send(response);
}

void RestAPI::setupRoutes() {
    if (!server) return;

    String jsonSchemaRoute = pageRoute + "/jsonSchema";
    String uiSchemaRoute   = pageRoute + "/uiSchema";

    server->on(jsonSchemaRoute.c_str(), HTTP_GET, std::bind(&RestAPI::handleJsonSchema, this, std::placeholders::_1));
    server->on(uiSchemaRoute.c_str(), HTTP_GET, std::bind(&RestAPI::handleUISchema, this, std::placeholders::_1));

    const char* baseRoute_str = baseRoute.c_str();
    server->on(baseRoute_str, HTTP_GET, std::bind(&RestAPI::handleRestGET, this, std::placeholders::_1));
    server->on(baseRoute_str, HTTP_PATCH | HTTP_POST | HTTP_PUT, NullHandler, nullptr, std::bind(&RestAPI::handleRestPATCH, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    server->on(baseRoute_str, HTTP_DELETE, std::bind(&RestAPI::handleRestDELETE, this, std::placeholders::_1));

    server->on(pageRoute.c_str(), HTTP_GET, std::bind(&RestAPI::handlePage, this, std::placeholders::_1));
}

// Helper functions

static AsyncResponseStream* beginJsonResponse(AsyncWebServerRequest* request) {
    return request->beginResponseStream("application/json");
};

static std::vector<String> splitPath(const String& baseRoute, AsyncWebServerRequest* req, const char* delimiter) {
    std::vector<String> result;

    if (!req->url().startsWith(baseRoute)) return result;

    String remainingURL = req->url().substring(baseRoute.length());

    const char* token = strtok((char*)remainingURL.c_str(), delimiter);
    while (token != nullptr) {
        result.push_back(token);
        token = strtok(nullptr, delimiter);
    }

    return result;
}

template <typename... Types>
static void doc2valueImpl(const String& key, JsonDocument& doc, ArduinoVariant& value, std::tuple<Types...>) {
    auto assignValue = [&](auto type) {
        using T = decltype(type);
        if (value.is<T>()) value = doc[key].as<T>();
    };
    (assignValue(Types{}), ...);
}

static void doc2value(const String& key, JsonDocument& doc, ArduinoVariant& value) {
    doc2valueImpl(key, doc, value, std::tuple<bool, double, float, String, int, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>{});
}

template <typename... Types>
static void value2docImpl(const String& key, JsonDocument& doc, ArduinoVariant& value, std::tuple<Types...>) {
    auto assignValue = [&](auto type) {
        using T = decltype(type);
        if (value.is<T>()) doc[key] = value.as<T>();
    };
    (assignValue(Types{}), ...);
}

static void value2doc(const String& key, JsonDocument& doc, ArduinoVariant& value) {
    value2docImpl(key, doc, value, std::tuple<bool, double, float, String, int, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>{});
}

static void NullHandler(AsyncWebServerRequest* req) {}

static RestParameter* findParameter(std::vector<RestParameter*>& parameters, const String& name) {
    for (auto parameter : parameters)
        if (parameter->key.equalsIgnoreCase(name)) return parameter;
    return nullptr;
}
