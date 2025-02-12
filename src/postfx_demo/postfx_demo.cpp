#include "postfx_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>

namespace engine_demos {
    using namespace glm;
    using namespace engine;

    [[maybe_unused]]
    constexpr vec3 x_axis = vec3(1,0,0), y_axis = vec3(0,1,0), z_axis = vec3(0,0,1);
    [[maybe_unused]]
    constexpr float pi = glm::pi<float>();

    scene make_postfx_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name) {
        noderef root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        noderef halftone_vp("halftone-vp", engine::viewport(
            get_rm().new_from<shader>(shader::from_file("assets/shaders/halftone_postfx.glsl")),
            glm::vec2(1./2.)
        ));

        noderef transparent_vp("transparent-vp", engine::viewport(
            get_rm().new_from<shader>(shader::from_file("assets/shaders/transparent_postfx.glsl")),
            glm::vec2(1./3.)
        ));

        noderef cam("camera", camera(), glm::translate(glm::mat4(1), vec3(0,50,250)));

        halftone_vp.add_child(std::move(cam));
        noderef castle(get_rm().get_nodetree_from_gltf("assets/castlebl.glb"), "castle");
        halftone_vp.add_child(std::move(castle));
        transparent_vp.add_child(std::move(halftone_vp));
        root.add_child(std::move(transparent_vp));

        rc<const stateless_script> rotate_script = get_rm().new_from(stateless_script {
            .process = [](const noderef& n, std::any&, application_channel_t& c) {
                n->set_transform(rotate(n->transform(), c.from_app.delta * pi / 16, y_axis));
            },
        });
        root->get_from_path("transparent-vp/halftone-vp/castle").attach_script(std::move(rotate_script));

        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
