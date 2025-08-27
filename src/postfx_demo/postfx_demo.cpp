#include "postfx_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>

namespace engine_demos {
    using namespace glm;
    using namespace engine;

    //TODO: put this into a constants header in the engine, under utils
    [[maybe_unused]]
    constexpr vec3 x_axis = vec3(1,0,0), y_axis = vec3(0,1,0), z_axis = vec3(0,0,1);
    [[maybe_unused]]
    constexpr float pi = glm::pi<float>();

    scene make_postfx_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        node root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        node halftone_viewport("halftone_vp", engine::viewport(
            glm::vec2(1./2.)
        ));

        node halftone_viewport_mesh("halftone_vp_mesh", mesh(material(
            get_rm().new_from<shader>(shader::from_file("assets/shaders/halftone_postfx.glsl")),
            halftone_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().get_whole_screen_vao()));

        node transparent_viewport("transparent_vp", engine::viewport(
            glm::vec2(1./3.)
        ));

        node transparent_viewport_mesh("transparent_vp_mesh", mesh(material(
            get_rm().new_from<shader>(shader::from_file("assets/shaders/transparent_postfx.glsl")),
            transparent_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().get_whole_screen_vao()));


        node cam("camera", camera(), glm::translate(glm::mat4(1), vec3(0,50,250)));

        halftone_viewport.add_child(std::move(cam));
        halftone_viewport.add_child(node(get_rm().get_nodetree_from_gltf("assets/castlebl.glb"), "castle"));
        halftone_viewport_mesh.add_child(std::move(halftone_viewport));
        transparent_viewport.add_child(std::move(halftone_viewport_mesh));
        transparent_viewport_mesh.add_child(std::move(transparent_viewport));
        root.add_child(std::move(transparent_viewport_mesh));

        rc<const stateless_script> rotate_script = get_rm().new_from(stateless_script {
            .process = [](const node& n, std::any&, application_channel_t& c) {
                n->set_transform(rotate(n->transform(), c.from_app().delta * pi / 16, y_axis));
            },
        });
        node_data::attach_script(root->get_descendant_from_path("transparent_vp_mesh/transparent_vp/halftone_vp_mesh/halftone_vp/castle"),std::move(rotate_script));

        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
