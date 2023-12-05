#pragma once

#include "global.h"

#include <optional>
#include <variant>
#include <cstdint>
#include <chrono>
#include <string>
#include <vector>
#include <array>

using PGresult = struct pg_result;

namespace AsyncPg {

/// Значение поля строки результата Sql запроса
using SqlValue = std::variant<
    std::monostate, std::optional<bool>,
    std::optional<int16_t>, std::optional<int32_t>, std::optional<int64_t>,
    std::optional<float>, std::optional<double>, std::optional<std::string>,
    std::optional<std::string>, std::optional<std::string>, std::optional<std::string>,
    std::optional<std::string>, std::optional<std::string>, std::optional<std::string>,
    std::optional<std::time_t>, std::optional<std::time_t>, std::optional<std::time_t>,
    std::optional<std::time_t>, std::optional<std::time_t>,
    std::optional<std::array<char, 16>>, std::optional<std::vector<char>>>;

/// Тип поля строки результата Sql запроса
enum SqlType : size_t {
    None,
    Boolean,
    SmallInt,
    Integer,
    BigInt,
    Real,
    Double,
    Decimal,
    Char,
    Json,
    Name,
    Text,
    VarChar,
    Xml,
    Date,
    Time,
    TimeTz,
    TimeStamp,
    TimeStampTz,
    Uuid,
    Bytea,
};

/// Конвертирует результат PostgreSql в значение поля строки результата Sql запроса
/// @param pgresult Результат PostgreSql
/// @param row Номер строки
/// @param col Номер колонки
/// @return Значение поля строки результата Sql запроса
ASYNCPGLIB SqlValue asSqlValue(PGresult* pgresult, int row, int col);

/// Создаёт значение поля строки результата Sql запроса
/// @param I Тип поля строки результата Sql запроса
/// @param Args Типы значений
/// @param args Значения
/// @return Значение поля строки результата Sql запроса
template<std::size_t I, class... Args>
SqlValue makeSqlValue(Args&&... args) {
    return SqlValue(std::in_place_index<I>, std::forward<Args>(args)...);
}

/// Конвертирует значение поля строки результата Sql запроса в значение PostgreSql
/// @param value Значение поля строки результата Sql запроса
/// @return Значение PostgreSql
ASYNCPGLIB std::tuple<unsigned int, std::size_t, char *> asPgValue(const SqlValue &value);


/// Конвертирует тип поля строки результата Sql запроса в тип PostgreSql
/// @param type Тип поля строки результата Sql запроса
/// @return Тип PostgreSql
ASYNCPGLIB unsigned int toPgType(SqlType type);

/// Конвертирует строку в глобальный идентификатор
/// @param str Строка
/// @return Глобальный идентификатор
ASYNCPGLIB std::array<char, 16> toByteUuid(std::string_view str);

/// Конвертирует глобальный идентификатор в строку
/// @param uuid Глобальный идентификатор
/// @return Строка
ASYNCPGLIB std::string fromByteUuid(const std::array<char, 16> &uuid);

}
