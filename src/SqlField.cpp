#include "SqlField.h"

#include "SqlValue.h"

namespace AsyncPg {

SqlField::SqlField(const SqlRecord &record, int col)
    : _record(record), _col(col)
{

}

bool SqlField::operator!=(const SqlField &other) const
{
    return _col != other._col;
}

SqlField &SqlField::operator++()
{
    _col++;
    return *this;
}

SqlField SqlField::operator*() const
{
    return *this;
}

SqlValue SqlField::value() const
{
    return asSqlValue(this->record().result().pgresult(), this->row(), this->column());
}

int SqlField::rows() const
{
    return _record.rows();
}

int SqlField::columns() const
{
    return _record.columns();
}

int SqlField::column() const
{
    return _col;
}

std::string SqlField::fieldName() const
{
    return _record.result().fieldName(_col);
}

int SqlField::row() const
{
    return _record.row();
}

const SqlRecord &SqlField::record() const
{
    return _record;
}

}
