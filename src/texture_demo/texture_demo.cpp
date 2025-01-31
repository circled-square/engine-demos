#include "texture_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <glm/glm.hpp>
#include <engine/resources_manager.hpp>
#include <array>

namespace engine_demos {
    using namespace engine;
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
                v_tex_coord = pos/2+.5; \
            }";
        const char *frag = "#version 330 core \n \
            in vec2 v_tex_coord;\
            out vec4 color; \
            uniform sampler2D u_texture_slot; \
            void main() { \
                color = texture(u_texture_slot, v_tex_coord).brga; \
            }";

        return gal::shader_program(vert, frag);
    }

    struct imgui_tex_script_state_t  {
        using framebuffer_t = gal::framebuffer<std::optional<gal::texture>>;
        gal::renderer renderer = gal::renderer();
        gal::vertex_array vao = make_whole_screen_vao();
        gal::shader_program shader = make_shader();
        gal::texture tex = gal::texture(gal::image("assets/example.png"));
        framebuffer_t fbo = framebuffer_t(gal::texture::empty({512, 512}, 4));

        imgui_tex_script_state_t() = default;
        imgui_tex_script_state_t(imgui_tex_script_state_t&&) = default;
        imgui_tex_script_state_t(const imgui_tex_script_state_t&) {} // copy ctor simply calls default ctor; only makes sense because the attributes never change
    };

    engine::scene make_texture_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name) {
        node root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        rc<const stateless_script> imgui_tex_script = get_rm().new_from(stateless_script {
            .construct = []() { return std::any(imgui_tex_script_state_t()); },
            .process = [](node& n, std::any& ss, application_channel_t& c) {
                imgui_tex_script_state_t& s = *std::any_cast<imgui_tex_script_state_t>(&ss);
                ImGui::Begin("Fbo Texture Window");
                {
                    s.fbo.bind();
                    {
                        glViewport(0, 0, s.fbo.resolution().x, s.fbo.resolution().y);
                        s.renderer.clear();

                        ASSERTS(s.fbo.resolution() == glm::ivec2(512,512));
                        ASSERTS(s.fbo.get_texture()->resolution() == glm::ivec2(512, 512));

                        const int texture_slot = 0;
                        s.tex.bind(texture_slot);
                        s.shader.set_uniform<int>("u_texture_slot", texture_slot);
                        s.renderer.draw(s.vao, s.shader);
                    }
                    s.fbo.unbind();

                    // Using a Child allow to fill all the space of the window.
                    ImGui::BeginChild("TextureRender1");
                    ImVec2 wsize = ImGui::GetWindowSize();
                    // Because I use the texture from OpenGL, I need to invert the V from the UV.
                    ImGui::Image((ImTextureID)s.fbo.get_texture()->get_gl_id(), wsize, ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::EndChild();
                }
                ImGui::End();

                ImGui::Begin("Texture Window");
                {
                    // Using a Child allow to fill all the space of the window.
                    ImGui::BeginChild("TextureRender2");
                    ImVec2 wsize = ImGui::GetWindowSize();
                    // Because I use the texture from OpenGL, I need to invert the V from the UV.
                    ImGui::Image((ImTextureID)s.tex.get_gl_id(), wsize, ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::EndChild();
                }
                ImGui::End();
            }
        });


        root.add_child(node("imgui-tex-node", null_node_data(), glm::mat4(1), std::move(imgui_tex_script)));

        return scene(scene_name, std::move(root));
    }
} // namespace engine_demos
