include(FetchContent)
FetchContent_Declare(Libevent
    GIT_REPOSITORY https://github.com/libevent/libevent.git
    GIT_TAG release-2.1.12-stable
    FIND_PACKAGE_ARGS NAMES Libevent
)
FetchContent_MakeAvailable(Libevent)
