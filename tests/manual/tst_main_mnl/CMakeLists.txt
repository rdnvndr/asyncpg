cmake_minimum_required(VERSION 3.10)
project(tst_main_mnl VERSION 1.0.0)

set(LIBRARIES
    asyncpg asio
)
include(../manual.cmake)

#target_link_libraries(tst_main_mnl PRIVATE asio)
