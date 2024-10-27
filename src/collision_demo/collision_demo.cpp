#include "collision_demo.hpp"
#include "collision_SAT.hpp"

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

    static mat4 compute_global_transform(node& n) {
        node* f = n.try_get_father();
        if(f) {
            return compute_global_transform(*f) * n.transform();
        } else {
            return n.transform();
        }
    }

    struct collider_state  {
        ::collision_shape shape;
        bool up = false, down = false, left = false, right = false, fwd = false, bwd = false;
        float move_speed = .5;
    };

    scene make_collision_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name) {
        gal::vertex_array cube_vao = gal::vertex_array::make<vertex_t>(vertex_data, std::span(indices.data(), indices.size()));
        material cube_material = make_retro_3d_material(get_rm().get_texture("assets/example.png"));
        mesh cube_mesh(cube_material, get_rm().new_from<gal::vertex_array>(std::move(cube_vao)));


        rc<const stateless_script> stillcube_script = get_rm().new_from(stateless_script {
            .process = [](node& n, std::any&, application_channel_t& app_chan) {
//                n.transform() = rotate(n.transform(), app_chan.from_app.delta * pi / 8, z_axis + y_axis / 2.f);
            },
        });

        rc<const stateless_script> othercube_script = get_rm().new_from(stateless_script{
            .construct = [](){ return std::any(collider_state(make_collision_shape_from_mesh(vertex_data, indices))); },
            .process = [](node& n, std::any& state, application_channel_t& app_chan) {
                collider_state& s = *std::any_cast<collider_state>(&state);

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

                //handle collisions
                const ::collision_shape& this_shape = s.shape;
                // there is currently no way for us to access this data from outside the stillcube node; fortunately, it is a cube just like us
                const ::collision_shape& other_shape = this_shape;

                const mat4& this_transform = compute_global_transform(n);
                node& other_node = n.get_father().get_child("stillcube");
                const mat4& other_transform = compute_global_transform(other_node);
                collision_result coll_result = check_collision(this_shape, this_transform, other_shape, other_transform);
                std::string collision_str = coll_result
                    ? std::format("Yes: ({}, {}, {}), {}", coll_result.get_versor().x, coll_result.get_versor().y, coll_result.get_versor().z, coll_result.get_depth())
                    : std::string("No");

                ImGui::Text("Collision detected?: %s", collision_str.c_str());

                if(coll_result)
                    n.transform() = glm::translate(n.transform(), coll_result.get_min_move_vector());
            }
        });

        node root("");
        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));
        root.add_child(node("stillcube", cube_mesh, glm::mat4(1), std::move(stillcube_script)));
        root.add_child(node("othercube", cube_mesh, glm::mat4(1), std::move(othercube_script)));
        root.add_child(node("camera", camera(), glm::inverse(glm::lookAt(vec3(3,6,6), vec3(0), vec3(0,1,0)))));
        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
