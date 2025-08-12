#include "freecam_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/application/window.hpp>


namespace engine_demos {
    using namespace glm;
    using namespace engine;

    static glm::mat4 to_rotation_mat(const glm::mat4& m) {
        glm::mat4 mat = m;
        mat[3] = vec4(0);
        return mat;
    }
    static glm::mat4 clear_first_3_rows(const glm::mat4& m) {
        glm::mat4 mat = m;
        mat[0] = mat[1] = mat[2] = vec4(0);
        return mat;
    }

    struct freecam_state  {
        bool up = false, down = false, left = false, right = false, fwd = false, bwd = false, go_faster = false;
        float current_y_rotation = 0.f;
        float move_speed = 30.0;
    };

    scene make_freecam_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        application_channel_t::to_app_t to_app { .wants_mouse_cursor_captured = true };
        node root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        node halftone_viewport("halftone_vp", engine::viewport(
            get_rm().new_from<shader>(shader::from_file("assets/shaders/halftone_postfx.glsl")),
            glm::vec2(1./2.)
        ));

        node transparent_viewport("transparent_vp", engine::viewport(
            get_rm().new_from<shader>(shader::from_file("assets/shaders/transparent_postfx.glsl")),
            glm::vec2(1./3.)
        ));

        rc<const stateless_script> freecam_script = get_rm().new_from(stateless_script {
            .construct = [](const node&){ return std::any(freecam_state()); },
            .process = [](const node& n, std::any& ss, application_channel_t& app_chan) {
                freecam_state& s = *std::any_cast<freecam_state>(&ss);

                for(const event_variant_t& event : app_chan.from_app().events) {
                    match_variant(event,
                    [&s, &app_chan](const key_event_t& e) {
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
                            }
                    }, [&s, &n](const mouse_move_event_t& e) {
                        glm::vec2 movement = e.movement;
                        constexpr float movement_multiplier = 0.002;
                        movement *= movement_multiplier;

                        float old_rotation = s.current_y_rotation;
                        s.current_y_rotation += movement.y;
                        s.current_y_rotation = std::clamp(s.current_y_rotation, -glm::pi<float>()/2.f * 0.99f, glm::pi<float>()/2.f * 0.99f);
                        movement.y = s.current_y_rotation - old_rotation;

                        glm::mat4 cam = n->transform();

                        cam = glm::rotate(glm::mat4(1), -movement.x, glm::vec3(0,1,0)) * to_rotation_mat(cam) + clear_first_3_rows(cam);
                        cam = glm::rotate(cam, -movement.y, glm::vec3(1,0,0));
                        n->set_transform(cam);
                    });
                    match_variant(event, [](auto){});
                }

                glm::vec3 movement = glm::vec3(
                    (s.right?1:0) - (s.left?1:0),
                    (s.up?1:0) - (s.down?1:0),
                    (s.bwd?1:0) - (s.fwd?1:0)
                );
                movement *= s.move_speed * app_chan.from_app().delta * (s.go_faster ? 3.0 : 1.0);

                n->set_transform(glm::translate(n->transform(), movement));
            },
        });


        node camera_node("camera", engine::camera(),
            glm::translate(glm::mat4(1), glm::vec3(0, 2, 0)), std::move(freecam_script));

        halftone_viewport.add_child(std::move(camera_node));
        halftone_viewport.add_child(node(get_rm().get_nodetree_from_gltf("assets/castlebl.glb")));
        transparent_viewport.add_child(std::move(halftone_viewport));
        root.add_child(std::move(transparent_viewport));

        return engine::scene(scene_name, std::move(root), render_flags_t{}, std::move(to_app));
    }
} // namespace engine_demos
