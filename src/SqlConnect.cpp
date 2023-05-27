#include "SqlConnect.h"
#include "SqlValue.h"

#include "asio/io_service.hpp"
#include "asio/local/stream_protocol.hpp"

#include <iostream>
#include <libpq-fe.h>
#include <thread>

namespace AsyncPg {

static asio::local::stream_protocol::socket socket(asio::io_context *service, PGconn *connect)
{
    asio::local::stream_protocol stream_protocol;
    auto *serv = service ? service : SqlConnect::service();
    return asio::local::stream_protocol::socket(*serv, stream_protocol, dup(PQsocket(connect)));
}

SqlConnect::SqlConnect(std::string_view connInfo, asio::io_context *service)
{
    _service = service;
    _connInfo = connInfo;
    _connect = PQconnectStart(connInfo.data());
    if (PQstatus(_connect) == CONNECTION_BAD) {
        _error = SqlError("Connection to database failed.", PQerrorMessage(_connect));
        return;
    }

    connecting();
}

SqlConnect::~SqlConnect()
{
    if (_connect)
        PQfinish(_connect);
}

void SqlConnect::execute(std::string_view sql)
{
    auto callback = [sql](SqlConnect *self) {
        auto result =
            PQsendQueryParams(self->connect(), sql.data(), 0, nullptr, nullptr, nullptr, nullptr, 1);
        if (result != 1) {
            self->_error = SqlError("Execution sql query failed.", PQerrorMessage(self->connect()));
            self->pop();
            return;
        }
        self->_error.clear();
        socket(self->_service, self->_connect).async_read_some(
            asio::null_buffers(),
            [self]([[maybe_unused]] std::error_code code, [[maybe_unused]] std::size_t size) {
                self->executing();
            });
    };
    push(callback);
}

void SqlConnect::execute(std::string_view sql, std::vector<SqlValue> params)
{
    auto callback = [sql, params = std::move(params)](SqlConnect *self) {
        const auto nParams = params.size();
        unsigned int types[nParams];
        char *values[nParams];
        int lengths[nParams];
        int formats[nParams];

        for (std::size_t i = 0, e = params.size(); i < e; ++i) {
            const auto &[oid, length, value] = asPgValue(params[i]);
            types[i] = oid;
            values[i] = value;
            lengths[i] = length;
            formats[i] = 1;
        }

        auto result = PQsendQueryParams(
            self->connect(), sql.data(), nParams, types, values, lengths, formats, 1);

        for (std::size_t i = 0, e = params.size(); i < e; ++i)
            delete[] values[i];

        if (result != 1) {
            self->_error = SqlError("Execution sql query failed.", PQerrorMessage(self->connect()));
            self->pop();
            return;
        }
        self->_error.clear();
        socket(self->_service, self->_connect).async_read_some(
            asio::null_buffers(),
            [self]([[maybe_unused]] std::error_code code, [[maybe_unused]] std::size_t size) {
                self->executing();
            });
    };
    push(callback);
}

void SqlConnect::prepare(std::string_view sql, std::vector<SqlType> sqlTypes)
{
    auto callback = [sql, sqlTypes = std::move(sqlTypes)](SqlConnect *self) {
        const auto nTypes = sqlTypes.size();
        unsigned int types[nTypes];
        for (std::size_t i = 0, e = nTypes; i < e; ++i) {
            types[i] = toPgType(sqlTypes[i]);
        }

        auto result = PQsendPrepare(
            self->connect(), "", sql.data(), nTypes, types);

        if (result != 1) {
            self->_error = SqlError("Preparation sql query failed.", PQerrorMessage(self->connect()));
            self->pop();
            return;
        }
        self->_error.clear();
        socket(self->_service, self->_connect).async_read_some(
            asio::null_buffers(),
            [self]([[maybe_unused]] std::error_code code, [[maybe_unused]] std::size_t size) {
                self->preparing();
            });
    };
    push(callback);
}

void SqlConnect::execute(std::vector<SqlValue> params)
{
    auto callback = [params = std::move(params)](SqlConnect *self) {
        const auto nParams = params.size();
        char *values[nParams];
        int lengths[nParams];
        int formats[nParams];

        for (std::size_t i = 0, e = params.size(); i < e; ++i) {
            const auto &[oid, length, value] = asPgValue(params[i]);
            values[i] = value;
            lengths[i] = length;
            formats[i] = 1;
        }

        auto result = PQsendQueryPrepared(
            self->connect(), "", nParams, values, lengths, formats, 1);

        for (std::size_t i = 0, e = params.size(); i < e; ++i)
            delete[] values[i];

        if (result != 1) {
            self->_error = SqlError("Execution sql query failed.", PQerrorMessage(self->connect()));
            self->pop();
            return;
        }
        self->_error.clear();
        socket(self->_service, self->_connect).async_read_some(
            asio::null_buffers(),
            [self]([[maybe_unused]] std::error_code code, [[maybe_unused]] std::size_t size) {
                self->executing();
            });
    };
    push(callback);
}

bool SqlConnect::cancel()
{
    while (!_callbackQueue.empty())
        _callbackQueue.pop();

    char errorBuffer[256];
    auto cancelObject = PQgetCancel(_connect);
    bool canceled = (PQcancel(cancelObject, errorBuffer, sizeof(errorBuffer)) != 0);

    if (!canceled)
        _error = SqlError("Can't stop current query.", PQerrorMessage(_connect));
    PQfreeCancel(cancelObject);

    return canceled;
}

void SqlConnect::post(const Callback &func)
{
    auto callback = [&func](SqlConnect *self) {
        func(self);
        self->pop();
    };
    push(callback);
}

SqlConnect SqlConnect::clone()
{
    return SqlConnect(_connInfo, _service);
}

asio::io_context *SqlConnect::service()
{
    static bool started = false;
    static asio::io_context service{static_cast<int>(std::thread::hardware_concurrency())};
    if (!started) {
        std::thread thread([](){
            asio::io_context::work work(*SqlConnect::service());
            SqlConnect::service()->run();
        });
        thread.detach();
        started = true;
    }

    return &service;
}

void SqlConnect::connecting()
{
    auto ret = PQconnectPoll(_connect);
    switch (ret) {
    case PGRES_POLLING_READING:
        socket(_service, _connect).async_read_some(
            asio::null_buffers(),
            [this]([[maybe_unused]] std::error_code code, [[maybe_unused]] std::size_t size) {
                connecting();
            });
        break;

    case PGRES_POLLING_WRITING:
        socket(_service, _connect).async_write_some(
            asio::null_buffers(),
            [this]([[maybe_unused]] std::error_code code, [[maybe_unused]] std::size_t size) {
                connecting();
            });
        break;

    case PGRES_POLLING_OK:
        pop();
        break;

    case PGRES_POLLING_FAILED:
        _error = SqlError("Connection to database failed.", PQerrorMessage(_connect));
        break;

    default:
        break;
    }
}

void SqlConnect::preparing()
{
    auto pgconn = connect();
    if (PQconsumeInput(pgconn) != 1) {
        _error = SqlError("Preparation sql query failed.", PQerrorMessage(pgconn));
        pop();
        return;
    }

    if (PQisBusy(pgconn) == 1) {
        socket(_service, _connect).async_read_some(
                asio::null_buffers(),
                [this]([[maybe_unused]] std::error_code code, [[maybe_unused]] std::size_t size) {
                    preparing();
                });
        return;
    }

    if (auto pgResult = PQgetResult(pgconn)) {
        if (PQresultStatus(pgResult) != PGRES_COMMAND_OK)
            _error = SqlError("Preparation sql query failed.", PQerrorMessage(pgconn));
        PQclear(pgResult);
    }

    while (auto pgResult = PQgetResult(pgconn))
        PQclear(pgResult);
    pop();
}

void SqlConnect::executing()
{
    auto pgconn = connect();
    if (PQconsumeInput(pgconn) != 1) {
        _error = SqlError("Execution sql query failed.", PQerrorMessage(pgconn));
        pop();
        return;
    }

    if (PQisBusy(pgconn) == 1) {
        socket(_service, _connect).async_read_some(
            asio::null_buffers(),
            [this]([[maybe_unused]] std::error_code code, [[maybe_unused]] std::size_t size) {
                executing();
            });
        return;
    }

    if (auto pgResult = PQgetResult(pgconn)) {
        if (PQresultStatus(pgResult) == PGRES_TUPLES_OK) {
            _result = SqlResult(pgResult);
        } else {
            _error = SqlError("Execution sql query failed.", PQerrorMessage(pgconn));
            PQclear(pgResult);
        }
    }

    while (auto pgResult = PQgetResult(pgconn))
        PQclear(pgResult);
    pop();
}

void SqlConnect::pop()
{
    if (!_callbackQueue.empty()) {
        auto callback = _callbackQueue.front();
        _callbackQueue.pop();
        callback(this);
    } else {
        _isExec = false;
    }
}

const SqlResult &SqlConnect::result() const
{
    return _result;
}

const SqlError &SqlConnect::error() const
{
    return _error;
}

bool SqlConnect::push(const SqlConnect::Callback &callback)
{
    bool isCall = false;
    if (_isExec) {
        _callbackQueue.push(callback);
    } else {
        callback(this);
        _isExec = true;
        isCall = true;
    }

    return isCall;
}

struct pg_conn *SqlConnect::connect()
{
    return _connect;
}

}
