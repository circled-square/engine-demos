#include "viewport_demo.hpp"

#include "../imgui_menu_node.hpp"
#include "../postfx_demo/postfx_demo.hpp"
#include "engine/scene/node.hpp"
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

    static rc<const gal::vertex_array> make_cube_vao() {
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

        auto vao = gal::vertex_array::make<vertex_t>(cube_mesh_verts, std::span(cube_mesh_indices.data(), cube_mesh_indices.size()));
        return get_rm().new_from(std::move(vao));
    }


    scene make_viewport_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        node root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));
        node transparent_viewport("transparent_vp", engine::viewport(get_rm().new_from(gal::texture::empty({512, 512}, 4))));


        node cube_television("cube",
            mesh(material(
                get_rm().load<shader>(internal_resource_name_t::simple_3d_shader),
                transparent_viewport->get<viewport>().fbo().get_texture()
            ), make_cube_vao()),
            glm::mat4(1),
            stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "rotate_and_set_clear_color")
        );

        node cam("camera", camera(), glm::inverse(glm::lookAt(vec3(0,1,2), vec3(0,0,0), vec3(0,1,0))));

        transparent_viewport.add_child(make_postfx_demo_node_tree());
        //the viewport does not have to be child of the mesh which shows its texture (though it's tidier, in my opinion)
        root.add_child(std::move(transparent_viewport));
        root.add_child(std::move(cube_television));
        root.add_child(std::move(cam));
        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
