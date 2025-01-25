#pragma once

#include <ESPAsyncWebServer.h>

#include "Settings.h"

class RestAPI : public Settings {
  public:
    RestAPI(const char* json);
    Variant& operator[](const char* key);

    using Settings::begin;
    using Settings::end;

    void begin(AsyncWebServer* server, const String& baseRoute, const String& pageRoute);

  protected:
    AsyncWebServer* server     = nullptr;
    String          baseRoute  = "";
    String          pageRoute  = "";
    const char*     jsonSchema = nullptr;

  protected:
    void pageGET(AsyncWebServerRequest* request);

    void apiGET(AsyncWebServerRequest* request);
    void apiPATCH(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t offset, size_t total);
    void elementGET(Variant& value, AsyncWebServerRequest* request);
    void elementPATCH(Variant& value, AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t offset, size_t total);
    void jsonSchemaGET(AsyncWebServerRequest* request);

    void setupRoutes();
};
