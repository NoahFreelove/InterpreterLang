cmake_minimum_required(VERSION 3.27)
project(InterpreterLang)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES "lang/*.cpp" "lang/tokenizer/*.cpp")

add_executable(InterpreterLang main.cpp "lang/lang.h" ${SOURCE_FILES})
