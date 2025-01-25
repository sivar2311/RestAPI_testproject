#include <WString.h>

#include <variant>

class Variant {
  private:
    using VariantType = std::variant<
        std::monostate,
        int,
        float,
        double,
        bool,
        String,
        int8_t,
        uint8_t,
        int16_t,
        uint16_t,
        int32_t,
        uint32_t,
        int64_t,
        uint64_t>;

    VariantType value;

  public:
    Variant() = default;

    Variant(int v);
    Variant(float v);
    Variant(double v);
    Variant(bool v);
    Variant(const char* v);
    Variant(const String& v);
    Variant(int8_t v);
    Variant(uint8_t v);
    Variant(int16_t v);
    Variant(uint16_t v);
    Variant(int32_t v);
    Variant(uint32_t v);
    Variant(int64_t v);
    Variant(uint64_t v);

    bool isInt() const;
    bool isFloat() const;
    bool isDouble() const;
    bool isBool() const;
    bool isString() const;
    bool isI8() const;
    bool isU8() const;
    bool isI16() const;
    bool isU16() const;
    bool isI32() const;
    bool isU32() const;
    bool isI64() const;
    bool isU64() const;

    int      asInt() const;
    float    asFloat() const;
    double   asDouble() const;
    bool     asBool() const;
    String   asString() const;
    int8_t   asI8() const;
    uint8_t  asU8() const;
    int16_t  asI16() const;
    uint16_t asU16() const;
    int32_t  asI32() const;
    uint32_t asU32() const;
    int64_t  asI64() const;
    uint64_t asU64() const;

    Variant& operator=(int v);
    Variant& operator=(float v);
    Variant& operator=(double v);
    Variant& operator=(bool v);
    Variant& operator=(const char* v);
    Variant& operator=(const String& v);
    Variant& operator=(int8_t v);
    Variant& operator=(uint8_t v);
    Variant& operator=(int16_t v);
    Variant& operator=(uint16_t v);
    Variant& operator=(int32_t v);
    Variant& operator=(uint32_t v);
    Variant& operator=(int64_t v);
    Variant& operator=(uint64_t v);

    operator int() const;
    operator float() const;
    operator double() const;
    operator bool() const;
    operator String() const;
    operator int8_t() const;
    operator uint8_t() const;
    operator int16_t() const;
    operator uint16_t() const;
    operator int32_t() const;
    operator uint32_t() const;
    operator int64_t() const;
    operator uint64_t() const;
};