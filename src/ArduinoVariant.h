#pragma once

#if (__cplusplus < 201703L)
#error "This library requires C++17 / Espressif32 Arduino 3.x"
#else

#include <Print.h>
#include <Printable.h>
#include <WString.h>
#include <stdint.h>
#include <Preferences.h>
#include <variant>

class ArduinoVariant : public Printable {
  private:
    using VariantType = std::variant<std::monostate, int, float, double, bool, String, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

    VariantType value;

  public:
    ArduinoVariant() = default;

    template <typename T, typename = std::enable_if_t<std::disjunction_v<std::is_same<T, std::monostate>, std::is_arithmetic<T>, std::is_same<T, String>>>>
    ArduinoVariant(T other);

    ArduinoVariant(const char* other);

    template <typename T>
    bool is() const;

    bool isValid() const;
    bool isInvalid() const;

    template <typename T, typename = std::enable_if_t<std::disjunction_v<std::is_same<T, std::monostate>, std::is_arithmetic<T>, std::is_same<T, String>>>>
    ArduinoVariant& operator=(T other);

    template <typename T>
    explicit operator T() const;

    template <typename T>
    T as() const;

    void clear();

    void load(const char* key, Preferences& pref);
    void save(const char* key, Preferences& pref) const;

  protected:
    size_t printTo(Print& printer) const;
};

template <typename T, typename>
ArduinoVariant::ArduinoVariant(T other)
    : value(other) {}

template <typename T>
bool ArduinoVariant::is() const {
    return std::holds_alternative<T>(value);
}

template <typename T, typename>
ArduinoVariant& ArduinoVariant::operator=(T other) {
    value = other;
    return *this;
}

template <typename T>
ArduinoVariant::operator T() const {
    return as<T>();
}

template <typename T>
T ArduinoVariant::as() const {
    return std::visit([](auto&& v) -> T {
        using Type = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            if constexpr (std::is_arithmetic_v<T>) {
                return T{};
            } else if constexpr (std::is_same_v<T, String>) {
                return String("");
            } else if constexpr (std::is_same_v<T, bool>) {
                return false;
            } else {
                return T{};
            }
        } else if constexpr (std::is_same_v<Type, bool>) {
            if constexpr (std::is_arithmetic_v<T>) {
                return static_cast<T>(v);
            } else if constexpr (std::is_same_v<T, String>) {
                return v ? "true" : "false";
            }
        } else if constexpr (std::is_arithmetic_v<Type> && std::is_arithmetic_v<T>) {
            return static_cast<T>(v);
        } else if constexpr (std::is_same_v<T, String>) {
            if constexpr (std::is_same_v<Type, String>) {
                return v;
            } else if constexpr (std::is_arithmetic_v<Type>) {
                return String(v);
            } else {
                return String("");
            }
        } else if constexpr (std::is_same_v<Type, String> && std::is_same_v<T, bool>) {
            return !v.isEmpty();
        } else if constexpr (std::is_same_v<T, bool>) {
            return static_cast<bool>(v);
        } else if constexpr (std::is_constructible_v<T, Type>) {
            return T(v);
        } else {
            return T();
        }
    },
                      value);
}

#endif