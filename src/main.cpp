#include <engine/entry_point.hpp>
#include <engine/resources_manager.hpp>

#include "texture_demo/texture_demo.hpp"
#include "3d_demo/3d_demo.hpp"
#include "gltf_demo/gltf_demo.hpp"
#include "postfx_demo/postfx_demo.hpp"
#include "freecam_demo/freecam_demo.hpp"


static engine::rc<engine::scene> get_start_scene();

int main() {
    engine::entry_point({1280, 720}, "demo", engine::window::creation_hints::MAXIMIZED, get_start_scene);
}

static engine::rc<engine::scene> get_start_scene() {
    auto names = std::make_shared<std::forward_list<const char*>>();
    auto add_scene = [&](const char* name, auto scene_ctor) {
        names->push_front(name);
        engine::get_rm().dbg_add_scene_constructor(name,
            std::function([names, scene_ctor=std::move(scene_ctor), scene_name=std::move(name)](){
                return scene_ctor(scene_name);
            })
        );
    };

    add_scene("texture demo", [=](const char* name) { return scene_demos::make_texture_demo(names, name); });
    add_scene("3d demo", [=](const char* name) { return scene_demos::make_3d_demo(names, name); });
    add_scene("gltf demo", [=](const char* name) { return scene_demos::make_gltf_demo(names, name); });
    add_scene("postfx demo", [=](const char* name) { return scene_demos::make_postfx_demo(names, name); });
    add_scene("freecam demo", [=](const char* name) { return scene_demos::make_freecam_demo(names, name); });

    return engine::get_rm().get_scene("freecam demo");
}

