#pragma once

#if (__cplusplus < 201703L)
#error "This library requires C++17 / Espressif32 Arduino 3.x"
#else

#include <ESPAsyncWebServer.h>

#include "ArduinoVariant.h"
#include <Preferences.h>
#include <RestParameter.h>

class RestAPI {
  public:
    using Req                    = AsyncWebServerRequest*;
    using ParameterChangeHandler = std::function<void(RestParameter& parameter)>;

  public:
    RestAPI(AsyncWebServer& server);
    RestAPI(AsyncWebServer* server);

    void addParameter(RestParameter& parameter);
    void addParameter(RestParameter* parameter);

    void begin(const String& baseRoute, const String& pageTitle, const String& buttonText);

    void onParameterChange(ParameterChangeHandler handler);

  protected:
    AsyncWebServer* server     = nullptr;
    String          baseRoute  = "";
    String          formRoute  = "";
    String          apiRoute   = "";
    String          pageTitle  = "Configuration";
    String          buttonText = "Send";

    ParameterChangeHandler parameterChangeHandler = nullptr;

    std::vector<RestParameter*> parameters;

  protected:

    void handlePage(Req request);

    void handleFormGET(Req request);
    void handleFormPOST(AsyncWebServerRequest* req, uint8_t* data, size_t size, size_t offset, size_t total);

    void handleRestGET(Req);
    void handleRestPATCH(Req req, uint8_t* data, size_t len, size_t offest, size_t total);
    void handleRestDELETE(Req);

    void setupRoutes();
};

#endif