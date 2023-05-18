#pragma once

#include "SqlError.h"
#include "SqlResult.h"
#include "SqlValue.h"

#include "asio/io_service.hpp"
#include "asio/local/stream_protocol.hpp"

#include <libpq-fe.h>

#include <queue>

namespace AsyncPg {

/// Соединение с базой данных
class SqlConnect
{
public:
    /// Функция обратного вызова
    using Callback = std::function<void(SqlConnect *)>;

    /// Конструктор класса
    /// @param connInfo Строка соединения с базой данных в URI формате
    /// @param service Сервис ввода-вывода
    explicit SqlConnect(std::string_view connInfo, asio::io_service *service = nullptr);

    /// Деструктор класса
    ~SqlConnect();

    /// Выполняет запрос к базе данных
    /// @param sql Запрос к базе данных
    void execute(std::string_view sql);

    /// Выполняет параметрический запрос к базе данных
    /// @param sql Запрос к базе данных
    /// @param params Параметры запроса
    void execute(std::string_view sql, std::vector<SqlValue> params);

    /// Создаёт параметрический запрос к базе данных
    /// @param sql Запрос к базе данных
    /// @param sqlTypes Типы параметров
    void prepare(std::string_view sql, std::vector<SqlType> sqlTypes);

    /// Выполняет подготовленный параметрический запрос к базе данных
    /// @param params Параметры запроса
    void execute(std::vector<SqlValue> params);

    /// Отменяет запрос к базе данных
    /// @return Результат операции
    bool cancel();

    /// Устанавливает обработчик результата выполнения запроса
    /// @param func Функция обратного вызова
    void post(const Callback &func);

    /// Создаёт копию текущего соединения с базой данных
    /// @return Соединение с базой данных
    SqlConnect clone();

    /// Возвращает ошибку выполнения запроса
    /// @return Ошибка выполнения запроса
    const SqlError &error() const;

    /// Возвращает результат выполнения запроса
    /// @return Результат выполнения запроса
    const SqlResult &result() const;

protected:
    /// Возвращает соединение PostgreSql
    /// @return Соединение PostgreSql
    PGconn *connect();

    /// Возвращает сокет
    /// @return Сокет
    asio::local::stream_protocol::socket socket();

    /// Возвращает сервис ввода-вывода
    /// @return Сервис ввода-вывода
    static asio::io_service *service();

    /// Производит соединение с PostgreSql
    void connecting();

    /// Производит подготовку параметрического SQL запроса
    void preparing();

    /// Производит запуск SQL запроса
    void executing();

    /// Добавляет обработчик результата SQL запроса в очередь
    /// @param callback Функция обратного вызова
    /// @return Была ли вызван callback
    bool push(const Callback &callback);

    /// Убирает обработчик результата SQL запроса из очереди
    void pop();

private:
    asio::io_service     *_service = nullptr;
    std::queue<Callback>  _callbackQueue;
    PGconn               *_connect = nullptr;
    std::string           _connInfo;
    SqlError              _error;
    SqlResult             _result;
    bool                  _isExec = true;
};

}
