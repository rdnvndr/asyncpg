#include "SqlResult.h"

#include "SqlRecord.h"

#include <libpq-fe.h>

#include <iostream>

namespace AsyncPg {

SqlResult::SqlResult(PGresult *pgresult)
{
    _result = pgresult;
    if (_result) {
        _rows   = PQntuples(_result);
        _columns = PQnfields(_result);
    }
}

SqlResult::SqlResult(SqlResult &&other) noexcept
{
    _result = other._result;
    _rows = other._rows;
    _columns = other._columns;

    other._result = nullptr;
}

SqlResult &SqlResult::operator=(SqlResult &&other) noexcept
{
    _result = other._result;
    _rows = other._rows;
    _columns = other._columns;

    other._result = nullptr;

    return *this;
}

SqlResult::~SqlResult()
{
    if (_result)
        PQclear(_result);
}

int SqlResult::rows() const
{
    return _rows;
}

int SqlResult::columns() const
{
    return _columns;
}

std::string SqlResult::fieldName(int column) const
{
    return std::string(PQfname(_result, column));
}

int SqlResult::column(std::string_view fieldName) const
{
    return PQfnumber(_result, fieldName.data());
}

pg_result *SqlResult::pgresult() const
{
    return _result;
}

bool SqlResult::operator!() const
{
    if (_result) {
        switch(PQresultStatus(_result)) {
        case PGRES_EMPTY_QUERY:     /* empty query string was executed */
        case PGRES_COMMAND_OK:      /* a query command that doesn't return
                                            * anything was executed properly by the
                                            * backend */
        case PGRES_TUPLES_OK:       /* a query command that returns tuples was
                                            * executed properly by the backend, PGresult
                                            * contains the result tuples */
        case PGRES_COPY_OUT:        /* Copy Out data transfer in progress */
        case PGRES_COPY_IN:         /* Copy In data transfer in progress */
        case PGRES_COPY_BOTH:       /* Copy In/Out data transfer in progress */
        case PGRES_NONFATAL_ERROR:  /* notice or warning message */
        case PGRES_SINGLE_TUPLE:    /* single tuple from larger resultset */
            return false;

        case PGRES_BAD_RESPONSE:    /* an unexpected response was recv'd from the
                                            * backend */
        case PGRES_FATAL_ERROR:     /* query failed */
            return true;
        }
    }
    return true;
}

SqlRecord SqlResult::at(int row) const
{
    return SqlRecord(*this, row);
}

SqlRecord SqlResult::begin() const
{
    return SqlRecord(*this, 0);
}

SqlRecord SqlResult::end() const
{
    return SqlRecord(*this, rows());
}

}
