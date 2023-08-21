#include "SqlError.h"

namespace AsyncPg {

SqlError::SqlError(ErrorCode code, std::string_view driverError)
    : std::error_code(code), _driverMessage(driverError)
{ }

const std::string &SqlError::driverMessage() const
{
    return _driverMessage;
}

void SqlError::clear()
{
    std::error_code::clear();
    _driverMessage.clear();
}

}
