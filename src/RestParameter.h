#pragma once

#include <Preferences.h>
#include <WString.h>

#include "ArduinoVariant.h"

class RestParameter {
  public:
    RestParameter(const String& key);
    RestParameter(const String& key, const ArduinoVariant&& value);
    RestParameter(const String& key, const ArduinoVariant&& value, const String& uiSchema);

    void load(Preferences& pref);
    void save(Preferences& pref) const;

    const String type() const;

  public:
    String         key;
    ArduinoVariant value;
    String         uiSchema;
};
