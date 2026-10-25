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

set(DXC_EXECUTABLE "${dxc_SOURCE_DIR}/bin/x64/dxc.exe")
set(SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(ALL_CSO_FILES "")

function(add_shader_compilation TARGET_NAME SHADER_FILE ENTRY_POINT PROFILE)
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
    set(OUTPUT_CSO "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/${SHADER_NAME}.cso")
    
    add_custom_command(
        OUTPUT "${OUTPUT_CSO}"
        COMMAND "${DXC_EXECUTABLE}"
            -T ${PROFILE} 
            -E ${ENTRY_POINT} 
            -HV 2021 
            -I "${dxc_SOURCE_DIR}/inc" 
            -I "${dxc_SOURCE_DIR}/inc/hlsl"
            -Fo "${OUTPUT_CSO}"
            "${SHADER_FILE}"
        DEPENDS "${SHADER_FILE}"
        COMMENT "Compiling ${SHADER_FILE} (${ENTRY_POINT}) -> ${OUTPUT_CSO}"
        VERBATIM
    )
    
    set(ALL_CSO_FILES ${ALL_CSO_FILES} "${OUTPUT_CSO}" PARENT_SCOPE)
endfunction()

add_shader_compilation(FluxionEngine "${SHADERS_DIR}/VertexShader.hlsl" "VSMain" "vs_6_6")
add_shader_compilation(FluxionEngine "${SHADERS_DIR}/MeshOpaqueSolid.hlsl" "PSMain" "ps_6_6")
add_shader_compilation(FluxionEngine "${SHADERS_DIR}/GridVS.hlsl" "VSMain" "vs_6_6")
add_shader_compilation(FluxionEngine "${SHADERS_DIR}/GridPS.hlsl" "PSMain" "ps_6_6")

add_custom_target(CompileShaders ALL DEPENDS ${ALL_CSO_FILES})
add_dependencies(FluxionEngine CompileShaders)

target_sources(FluxionEngine PRIVATE ${ALL_CSO_FILES})

target_link_libraries(FluxionEngine PUBLIC
    d3d12
    dxgi
    dxguid
    d3dcompiler
    DirectXTex
    FluxionDXC
)