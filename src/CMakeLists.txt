add_library(imgui_menu_node STATIC imgui_menu_node.cpp)
target_link_libraries(imgui_menu_node PRIVATE engine)

add_library(texture_demo STATIC texture_demo/texture_demo.cpp)
target_link_libraries(texture_demo PRIVATE engine imgui_menu_node)

add_library(3d_demo STATIC 3d_demo/3d_demo.cpp)
target_link_libraries(3d_demo PRIVATE engine imgui_menu_node)

add_library(gltf_demo STATIC gltf_demo/gltf_demo.cpp)
target_link_libraries(gltf_demo PRIVATE engine imgui_menu_node)

add_library(postfx_demo STATIC postfx_demo/postfx_demo.cpp)
target_link_libraries(postfx_demo PRIVATE engine imgui_menu_node)

add_library(freecam_demo STATIC freecam_demo/freecam_demo.cpp)
target_link_libraries(freecam_demo PRIVATE engine imgui_menu_node)

add_library(demos INTERFACE)
target_link_libraries(demos INTERFACE texture_demo 3d_demo gltf_demo postfx_demo freecam_demo)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE engine demos)
