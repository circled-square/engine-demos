#include "collision_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <engine/resources_manager.hpp>
#include <engine/materials.hpp>
#include <engine/window/window.hpp>

namespace engine_demos {
    using namespace glm;
    using namespace engine;

    constexpr vec3 x_axis = vec3(1,0,0), y_axis = vec3(0,1,0), z_axis = vec3(0,0,1);
    constexpr float pi = glm::pi<f32>();

    using vertex_t = engine::retro_3d_shader_vertex_t;


    static constexpr std::array<vertex_t, 16> vertex_data {
        vertex_t
        { { .5,  .5,  .5}, {1., 1.} },
        { { .5,  .5, -.5}, {0., 1.} },
        { {-.5,  .5, -.5}, {1., 1.} },
        { {-.5,  .5,  .5}, {0., 1.} },

        { { .5, -.5,  .5}, {1., 0.} },
        { { .5, -.5, -.5}, {0., 0.} },
        { {-.5, -.5, -.5}, {1., 0.} },
        { {-.5, -.5,  .5}, {0., 0.} },

        //top verts with tex coords fixed for top face
        { { .5,  .5,  .5}, {1., 1.} },
        { { .5,  .5, -.5}, {1., 0.} },
        { {-.5,  .5, -.5}, {0., 0.} },
        { {-.5,  .5,  .5}, {0., 1.} },

        //bottom verts with tex coords fixed for bottom face
        { { .5, -.5,  .5}, {1., 1.} },
        { { .5, -.5, -.5}, {1., 0.} },
        { {-.5, -.5, -.5}, {0., 0.} },
        { {-.5, -.5,  .5}, {0., 1.} },
    };
    static constexpr std::array<uvec3, 12> indices {
        uvec3
        {8,  9,  10},
        {10, 11, 8 },

        {12, 15, 14},
        {14, 13, 12},

        {1, 0, 4},
        {4, 5, 1},

        {2, 1, 5},
        {5, 6, 2},

        {3, 2, 6},
        {6, 7, 3},

        {0, 3, 7},
        {7, 4, 0},
    };

    struct kbdcube_state  {
        bool up = false, down = false, left = false, right = false, fwd = false, bwd = false;
        float move_speed = .5;
    };

    scene make_collision_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name) {
        gal::vertex_array cube_vao = gal::vertex_array::make<vertex_t>(vertex_data, std::span(indices.data(), indices.size()));
        material cube_material(get_rm().get_retro_3d_shader(), get_rm().get_texture("assets/example.png"));

        mesh cube_mesh(cube_material, get_rm().new_from<gal::vertex_array>(std::move(cube_vao)));


        rc<const stateless_script> stillcube_script = get_rm().new_from(stateless_script {
            .process = [](node& n, std::any&, application_channel_t& app_chan) {
                n.transform() = rotate(n.transform(), app_chan.from_app.delta * pi / 8, z_axis + y_axis / 2.f);
            },
        });

        rc<const stateless_script> kbdcube_script = get_rm().new_from(stateless_script{
            .construct = [](){
                return std::any(kbdcube_state());
            },
            .process = [](node& n, std::any& state, application_channel_t& app_chan) {
                kbdcube_state& s = *std::any_cast<kbdcube_state>(&state);

                // handle kbd input
                for(const event_variant_t& event : app_chan.from_app.events) {
                    match_variant(event, [&s, &app_chan](const key_event_t& e) {
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
                            }
                    }, [](const auto&) {});
                }
                vec3 move_vec = vec3{
                    (int)s.right - (int)s.left,
                    (int)s.up - (int)s.down,
                    (int)s.bwd - (int)s.fwd,
                } * s.move_speed * app_chan.from_app.delta;
                n.transform() = glm::translate(n.transform(), move_vec);
            }
        });

        collision_shape col_shape = collision_shape::from_mesh(
            (void*)vertex_data.data(), vertex_data.size(), offsetof(vertex_t, pos), sizeof(vertex_t),
            std::span<const glm::uvec3>(indices.begin(), indices.end()),
            0, {0}
        );


        node root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        root.add_child(node("camera", camera(), glm::inverse(glm::lookAt(vec3(3,6,6), vec3(0), vec3(0,1,0)))));

        node stillcube("stillcube", cube_mesh, glm::mat4(1), std::move(stillcube_script));
        stillcube.add_child(node("colshape", col_shape));
        root.add_child(std::move(stillcube));


        node kbdcube("kbdcube", cube_mesh, glm::mat4(1), std::move(kbdcube_script));
        kbdcube.set_collision_behaviour(engine::collision_behaviour {
            .moves_away_on_collision = true,
        });
        node kbd_colshape_node("colshape", std::move(col_shape));
        kbd_colshape_node.set_collision_behaviour(engine::collision_behaviour {
            .passes_events_to_father = true,
        });
        kbdcube.add_child(std::move(kbd_colshape_node));
        root.add_child(std::move(kbdcube));

        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
