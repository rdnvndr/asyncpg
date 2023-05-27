#pragma once

typedef struct pg_result PGresult;

namespace AsyncPg {

class SqlRecord;

/// Результат Sql запроса
class SqlResult
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

    /// Возвращает результат PostgreSql
    /// @return Результат PostgreSql
    PGresult *pgresult() const;

    /// Возвращает флаг наличия результата Sql запроса
    /// @return Флаг наличия результата Sql запроса
    bool operator!() const;

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
