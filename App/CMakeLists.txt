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

target_link_libraries(FluxionApp PRIVATE FluxionEngine)