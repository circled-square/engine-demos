#ifndef ENGINE_DEMOS_IMGUI_MENU_NODE_HPP
#define ENGINE_DEMOS_IMGUI_MENU_NODE_HPP

#include <engine/scene.hpp>
#include <GAL/renderer/renderer.hpp>

#include <memory>
#include <forward_list>

#include <glad/glad.h>
#include <imgui.h>

namespace engine_demos {
    engine::noderef make_imgui_menu_node(std::shared_ptr<std::forward_list<const char*>> scene_names, std::string scene_name);
};

#endif // ENGINE_DEMOS_IMGUI_MENU_NODE_HPP
