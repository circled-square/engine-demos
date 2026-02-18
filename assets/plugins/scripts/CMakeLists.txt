cmake_minimum_required(VERSION 3.25)


project(scripts_plugin VERSION 0.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 23)

add_library(engine SHARED IMPORTED)
set_property(TARGET engine APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_property(TARGET engine PROPERTY LINKER_LANGUAGE CXX)
if(${WIN32})
    set_target_properties(engine PROPERTIES
        IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../../install_dir/release_windows/libengine.dll.a"
        IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../../install_dir/release_windows/libengine.dll"
    )
    target_include_directories(engine INTERFACE ../../../install_dir/release_windows/include/)
    include_directories(../../../install_dir/release_windows/include/)
else()
    set_target_properties(engine PROPERTIES
        IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../../install_dir/release/libengine.so"
    )
    target_include_directories(engine INTERFACE ../../../install_dir/release/include/)
    include_directories(../../../install_dir/release/include/)
endif()

add_compile_definitions(SLOGGA_AS_SHARED_LIB)
add_compile_definitions(GAL_AS_SHARED_LIB)

add_library(scripts SHARED scripts.cpp)
target_link_libraries(scripts PRIVATE engine)
set_target_properties(scripts PROPERTIES
    PREFIX ""
    IMPORT_PREFIX ""

    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS scripts
    LIBRARY DESTINATION lib/
    RUNTIME DESTINATION lib/ #necessary because windows dlls are RUNTIME, not LIBRARY
)
