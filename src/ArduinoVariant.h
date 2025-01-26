#pragma once

#include <WString.h>
#include <stdint.h>
#include <Printable.h>
#include <variant>
#include <Print.h>

class Variant : public Printable {
  private:
    using VariantType = std::variant<std::monostate, int, float, double, bool, String, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

    VariantType value;

  public:
    Variant() = default;

    template <typename T, typename = std::enable_if_t<std::disjunction_v<std::is_same<T, std::monostate>, std::is_arithmetic<T>, std::is_same<T, String>>>>
    Variant(T other);

    Variant(const char* other);

    template <typename T>
    bool is() const;

    template <typename T, typename = std::enable_if_t<std::disjunction_v<std::is_same<T, std::monostate>, std::is_arithmetic<T>, std::is_same<T, String>>>>
    Variant& operator=(T other);

    template <typename T>
    explicit operator T() const;

    template <typename T>
    T as() const;

    size_t printTo(Print& printer) const;
};

template <typename T, typename>
Variant::Variant(T other)
    : value(other) {}



template <typename T>
bool Variant::is() const {
    return std::holds_alternative<T>(value);
}

template <typename T, typename>
Variant& Variant::operator=(T other) {
    value = other;
    return *this;
}

template <typename T>
Variant::operator T() const {
    return as<T>();
}

template <typename T>
T Variant::as() const {
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
    }, value);
}