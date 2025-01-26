#include "Settings.h"

Variant& Settings::operator[](const char* key) {
    for (auto& [_key, value] : settings) {
        if (_key == key) return value;
    }
    Variant value;
    settings.push_back(std::make_pair(key, value));
    return settings.back().second;
}

Settings::Iterator Settings::begin() {
    return settings.begin();
}

Settings::Iterator Settings::end() {
    return settings.end();
}
