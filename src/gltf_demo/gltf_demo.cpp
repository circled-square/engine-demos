#include "gltf_demo.hpp"

#include "../imgui_menu_node.hpp"
#include "engine/resources_manager/resource_concept.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/constants.hpp>

// This is a simple 3d rendered demo showing gltf importing in action

namespace engine_demos {
    using namespace engine;

    scene make_gltf_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        node root = make_gltf_demo_node_tree();
        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        return scene(scene_name, std::move(root));
    }


    node make_gltf_demo_node_tree() {
        node root("");


        get_rm().set_default_3d_shader(nullptr);
        node castle(get_rm().load<nodetree_blueprint>("castlebl.glb"), "castle");
        node_data::attach_script(castle, stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "rotate"));

        root.add_child(std::move(castle));

        root.add_child(node("camera", engine::camera(), glm::translate(glm::mat4(1), glm::vec3(0,50,250))));

        return root;
    }
} // namespace engine_demos
