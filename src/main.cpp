#include <engine/entry_point.hpp>
#include <engine/resources_manager.hpp>

#include "texture_demo/texture_demo.hpp"
#include "3d_demo/3d_demo.hpp"
#include "gltf_demo/gltf_demo.hpp"
#include "postfx_demo/postfx_demo.hpp"
#include "freecam_demo/freecam_demo.hpp"


static engine::rc<engine::basic_scene> get_start_scene();

int main() {
    engine::entry_point({1280, 720}, "demo", engine::window::window_creation_hints::MAXIMIZED, get_start_scene);
}

static engine::rc<engine::basic_scene> get_start_scene() {
    auto names = std::make_shared<std::forward_list<const char*>>();
    auto add_scene = [&](const char* name, auto scene_ctor) {
        names->push_front(name);
        engine::get_rm().dbg_add_scene_constructor(name, std::function(std::move(scene_ctor)));
    };

    add_scene("texture demo", [=]() { return scene_demos::texture_demo(names); });
    add_scene("3d demo", [=]() { return scene_demos::three_dimensional_demo(names); });
    add_scene("gltf demo", [=]() { return scene_demos::gltf_demo(names); });
    add_scene("postfx demo", [=]() { return scene_demos::postfx_demo(names); });
    add_scene("freecam demo", [=]() { return scene_demos::freecam_demo(names); });

    return engine::get_rm().get_scene("freecam demo");
}

