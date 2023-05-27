#include "SqlRecord.h"

#include "SqlField.h"

namespace AsyncPg {

SqlRecord::SqlRecord(const SqlResult &result, int row)
    : _result(result), _row(row)
{

}

bool SqlRecord::operator!=(const SqlRecord &other) const
{
    return _row != other._row;
}

SqlRecord &SqlRecord::operator++()
{
    _row++;
    return *this;
}

SqlRecord SqlRecord::operator *() const
{
    return *this;
}

int SqlRecord::rows() const
{
    return _result.rows();
}

int SqlRecord::columns() const
{
    return _result.columns();
}

int SqlRecord::row() const
{
    return _row;
}

const SqlResult &SqlRecord::result() const
{
    return _result;
}

SqlField SqlRecord::begin() const
{
    return SqlField(*this, 0);
}

SqlField SqlRecord::end() const
{
    return SqlField(*this, columns());
}

}
