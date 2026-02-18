#include "freecam_demo.hpp"

#include "../imgui_menu_node.hpp"
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

    struct freecam_state  {
        bool up = false, down = false, left = false, right = false, fwd = false, bwd = false, go_faster = false;
        float current_y_rotation = 0.f;
        float move_speed = 30.0;
    };

    scene make_freecam_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        application_channel_t::to_app_t to_app { .wants_mouse_cursor_captured = true };
        node root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        node dither_viewport("dither_vp", engine::viewport(
            vec2(1./6.)
        ));

        node dither_viewport_mesh("dither_vp_mesh", mesh(material(
            get_rm().load<shader>("shaders/postfx/dither.glsl"),
            dither_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().load<gal::vertex_array>(internal_resource_name_t::whole_screen_vao)));

        stateless_script freecam_script { script_vtable {
            .construct = [](const node&){ return std::any(freecam_state()); },
            .process = [](const node& n, std::any& ss, application_channel_t& app_chan) {
                rc<node_data> father = n->get_father_checked();
                freecam_state& s = *std::any_cast<freecam_state>(&ss);

                for(const event_variant_t& event : app_chan.from_app().events) {
                    match_variant(event,
                    [&s, &app_chan, &father](const key_event_t& e) {
                            using namespace engine::window;
                            if (e.key == key_codes::SPACE) {
                                s.up = e.action != key_action_codes::RELEASE;
                            } else if (e.key == key_codes::LEFT_CONTROL) {
                                s.down = e.action != key_action_codes::RELEASE;
                            } else if (e.key == key_codes::A) {
                                s.left = e.action != key_action_codes::RELEASE;
                            } else if (e.key == key_codes::D) {
                                s.right = e.action != key_action_codes::RELEASE;
                            } else if (e.key == key_codes::W) {
                                s.fwd = e.action != key_action_codes::RELEASE;
                            } else if (e.key == key_codes::S) {
                                s.bwd = e.action != key_action_codes::RELEASE;
                            } else if (e.key == key_codes::LEFT_SHIFT) {
                                s.go_faster = e.action != key_action_codes::RELEASE;
                            } else if (e.key == key_codes::C && e.action == key_action_codes::RELEASE) {
                                //toggle mouse capture
                                app_chan.to_app().wants_mouse_cursor_captured = !app_chan.from_app().mouse_cursor_is_captured;
                            } else if (e.key == key_codes::F5 && e.action == key_action_codes::RELEASE) {
                                // have the resources manager hot-reload the dither shader
                                get_rm().hot_reload<shader>("shaders/postfx/dither.glsl");
                            }
                    }, [&s, &app_chan, &n, &father](const mouse_move_event_t& e) {
                        if(!app_chan.from_app().mouse_cursor_is_captured)
                            return;
                        vec2 movement = e.movement;
                        constexpr float movement_multiplier = 0.002;
                        movement *= movement_multiplier;

                        float old_rotation = s.current_y_rotation;
                        s.current_y_rotation += movement.y;
                        s.current_y_rotation = std::clamp(s.current_y_rotation, -pi/2.f * 0.99f, pi/2.f * 0.99f);
                        movement.y = s.current_y_rotation - old_rotation;

                        father->set_transform(glm::rotate(father->transform(), -movement.x, y_axis));
                        n->set_transform(glm::rotate(n->transform(), -movement.y, x_axis));
                    });
                }

                vec3 movement(
                    (s.right?1:0) - (s.left?1:0),
                    (s.up   ?1:0) - (s.down?1:0),
                    (s.bwd  ?1:0) - (s.fwd ?1:0)
                );
                movement *= s.move_speed * app_chan.from_app().delta * (s.go_faster ? 3.0 : 1.0);

                father->set_transform(glm::translate(father->transform(), movement));
            },
        }};


        node camera_father_node("camera_father");
        node camera_node("camera", engine::camera(),
            glm::translate(mat4(1), vec3(0, 2, 0)), std::move(freecam_script));
        
        camera_father_node.add_child(std::move(camera_node));
        dither_viewport.add_child(std::move(camera_father_node));
        get_rm().set_default_3d_shader(get_rm().load<shader>("shaders/3d/light_falloff.glsl"));
        dither_viewport.add_child(get_rm().load_mut<nodetree_blueprint>("castlebl.glb")->into_node());
        dither_viewport_mesh.add_child(std::move(dither_viewport));
        root.add_child(std::move(dither_viewport_mesh));

        return engine::scene(scene_name, std::move(root), std::move(to_app));
    }
} // namespace engine_demos
