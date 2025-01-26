#include "ArduinoVariant.h"

Variant::Variant(const char* other)
    : value(String(other)) {}

size_t Variant::printTo(Print& printer) const {
    return printer.print(as<String>());
}

template <>
const char* Variant::as<const char*>() const { return ""; }