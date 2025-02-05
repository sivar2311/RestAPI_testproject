#pragma once

#include <Preferences.h>
#include <WString.h>

#include "ArduinoVariant.h"

class RestParameter {
  public:
    struct MinMax {
        ArduinoVariant min = {};
        ArduinoVariant max = {};
    };

    static const bool Password = true;

  public:
    RestParameter(const String& key);
    RestParameter(const String& key, const ArduinoVariant&& value);
    RestParameter(const String& key, const ArduinoVariant&& value, bool isPassword);
    RestParameter(const String& key, const ArduinoVariant&& value, const MinMax&& minMax);

    void load(Preferences& pref);
    void save(Preferences& pref) const;

    const String type() const;

    bool operator==(RestParameter& other) const;

    bool isNumber() const;
    bool isString() const;
    bool isBool() const;

  public:
    String         key;
    ArduinoVariant value;
    ArduinoVariant min        = {};
    ArduinoVariant max        = {};
    bool           isPassword = false;
};
