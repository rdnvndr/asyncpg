#include "SqlValue.h"

#include <libpq-fe.h>

#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <charconv>
#include <algorithm>
#include <iterator>
#include <memory>
#include <tuple>
#include <variant>

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)
#	define __WINDOWS__
#endif
#if defined(__linux__) || defined(__CYGWIN__)
#	include <endian.h>
#elif defined(__APPLE__)
#	include <libkern/OSByteOrder.h>
#	define htobe16(x) OSSwapHostToBigInt16(x)
#	define htole16(x) OSSwapHostToLittleInt16(x)
#	define be16toh(x) OSSwapBigToHostInt16(x)
#	define le16toh(x) OSSwapLittleToHostInt16(x)
#	define htobe32(x) OSSwapHostToBigInt32(x)
#	define htole32(x) OSSwapHostToLittleInt32(x)
#	define be32toh(x) OSSwapBigToHostInt32(x)
#	define le32toh(x) OSSwapLittleToHostInt32(x)
#	define htobe64(x) OSSwapHostToBigInt64(x)
#	define htole64(x) OSSwapHostToLittleInt64(x)
#	define be64toh(x) OSSwapBigToHostInt64(x)
#	define le64toh(x) OSSwapLittleToHostInt64(x)
#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN
#elif defined(__OpenBSD__)
#	include <sys/endian.h>
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#	include <sys/endian.h>
#	define be16toh(x) betoh16(x)
#	define le16toh(x) letoh16(x)
#	define be32toh(x) betoh32(x)
#	define le32toh(x) letoh32(x)
#	define be64toh(x) betoh64(x)
#	define le64toh(x) letoh64(x)
#elif defined(__WINDOWS__)
#	include <winsock2.h>
#	if BYTE_ORDER == LITTLE_ENDIAN
#		define htobe16(x) htons(x)
#		define htole16(x) (x)
#		define be16toh(x) ntohs(x)
#		define le16toh(x) (x)
#		define htobe32(x) htonl(x)
#		define htole32(x) (x)
#		define be32toh(x) ntohl(x)
#		define le32toh(x) (x)
#		define htobe64(x) htonll(x)
#		define htole64(x) (x)
#		define be64toh(x) ntohll(x)
#		define le64toh(x) (x)
#	elif BYTE_ORDER == BIG_ENDIAN
/* that would be xbox 360 */
#		define htobe16(x) (x)
#		define htole16(x) __builtin_bswap16(x)
#		define be16toh(x) (x)
#		define le16toh(x) __builtin_bswap16(x)
#		define htobe32(x) (x)
#		define htole32(x) __builtin_bswap32(x)
#		define be32toh(x) (x)
#		define le32toh(x) __builtin_bswap32(x)
#		define htobe64(x) (x)
#		define htole64(x) __builtin_bswap64(x)
#		define be64toh(x) (x)
#		define le64toh(x) __builtin_bswap64(x)
#	else
#		error byte order not supported
#	endif
#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN
#else
#	error platform not supported
#endif



namespace AsyncPg {

#define BOOLOID 16
#define CHAROID 18
#define NAMEOID 19
#define JSONOID 114
#define XMLOID 142
#define VARCHAROID 1043
#define TEXTOID 25
#define INT8OID 20
#define INT2OID 21
#define INT4OID 23
#define NUMERICOID 1700
#define FLOAT4OID 700
#define FLOAT8OID 701
#define DATEOID 1082
#define TIMEOID 1083
#define TIMETZOID 1266
#define TIMESTAMPOID 1114
#define TIMESTAMPTZOID 1184
#define BYTEAOID 17
#define UUIDOID 2950

#define POSTGRES_EPOCH_USEC 946684800000000
#define POSTGRES_DAY_USEC 86400000000

template <typename T>
constexpr T htonT (T value) noexcept
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    char* ptr = reinterpret_cast<char*>(&value);
    std::reverse(ptr, ptr + sizeof(T));
#endif
    return value;
}

#if BIG_ENDIAN
template <typename T>
constexpr static T& swap_endian(T& pX)
{
    return pX;
}
#else
template <typename T>
constexpr static T& swap_endian(T& pX)
{
    char& raw = reinterpret_cast<char&>(pX);
    std::reverse(&raw, &raw + sizeof(T));
    return pX;
}
#endif

static const char *asChar(PGresult* pgresult, int row, int col)
{
    return PQgetvalue(pgresult, row, col);
}

static bool asBool(PGresult* pgresult, int row, int col)
{
    return *asChar(pgresult, row, col) != 0;
}

static int8_t asInt8(PGresult* pgresult, int row, int col)
{
    void *ptr = PQgetvalue(pgresult, row, col);
    return *static_cast<int8_t*>(ptr);
}

static int16_t asInt16(PGresult* pgresult, int row, int col)
{
    void *ptr = PQgetvalue(pgresult, row, col);
    return htonT(*static_cast<int16_t*>(ptr));
}

static int32_t asInt32(PGresult* pgresult, int row, int col)
{
    void *ptr = PQgetvalue(pgresult, row, col);
    return htonT(*static_cast<int32_t*>(ptr));
}

static int64_t asInt64(PGresult* pgresult, int row, int col)
{
    void *ptr = PQgetvalue(pgresult, row, col);
    return htonT(*static_cast<int64_t*>(ptr));
}

static std::time_t asTimeStamp(PGresult* pgresult, int row, int col)
{
    return (asInt64(pgresult, row, col) + POSTGRES_EPOCH_USEC) / 1000000;
}

static std::time_t asTimeStampTz(PGresult* pgresult, int row, int col)
{
    return (asInt64(pgresult, row, col) + POSTGRES_EPOCH_USEC) / 1000000;
}

static std::time_t asTime(PGresult* pgresult, int row, int col)
{
    return asInt64(pgresult, row, col) / 1000000;
}

static std::time_t asTimeTz(PGresult* pgresult, int row, int col)
{
    return asInt64(pgresult, row, col) / 1000000;
}

static std::time_t asDate(PGresult* pgresult, int row, int col)
{
    return (asInt32(pgresult, row, col) * POSTGRES_DAY_USEC + POSTGRES_EPOCH_USEC) / 1000000;
}

static std::vector<char> asVector(PGresult* pgresult, int row, int col)
{
    const auto len = PQgetlength(pgresult, row, col);
    auto ptr = PQgetvalue(pgresult, row, col);
    std::vector<char> result;
    result.reserve(len);
    result.assign(ptr, ptr + len);
    return result;
}

static std::vector<char> asBytea(PGresult* pgresult, int row, int col)
{
    const auto len = PQgetlength(pgresult, row, col);
    auto ptr = PQgetvalue(pgresult, row, col);
    std::vector<char> result;
    result.reserve(len);
    result.assign(ptr, ptr + len);
    return result;
}

static std::array<char, 16> asUuid(PGresult* pgresult, int row, int col)
{
    auto ptr = PQgetvalue(pgresult, row, col);
    std::array<char, 16> result {*ptr};
    return result;
}

static std::string asString(PGresult* pgresult, int row, int col)
{
    const auto len = PQgetlength(pgresult, row, col);
    auto ptr = PQgetvalue(pgresult, row, col);
    return std::string(ptr, len);
}

static double asDouble(PGresult* pgresult, int row, int col)
{
    double ptr = *reinterpret_cast<double*>(PQgetvalue(pgresult, row, col));
    return htonT(ptr);
}

static float asFloat(PGresult* pgresult, int row, int col)
{
    union {
        int32_t value;
        float   retval;
    } castunion{};

    castunion.value = asInt32(pgresult, row, col);
    return castunion.retval;
}

static std::string asDecimal(PGresult* pgresult, int row, int col)
{
    std::string str;
    auto *decimal = reinterpret_cast<int16_t *>(PQgetvalue(pgresult, row, col));
    auto ndigits  = htonT(decimal[0]);
    auto width    = htonT(decimal[1]);
    auto sign     = htonT(decimal[2]);
    auto dscale   = htonT(decimal[3]);
    if (sign != 0)
        str += "-";

    if (width < 0) {
        str += "0.";
        ++width;
        while (width++ < 0)
            str += "0000";
        width = -1;
    }

    for (int n = 0; n < ndigits; ++n) {
        std::string digit = std::to_string(htonT(decimal[4 + n]));
        if (!str.empty())
            str += std::string(4 - digit.size(), '0');
        str += digit;

        if (width == 0 && ndigits - 1 != n)
            str += ".";

        if (dscale > 0 && width < 0)
            dscale -= 4;

        if (width >= 0)
            --width;
    }

    while (width-- >= 0)
        str += "0000";

    return (dscale < 0) ? str.substr(0, str.size() + dscale) : str;
}

SqlValue asSqlValue(PGresult *pgresult, int row, int col)
{
    SqlValue result;

    if (!pgresult)
        return result;

    auto oid = PQftype(pgresult, col);
    switch (oid) {
    case BOOLOID:
        result.emplace<SqlType::Boolean>(asBool(pgresult, row, col));
        break;
    case INT2OID:
        result.emplace<SqlType::SmallInt>(asInt16(pgresult, row, col));
        break;
    case INT4OID:
        result.emplace<SqlType::Integer>(asInt32(pgresult, row, col));
        break;
    case INT8OID:
        result.emplace<SqlType::BigInt>(asInt64(pgresult, row, col));
        break;
    case FLOAT4OID:
        result.emplace<SqlType::Real>(asFloat(pgresult, row, col));
        break;
    case FLOAT8OID:
        result.emplace<SqlType::Double>(asDouble(pgresult, row, col));
        break;
    case NUMERICOID:
        result.emplace<SqlType::Decimal>(asDecimal(pgresult, row, col));
        break;
    case TIMESTAMPOID:
        result.emplace<SqlType::TimeStamp>(asTimeStamp(pgresult, row, col));
        break;
    case TIMESTAMPTZOID:
        result.emplace<SqlType::TimeStampTz>(asTimeStampTz(pgresult, row, col));
        break;
    case TIMEOID:
        result.emplace<SqlType::Time>(asTime(pgresult, row, col));
        break;
    case TIMETZOID:
        result.emplace<SqlType::TimeTz>(asTimeTz(pgresult, row, col));
        break;
    case BYTEAOID:
        result.emplace<SqlType::Bytea>(asBytea(pgresult, row, col));
        break;
    case DATEOID:
        result.emplace<SqlType::Date>(asDate(pgresult, row, col));
        break;
    case UUIDOID:
        result.emplace<SqlType::Uuid>(asUuid(pgresult, row, col));
        break;
    case CHAROID:
        result.emplace<SqlType::Char>(asString(pgresult, row, col));
    case NAMEOID:
        result.emplace<SqlType::Name>(asString(pgresult, row, col));
    case JSONOID:
        result.emplace<SqlType::Json>(asString(pgresult, row, col));
    case XMLOID:
        result.emplace<SqlType::Xml>(asString(pgresult, row, col));
    case VARCHAROID:
        result.emplace<SqlType::VarChar>(asString(pgresult, row, col));
    case TEXTOID:
        result.emplace<SqlType::Text>(asString(pgresult, row, col));
        break;
    default:
        break;
    }

    return result;
}

static std::tuple<unsigned int, std::size_t, char *> fromBool(const std::optional<bool> &value)
{
    if (!value)
        return std::make_tuple(BOOLOID, 0, nullptr);

    char *v = new char[1];
    *v = *value ? 1 : 0;
    return std::make_tuple(BOOLOID, 1, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromInt16(const std::optional<int16_t> &value)
{
    if (!value)
        return std::make_tuple(INT2OID, 0, nullptr);

    char *v = new char[2];
    *reinterpret_cast<int16_t *>(v) = htonT(*value);
    return std::make_tuple(INT2OID, 2, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromInt32(const std::optional<int32_t> &value)
{
    if (!value)
        return std::make_tuple(INT4OID, 0, nullptr);

    char *v = new char[4];
    *reinterpret_cast<int32_t *>(v) = htonT(*value);
    return std::make_tuple(INT4OID, 4, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromInt64(const std::optional<int64_t> &value)
{
    if (!value)
        return std::make_tuple(INT8OID, 0, nullptr);

    char *v = new char[8];
    *reinterpret_cast<int64_t *>(v) = htonT(*value);
    return std::make_tuple(INT8OID, 8, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromFloat(const std::optional<float> &value)
{
    if (!value)
        return std::make_tuple(FLOAT4OID, 0, nullptr);

    union {
        int32_t value;
        float   retval;
    } castunion{};
    castunion.retval = *value;
    char *v = new char[4];
    *reinterpret_cast<int32_t *>(v) = htonT(castunion.value);
    return std::make_tuple(FLOAT4OID, 4, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromDouble(const std::optional<double> &value)
{
    if (!value)
        return std::make_tuple(FLOAT8OID, 0, nullptr);

    union {
        int64_t value;
        double  retval;
    } castunion{};

    castunion.retval = *value;
    char *v = new char[8];
    *reinterpret_cast<int64_t *>(v) = htonT(castunion.value);
    return std::make_tuple(FLOAT8OID, 8, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromDecimal(
    const std::optional<std::string> &value)
{
    if (!value || value->empty())
        return std::make_tuple(NUMERICOID, 0, nullptr);

    std::deque<int16_t> decimal;
    int16_t ndigits  = 0;
    int16_t width    = -1;
    int16_t sign     = value->substr(0, 1) == "-" ? 1 : 0;
    int16_t dscale   = 0;

    auto pos = value->find('.');
    bool existIntPart  = (
        value->substr(sign, (pos == std::string::npos ? value->length() - sign : pos)) != "0");
    bool existFracPart = (pos != std::string::npos || pos + 1 > value->length());

    if (existIntPart) {
        int beg = (sign != 0) ? 1 : 0;
        int end = static_cast<int>(pos == std::string::npos ? value->length() : pos) - 1;

        std::size_t count   = 0;
        std::size_t notZeroPos = end;
        for (auto i = end; i >= beg; --i) {
            if (value->at(i) != '0')
                notZeroPos = i;

            if (++count == 4 || i == beg) {
                auto zeroCount = notZeroPos - i;
                int16_t digit = static_cast<int16_t>(
                    std::stoi(value->substr(i + zeroCount, count - zeroCount)));
                count = 0;
                ++width;

                if (!existFracPart && digit == 0 && !decimal.empty())
                    continue;

                decimal.emplace_front(htonT(digit));
            }
        }
    }

    if (existFracPart) {
        std::size_t count = 0;
        for (auto i = pos + 1, e = value->length(); i < e; ++i) {
            if (++count == 4 || i + 1 == e) {
                std::string sdigit = value->substr(i - count + 1, count);
                if (count < 4)
                    sdigit += std::string(4 - count, '0');
                auto digit = static_cast<int16_t>(std::stoi(sdigit));
                dscale = static_cast<int16_t>(dscale + count);
                count = 0;

                if (!existIntPart && digit == 0 && decimal.empty()) {
                    --width;
                    continue;
                }

                decimal.emplace_back(htonT(digit));
            }
        }
    }

    ndigits = static_cast<int16_t>(decimal.size());
    std::size_t size = 8 + ndigits * 2;
    auto *v = new char[size];
    auto *vv = reinterpret_cast<int16_t *>(v);

    std::size_t it = 0;
    vv[it++] = htonT(ndigits);
    vv[it++] = htonT(width);
    vv[it++] = htonT(sign);
    vv[it++] = htonT(dscale);

    for (int16_t digit : decimal)
        vv[it++] = digit;

    return std::make_tuple(NUMERICOID, size, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromTimeStamp(
    unsigned int oid, const std::optional<std::time_t> &value)
{
    if (!value)
        return std::make_tuple(oid, 0, nullptr);

    char *v = new char[8];
    *reinterpret_cast<int64_t *>(v) = htonT(*value * 1000000 - POSTGRES_EPOCH_USEC);

    return std::make_tuple(oid, 8, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromTime(
    unsigned int oid, const std::optional<std::time_t> &value)
{
    if (!value)
        return std::make_tuple(oid, 0, nullptr);

    char *v = new char[8];
    *reinterpret_cast<int64_t *>(v) = htonT(*value * 1000000);

    return std::make_tuple(oid, 8, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromDate(const std::optional<std::time_t> &value)
{
    if (!value)
        return std::make_tuple(DATEOID, 0, nullptr);

    char *v = new char[4];
    *reinterpret_cast<int32_t *>(v) = static_cast<int32_t>(
        htonT((*value * 1000000 - POSTGRES_EPOCH_USEC) / POSTGRES_DAY_USEC));

    return std::make_tuple(DATEOID, 4, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromString(
    unsigned int oid, const std::optional<std::string> &value)
{
    if (!value)
        return std::make_tuple(oid, 0, nullptr);

    auto size = value->size();
    char *v = new char[size];
    for (std::size_t i = 0; i < size; ++i)
        v[i] = value->data()[i];

    return std::make_tuple(oid, size, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromUuid(
    const std::optional<std::array<char, 16>> &value)
{
    if (!value)
        return std::make_tuple(UUIDOID, 0, nullptr);

    auto size = value->size();
    char *v = new char[size];
    for (std::size_t i = 0; i < size; ++i)
        v[i] = value->data()[i];

    return std::make_tuple(UUIDOID, size, v);
}

static std::tuple<unsigned int, std::size_t, char *> fromBytea(
    const std::optional<std::vector<char>> &value)
{
    if (!value)
        return std::make_tuple(BYTEAOID, 0, nullptr);

    auto size = value->size();
    char *v = new char[size];
    for (std::size_t i = 0; i < size; ++i)
        v[i] = value->data()[i];

    return std::make_tuple(BYTEAOID, size, v);
}

std::tuple<unsigned int, std::size_t, char *> asPgValue(const SqlValue &value)
{
    switch (value.index()) {
    case SqlType::Boolean:
        return fromBool(std::get<SqlType::Boolean>(value));
    case SqlType::SmallInt:
        return fromInt16(std::get<SqlType::SmallInt>(value));
    case SqlType::Integer:
        return fromInt32(std::get<SqlType::Integer>(value));
    case SqlType::BigInt:
        return fromInt64(std::get<SqlType::BigInt>(value));
    case SqlType::Real:
        return fromFloat(std::get<SqlType::Real>(value));
    case SqlType::Double:
        return fromDouble(std::get<SqlType::Double>(value));
    case SqlType::Decimal:
        return fromDecimal(std::get<SqlType::Decimal>(value));
    case SqlType::TimeStamp:
        return fromTimeStamp(TIMESTAMPOID, std::get<SqlType::TimeStamp>(value));
    case SqlType::TimeStampTz:
        return fromTimeStamp(TIMESTAMPTZOID, std::get<SqlType::TimeStampTz>(value));
    case SqlType::Time:
        return fromTime(TIMEOID, std::get<SqlType::Time>(value));
    case SqlType::TimeTz:
        return fromTime(TIMETZOID, std::get<SqlType::TimeTz>(value));
    case SqlType::Date:
        return fromDate(std::get<SqlType::Date>(value));
    case SqlType::Bytea:
        return fromBytea(std::get<SqlType::Bytea>(value));
    case SqlType::Uuid:
        return fromUuid(std::get<SqlType::Uuid>(value));
    case SqlType::Char:
        return fromString(CHAROID, std::get<SqlType::Char>(value));
    case SqlType::VarChar:
        return fromString(VARCHAROID, std::get<SqlType::VarChar>(value));
    case SqlType::Name:
        return fromString(NAMEOID, std::get<SqlType::Name>(value));
    case SqlType::Json:
        return fromString(JSONOID, std::get<SqlType::Json>(value));
    case SqlType::Xml:
        return fromString(XMLOID, std::get<SqlType::Xml>(value));
    case SqlType::Text:
        return fromString(TEXTOID, std::get<SqlType::Text>(value));
    default:
        break;
    }

    return std::make_tuple(0, 0, nullptr);
}

unsigned int toPgType(SqlType type)
{
    switch (type) {
    case SqlType::Boolean:
        return BOOLOID;
    case SqlType::SmallInt:
        return INT2OID;
    case SqlType::Integer:
        return INT4OID;
    case SqlType::BigInt:
        return INT8OID;
    case SqlType::Real:
        return FLOAT4OID;
    case SqlType::Double:
        return FLOAT8OID;
    case SqlType::Decimal:
        return NUMERICOID;
    case SqlType::TimeStamp:
        return TIMESTAMPOID;
    case SqlType::TimeStampTz:
        return TIMESTAMPTZOID;
    case SqlType::Time:
        return TIMEOID;
    case SqlType::TimeTz:
        return TIMETZOID;
    case SqlType::Date:
        return DATEOID;
    case SqlType::Bytea:
        return BYTEAOID;
    case SqlType::Uuid:
        return UUIDOID;
    case SqlType::Char:
        return CHAROID;
    case SqlType::VarChar:
        return VARCHAROID;
    case SqlType::Name:
        return NAMEOID;
    case SqlType::Json:
        return JSONOID;
    case SqlType::Xml:
        return XMLOID;
    case SqlType::Text:
        return TEXTOID;
    default:
        break;
    }
    return 0;
}

}
