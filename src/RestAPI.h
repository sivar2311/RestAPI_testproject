#pragma once

#if (__cplusplus < 201703L)
#error "This library requires C++17 / Espressif32 Arduino 3.x"
#else

#include <ESPAsyncWebServer.h>

// #include "ArduinoVariant.h"
#include <Preferences.h>
#include <RestParameter.h>

class RestAPI {
  public:
    using Req                    = AsyncWebServerRequest*;
    using ParameterChangeHandler = std::function<void(RestParameter& parameter)>;

  public:
    RestAPI(AsyncWebServer* server);

    void addParameter(RestParameter* parameter);

    void begin(const String& baseRoute, const String& pageRoute, const String& pageTitle, const String& buttonText);

    void onParameterChange(ParameterChangeHandler handler);

  protected:
    AsyncWebServer* server     = nullptr;
    String          baseRoute  = "";
    String          pageRoute  = "";
    String          pageTitle  = "Configuration";
    String          buttonText = "Send";

    ParameterChangeHandler parameterChangeHandler = nullptr;

    std::vector<RestParameter*> parameters;

  protected:
    void pageGET(Req request);
    void jsonSchemaGET(Req request);
    void uiSchemaGET(Req request);

    void handleGET(Req);
    void handlePATCH(Req req, uint8_t* data, size_t len, size_t offest, size_t total);
    void handleDELETE(Req);

    void setupRoutes();
};

#endif