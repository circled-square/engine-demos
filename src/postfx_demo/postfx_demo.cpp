#include "postfx_demo.hpp"

#include "../gltf_demo/gltf_demo.hpp"
#include "engine/resources_manager/resource_concept.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/constants.hpp>

// This 3d rendered demo shows postfx shaders applied to a gltf-imported model

namespace engine_demos {
    using namespace engine;

    scene make_postfx_demo() {
        auto root = make_postfx_demo_node_tree();

        node::add_child(*root, node::make("menu", stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "imgui_dbgmenu")));

        return scene("postfx demo", std::move(root));
    }

    std::unique_ptr<node> make_postfx_demo_node_tree() {
        auto root = node::make("");

        auto halftone_viewport = node::make("halftone_vp", engine::viewport(glm::vec2(1./2.)));

        auto halftone_viewport_mesh = node::make("halftone_vp_mesh", mesh(material(
            get_rm().load<shader>("shaders/postfx/halftone.glsl"),
            halftone_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().load<gal::vertex_array>(internal_resource_name_t::whole_screen_vao)));

        auto transparent_viewport = node::make("transparent_vp", engine::viewport(glm::vec2(1./3.)));

        auto transparent_viewport_mesh = node::make("transparent_vp_mesh", mesh(material(
            get_rm().load<shader>("shaders/postfx/transparent.glsl"),
            transparent_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().load<gal::vertex_array>(internal_resource_name_t::whole_screen_vao)));


        node::add_child(*halftone_viewport, make_gltf_demo_node_tree());
        node::add_child(*halftone_viewport_mesh, std::move(halftone_viewport));
        node::add_child(*transparent_viewport, std::move(halftone_viewport_mesh));
        node::add_child(*transparent_viewport_mesh, std::move(transparent_viewport));
        node::add_child(*root, std::move(transparent_viewport_mesh));

        return root;
    }
} // namespace engine_demos
