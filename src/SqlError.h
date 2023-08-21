#pragma once

#include "global.h"
#include "SqlErrorCategory.h"

#include <string_view>
#include <string>
#include <system_error>

namespace AsyncPg {

/// Ошибка Sql запроса
class ASYNCPGLIB SqlError : public std::error_code
{
public:
    /// Конструктор класса по умолчанию
    SqlError() = default;

    /// Конструктор класса
    /// @param code Код ошибки
    /// @param driverError Сообщение об ошибке PostgreSql
    explicit SqlError(ErrorCode code, std::string_view driverError = "");

    /// Возвращает сообщение об ошибке PostgreSql
    /// @return Сообщение об ошибке PostgreSql
    const std::string& driverMessage() const;

    /// Очищает ошибку
    void clear();

private:
    std::string _driverMessage;
};

}
