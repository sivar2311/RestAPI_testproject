#include "ArduinoVariant.h"

Variant::Variant(int v)
    : value(v) {}
Variant::Variant(float v)
    : value(v) {}
Variant::Variant(double v)
    : value(v) {}
Variant::Variant(bool v)
    : value(v) {}
Variant::Variant(const char* v)
    : value(String(v)) {}
Variant::Variant(const String& v)
    : value(v) {}
Variant::Variant(int8_t v)
    : value(v) {}
Variant::Variant(uint8_t v)
    : value(v) {}
Variant::Variant(int16_t v)
    : value(v) {}
Variant::Variant(uint16_t v)
    : value(v) {}
Variant::Variant(int32_t v)
    : value(v) {}
Variant::Variant(uint32_t v)
    : value(v) {}
Variant::Variant(int64_t v)
    : value(v) {}
Variant::Variant(uint64_t v)
    : value(v) {}

bool Variant::isInt() const { return std::holds_alternative<int>(value); }
bool Variant::isFloat() const { return std::holds_alternative<float>(value); }
bool Variant::isDouble() const { return std::holds_alternative<double>(value); }
bool Variant::isBool() const { return std::holds_alternative<bool>(value); }
bool Variant::isString() const { return std::holds_alternative<String>(value); }
bool Variant::isI8() const { return std::holds_alternative<int8_t>(value); }
bool Variant::isU8() const { return std::holds_alternative<uint8_t>(value); }
bool Variant::isI16() const { return std::holds_alternative<int16_t>(value); }
bool Variant::isU16() const { return std::holds_alternative<uint16_t>(value); }
bool Variant::isI32() const { return std::holds_alternative<int32_t>(value); }
bool Variant::isU32() const { return std::holds_alternative<uint32_t>(value); }
bool Variant::isI64() const { return std::holds_alternative<int64_t>(value); }
bool Variant::isU64() const { return std::holds_alternative<uint64_t>(value); }

int Variant::asInt() const {
    return std::visit([](auto&& v) -> int {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<int>(v);
        } else {
            return 0;
        }
    },
                      value);
}

float Variant::asFloat() const {
    return std::visit([](auto&& v) -> float {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<float>(v);
        } else {
            return 0.0f;
        }
    },
                      value);
}

double Variant::asDouble() const {
    return std::visit([](auto&& v) -> double {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<double>(v);
        } else {
            return 0.0;
        }
    },
                      value);
}

bool Variant::asBool() const {
    return std::visit([](auto&& v) -> bool {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<bool>(v);
        } else if constexpr (std::is_same_v<T, String>) {
            return !v.isEmpty();
        } else {
            return false;
        }
    },
                      value);
}

String Variant::asString() const {
    return std::visit([](auto&& v) -> String {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, String>) {
            return v;
        } else if constexpr (std::is_arithmetic_v<T>) {
            return String(v);
        } else {
            return "";
        }
    },
                      value);
}

int8_t Variant::asI8() const {
    return std::visit([](auto&& v) -> int8_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<int8_t>(v);
        } else {
            return 0;
        }
    },
                      value);
}

uint8_t Variant::asU8() const {
    return std::visit([](auto&& v) -> uint8_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<uint8_t>(v);
        } else {
            return 0;
        }
    },
                      value);
}

int16_t Variant::asI16() const {
    return std::visit([](auto&& v) -> int16_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<int16_t>(v);
        } else {
            return 0;
        }
    },
                      value);
}

uint16_t Variant::asU16() const {
    return std::visit([](auto&& v) -> uint16_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<uint16_t>(v);
        } else {
            return 0;
        }
    },
                      value);
}

int32_t Variant::asI32() const {
    return std::visit([](auto&& v) -> int32_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<int32_t>(v);
        } else {
            return 0;
        }
    },
                      value);
}

uint32_t Variant::asU32() const {
    return std::visit([](auto&& v) -> uint32_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<uint32_t>(v);
        } else {
            return 0;
        }
    },
                      value);
}

int64_t Variant::asI64() const {
    return std::visit([](auto&& v) -> int64_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<int64_t>(v);
        } else {
            return 0;
        }
    },
                      value);
}

uint64_t Variant::asU64() const {
    return std::visit([](auto&& v) -> uint64_t {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<uint64_t>(v);
        } else {
            return 0;
        }
    },
                      value);
}

Variant& Variant::operator=(int v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(float v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(double v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(bool v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(const char* v) {
    value = String(v);
    return *this;
}
Variant& Variant::operator=(const String& v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(int8_t v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(uint8_t v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(int16_t v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(uint16_t v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(int32_t v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(uint32_t v) {
    value = v;
    return *this;
}
Variant& Variant::operator=(int64_t v) {
    value = v;
    return *this;
}

Variant& Variant::operator=(uint64_t v) {
    value = v;
    return *this;
}

Variant::operator int() const { return asInt(); }
Variant::operator float() const { return asFloat(); }
Variant::operator double() const { return asDouble(); }
Variant::operator bool() const { return asBool(); }
Variant::operator String() const { return asString(); }
Variant::operator int8_t() const { return asI8(); }
Variant::operator uint8_t() const { return asU8(); }
Variant::operator int16_t() const { return asI16(); }
Variant::operator uint16_t() const { return asU16(); }
Variant::operator int32_t() const { return asI32(); }
Variant::operator uint32_t() const { return asU32(); }
Variant::operator int64_t() const { return asI64(); }
Variant::operator uint64_t() const { return asU64(); }