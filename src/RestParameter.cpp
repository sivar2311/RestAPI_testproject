#include "RestParameter.h"

RestParameter::RestParameter(const String& key)
    : key(key), uiSchema("") {}

RestParameter::RestParameter(const String& key, const ArduinoVariant&& value)
    : key(key), value(value), uiSchema("") {}

RestParameter::RestParameter(const String& key, const ArduinoVariant&& value, const String& uiSchema)
    : key(key), value(value), uiSchema(uiSchema) {}

void RestParameter::load(Preferences& pref) {
    value.load(key.c_str(), pref);
}

void RestParameter::save(Preferences& pref) {
    value.save(key.c_str(), pref);
}
