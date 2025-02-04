#include "RestParameter.h"

RestParameter::RestParameter(const String& key)
    : key(key), uiSchema("") {  }

RestParameter::RestParameter(const String& key, const ArduinoVariant&& value)
    : key(key), value(value), uiSchema("") { }

RestParameter::RestParameter(const String& key, const ArduinoVariant&& value, const String& uiSchema)
    : key(key), value(value), uiSchema(uiSchema) { }

void RestParameter::load(Preferences& pref) {
    value.load(key.c_str(), pref);
}

void RestParameter::save(Preferences& pref) const {
    value.save(key.c_str(), pref);
}

const String RestParameter::type() const {
    if (value.is<bool>()) return "boolean";
    if (value.is<int>() || value.is<float>() || value.is<double>() || value.is<int8_t>() || value.is<uint8_t>() || value.is<int16_t>() || value.is<uint16_t>() || value.is<int32_t>() || value.is<uint32_t>() || value.is<int64_t>() || value.is<uint64_t>()) return "number";
    if (value.is<String>()) return "string";
    return "unknown";
}

bool RestParameter::operator==(RestParameter& other) const {
    if (this == &other) return true;
    return (other.key == key);
}
