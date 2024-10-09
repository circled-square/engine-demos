#include "gltf_demo.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>

namespace scene_demos {
    using namespace glm;
    using namespace engine;

    [[maybe_unused]]
    constexpr vec3 x_axis = vec3(1,0,0), y_axis = vec3(0,1,0), z_axis = vec3(0,0,1);
    [[maybe_unused]]
    constexpr float pi = glm::pi<f32>();

    scene make_gltf_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name) {
        node root("");
        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        rc<const stateless_script> rotate_script = get_rm().new_from(stateless_script {
            .process = [](node& n, std::any&, application_channel_t& c) {
                n.transform() = rotate(n.transform(), c.from_app.delta * pi / 16, y_axis);
            },
        });
        root.add_child(engine::node(engine::get_rm().get_nodetree_from_gltf("resources/castlebl.glb"), "castle"));
        root.get_child("castle").attach_script(std::move(rotate_script));
        root.add_child(engine::node("camera", engine::camera(), glm::translate(glm::mat4(1), vec3(0,50,250))));

        return scene(scene_name, std::move(root));
    }
} // scene_demos
