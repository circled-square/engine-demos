#include "3d_demo.hpp"

#include "../imgui_menu_node.hpp"
#include "engine/scene/application_channel.hpp"
#include "engine/scene/renderer/mesh/material.hpp"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <engine/resources_manager.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/utils/format_glm.hpp>
#include <engine/utils/constants.hpp>
#include <variant>
#include <random>

/* This demo demonstrates a few things:
 * - 3d rendering
 * - use of custom texture on a raw-data 3d mesh (the cube)
 * - use of the scene tree
 * - use of script to programmatically generate a scene (container_script)
 * - use of script to manipulate objects' transforms (camera and centre cube)
 * - custom script state for the camera (the time, which would be unnecessary since it can be retrieved through the application_channel_t)
 * - custom uniform passed to the (centre) cube's material (to offset its texture), without having any effect on other instances (other cubes) of the same material (done through its own script)
 */

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

    struct camera_script_state  {
        float time = 0.f;
    };


    scene make_3d_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        node root("");

        constexpr int cube_amount_constant = 5; // n_of_cubes = (cube_amount_constant * 2 + 1) ^ 3
        constexpr int nodetree_depth = 0; // number of useless layers to add (useful to artificially make the scene more CPU intensive without making it more GPU intensive)
        constexpr float rand_displacement_amount = 100;
        constexpr float cube_scale = .5f;
        constexpr float camera_relative_distance = 1.0f;

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        stateless_script container_script { script_vtable {
            .construct = [](const node& n) {
                stateless_script centre_cube_script { script_vtable {
                    .process = [](const node& n, std::any& state, application_channel_t& app_chan) {
                        n->set_transform(rotate(n->transform(), app_chan.from_app().delta * pi / 8, z_axis + y_axis / 2.f));

                        //state initialization
                        if(state.type() != typeid(float)) {
                            state = (float)0.f;
                        }

                        EXPECTS(state.type() == typeid(float));
                        float diff = std::any_cast<float>(state) - app_chan.from_app().delta;

                        n->get<mesh>()
                            .primitives()[0].primitive_material
                            .get_custom_uniforms()[0].second = engine::uniform_value_variant(diff * 200.f);


                        state = (float)app_chan.from_app().delta;
                    },
                }};
                gal::vertex_array cube_vao = gal::vertex_array::make<vertex_t>(vertex_data, std::span(indices.data(), indices.size()));
                material cube_material(
                    get_rm().load<shader>("shaders/3d/custom_uniform_example.glsl"),
                    get_rm().load<gal::texture>("example.png")
                );
                cube_material.get_custom_uniforms().push_back(std::make_pair(std::string("u_custom"), engine::uniform_value_variant(0.f)));
                mesh cube(cube_material, get_rm().new_from<gal::vertex_array>(std::move(cube_vao)));

                const int k = cube_amount_constant;
                std::minstd_rand rng((std::random_device()()));
                std::uniform_real_distribution<float> distr(-1., 1.);

                for(int x = -k; x <= k; x++) {
                    for(int y = -k; y <= k; y++) {
                        for(int z = -k; z <= k; z++) {
                            vec3 displacement = vec3{x,y,z} == vec3(0) ? vec3(0) : vec3{x,y,z} + rand_displacement_amount * powf((float)distr(rng), 1.5f) * (vec3{x,y,z} == vec3{0} ? vec3{0} : glm::normalize(vec3{distr(rng), distr(rng), distr(rng)}));

                            node c(
                                std::format("cube_{},{},{}", x, y, z),
                                cube,
                                scale(translate(mat4(1), displacement), vec3(cube_scale))
                            );

                            //artificially increase depth of tree
                            for(int i = 0; i < nodetree_depth; i++) {
                                node p(c->name());
                                p.add_child(c);
                                c = p;
                            }
                            n.add_child(c);
                        }
                    }
                }
                node_data::attach_script(n->get_child("cube_0,0,0"), std::move(centre_cube_script));

                return std::any(std::monostate());
            },
        }};
        root.add_child(node("cubes_container",std::monostate(), mat4(1), std::move(container_script)));

        stateless_script cam_script { script_vtable {
            .construct = [](const node&) { return std::any(camera_script_state()); },
            .process = [](const node& n, std::any& ss, application_channel_t& app_chan) {
                camera_script_state& s = *std::any_cast<camera_script_state>(&ss);
                s.time += app_chan.from_app().delta;

                float distance = cube_amount_constant * camera_relative_distance;
                vec3 pos = distance * vec3{ sin(s.time), sin(s.time), cos(s.time) };

                n->set_transform(glm::inverse(glm::lookAt(pos, vec3(0), vec3(0,1,0))));
            }
        }};
        root.add_child(node("camera", camera(), glm::translate(mat4(1), vec3(0,0,4)), std::move(cam_script)));

        return scene(std::move(scene_name), std::move(root));
    }
} // namespace engine_demos
