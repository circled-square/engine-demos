#include <engine/entry_point.hpp>
#include <engine/resources_manager.hpp>

#include "texture_demo/texture_demo.hpp"
#include "3d_demo/3d_demo.hpp"
#include "gltf_demo/gltf_demo.hpp"
#include "postfx_demo/postfx_demo.hpp"
#include "freecam_demo/freecam_demo.hpp"
#include "collision_demo/collision_demo.hpp"


static engine::rc<engine::scene> get_start_scene();

int main() {
    engine::entry_point({1280, 720}, "demo", engine::window::hints::MAXIMIZED, get_start_scene);
}

static engine::rc<engine::scene> get_start_scene() {
    //all scenes need to know the names of all other scenes for imgui_menu_node to work correctly
    auto names = std::make_shared<std::forward_list<std::string>>();

    using scene_ctor_t = engine::scene(*)(std::shared_ptr<std::forward_list<std::string>>, std::string);

    std::pair<std::string, scene_ctor_t> constructors[] = {
        { "texture demo",   engine_demos::make_texture_demo },
        { "3d demo",        engine_demos::make_3d_demo },
        { "gltf demo",      engine_demos::make_gltf_demo },
        { "postfx demo",    engine_demos::make_postfx_demo },
        { "freecam demo",   engine_demos::make_freecam_demo },
        { "collision demo", engine_demos::make_collision_demo }
    };

    for (auto& [name, ctor] : constructors) {
        names->push_front(name);
        engine::get_rm().dbg_add_scene_constructor(name, [=](){ return ctor(names, name); });
    }

    return engine::get_rm().get_scene("collision demo");
}
