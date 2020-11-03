cmake_minimum_required(VERSION 3.16)
project("tofi"
        VERSION 0.1
        DESCRIPTION "Terminal based launcher"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND SOURCES
    ./src/main.cpp
    ./src/tofi.cpp
    ./src/modes/dmenu.cpp
    ./src/modes/run.cpp
    ./src/modes/script.cpp
    ./src/utils/command.cpp
    ./src/utils/regex.cpp
)

list(APPEND LIBRARIES
    ftxui::screen
    ftxui::dom
    ftxui::component    
    mtl)

list(APPEND INCLUDE_DIRS
    ./include
    "${PROJECT_BINARY_DIR}")

list(APPEND CFLAGS -Wall)

include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
)

FetchContent_Declare(
    mtl
    GIT_REPOSITORY https://github.com/scaryrawr/mtl
    GIT_TAG origin/main
)

FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(mtl)

if(WIN32)
    FetchContent_Declare(
        WIL
        GIT_REPOSITORY https://github.com/microsoft/wil
    )

    FetchContent_MakeAvailable(WIL)

    FetchContent_Declare(
        getopt
        GIT_REPOSITORY https://github.com/scaryrawr/getopt
        GIT_TAG origin/main
    )

    FetchContent_MakeAvailable(getopt)

    list(APPEND SOURCES ./src/utils/win32/spawn.cpp)
    list(APPEND LIBRARIES WIL getopt)
else()
    list(APPEND SOURCES ./src/utils/posix/spawn.cpp)
endif()

find_package(PkgConfig)
if(${PKGCONFIG_FOUND})
    pkg_check_modules(PCRE2 libpcre2-32)
    if(${PCRE2_FOUND})
        list(APPEND SOURCES ./src/utils/regex_pcre.cpp)
        list(APPEND CFLAGS -DPCRE2_CODE_UNIT_WIDTH=32 ${PCRE2_CFLAGS})
        list(APPEND LIBRARIES ${PCRE2_LIBRARIES})
        list(APPEND INCLUDE_DIRS ${PCRE2_INCLUDE_DIRS})
    else()
        list(APPEND SOURCES ./src/utils/regex_stl.cpp)
    endif()

    pkg_check_modules(GIOMM giomm-2.4)
    if(${GIOMM_FOUND})
        list(APPEND LIBRARIES ${GIOMM_LIBRARIES})
        list(APPEND INCLUDE_DIRS ${GIOMM_INCLUDE_DIRS})
        list(APPEND CFLAGS ${GIOMM_CFLAGS})
        list(APPEND SOURCES ./src/modes/drun.cpp)
    endif()

    pkg_check_modules(GTKMM gtkmm-3.0)
    if(${GTKMM_FOUND})
        list(APPEND LIBRARIES ${GTKMM_LIBRARIES})
        list(APPEND INCLUDE_DIRS ${GTKMM_INCLUDE_DIRS})
        list(APPEND CFLAGS ${GTKMM_CFLAGS})
        list(APPEND SOURCES ./src/modes/recent.cpp)
    endif()

    # I3IPC depends on Package Config
    option(I3IPC_FOUND "Build with i3ipc enabled" ON)
    if(I3IPC_FOUND)
        FetchContent_Declare(
            i3ipc++
            GIT_REPOSITORY https://github.com/scaryrawr/i3ipcpp
            GIT_TAG origin/main
        )

        FetchContent_MakeAvailable(i3ipc++)
        list(APPEND LIBRARIES i3ipc++)
        list(APPEND SOURCES ./src/modes/i3wm.cpp)
    endif()
else()
    list(APPEND SOURCES ./src/utils/regex_stl.cpp)
endif()

configure_file(TofiConfig.h.cmake TofiConfig.h)

add_executable(tofi ${SOURCES})

target_link_libraries(tofi PRIVATE ${LIBRARIES})
target_include_directories(tofi PRIVATE ${INCLUDE_DIRS})
target_compile_options(tofi PRIVATE ${CFLAGS})

install(TARGETS tofi)
