cmake_minimum_required(VERSION 3.10)
project(tst_main_mnl VERSION 1.0.0)

set(LIBRARIES asyncpg)
include(../manual.cmake)

find_package(LibEvent)
target_link_libraries(${PROJECT_NAME} ${LibEvent_LIBRARIES})
