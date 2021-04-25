project(uasm)

set(SOURCES
    src/args.cpp
    src/assembler.cpp
    src/helper.cpp
    src/machine.cpp
    src/main.cpp  
)

add_executable(uasm ${SOURCES})
target_include_directories(uasm PUBLIC inc)