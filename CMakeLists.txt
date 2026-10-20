cmake_minimum_required(VERSION 3.20)
project(FluxionEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_definitions(UNICODE _UNICODE)
    add_compile_options(/utf-8)
endif()

add_subdirectory(Engine)
add_subdirectory(App)

# add_subdirectory(Editor)