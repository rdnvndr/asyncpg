cmake_minimum_required(VERSION 3.10)
project(asyncpg VERSION 1.0.0)

add_subdirectory(3rdparty)
add_subdirectory(src)

if ( ${CMAKE_TESTING_ENABLED} )
    add_subdirectory(tests)
endif()
