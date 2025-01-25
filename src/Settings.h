#pragma once

#include "ArduinoVariant.h"
#include <WString.h>
#include <map>

class Settings {
  public:
    using Iterator = std::map<String, Variant>::iterator;

  public:
    Iterator begin();
    Iterator end();
    Variant& operator[](const char* key);

  protected:
    std::map<String, Variant> settings;
};
