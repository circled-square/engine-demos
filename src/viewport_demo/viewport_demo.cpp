#include "viewport_demo.hpp"

#include "../imgui_menu_node.hpp"
#include "engine/scene/node/script.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/utils/constants.hpp>

// This 3d rendered demo shows postfx shaders applied to a gltf-imported model

namespace engine_demos {
    using namespace engine;
    using glm::vec3; using glm::uvec3; using glm::vec4; using glm::mat4;

    using vertex_t = engine::retro_3d_shader_vertex_t;

    static constexpr std::array<vertex_t, 16> cube_mesh_verts {
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
    static constexpr std::array<uvec3, 12> cube_mesh_indices {
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

    scene make_viewport_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        node root("");
        rc<const stateless_script> root_script = get_rm().new_from(stateless_script {
            .process = [](const node&, std::any&, application_channel_t& c) {
                c.to_app().clear_color = vec4(0.2, 0.2, 0.2, 1.0);
            }
        });
        node_data::attach_script(root, std::move(root_script));

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        node halftone_viewport("halftone_vp", engine::viewport(get_rm().new_mut_from(gal::texture::empty({256, 256}, 4))));

        gal::vertex_array cube_vao = gal::vertex_array::make<vertex_t>(cube_mesh_verts, std::span(cube_mesh_indices.data(), cube_mesh_indices.size()));

        node halftone_viewport_mesh("halftone_vp_mesh", mesh(material(
            get_rm().new_from(shader::from_file("assets/shaders/halftone_postfx.glsl")),
            halftone_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().get_whole_screen_vao()));

        node transparent_viewport("transparent_vp", engine::viewport(get_rm().new_mut_from(gal::texture::empty({512, 512}, 4))));

        node cube("cube", mesh(material(
            get_rm().get_retro_3d_shader(),
            transparent_viewport->get<viewport>().fbo().get_texture()
        ), get_rm().new_from(std::move(cube_vao))));


        node subscene_cam("subscene_camera", camera(), glm::translate(mat4(1), vec3(0,50,250)));
        node cam("camera", camera(), glm::inverse(glm::lookAt(vec3(0,1,2), vec3(0,0,0), vec3(0,1,0))));

        halftone_viewport.add_child(std::move(subscene_cam));
        halftone_viewport.add_child(node(get_rm().get_nodetree_from_gltf("assets/castlebl.glb"), "castle"));
        halftone_viewport_mesh.add_child(std::move(halftone_viewport));
        transparent_viewport.add_child(std::move(halftone_viewport_mesh));
        root.add_child(std::move(transparent_viewport));
        root.add_child(std::move(cube));
        root.add_child(std::move(cam));

        rc<const stateless_script> rotate_script = get_rm().new_from(stateless_script {
            .process = [](const node& n, std::any&, application_channel_t& c) {
                n->set_transform(rotate(n->transform(), c.from_app().delta * pi / 16, y_axis));
            },
        });
        node_data::attach_script(root->get_descendant_from_path("transparent_vp/halftone_vp_mesh/halftone_vp/castle"), rotate_script);
        node_data::attach_script(root->get_child("cube"), std::move(rotate_script));

        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
