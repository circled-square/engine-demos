#include "texture_demo.hpp"

#include "../imgui_menu_node.hpp"
#include <glm/glm.hpp>
#include <engine/resources_manager.hpp>
#include <array>

/* This is a fairly simple, lower level demo meant to show direct use of the GAL (Graphics Abstraction Layer) library.
 * All rendering is done on 2 imgui windows; the first simply shows a texture, the second shows the result of
 * rendering to a framebuffer object, using a custom shader. The use of anything under engine:: namespace is avoided
 * wherever possible.
 */

namespace engine_demos {
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
        std::string vert = "#version 440 core \n \
            layout(location = 0) in vec2 pos; \
            out vec2 v_tex_coord;\
            void main() { \
                gl_Position = vec4(pos, 0, 1); \
                v_tex_coord = pos/2+.5; \
            }";
        std::string frag = "#version 330 core \n \
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
        framebuffer_t fbo = framebuffer_t(gal::texture::empty({64, 64}, 4));

        imgui_tex_script_state_t() = default;
        imgui_tex_script_state_t(imgui_tex_script_state_t&&) = default;
        imgui_tex_script_state_t(const imgui_tex_script_state_t&) {} // copy ctor simply calls default ctor; only makes sense because the attributes never change
    };

    engine::scene make_texture_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        engine::node root("");

        root.add_child(make_imgui_menu_node(std::move(scene_names), scene_name));

        engine::stateless_script imgui_tex_script { engine::script_vtable {
            .construct = [](const engine::node&) { return std::any(imgui_tex_script_state_t()); },
            .process = [](const engine::node& n, std::any& ss, engine::application_channel_t& c) {
                imgui_tex_script_state_t& s = *std::any_cast<imgui_tex_script_state_t>(&ss);

                ImGui::SetCurrentContext(c.from_app().get_current_imgui_context());

                ImGui::Begin("Texture Window");
                {
                    // Using a Child allows to fill all the space of the window.
                    ImGui::BeginChild("TextureRender2");
                    ImVec2 wsize = ImGui::GetWindowSize();
                    // Because I use the texture from OpenGL, I need to invert the V from the UV.
                    ImGui::Image((ImTextureID)s.tex.get_gl_id(), wsize, ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::EndChild();
                }
                ImGui::End();

                ImGui::Begin("Fbo Texture Window");
                {
                    s.fbo.bind();
                    {
                        s.renderer.change_viewport_size(s.fbo.resolution());
                        s.renderer.clear();

                        const int texture_slot = 0;
                        s.tex.bind(texture_slot);
                        s.shader.set_uniform<int>("u_texture_slot", texture_slot);
                        s.renderer.draw(s.vao, s.shader);
                    }
                    s.fbo.unbind();

                    // Using a Child allows to fill all the space of the window.
                    ImGui::BeginChild("TextureRender1");
                    ImVec2 wsize = ImGui::GetWindowSize();
                    // Because I use the texture from OpenGL, I need to invert the V from the UV.
                    ImGui::Image((ImTextureID)s.fbo.get_texture()->get_gl_id(), wsize, ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::EndChild();
                }
                ImGui::End();
            }
        }};


        root.add_child(engine::node("imgui-tex-node", std::monostate(), glm::mat4(1), std::move(imgui_tex_script)));

        return engine::scene(scene_name, std::move(root));
    }
} // namespace engine_demos
