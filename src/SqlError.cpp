#include "SqlError.h"

namespace AsyncPg {

SqlError::SqlError(const SqlError &other)
    : _isError(other._isError), _errorMessage(other._errorMessage),
      _driverMessage(other._driverMessage)
{ }

SqlError::SqlError(std::string_view errorMessage, std::string_view driverError)
    : _isError(true), _errorMessage(errorMessage), _driverMessage(driverError)
{ }

const char *SqlError::what() const noexcept
{
    return _errorMessage.c_str();
}

const std::string &SqlError::errorMessage() const
{
    return _errorMessage;
}

const std::string &SqlError::driverMessage() const
{
    return _driverMessage;
}

SqlError::operator bool() const
{
    return _isError;
}

void SqlError::clear()
{
    _isError = false;
    _errorMessage.clear();
    _driverMessage.clear();
}

}
