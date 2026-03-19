#include "gltf_demo.hpp"

#include "engine/resources_manager/resource_concept.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/constants.hpp>

// This is a simple 3d rendered demo showing gltf importing in action

namespace engine_demos {
    using namespace engine;

    scene make_gltf_demo() {
        auto root = make_gltf_demo_node_tree();
        node::add_child(root, node::make("menu", stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "imgui_dbgmenu")));

        return scene("gltf demo", std::move(root));
    }


    rc<node> make_gltf_demo_node_tree() {
        auto root = node::make("");


        get_rm().set_default_3d_shader(std::nullopt);
        auto castle = node::deep_copy(get_rm().load<nodetree_blueprint>("castlebl.glb"), "castle");
        node::attach_script(castle, stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "gltf_demo.rotate"), std::monostate());

        node::add_child(root, std::move(castle));

        node::add_child(root, node::make("camera", engine::camera(), glm::translate(glm::mat4(1), glm::vec3(0,50,250))));

        return root;
    }
} // namespace engine_demos
