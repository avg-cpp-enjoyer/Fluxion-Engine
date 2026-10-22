file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS 
    "src/*.cpp"
    "include/*.hpp"
    "include/*.inl"
)

add_library(FluxionEngine STATIC ${ENGINE_SOURCES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_SOURCES})

target_compile_definitions(FluxionEngine PRIVATE 
    FLUXION_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
    FLUXION_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)

target_include_directories(FluxionEngine PUBLIC BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/directx
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(FluxionEngine PUBLIC
    d3d12
    dxgi
    dxguid
    d3dcompiler
)