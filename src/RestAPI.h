#pragma once

#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#include "RestParameter.h"

class RestAPI {
  public:
    using Req                    = AsyncWebServerRequest*;
    using ParameterChangeHandler = std::function<void(RestParameter& parameter)>;

  public:
    RestAPI(AsyncWebServer* server);
    RestAPI(AsyncWebServer& server);

    void addParameter(RestParameter* parameter);
    void addParameter(RestParameter& parameter);

    void begin(const String& pageRoute, const String& pageTitle, const String& buttonText, const String& baseRoute = "");

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
    void handlePage(Req request);
    void handleJsonSchema(Req request);
    void handleUISchema(Req request);

    void handleRestGET(Req);
    void handleRestPATCH(Req req, uint8_t* data, size_t len, size_t offest, size_t total);
    void handleRestDELETE(Req);

    void setupRoutes();
};