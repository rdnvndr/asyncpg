﻿cmake_minimum_required(VERSION 3.5)

project(asyncpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_SOURCE_DIR}/cmake/libs.cmake")
target_compile_definitions(${PROJECT_NAME} PRIVATE ASYNCPG_LIBRARY)

find_package(PostgreSQL)
target_include_directories(${PROJECT_NAME} PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PostgreSQL_LIBRARIES})

include("${CMAKE_SOURCE_DIR}/cmake/libevent.cmake")
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBEVENT_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBEVENT_LIBRARIES})
