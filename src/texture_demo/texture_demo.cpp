#include "texture_demo.hpp"
#include <glm/glm.hpp>
#include <array>

namespace scene_demos {
    static gal::vertex_array make_whole_screen_vao() {
        struct post_process_vertex_t {
            glm::vec2 pos;
            using layout_t = decltype(gal::static_vertex_layout(pos));
        };

        //coords of big triangle covering the whole screen
        static const std::array<post_process_vertex_t, 3> whole_screen_vertices {
            post_process_vertex_t
            {{-1,-1}}, {{3,-1}}, {{-1,3}},
        };
        static const std::array<glm::uvec3, 1> whole_screen_indices = {
            glm::uvec3 {0, 1, 2},
        };

        gal::vertex_buffer vbo(whole_screen_vertices);
        gal::index_buffer ibo(whole_screen_indices);

        return gal::vertex_array(std::move(vbo), std::move(ibo), post_process_vertex_t::layout_t::to_vertex_layout());
    }

    static gal::shader_program make_shader() {
        std::vector<gal::shader_program> ret;
        const char *vert = "#version 440 core \n \
            layout(location = 0) in vec2 pos; \
            out vec2 v_tex_coord;\
            void main() { \
                gl_Position = vec4(pos, 0, 1); \
                v_tex_coord = (pos + vec2(1,1)) / 2.0; \
            }";
        const char *frag = "#version 330 core \n \
            in vec2 v_tex_coord;\
            out vec4 color; \
            uniform sampler2D u_texture_slot; \
            void main() { \
                color = texture(u_texture_slot, v_tex_coord); \
            }";

        return gal::shader_program(vert, frag);
    }

    texture_demo::texture_demo(std::shared_ptr<std::forward_list<const char*>> scene_names)
        : menu_demo(std::move(scene_names)),
          m_renderer(),
          m_vao(make_whole_screen_vao()),
          m_shader(make_shader()),
          m_img("resources/example.png"),
          m_tex(m_img)
    {}

    // Usually we would not be overriding this method, and we would not be using it to render graphics, but this
    // demo is a bit of a hack to demonstrate GAL usage, not a guide on how to use the engine.
    // To make sure the menu_demo window is drawn we need to call menu_demo::render_ui after our own rendering.
    void texture_demo::render_ui(float frame_time) {
        const int texture_slot = 0;
        m_tex.bind(texture_slot);
        m_shader.set_uniform<int>("u_texture_slot", texture_slot);

        m_renderer.clear();

        m_renderer.draw(m_vao, m_shader);

        menu_demo::render_ui(frame_time);
    }

    const char* texture_demo::get_name() const { return "texture demo"; }
} // scene_demos
