#include <engine/entry_point.hpp>
#include <engine/resources_manager.hpp>

#include "texture_demo/texture_demo.hpp"
#include "3d_demo/3d_demo.hpp"
#include "gltf_demo/gltf_demo.hpp"
#include "postfx_demo/postfx_demo.hpp"
#include "freecam_demo/freecam_demo.hpp"
#include "viewport_demo/viewport_demo.hpp"
#include "collision_demo/collision_demo.hpp"

static engine::rc<engine::scene> get_start_scene();

#ifdef ENGINE_BEING_COMPILED
#error ENGINE_BEING_COMPILED macro should NOT be defined in translation units outside engine!
#endif


int main() {
    engine::entry_point({1280, 720}, "demo", engine::window::hints{ .maximised = true }, get_start_scene);
}

static engine::rc<engine::scene> get_start_scene() {
    //all scenes need to know the names of all other scenes for imgui_menu_node to work correctly
    using scene_ctor_t = engine::scene(*)();

    std::pair<std::string, scene_ctor_t> constructors[] = {
        { "texture demo",   engine_demos::make_texture_demo },
        { "3d demo",        engine_demos::make_3d_demo },
        { "gltf demo",      engine_demos::make_gltf_demo },
        { "postfx demo",    engine_demos::make_postfx_demo },
        { "freecam demo",   engine_demos::make_freecam_demo },
        { "viewport demo",  engine_demos::make_viewport_demo },
        { "collision demo", engine_demos::make_collision_demo },
    };

    for (auto& [name, ctor] : constructors) {
        engine::get_rm().dbg_add_scene_constructor(name, ctor);
    }

    return engine::get_rm().load_mut<engine::scene>("3d demo");
}
