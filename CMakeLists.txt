project(uasm)
cmake_minimum_required(VERSION 3.20)

set(SOURCES
    src/args.cpp
    src/assembler.cpp
    src/helper.cpp
    src/machine.cpp
    src/main.cpp
    src/log.cpp
)

set (CMAKE_CXX_STANDARD 20)


add_executable(uasm ${SOURCES})

if(MSVC)
  target_compile_options(uasm PRIVATE /W4 /WX)
else()
  target_compile_options(uasm PRIVATE -Wall -Wextra -pedantic -Werror)
endif()


target_include_directories(uasm PUBLIC inc)