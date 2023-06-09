﻿# Файлы для сборки
file(GLOB HEADERS   "*.h")
file(GLOB SOURCES   "*.cpp")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

# Пути к бинарным файлам
set(BINARY_DIR "${CMAKE_BINARY_DIR}")
set(APPPATH  "bin")
set(INCPATH  "include/${CMAKE_PROJECT_NAME}")
set(RINCPATH "include")
set(DOCPATH  "share/doc/${CMAKE_PROJECT_NAME}")
set(TESTPATH "bin")
set(RPLUGINPATH "lib/${CMAKE_PROJECT_NAME}/plugins")
add_definitions(-DPLUGIN_DIR="""../${RPLUGINPATH}""")

if (UNIX)
    set(RLIBRARYPATH     "lib/${CMAKE_PROJECT_NAME}")
    set(RLIBRARYTESTPATH "lib/${CMAKE_PROJECT_NAME}")
endif (UNIX)
if (WIN32)
    set(RLIBRARYPATH     "${APPPATH}")
    set(RLIBRARYTESTPATH "${APPPATH}")
endif (WIN32)

