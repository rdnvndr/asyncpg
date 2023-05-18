#pragma once

#include <exception>
#include <string_view>
#include <string>

namespace AsyncPg {

/// Ошибка Sql запроса
class SqlError : std::exception
{
public:
    /// Конструктор класса по умолчанию
    SqlError() = default;

    /// Конструктор копирования
    SqlError(const SqlError& other);

    /// Конструктор класса
    /// @param errorMessage Сообщение об ошибке
    /// @param driverError Сообщение об ошибке PostgreSql
    explicit SqlError(std::string_view errorMessage, std::string_view driverError = "");

    /// Возвращает сообщение об ошибке
    /// @return Сообщение об ошибке
    const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override;

    /// Возвращает сообщение об ошибке
    /// @return Сообщение об ошибке
    const std::string& errorMessage() const;

    /// Возвращает сообщение об ошибке PostgreSql
    /// @return Сообщение об ошибке PostgreSql
    const std::string& driverMessage() const;

    /// Возвращает true при наличии ошибки, иначе false
    /// @return true при наличии ошибки, иначе false
    operator bool() const;

    /// Очищает ошибку
    void clear();

private:
    bool        _isError = false;
    std::string _errorMessage;
    std::string _driverMessage;
};

}
