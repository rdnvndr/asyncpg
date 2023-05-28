cmake_minimum_required(VERSION 3.10)
project(tst_main_mnl VERSION 1.0.0)

set(LIBRARIES asyncpg)
include(../manual.cmake)

include("${CMAKE_SOURCE_DIR}/cmake/libevent.cmake")
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBEVENT_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBEVENT_LIBRARIES})
