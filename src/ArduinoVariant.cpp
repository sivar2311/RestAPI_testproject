#if (__cplusplus < 201703L)
#error "This library requires C++17 / Espressif32 Arduino 3.x"
#else

#include "ArduinoVariant.h"

ArduinoVariant::ArduinoVariant(const char* other)
    : value(String(other)) {}

size_t ArduinoVariant::printTo(Print& printer) const {
    return printer.print(as<String>());
}

template <>
const char* ArduinoVariant::as<const char*>() const { return ""; }

void ArduinoVariant::clear() {
    std::visit([&](auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            v = 0;
        } else if constexpr (std::is_same_v<T, String>) {
            v = "";
        }
    },
               value);
}

bool ArduinoVariant::isValid() const {
    return !std::holds_alternative<std::monostate>(value);
}

bool ArduinoVariant::isInvalid() const {
    return std::holds_alternative<std::monostate>(value);
}

void ArduinoVariant::load(const char* key, Preferences& prefs) {
    std::visit([&](auto&& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, String>) val = prefs.getString(key, val);
            else if constexpr (std::is_same_v<T, bool>) prefs.getBool(key, val);
            else if constexpr (std::is_same_v<T, double>) prefs.getDouble(key, val);
            else if constexpr (std::is_same_v<T, float>) prefs.getFloat(key, val);
            else if constexpr (std::is_same_v<T, int8_t>) prefs.getChar(key, val);
            else if constexpr (std::is_same_v<T, uint8_t>) prefs.getUChar(key, val);
            else if constexpr (std::is_same_v<T, int16_t>) prefs.getShort(key, val);
            else if constexpr (std::is_same_v<T, uint16_t>) prefs.getUShort(key, val);
            else if constexpr (std::is_same_v<T, int32_t>) prefs.getInt(key, val);
            else if constexpr (std::is_same_v<T, uint32_t>) prefs.getUInt(key, val);
            else if constexpr (std::is_same_v<T, int64_t>) prefs.getLong64(key, val);
            else if constexpr (std::is_same_v<T, uint64_t>) prefs.getULong64(key, val);
    }, value);
}

void ArduinoVariant::save(const char* key, Preferences& prefs) const {
        std::visit([&](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, String>) prefs.putString(key, val);
            else if constexpr (std::is_same_v<T, bool>) prefs.putBool(key, val);
            else if constexpr (std::is_same_v<T, double>) prefs.putDouble(key, val);
            else if constexpr (std::is_same_v<T, float>) prefs.putFloat(key, val);
            else if constexpr (std::is_same_v<T, int8_t>) prefs.putChar(key, val);
            else if constexpr (std::is_same_v<T, uint8_t>) prefs.putUChar(key, val);
            else if constexpr (std::is_same_v<T, int16_t>) prefs.putShort(key, val);
            else if constexpr (std::is_same_v<T, uint16_t>) prefs.putUShort(key, val);
            else if constexpr (std::is_same_v<T, int32_t>) prefs.putInt(key, val);
            else if constexpr (std::is_same_v<T, uint32_t>) prefs.putUInt(key, val);
            else if constexpr (std::is_same_v<T, int64_t>) prefs.putLong64(key, val);
            else if constexpr (std::is_same_v<T, uint64_t>) prefs.putULong64(key, val);
        }, value);
}


#endif