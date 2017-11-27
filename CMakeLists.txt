cmake_minimum_required(VERSION 3.9)
project(ast-variants CXX)

include_directories(include)

if(MSVC)
    add_compile_options(/std:c++17 /W4 /wd4100 /wd4456 /permissive- /GR-)
else()
    add_compile_options(
        -flto-visiblity-public-std
        -std=c++17
        -Wall
        -Wextra
        -pedantic
        -fno-ms-compatibility
        -fno-delayed-template-parsing
        -fno-rtti
    )
endif()

add_executable(ast source/main.cpp)
