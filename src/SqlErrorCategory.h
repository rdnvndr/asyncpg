#pragma once

#include <system_error>

namespace AsyncPg { enum class ErrorCode; }
namespace std { template<> struct is_error_code_enum<AsyncPg::ErrorCode>: true_type {}; }

namespace AsyncPg {

enum class ErrorCode {
    Ok                = 0,
    ConnectionFailed  = 1,
    ExecutionFailed   = 2,
    PreparationFailed = 3,
    CancelFailed      = 4,
};

std::error_code make_error_code(AsyncPg::ErrorCode e);

}
