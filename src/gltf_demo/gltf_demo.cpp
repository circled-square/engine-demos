#include "gltf_demo.hpp"

#include "engine/resources_manager/resource_concept.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/constants.hpp>

// This is a simple 3d rendered demo showing gltf importing in action

namespace engine_demos {
    using namespace engine;

    scene make_gltf_demo() {
        node root = make_gltf_demo_node_tree();
        root.add_child(node("menu", std::monostate(), glm::mat4(1), stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "imgui_dbgmenu")));

        return scene("gltf demo", std::move(root));
    }


    node make_gltf_demo_node_tree() {
        node root("");


        get_rm().set_default_3d_shader(std::nullopt);
        node castle(get_rm().load<nodetree_blueprint>("castlebl.glb"), "castle");
        node_data::attach_script(castle, stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "gltf_demo.rotate"), std::monostate());

        root.add_child(std::move(castle));

        root.add_child(node("camera", engine::camera(), glm::translate(glm::mat4(1), glm::vec3(0,50,250))));

        return root;
    }
} // namespace engine_demos
