cmake_minimum_required(VERSION 3.20)
project(FluxionEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_definitions(UNICODE _UNICODE)
    add_compile_options(/utf-8)
endif()

include(FetchContent)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    DirectXTex
    GIT_REPOSITORY https://github.com/microsoft/DirectXTex.git
    GIT_TAG main
)
FetchContent_MakeAvailable(DirectXTex)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(DirectXTex PROPERTIES FOLDER "ThirdParty")

set(DXC_TAG "v1.9.2609")
set(DXC_ZIP_NAME "dxc_2026_09_29.zip")
set(DXC_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${DXC_TAG}/${DXC_ZIP_NAME}")

FetchContent_Declare(
    dxc
    URL ${DXC_URL}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(ENABLE_SPIRV_CODEGEN OFF CACHE BOOL "" FORCE)
set(DXC_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(dxc)

add_library(FluxionDXC INTERFACE)
target_include_directories(FluxionDXC INTERFACE ${dxc_SOURCE_DIR}/inc)
target_link_libraries(FluxionDXC INTERFACE ${dxc_SOURCE_DIR}/lib/x64/dxcompiler.lib)
set_target_properties(FluxionDXC PROPERTIES FOLDER "ThirdParty")

add_subdirectory(Engine)
add_subdirectory(App)

# add_subdirectory(Editor)