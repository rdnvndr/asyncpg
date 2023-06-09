﻿include("${CMAKE_SOURCE_DIR}/cmake/main.cmake")

string(REGEX MATCH "\/3rdparty\/${PROJECT_NAME}\/src$" PARTY "${PROJECT_SOURCE_DIR}")
if ("${PARTY}" STREQUAL "/3rdparty/${PROJECT_NAME}/src"
    OR "${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")

    # Пути к бинарным файлам
    set(INCPATH  "include")
    set(RINCPATH "../include")
    set(DOCPATH "share/doc/${PROJECT_NAME}")
    if (UNIX)
        set(RLIBRARYPATH     "lib/${PROJECT_NAME}")
        set(RLIBRARYTESTPATH "lib/${PROJECT_NAME}")
    endif (UNIX)
    if (WIN32)
        set(RLIBRARYPATH     "./")
        set(RLIBRARYTESTPATH "./")
    endif (WIN32)
endif()

# Места нахождения бинарных файлов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}/${APPPATH}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARY_DIR}/${RLIBRARYPATH}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BINARY_DIR}/${RLIBRARYPATH}")

# Сборка проекта
include("${CMAKE_SOURCE_DIR}/cmake/rpath.cmake")
add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/${RINCPATH}
)
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# Установка проекта
install (TARGETS ${PROJECT_NAME} DESTINATION ${RLIBRARYPATH})
file(GLOB PUBHEADERS   "${CMAKE_SOURCE_DIR}/${RINCPATH}/${PROJECT_NAME}/*.h")
foreach(PUBFILENAME ${PUBHEADERS})
    string(REPLACE "${CMAKE_SOURCE_DIR}/${RINCPATH}/${PROJECT_NAME}/" "" FILENAME ${PUBFILENAME})
    install(FILES ${FILENAME} DESTINATION "${RINCPATH}/${PROJECT_NAME}")
endforeach()
