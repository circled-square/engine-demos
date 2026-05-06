cmake_minimum_required(VERSION 3.25)


project(scripts_plugin VERSION 0.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
install(FILES ${CMAKE_BINARY_DIR}/compile_commands.json DESTINATION .)

add_library(engine SHARED IMPORTED)
set_property(TARGET engine APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_property(TARGET engine PROPERTY LINKER_LANGUAGE CXX)
if(${WIN32})
    set_target_properties(engine PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/engine/release_windows/libengine.dll.a"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/engine/install_dir/release_windows/libengine.dll"
    )
    target_include_directories(engine INTERFACE engine/release_windows/include/)
    include_directories(engine/release_windows/include/)
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ENGINE_SUBDIR "debug")
    else()
        set(ENGINE_SUBDIR "release")
    endif()

    set_target_properties(engine PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/engine/${ENGINE_SUBDIR}/libengine.so"
    )
    target_include_directories(engine INTERFACE engine/${ENGINE_SUBDIR}/include/)
endif()

add_compile_definitions(SLOGGA_AS_SHARED_LIB)
add_compile_definitions(GAL_AS_SHARED_LIB)

add_library(scripts SHARED scripts.cpp)
target_link_libraries(scripts PRIVATE engine)
set_target_properties(scripts PROPERTIES
    IMPORT_PREFIX ""

    POSITION_INDEPENDENT_CODE ON
)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(scripts PROPERTIES PREFIX "dbgsym_")
else()
    set_target_properties(scripts PROPERTIES PREFIX "")
endif()

install(TARGETS scripts
    LIBRARY DESTINATION lib/
    RUNTIME DESTINATION lib/ #necessary because windows dlls are RUNTIME, not LIBRARY
)
