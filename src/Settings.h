#pragma once

#include "ArduinoVariant.h"
#include <WString.h>
#include <vector>

class Settings {
  public:
    // using Iterator = std::map<String, Variant>::iterator;
    using Iterator = std::vector<std::pair<String, Variant>>::iterator;

  public:
    Iterator begin();
    Iterator end();
    Variant& operator[](const char* key);

  protected:
  std::vector<std::pair<String, Variant>> settings;
    // std::map<String, Variant> settings;
};
