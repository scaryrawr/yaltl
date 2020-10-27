cmake_minimum_required(VERSION 3.11)
project("tofi"
        VERSION 0.1
        DESCRIPTION "Terminal based launcher"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(PkgConfig REQUIRED)

pkg_check_modules(GIOMM REQUIRED giomm-2.4)

include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
)

FetchContent_Declare(
    i3ipc++
    GIT_REPOSITORY https://github.com/scaryrawr/i3ipcpp
    GIT_TAG origin/main
)

set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(i3ipc++)

add_executable(tofi
               ./src/main.cpp
               ./src/tofi.cpp
               ./src/modes/dmenu.cpp
               ./src/modes/drun.cpp
               ./src/modes/i3wm.cpp
               ./src/modes/run.cpp
               ./src/modes/script.cpp
               ./src/utils/command.cpp
               ./src/utils/spawn.cpp)

target_include_directories(tofi PRIVATE include)

target_link_libraries(tofi ftxui::screen ftxui::dom ftxui::component i3ipc++ ${GIOMM_LIBRARIES})
target_include_directories(tofi PRIVATE ${GIOMM_INCLUDE_DIRS})
target_compile_options(tofi PRIVATE ${GIOMM_CFLAGS_OTHER})

install(TARGETS tofi)
