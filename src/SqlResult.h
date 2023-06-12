#pragma once

#include "global.h"

#include <string>

typedef struct pg_result PGresult;

namespace AsyncPg {

class SqlRecord;

/// Результат Sql запроса
class ASYNCPGLIB SqlResult
{
public:
    /// Конструктор класса
    /// @param pgresult Результат PostgreSql
    explicit SqlResult(PGresult *pgresult = nullptr);
    ~SqlResult();

    /// Конструктор копирования
    SqlResult(const SqlResult&) = delete;

    /// Оператор копирования
    void operator=(const SqlResult&) = delete;

    /// Конструктор перемещения
    /// @param other Результата Sql запроса
    SqlResult(SqlResult &&other)noexcept;

    /// Оператор перемещения
    /// @param other Результата Sql запроса
    /// @return Результата Sql запроса
    SqlResult& operator=(SqlResult &&other) noexcept;

    /// Возвращает количество строк в результате Sql запроса
    /// @return Количество строк
    int rows() const;

    /// Возвращает количество колонок в строке результат Sql запроса
    /// @return Количество колонок в строке результат Sql запроса
    int columns() const;

    /// Возвращает наименование колонки по номеру колонки
    /// @param column Номер колонки
    /// @return Наименование колонки
    std::string fieldName(int column) const;

    /// Возвращает номер колонки по наименованию поля
    /// @param fieldName Наименование колонки
    /// @return Номер колонки
    int column(std::string_view fieldName) const;

    /// Возвращает результат PostgreSql
    /// @return Результат PostgreSql
    PGresult *pgresult() const;

    /// Возвращает флаг наличия результата Sql запроса
    /// @return Флаг наличия результата Sql запроса
    bool operator!() const;

    /// Возвращает указанную строку Sql запроса
    /// @param row Номер строки
    /// @return Строка Sql запроса
    SqlRecord at(int row) const;

    /// Возвращает первую строку Sql запроса
    /// @return Первая строка Sql запроса
    SqlRecord begin() const;

    /// Возвращает последнюю строку Sql запроса
    /// @return Последняя строка Sql запроса
    SqlRecord end() const;

private:
    PGresult *_result  = nullptr;
    int       _rows    = 0;
    int       _columns = 0;
};

}
