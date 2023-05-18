cmake_minimum_required(VERSION 3.5)

project(asyncpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB HEADERS   "*.h")
file(GLOB SOURCES   "*.cpp")

add_executable(asyncpg ${HEADERS} ${SOURCES})
find_package(PostgreSQL)

include(FetchContent)
FetchContent_Declare(asio
  GIT_REPOSITORY git@github.com:chriskohlhoff/asio.git
  GIT_TAG master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  )
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
  FetchContent_Populate(asio)
endif()
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)

target_link_libraries(asyncpg PRIVATE ${PostgreSQL_LIBRARIES} asio)

install(TARGETS asyncpg  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
