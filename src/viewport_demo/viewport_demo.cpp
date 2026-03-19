#include "viewport_demo.hpp"

#include "../postfx_demo/postfx_demo.hpp"
#include "engine/scene/node.hpp"
#include "engine/scene/node/script.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/utils/constants.hpp>

// This 3d rendered demo shows postfx shaders applied to a gltf-imported model

namespace engine_demos {
    using namespace engine;

    scene make_viewport_demo() {
        auto root = node::make("");
        auto scripts_lib = get_rm().load<dylib::library>("plugins/scripts/lib/scripts");

        node::add_child(root, node::make("menu", stateless_script::from(scripts_lib, "imgui_dbgmenu")));
        auto transparent_viewport = node::make("transparent_vp", engine::viewport(get_rm().new_from(gal::texture::empty({512, 512}, 4))));


        auto vp_texture = transparent_viewport->get<viewport>().fbo().get_texture();
        auto cube_television = node::make("cube", stateless_script::from(scripts_lib, "viewport_demo.viewport_cube"), vp_texture);

        using glm::vec3;
        auto cam = node::make("camera", camera(), glm::inverse(glm::lookAt(vec3(0,1,2), vec3(0,0,0), vec3(0,1,0))));

        node::add_child(transparent_viewport, make_postfx_demo_node_tree());
        //the viewport does not have to be child of the mesh which shows its texture (though it's tidier, in my opinion)
        node::add_child(root, std::move(transparent_viewport));
        node::add_child(root, std::move(cube_television));
        node::add_child(root, std::move(cam));
        return scene("viewport demo", std::move(root));
    }
} // namespace engine_demos
