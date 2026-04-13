add_library(postfx_demo STATIC postfx_demo/postfx_demo.cpp)
target_link_libraries(postfx_demo PRIVATE engine)

add_library(freecam_demo STATIC freecam_demo/freecam_demo.cpp)
target_link_libraries(freecam_demo PRIVATE engine)

add_library(viewport_demo STATIC viewport_demo/viewport_demo.cpp)
target_link_libraries(viewport_demo PRIVATE engine)

add_library(demos INTERFACE)
target_link_libraries(demos INTERFACE postfx_demo freecam_demo viewport_demo)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE engine demos)

set_target_properties(main PROPERTIES
    INSTALL_RPATH "\${ORIGIN}" #executables under linux do not look for libraries in their same directory by default. this "fixes" that
)

install(TARGETS main
    RUNTIME DESTINATION .
)
