set(LibEvent_EXTRA_PREFIXES /usr /usr/local /opt/local "$ENV{HOME}")
foreach(prefix ${LibEvent_EXTRA_PREFIXES})
    list(APPEND LibEvent_INCLUDE_PATHS "${prefix}/include")
    list(APPEND LibEvent_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(LibEvent_INCLUDE_DIR event.h PATHS ${LibEvent_INCLUDE_PATHS})
find_library(LibEvent_LIBRARIES NAMES event PATHS ${LibEvent_LIB_PATHS})

if (LibEvent_LIBRARIES AND LibEvent_INCLUDE_DIR)
  set(LibEvent_FOUND TRUE)
  set(LibEvent_LIBRARIES ${LIBEVENT_LIB})
else ()
  set(LibEvent_FOUND FALSE)
endif ()

if (LibEvent_FOUND)
  if (NOT LibEvent_FIND_QUIETLY)
    message(STATUS "Found LibEvent: ${LIBEVENT_LIB}")
  endif ()
else ()
    if (LibEvent_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find LibEvent.")
    endif ()
    message(STATUS "LibEvent NOT found.")
endif ()

mark_as_advanced(
    LIBEVENT_LIB
    LibEvent_INCLUDE_DIR
)
