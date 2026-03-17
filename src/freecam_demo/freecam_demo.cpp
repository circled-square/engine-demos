#include "freecam_demo.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/application/window.hpp>
#include <engine/utils/constants.hpp>

/* This demo showcases a fairly complex use of the engine, with postfx, gltf importing,
 * keyboard controls, mouse capture, scripting... the list goes on (it doesn't)
 */

namespace engine_demos {
    using namespace engine;
    using glm::mat4; using glm::vec4; using glm::vec3; using glm::vec2;

    scene make_freecam_demo() {
        application_channel_t::to_app_t to_app { .wants_mouse_cursor_captured = true };
        node root("");

        root.add_child(node("menu", std::monostate(), glm::mat4(1), stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "imgui_dbgmenu")));

        node dither_viewport("dither_vp", engine::viewport(
            vec2(1./6.)
        ));

        node dither_viewport_mesh("dither_vp_mesh", mesh(material(
            get_rm().load<shader>("shaders/postfx/dither.glsl"),
            dither_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().load<gal::vertex_array>(internal_resource_name_t::whole_screen_vao)));

        node camera_father_node("camera_father");
        node camera_node("camera", engine::camera(),
            glm::translate(mat4(1), vec3(0, 2, 0)),
            stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "freecam_demo.cam")
        );
        
        camera_father_node.add_child(std::move(camera_node));
        dither_viewport.add_child(std::move(camera_father_node));
        get_rm().set_default_3d_shader(get_rm().load<shader>("shaders/3d/light_falloff.glsl"));
        dither_viewport.add_child(get_rm().load_mut<nodetree_blueprint>("castlebl.glb")->into_node());
        dither_viewport_mesh.add_child(std::move(dither_viewport));
        root.add_child(std::move(dither_viewport_mesh));

        return engine::scene("freecam demo", std::move(root), std::move(to_app));
    }
} // namespace engine_demos
