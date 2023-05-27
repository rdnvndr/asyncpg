#include <asyncpg/SqlConnect.h>
#include <asyncpg/SqlField.h>
#include <asyncpg/SqlResult.h>
#include <asyncpg/SqlRecord.h>
#include <asyncpg/SqlValue.h>

#include <asio/io_service.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <iostream>
#include <thread>
#include <variant>

int main(int argc, char* argv[])
{
    asio::io_service service{static_cast<int>(std::thread::hardware_concurrency())};
    AsyncPg::SqlConnect connect("postgresql://postgres:1@localhost/RPLMCAPP", &service);

    connect.prepare(R"(SELECT * FROM "dbo"."test" WHERE "a" >= $1)", {AsyncPg::SqlType::Decimal});
    connect.execute({AsyncPg::makeSqlValue<AsyncPg::SqlType::Decimal>("1.001")});

//    connect.execute(
//        R"(SELECT * FROM "dbo"."test" WHERE "a" >= $1)",
//        {AsyncPg::makeSqlValue<AsyncPg::SqlType::Decimal>("1.001")});
    connect.post([](AsyncPg::SqlConnect *self) {
        if (self->error())
            return;

        for (const auto &record : self->result()) {
            for (const auto &field : record) {
                if (auto value = field.value<AsyncPg::SqlType::Decimal>())
                    std::cout << *value;
                else
                    std::cout << "NULL";
                std::cout << std::endl;
            }
        }
    });
    service.run();
}
