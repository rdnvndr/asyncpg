#pragma once

#include "global.h"

#include "SqlRecord.h"
#include "SqlValue.h"

namespace AsyncPg {

/// Поле строки результата Sql запроса
class ASYNCPGLIB SqlField
{
public:
    /// Конструктор класса
    /// @param record Строка результата Sql запроса
    /// @param col Номер колонки строки результата Sql запроса
    SqlField(const SqlRecord &record, int col = 0);

    /// Оператор сравнения на неравенство
    /// @param other Поле строки результата Sql запроса
    /// @return Результат сравнения
    bool operator!=(const SqlField &other) const;

    /// Оператор инкрементирования
    /// @return Результат инкрементирования
    SqlField &operator++();

    /// Оператор разыменования
    /// @return Результат разыменования
    SqlField operator*() const;

    /// Возвращает значение поля строки результата Sql запроса
    /// @return Значение поля строки результата Sql запроса
    SqlValue value() const;

    /// Возвращает значение поля строки результата Sql запроса
    /// @param I Тип поля строки результата Sql запроса
    /// @return Значение поля строки результата Sql запроса
    template<std::size_t I>
    std::variant_alternative_t<I, SqlValue>
    value() const
    {
        auto value = this->value();
        if (auto *result = std::get_if<I>(&value))
            return *result;
        return std::nullopt;
    }

    /// Возвращает количество строк в результате Sql запроса
    /// @return Количество строк
    int rows() const;

    /// Возвращает номер строки результата Sql запроса
    /// @return Номер строки результата Sql запроса
    int row() const;

    /// Возвращает количество колонок в строке результата Sql запроса
    /// @return Количество колонок в строке результата Sql запроса
    int columns() const;

    /// Возвращает номер колонки в строке результата Sql запроса
    /// @return Номер колонки в строке результата Sql запроса
    int column() const;

    /// Возвращает наименование колонки
    /// @return Наименование колонки
    std::string fieldName() const;

    /// Возвращает строку результата Sql запроса
    /// @return Строка результата Sql запроса
    const SqlRecord &record() const;

private:
    const SqlRecord &_record;
    int _col = 0;
};

}
