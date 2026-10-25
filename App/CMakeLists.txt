file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS 
    "src/*.cpp"
    "include/*.hpp"
)

add_executable(FluxionApp WIN32 ${APP_SOURCES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${APP_SOURCES})

target_include_directories(FluxionApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}
)

add_custom_command(TARGET FluxionApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dxc_SOURCE_DIR}/bin/x64/dxcompiler.dll"
        "${dxc_SOURCE_DIR}/bin/x64/dxil.dll"
        $<TARGET_FILE_DIR:FluxionApp>
    COMMENT "Copying DXC DLLs to output directory"
)

target_link_libraries(FluxionApp PRIVATE FluxionEngine)