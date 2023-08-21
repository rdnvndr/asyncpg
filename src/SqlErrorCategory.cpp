#include "SqlErrorCategory.h"

namespace AsyncPg {

struct ErrorCategory : std::error_category
{
    static const ErrorCategory &Instance();
    const char * name() const noexcept override;
    std::string message(int ev) const override;
};

const ErrorCategory &ErrorCategory::Instance()
{
    static ErrorCategory instance;
    return instance;
}

const char *ErrorCategory::name() const noexcept
{
    return "AsyncPg";
}

std::string ErrorCategory::message(int ev) const
{
    switch(static_cast<ErrorCode>(ev)) {
    case ErrorCode::ConnectionFailed:
        return "Connection to database failed.";
    case ErrorCode::ExecutionFailed:
        return "Execution sql query failed.";
    case ErrorCode::PreparationFailed:
        return "Preparation sql query failed.";
    case ErrorCode::CancelFailed:
        return "Can't stop current query.";
    default:
        return "(unrecognized error)";
    }
}

std::error_code make_error_code(ErrorCode e)
{
    return {static_cast<int>(e), ErrorCategory::Instance()};
}

}
