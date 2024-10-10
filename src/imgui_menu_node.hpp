#ifndef MENU_DEMO_HPP
#define MENU_DEMO_HPP

#include <engine/scene/scene.hpp>
#include <GAL/renderer/renderer.hpp>

#include <memory>
#include <forward_list>

#include <glad/glad.h>
#include <imgui.h>

namespace scene_demos {
    engine::node make_imgui_menu_node(std::shared_ptr<std::forward_list<const char*>> scene_names, std::string scene_name);
};

#endif // MENU_DEMO_HPP
