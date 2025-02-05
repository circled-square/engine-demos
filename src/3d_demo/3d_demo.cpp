#include "3d_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <engine/resources_manager.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>

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

    struct camera_script_state  {
        float time = 0.f;
    };

    scene make_3d_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name) {
        node root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        rc<const stateless_script> container_script = get_rm().new_from(stateless_script {
            .attach = [](node& n, std::any&) {
                rc<const stateless_script> centre_cube_script = get_rm().new_from(stateless_script {
                    .process = [](node& n, std::any&, application_channel_t& app_chan) {
                        n.transform() = rotate(n.transform(), app_chan.from_app.delta * pi / 8, z_axis + y_axis / 2.f);
                    },
                });
                gal::vertex_array cube_vao = gal::vertex_array::make<vertex_t>(vertex_data, std::span(indices.data(), indices.size()));
                material cube_material(get_rm().get_retro_3d_shader(), get_rm().get_texture("assets/example.png"));
                mesh cube(cube_material, get_rm().new_from<gal::vertex_array>(std::move(cube_vao)));

                for(int x = -1; x <= 1; x++) {
                    for(int y = -1; y <= 1; y++) {
                        for(int z = -1; z <= 1; z++) {
                            n.add_child(node(
                                std::format("cube_{},{},{}", x, y, z),
                                cube,
                                scale(translate(mat4(1), vec3(x,y,z)), vec3(.5, .5, .5))
                            ));
                        }
                    }
                }
                n.get_from_path("cube_0,0,0").attach_script(std::move(centre_cube_script));
            },
        });
        root.add_child(engine::node("cubes_container",engine::null_node_data(), glm::mat4(1), script(std::move(container_script))));        rc<const stateless_script> cam_script = get_rm().new_from(stateless_script {
            .construct = []() { return std::any(camera_script_state()); },
            .process = [](node& n, std::any& ss, application_channel_t& app_chan) {
                camera_script_state& s = *std::any_cast<camera_script_state>(&ss);
                s.time += app_chan.from_app.delta;

                vec3 pos = { sin(s.time)*4, sin(s.time)*4, cos(s.time)*4 };

                n.transform() = glm::inverse(glm::lookAt(pos, vec3(0), vec3(0,1,0)));
            }
        });

        root.add_child(node("camera", camera(), glm::translate(glm::mat4(1), glm::vec3(0,0,4)), script(std::move(cam_script))));

        return scene(scene_name, std::move(root), engine::render_flags { .face_culling = engine::face_culling_t::front });
    }
} // namespace engine_demos
