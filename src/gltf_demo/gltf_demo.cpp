#include "gltf_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>

// This is a simple 3d rendered demo showing gltf importing in action

namespace engine_demos {
    using namespace glm;
    using namespace engine;

    [[maybe_unused]]
    constexpr vec3 x_axis = vec3(1,0,0), y_axis = vec3(0,1,0), z_axis = vec3(0,0,1);
    [[maybe_unused]]
    constexpr float pi = glm::pi<f32>();

    scene make_gltf_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        node root("");
        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        rc<const stateless_script> rotate_script = get_rm().new_from(stateless_script {
            .process = [](const node& n, std::any&, application_channel_t& c) {
                n->set_transform(rotate(n->transform(), c.from_app().delta * pi / 16, y_axis));
            },
        });

        node castle(get_rm().get_nodetree_from_gltf("assets/castlebl.glb"), "castle");

        node_data::attach_script(castle, std::move(rotate_script));

        root.add_child(std::move(castle));

        root.add_child(node("camera", engine::camera(), glm::translate(glm::mat4(1), vec3(0,50,250))));

        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
