#pragma once

#include "global.h"

#include "SqlResult.h"

namespace AsyncPg {

class ASYNCPGLIB SqlField;

/// Строка результата Sql запроса
class SqlRecord
{
public:
    /// Конструктор класса
    /// @param result Результата Sql запроса
    /// @param row Номер строки результата Sql запроса
    SqlRecord(const SqlResult &result, int row);

    /// Оператор сравнения на неравенство
    /// @param other Строка результата Sql запроса
    /// @return Результат сравнения
    bool operator!=(const SqlRecord &other) const;

    /// Оператор инкрементирования
    /// @return Результат инкрементирования
    SqlRecord &operator++();

    /// Оператор разименования
    /// @return Результат разименования
    SqlRecord operator*() const;

    /// Возвращает количество строк в результате Sql запроса
    /// @return Количество строк
    int rows() const;

    /// Возвращает количество колонок в строке результата Sql запроса
    /// @return Количество колонок в строке результата Sql запроса
    int columns() const;

    /// Возвращает номер строки результата Sql запроса
    /// @return Номер строки результата Sql запроса
    int row() const;

    /// Возвращает результат Sql запроса
    /// @return Результата Sql запроса
    const SqlResult &result() const;

    /// Возвращает указанное поле строки результата Sql запроса
    /// @param column Номер колонки
    /// @return Поле строки результата Sql запроса
    SqlField at(int column) const;

    /// Возвращает указанное поле строки результата Sql запроса
    /// @param column Наименование поля
    /// @return Поле строки результата Sql запроса
    SqlField at(std::string_view fieldName) const;

    /// Возвращает первое поле строки результата Sql запроса
    /// @return Первое поле строки результата Sql запроса
    SqlField begin() const;

    /// Возвращает последнее поле строки результата Sql запроса
    /// @return Последнее поле строки результата Sql запроса
    SqlField end() const;

private:
    const SqlResult &_result;
    int _row;
};

}
