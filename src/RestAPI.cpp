#if (__cplusplus < 201703L)
#error "This library requires C++17 / Espressif32 Arduino 3.x"
#else

#include "RestAPI.h"

#include <ArduinoJson.h>
#include <AsyncJson.h>

#include <variant>
static AsyncResponseStream* beginJsonResponse(AsyncWebServerRequest* request);
static std::vector<String>  splitPath(const String& basePath, AsyncWebServerRequest* req, const char* delimiter = "/");
static void                 value2json(ArduinoVariant& value, JsonVariant&& json);
static void                 doc2value(const String& key, JsonDocument& doc, ArduinoVariant& value);
static void                 value2doc(const String& key, JsonDocument& doc, ArduinoVariant& value);
static void                 NullHandler(AsyncWebServerRequest* request);
static RestParameter*       findParameter(std::vector<RestParameter*>& parameters, const String& name);

RestAPI::RestAPI(AsyncWebServer* server)
    : server(server) {}

RestAPI::RestAPI(AsyncWebServer& server)
    : server(&server) {}

void RestAPI::begin(const String& baseRoute, const String& pageTitle, const String& buttonText) {
    if (!server) return;
    this->baseRoute  = baseRoute;
    this->formRoute  = baseRoute + "/form";
    this->apiRoute   = baseRoute + "/api";
    this->pageTitle  = pageTitle;
    this->buttonText = buttonText;
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
    extern const char* webPage;
    request->send(200, "text/html", webPage, [&](const String& key) -> String {
        if (key == "FORM_ROUTE") return baseRoute + "/form";
        if (key == "PAGE_TITLE") return pageTitle;
        if (key == "BUTTON_TEXT") return buttonText;
        return "";
    });
}

void RestAPI::handleFormGET(AsyncWebServerRequest* request) {
    JsonDocument doc;

    JsonObject props = doc.to<JsonObject>();

    for (auto& parameter : parameters) {
        JsonObject element = props[parameter->key].to<JsonObject>();
        element["type"]    = parameter->type();
        value2json(parameter->value, element["value"].to<JsonVariant>());
        if (parameter->min.isValid()) value2json(parameter->min, element["min"].to<JsonVariant>());
        if (parameter->max.isValid()) value2json(parameter->max, element["max"].to<JsonVariant>());
        if (parameter->isString() && parameter->isPassword) element["password"] = true;
    }

    auto response = beginJsonResponse(request);
    serializeJson(doc, *response);
    request->send(response);
}

static void setErrorKeyNotFound(JsonDocument& responseDoc, AsyncResponseStream* response, const String& key) {
    response->setCode(404);
    responseDoc["error"] = "'" + key + "' not found";
}

void RestAPI::handleRestGET(AsyncWebServerRequest* req) {
    auto pathElements = splitPath(apiRoute, req);
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

void RestAPI::handleFormPOST(AsyncWebServerRequest* req, uint8_t* data, size_t size, size_t offset, size_t total) {
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

    for (auto jsonPair : requestDoc.as<JsonObject>()) {
        auto key       = jsonPair.key().c_str();
        auto parameter = findParameter(parameters, key);
        if (parameter) {
            doc2value(key, requestDoc, parameter->value);
            value2doc(key, responseDoc, parameter->value);
        }
        if (parameterChangeHandler) parameterChangeHandler(*parameter);
    }

    serializeJson(responseDoc, *response);
    req->send(response);    
}

void RestAPI::handleRestPATCH(AsyncWebServerRequest* req, uint8_t* data, size_t size, size_t offset, size_t total) {
    auto pathElements = splitPath(apiRoute, req);

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
        Serial.println("im /user/api/element zweig");
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
                value2doc(key, responseDoc, parameter->value);
            }
            if (parameterChangeHandler) parameterChangeHandler(*parameter);
        }
    }

    serializeJson(responseDoc, *response);
    req->send(response);
}

void RestAPI::handleRestDELETE(AsyncWebServerRequest* req) {
    auto pathElements = splitPath(apiRoute, req);

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

    server->on(formRoute.c_str(), HTTP_GET | HTTP_POST, std::bind(&RestAPI::handleFormGET, this, std::placeholders::_1), nullptr, std::bind(&RestAPI::handleFormPOST, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    server->on(apiRoute.c_str(), HTTP_GET, std::bind(&RestAPI::handleRestGET, this, std::placeholders::_1));
    server->on(apiRoute.c_str(), HTTP_PATCH | HTTP_POST | HTTP_PUT, NullHandler, nullptr, std::bind(&RestAPI::handleRestPATCH, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    server->on(apiRoute.c_str(), HTTP_DELETE, std::bind(&RestAPI::handleRestDELETE, this, std::placeholders::_1));

    server->on(baseRoute.c_str(), HTTP_GET, std::bind(&RestAPI::handlePage, this, std::placeholders::_1));
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
    doc2valueImpl(key, doc, value, ArduinoVariant::VariantTuple{});
}

template <typename... Types>
static void value2jsonImpl(ArduinoVariant& value, JsonVariant& json, std::tuple<Types...>) {
    auto assignValue = [&](auto type) {
        using T = decltype(type);
        if (value.is<T>()) json.set(static_cast<T>(value));
    };
    (assignValue(Types{}), ...);
}

static void value2json(ArduinoVariant& value, JsonVariant&& json) {
    value2jsonImpl(value, json, ArduinoVariant::VariantTuple{});
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
    value2docImpl(key, doc, value, ArduinoVariant::VariantTuple{});
}

static void NullHandler(AsyncWebServerRequest* req) {}

static RestParameter* findParameter(std::vector<RestParameter*>& parameters, const String& name) {
    for (auto parameter : parameters)
        if (parameter->key.equalsIgnoreCase(name)) return parameter;
    return nullptr;
}

#endif