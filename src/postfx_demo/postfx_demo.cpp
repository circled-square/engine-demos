#include "postfx_demo.hpp"

#include "../imgui_menu_node.hpp"
#include "../gltf_demo/gltf_demo.hpp"
#include "engine/resources_manager/resource_concept.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/constants.hpp>

// This 3d rendered demo shows postfx shaders applied to a gltf-imported model

namespace engine_demos {
    using namespace engine;

    scene make_postfx_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        node root = make_postfx_demo_node_tree();

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        return scene(scene_name, std::move(root));
    }

    node make_postfx_demo_node_tree() {
        node root("");

        node halftone_viewport("halftone_vp", engine::viewport(
            glm::vec2(1./2.)
        ));

        node halftone_viewport_mesh("halftone_vp_mesh", mesh(material(
            get_rm().load<shader>("shaders/postfx/halftone.glsl"),
            halftone_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().load<gal::vertex_array>(internal_resource_name_t::whole_screen_vao)));

        node transparent_viewport("transparent_vp", engine::viewport(
            glm::vec2(1./3.)
        ));

        node transparent_viewport_mesh("transparent_vp_mesh", mesh(material(
            get_rm().load<shader>("shaders/postfx/transparent.glsl"),
            transparent_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().load<gal::vertex_array>(internal_resource_name_t::whole_screen_vao)));


        halftone_viewport.add_child(make_gltf_demo_node_tree());
        halftone_viewport_mesh.add_child(std::move(halftone_viewport));
        transparent_viewport.add_child(std::move(halftone_viewport_mesh));
        transparent_viewport_mesh.add_child(std::move(transparent_viewport));
        root.add_child(std::move(transparent_viewport_mesh));

        return root;
    }
} // namespace engine_demos
