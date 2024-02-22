cmake_minimum_required(VERSION 3.27)
project(InterpreterLang)

set(CMAKE_CXX_STANDARD 17)
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
endif()
file(GLOB_RECURSE SOURCE_FILES "lang/*.cpp" "lang/tokenizer/*.cpp" "lang/evaluator/*.cpp" "lang/memory/*.cpp")

add_executable(InterpreterLang main.cpp "lang/lang.h" "lang/memory/memory.h" ${SOURCE_FILES})
