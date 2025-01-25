#include "Settings.h"

Variant& Settings::operator[](const char* key) {
    return settings.operator[](key);
}

Settings::Iterator Settings::begin() {
    return settings.begin();
}

Settings::Iterator Settings::end() {
    return settings.end();
}
