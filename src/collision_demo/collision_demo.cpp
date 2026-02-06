#include "collision_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <engine/resources_manager.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/application/window.hpp>
#include <engine/utils/constants.hpp>

// This demo illustrates collisions and different collision behaviours

namespace engine_demos {
    using namespace engine;
    using glm::mat4; using glm::uvec3; using glm::vec3;

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
        float move_speed = 1.;
    };

    scene make_collision_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        gal::vertex_array cube_vao = gal::vertex_array::make<vertex_t>(vertex_data, indices);
        material cube_material(get_rm().load<shader>(internal_resource_name_t::simple_3d_shader), get_rm().load<gal::texture>("example.png"));

        mesh cube_mesh(cube_material, get_rm().new_from<gal::vertex_array>(std::move(cube_vao)));


        rc<const stateless_script> stillcube_script = get_rm().new_from(stateless_script {
            .process = [](const node& n, std::any&, application_channel_t& app_chan) {
                n->set_transform(rotate(n->transform(), app_chan.from_app().delta * pi / 8, z_axis + y_axis / 2.f));
            },
        });

        rc<const stateless_script> kbdcube_script = get_rm().new_from(stateless_script{
            .construct = [](const node&) {
                return std::any(kbdcube_state());
            },
            .process = [](const node& n, std::any& state, application_channel_t& app_chan) {
                EXPECTS(state.type() == typeid(kbdcube_state));
                kbdcube_state& s = *std::any_cast<kbdcube_state>(&state);

                // handle kbd input
                for(const event_variant_t& event : app_chan.from_app().events) {
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
                vec3 move_vec = vec3 {
                    (int)s.right - (int)s.left,
                    (int)s.up    - (int)s.down,
                    (int)s.bwd   - (int)s.fwd,
                } * s.move_speed * app_chan.from_app().delta;
                n->set_transform(glm::translate(n->transform(), move_vec));
            }
        });

        rc<const collision_shape> cube_col_shape = get_rm().new_from(collision_shape::from_mesh(
            stride_span<const vec3>(vertex_data.data(), offsetof(vertex_t, pos), sizeof(vertex_t), vertex_data.size()),
            std::span<const uvec3>(indices.begin(), indices.end()),
            collision_layer(1) | collision_layer(2), collision_layer(1)
        ));


        node root("");
        {
            root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

            root.add_child(node("camera", camera(), glm::inverse(glm::lookAt(vec3(3,6,6), vec3(0), vec3(0,1,0)))));

            node cone(get_rm().load<nodetree_blueprint>("cone_with_collision.glb"), "cone");
            {
                cone->set_transform(glm::translate(cone->transform(), vec3(2, 0, 0)));
            }
            root.add_child(std::move(cone));

            node stillcube("stillcube", cube_mesh, mat4(1), std::move(stillcube_script));
            {
                stillcube.add_child(node("colshape", cube_col_shape));
            }
            root.add_child(std::move(stillcube));


            node kbdcube("kbdcube", cube_mesh, mat4(1), std::move(kbdcube_script));
            {
                kbdcube->set_collision_behaviour(engine::node_collision_behaviour {
                    .moves_away_on_collision = true,
                });

                node kbdcube_colshape_node("colshape", std::move(cube_col_shape));
                {
                    kbdcube_colshape_node->set_collision_behaviour(engine::node_collision_behaviour {
                        .passes_events_to_father = true,
                    });
                }
                kbdcube.add_child(std::move(kbdcube_colshape_node));
            }
            root.add_child(std::move(kbdcube));
        }


        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
