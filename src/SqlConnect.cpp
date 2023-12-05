#include "SqlConnect.h"
#include "SqlError.h"
#include "SqlValue.h"

#include <event.h>
#include <event2/event.h>
#include <libpq-fe.h>

#include <iostream>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace AsyncPg {

static void ev_connecting(evutil_socket_t /*fd*/, short /*what*/, void *arg)
{
    auto *sqlConnect = reinterpret_cast<SqlConnect *>(arg);
    sqlConnect->connecting();
}

static void ev_preparing(evutil_socket_t /*fd*/, short /*what*/, void *arg)
{
    auto *sqlConnect = reinterpret_cast<SqlConnect *>(arg);
    sqlConnect->preparing();
}

static void ev_executing(evutil_socket_t /*fd*/, short /*what*/, void *arg)
{
    auto *sqlConnect = reinterpret_cast<SqlConnect *>(arg);
    sqlConnect->executing();
}

SqlConnect::SqlConnect(std::string_view connInfo, event_base *evbase)
{
    _evbase = evbase;
    _connInfo = connInfo;
    _connect = PQconnectStart(connInfo.data());
    if (PQstatus(_connect) == CONNECTION_BAD) {
        _error = SqlError(ErrorCode::ConnectionFailed, PQerrorMessage(_connect));
        return;
    }
#ifdef _WIN32
    _socket = _dup(PQsocket(_connect));
#else
    _socket = dup(PQsocket(_connect));
#endif
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
            self->_error = SqlError(ErrorCode::ExecutionFailed, PQerrorMessage(self->connect()));
            self->pop();
            return;
        }
        self->_error.clear();

        auto event = event_new(self->_evbase, self->_socket, EV_READ, ev_executing, self);
        event_add(event, nullptr);
    };
    push(callback);
}

void SqlConnect::execute(std::string_view sql, std::vector<SqlValue> params)
{
    auto callback = [sql, params = std::move(params)](SqlConnect *self) {
        const auto nParams = params.size();
        auto *types = new unsigned int[nParams];
        char **values = new char* [nParams];
        int *lengths = new int[nParams];
        int *formats = new int[nParams];

        for (std::size_t i = 0, e = params.size(); i < e; ++i) {
            const auto &[oid, length, value] = asPgValue(params[i]);
            types[i] = oid;
            values[i] = value;
            lengths[i] = static_cast<int>(length);
            formats[i] = 1;
        }

        auto result = PQsendQueryParams(
            self->connect(), sql.data(), static_cast<int>(nParams),
            types, values, lengths, formats, 1);

        for (std::size_t i = 0, e = params.size(); i < e; ++i)
            delete[] values[i];

        delete[] types;
        delete[] values;
        delete[] lengths;
        delete[] formats;

        if (result != 1) {
            self->_error = SqlError(ErrorCode::ExecutionFailed, PQerrorMessage(self->connect()));
            self->pop();
            return;
        }
        self->_error.clear();

        auto event = event_new(self->_evbase, self->_socket, EV_READ, ev_executing, self);
        event_add(event, nullptr);
    };
    push(callback);
}

void SqlConnect::prepare(std::string_view sql, std::vector<SqlType> sqlTypes)
{
    auto callback = [sql, sqlTypes = std::move(sqlTypes)](SqlConnect *self) {
        const auto nTypes = sqlTypes.size();
        auto *types = new unsigned int[nTypes];
        for (std::size_t i = 0, e = nTypes; i < e; ++i) {
            types[i] = toPgType(sqlTypes[i]);
        }

        auto result = PQsendPrepare(
            self->connect(), "", sql.data(), static_cast<int>(nTypes), types);

        delete[] types;

        if (result != 1) {
            self->_error = SqlError(ErrorCode::PreparationFailed, PQerrorMessage(self->connect()));
            self->pop();
            return;
        }
        self->_error.clear();

        auto event = event_new(self->_evbase, self->_socket, EV_READ, ev_preparing, self);
        event_add(event, nullptr);
    };
    push(callback);
}

void SqlConnect::execute(std::vector<SqlValue> params)
{
    auto callback = [params = std::move(params)](SqlConnect *self) {
        const auto nParams = params.size();
        char **values = new char* [nParams];
        int *lengths = new int[nParams];
        int *formats = new int[nParams];

        for (std::size_t i = 0, e = params.size(); i < e; ++i) {
            const auto &[oid, length, value] = asPgValue(params[i]);
            values[i] = value;
            lengths[i] = static_cast<int>(length);
            formats[i] = 1;
        }

        auto result = PQsendQueryPrepared(
            self->connect(), "", static_cast<int>(nParams), values, lengths, formats, 1);

        for (std::size_t i = 0, e = params.size(); i < e; ++i)
            delete[] values[i];

        delete[] values;
        delete[] lengths;
        delete[] formats;

        if (result != 1) {
            self->_error = SqlError(ErrorCode::ExecutionFailed, PQerrorMessage(self->connect()));
            self->pop();
            return;
        }
        self->_error.clear();

        auto event = event_new(self->_evbase, self->_socket, EV_READ, ev_executing, self);
        event_add(event, nullptr);
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
        _error = SqlError(ErrorCode::CancelFailed, PQerrorMessage(_connect));
    PQfreeCancel(cancelObject);

    return canceled;
}

void SqlConnect::post(Callback func)
{
    auto callback = [func = std::move(func)](SqlConnect *self) {
        func(self);
        self->pop();
    };
    push(callback);
}

SqlConnect SqlConnect::clone()
{
    return SqlConnect(_connInfo, _evbase);
}

void SqlConnect::connecting()
{
    auto ret = PQconnectPoll(_connect);
    switch (ret) {
    case PGRES_POLLING_READING: {
        auto event = event_new(_evbase, _socket, EV_READ, ev_connecting, this);
        event_add(event, nullptr);
    } break;
    case PGRES_POLLING_WRITING: {
        auto event = event_new(_evbase, _socket, EV_WRITE, ev_connecting, this);
        event_add(event, nullptr);
    } break;
    case PGRES_POLLING_OK:
        pop();
        break;
    case PGRES_POLLING_FAILED:
        _error = SqlError(ErrorCode::ConnectionFailed, PQerrorMessage(_connect));
        break;
    default:
        break;
    }
}

void SqlConnect::preparing()
{
    auto pgconn = connect();
    if (PQconsumeInput(pgconn) != 1) {
        _error = SqlError(ErrorCode::PreparationFailed, PQerrorMessage(pgconn));
        pop();
        return;
    }

    if (PQisBusy(pgconn) == 1) {
        auto event = event_new(_evbase, _socket, EV_READ, ev_preparing, this);
        event_add(event, nullptr);
        return;
    }

    if (auto pgResult = PQgetResult(pgconn)) {
        if (PQresultStatus(pgResult) != PGRES_COMMAND_OK)
            _error = SqlError(ErrorCode::PreparationFailed, PQerrorMessage(pgconn));
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
        _error = SqlError(ErrorCode::ExecutionFailed, PQerrorMessage(pgconn));
        pop();
        return;
    }

    if (PQisBusy(pgconn) == 1) {
        auto event = event_new(_evbase, _socket, EV_READ, ev_executing, this);
        event_add(event, nullptr);
        return;
    }

    if (auto pgResult = PQgetResult(pgconn)) {
        if (PQresultStatus(pgResult) == PGRES_TUPLES_OK) {
            _result = SqlResult(pgResult);
        } else {
            _error = SqlError(ErrorCode::ExecutionFailed, PQerrorMessage(pgconn));
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
