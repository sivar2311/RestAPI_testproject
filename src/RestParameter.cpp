#include "RestParameter.h"

RestParameter::RestParameter(const String& key)
    : key(key) {}

RestParameter::RestParameter(const String& key, const ArduinoVariant&& value)
    : key(key), value(value) {}

RestParameter::RestParameter(const String& key, const ArduinoVariant&& value, bool isPassword)
: key(key), value(value), isPassword(isPassword) {}

RestParameter::RestParameter(const String& key, const ArduinoVariant&& value, const MinMax&& minMax)
    : key(key), value(value), min(minMax.min), max(minMax.max) {}

void RestParameter::load(Preferences& pref) {
    value.load(key.c_str(), pref);
}

void RestParameter::save(Preferences& pref) const {
    value.save(key.c_str(), pref);
}

bool RestParameter::isNumber() const {
    return (value.is<int>() || value.is<float>() || value.is<double>() || value.is<int8_t>() || value.is<uint8_t>() || value.is<int16_t>() || value.is<uint16_t>() || value.is<int32_t>() || value.is<uint32_t>() || value.is<int64_t>() || value.is<uint64_t>());
}

bool RestParameter::isString() const {
    return (value.is<String>());
}

bool RestParameter::isBool() const {
    return value.is<bool>();
}

const String RestParameter::type() const {
    if (isBool())
        return "boolean";
    else if (isNumber())
        return "number";
    else if (isString())
        return "string";
    else
        return "unknown";
}

bool RestParameter::operator==(RestParameter& other) const {
    if (this == &other) return true;
    return (other.key == key);
}
