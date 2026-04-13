#include "postfx_demo.hpp"

#include "engine/resources_manager/resource_concept.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/constants.hpp>

// This 3d rendered demo shows postfx shaders applied to a gltf-imported model

namespace engine_demos {
    using namespace engine;

    scene make_postfx_demo() {
        auto root = make_postfx_demo_node_tree();

        root->add_child(node::make("menu", stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "imgui_dbgmenu")));

        return scene("postfx demo", std::move(root));
    }

    static std::unique_ptr<node> make_gltf_demo_node_tree() {
        auto root = node::make("");


        get_rm().set_default_3d_shader(std::nullopt);
        auto castle = node::deep_copy(get_rm().load<nodetree_blueprint>("castlebl.glb"), "castle");
        castle->attach_script(stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "gltf_demo.rotate"), std::monostate());

        root->add_child(std::move(castle));

        root->add_child(node::make("camera", engine::camera(), glm::translate(glm::mat4(1), glm::vec3(0,50,250))));

        return root;
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


        halftone_viewport->add_child(make_gltf_demo_node_tree());
        halftone_viewport_mesh->add_child(std::move(halftone_viewport));
        transparent_viewport->add_child(std::move(halftone_viewport_mesh));
        transparent_viewport_mesh->add_child(std::move(transparent_viewport));
        root->add_child(std::move(transparent_viewport_mesh));

        return root;
    }
} // namespace engine_demos
