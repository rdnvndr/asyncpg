cmake_minimum_required(VERSION 3.5)

project(asyncpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_SOURCE_DIR}/cmake/libs.cmake")

find_package(PostgreSQL)
find_package(LibEvent)

target_link_libraries(asyncpg PRIVATE ${PostgreSQL_LIBRARIES} ${LibEvent_LIBRARIES})
