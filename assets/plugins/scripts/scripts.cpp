#include "scripts.hpp"
#include <slogga/log.hpp>
#include <slogga/asserts.hpp>
#include <engine/utils/constants.hpp>
#include <engine/scene/node.hpp>
#include <engine/scene/application_channel.hpp>
#include <engine/application/window.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/resources_manager/rc.hpp>
#include <engine/utils/format_glm.hpp>
#include <engine/utils/lin_algebra.hpp>
#include <imgui.h>
#include <queue>
#include <set>
#include <random>

using namespace engine::constants;
using engine::get_rm;
using engine::node;
using engine::rc;

namespace cube {
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
    static constexpr std::array<glm::uvec3, 12> indices {
        glm::uvec3
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

    static rc<const engine::collision_shape> make_col_shape() {
        return get_rm().new_from(engine::collision_shape::from_mesh(
            engine::stride_span<const glm::vec3>(vertex_data.data(), offsetof(vertex_t, pos), sizeof(vertex_t), vertex_data.size()),
            std::span<const glm::uvec3>(indices.begin(), indices.end()),
            engine::collision_layer(1) | engine::collision_layer(2), engine::collision_layer(1)
        ));
    }
    static gal::vertex_array make_vao() {
        return gal::vertex_array::make<vertex_t>(vertex_data, std::span(indices.data(), indices.size()));
    }
    static engine::mesh make_mesh() {
        auto vao = get_rm().new_from<gal::vertex_array>(make_vao());
        auto shader = get_rm().load<engine::shader>(engine::internal_resource_name_t::simple_3d_shader);
        auto texture = get_rm().load<gal::texture>("example.png");
        auto material = engine::material(std::move(shader), std::move(texture));
        auto mesh = engine::mesh(std::move(material), std::move(vao));

        return mesh;
    }
}

namespace imgui_dbgmenu {
    struct state {
        static constexpr std::array<const char*, 7> scene_names {
            "texture_demo.yml",
            "3d_demo.yml",
            "gltf_demo.yml",
            "postfx demo",
            "freecam demo",
            "viewport demo",
            "collision_demo.yml",
        };

        std::queue<float> delta_time_hist;
        std::multiset<float, std::greater<float>> sorted_delta_times;
        float delta_time_total = 0.f; //always contains the sum of all the elements of m_delta_time_hist
        bool is_first_frame = true;

        float max_delta_time_total = 3.f;
    };
    constexpr engine::script_vtable script {
        .construct = [](node&, const std::any&) { return std::any(state()); },
        .process = [](node& n, std::any& ss, engine::application_channel_t& c) {
            ImGui::SetCurrentContext(c.from_app().get_current_imgui_context());

            state& s = *std::any_cast<state>(&ss);

            if(ImGui::Begin("Scene Menu", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
                ImGui::Text("Active Demo: %s", c.from_app().scene_name.c_str());

                for(const char* name : state::scene_names) {
                    if(ImGui::Button(name)) {
                        c.to_app().scene_to_change_to = get_rm().load_mut<engine::scene>(name);
                    }
                }

                //TODO: use ImPlot to plot framerate

                float delta = c.from_app().delta;
                s.delta_time_hist.push(delta);
                s.sorted_delta_times.insert(delta);
                s.delta_time_total += delta;

                while (s.delta_time_total > s.max_delta_time_total && s.delta_time_hist.size() > 1) {
                    float removed_delta = s.delta_time_hist.front();
                    s.delta_time_total -= removed_delta;
                    s.sorted_delta_times.erase(s.sorted_delta_times.lower_bound(removed_delta));
                    s.delta_time_hist.pop();
                }
                ASSERTS(s.delta_time_hist.size() == s.sorted_delta_times.size());


                ImGui::Text("framerate\t%.1f FPS\n", 1.f/delta);
                if(ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Indent(16.f);

                    auto iterator = s.sorted_delta_times.begin();
                    size_t index = 0;

                    for(; index < s.sorted_delta_times.size()/1000; index++) iterator++;
                    float low_0_1_percent = 1.f / *iterator; // low 0.1%
                    for(; index < s.sorted_delta_times.size()/100; index++) iterator++;
                    float low_1_percent = 1.f / *iterator; // low 1%
                    float average = 1.f/ (s.delta_time_total / s.delta_time_hist.size()); // average fps

                    ImGui::Text(
                        "average\t%.1f FPS\n"
                        "low 1%%\t%.1f FPS\n"
                        "low 0.1%%\t%.1f FPS\n",
                        average, low_1_percent, low_0_1_percent
                    );

                    ImGui::Text("hist size: %lu", s.delta_time_hist.size());

                    ImGui::Unindent(16.f);
                }

                // traverse the tree to show it to the user
                if(ImGui::CollapsingHeader("Scene Hierarchy")) {
                    ImGui::Indent(16.f);
                    // find root
                    node* root = &n;
                    while(true) {
                        EXPECTS(root);
                        node* new_root = root->get_father();
                        if(!new_root)
                            break;
                        else
                            root = new_root;
                    }
                    ENSURES(root);
                    //dfs
                    struct stack_tuple_t {
                        node* n;
                        enum {preorder, postorder} visit_type;
                    };
                    std::vector<stack_tuple_t> dfs_stack;
                    dfs_stack.push_back({ std::move(root), stack_tuple_t::preorder });
                    while(!dfs_stack.empty()) {
                        auto[n, visit_type] = dfs_stack.back();
                        dfs_stack.pop_back();
                        std::string n_name;

                        if(n->get_father()) {
                            n_name = n->name();
                        } else {
                            n_name = std::format("(root) {}", n->name());
                        }


                        const char* name = n_name.empty() ? "(empty name)" : n_name.c_str();
                        std::string tooltip_contents = std::format("{}", engine::extract_position(n->transform()));


                        if(visit_type == stack_tuple_t::preorder) {
                            if(n->children().empty()) {
                                // this is a leaf (just print the name)
                                ImGui::Indent(8.f);
                                ImGui::Text("- %s", name);

                                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                                    ImGui::SetTooltip(tooltip_contents.c_str());

                                ImGui::Unindent(8.f);
                            } else {
                                // this is not a leaf (print the name in a collapsing header and its children under it)
                                ImGuiTreeNodeFlags flags = n->children().size() >= 5 ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
                                std::string imgui_id = std::format("{}##{}", name, (void*)n);
                                bool collapsing_header_uncollapsed = ImGui::CollapsingHeader(imgui_id.c_str(), flags);
                                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                                    ImGui::SetTooltip(tooltip_contents.c_str());

                                if(collapsing_header_uncollapsed) {
                                    ImGui::Indent(8.f);
                                    dfs_stack.push_back({n, stack_tuple_t::postorder});

                                    for(std::int64_t i = n->children().size()-1; i >= 0; i--)
                                        dfs_stack.push_back({ &n->children()[i], stack_tuple_t::preorder });
                                }
                            }
                        } else {
                            ASSERTS(visit_type == stack_tuple_t::postorder);
                            ImGui::Unindent(8.f);
                        }
                    }
                    ImGui::Unindent(16.f);
                }

                if(ImGui::Button("Collect Garbage"))
                    get_rm().collect_garbage();

                ImGui::End();
            }

            if(s.is_first_frame) {
                ImGui::SetWindowFocus(nullptr);
                s.is_first_frame = false;
            }
        }
    };
}

namespace demo_texture {
    struct window_state {
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

        using framebuffer_t = gal::framebuffer<std::optional<gal::texture>>;

        gal::renderer renderer = gal::renderer();
        gal::vertex_array vao = make_whole_screen_vao();
        gal::shader_program shader = make_shader();
        gal::texture tex = gal::texture(gal::image("assets/example.png"));
        framebuffer_t fbo = framebuffer_t(gal::texture::empty({64, 64}, 4));

        window_state() = default;
        window_state(const window_state&) {} // copy ctor simply calls default ctor; only makes sense because the attributes never change
    };

    constexpr engine::script_vtable windows_script {
        .construct = [](node&, const std::any&) { return std::any(window_state()); },
        .process = [](node& n, std::any& ss, engine::application_channel_t& c) {
            window_state& s = *std::any_cast<window_state>(&ss);

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
    };
}
namespace demo_freecam {
    struct cam_state  {
        bool up = false, down = false, left = false, right = false, fwd = false, bwd = false, go_faster = false;
        float current_y_rotation = 0.f;
    };
    constexpr engine::script_vtable cam_script {
        .construct = [](node&, const std::any&){ return std::any(cam_state()); },
        .process = [](node& n, std::any& ss, engine::application_channel_t& app_chan) {
            node& father = n.get_father_checked();
            cam_state& s = *std::any_cast<cam_state>(&ss);

            for(const engine::event_variant_t& event : app_chan.from_app().events) {
                namespace key_codes = engine::window::key_codes;
                match_variant(event,
                [&s, &app_chan, &father](const engine::key_event_t& e) {
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
                        } else if (e.key == key_codes::LEFT_SHIFT) {
                            s.go_faster = e.action != key_action_codes::RELEASE;
                        } else if (e.key == key_codes::C && e.action == key_action_codes::RELEASE) {
                            //toggle mouse capture
                            app_chan.to_app().wants_mouse_cursor_captured = !app_chan.from_app().mouse_cursor_is_captured;
                        } else if (e.key == key_codes::F5 && e.action == key_action_codes::RELEASE) {
                            // have the resources manager hot-reload the dither shader
                            get_rm().hot_reload<engine::shader>("shaders/postfx/dither.glsl");
                        }
                }, [&s, &app_chan, &n, &father](const engine::mouse_move_event_t& e) {
                    if(!app_chan.from_app().mouse_cursor_is_captured)
                        return;
                    glm::vec2 movement = e.movement;
                    constexpr float movement_multiplier = 0.002;
                    movement *= movement_multiplier;

                    float old_rotation = s.current_y_rotation;
                    s.current_y_rotation += movement.y;
                    s.current_y_rotation = std::clamp(s.current_y_rotation, -pi/2.f * 0.99f, pi/2.f * 0.99f);
                    movement.y = s.current_y_rotation - old_rotation;

                    father.set_transform(glm::rotate(father.transform(), -movement.x, y_axis));
                    n.set_transform(glm::rotate(n.transform(), -movement.y, x_axis));
                });
            }

            glm::vec3 movement(
                (s.right?1:0) - (s.left?1:0),
                (s.up   ?1:0) - (s.down?1:0),
                (s.bwd  ?1:0) - (s.fwd ?1:0)
            );

            float move_speed = 30.0;
            movement *= move_speed * app_chan.from_app().delta * (s.go_faster ? 3.0 : 1.0);

            father.set_transform(glm::translate(father.transform(), movement));
        },
    };

    constexpr engine::script_vtable set_shader_script {
        .construct = [](node& self, const std::any&){
            for(auto& primitive : self.children()[0].get<engine::mesh>().primitives())
                primitive.primitive_material.set_shader(get_rm().load<engine::shader>("shaders/3d/light_falloff.glsl"));
            return std::any(std::monostate());
        }
    };
}
namespace demo_3d {
    constexpr int cube_amount_constant = 10; // n_of_cubes = (cube_amount_constant * 2 + 1) ^ 3
    constexpr float camera_relative_distance = 1.0f;
    constexpr int nodetree_depth = 2; // number of useless layers to add (useful to artificially make the scene more CPU intensive without making it more GPU intensive)
    constexpr float rand_displacement_amount = 100;
    constexpr float cube_scale = .5f;

    struct cam_state  {
        float time = 0.f;
    };

    constexpr engine::script_vtable cube_spawner_script {
        .construct = [](node& n, const std::any&) {
            gal::vertex_array cube_vao = cube::make_vao();
            engine::material cube_material(
                get_rm().load<engine::shader>("shaders/3d/custom_uniform_example.glsl"),
                get_rm().load<gal::texture>("example.png")
            );
            cube_material.get_custom_uniforms().push_back(std::make_pair(std::string("u_custom"), engine::uniform_value_variant(0.f)));
            // *cube_material.get_shader();
            engine::mesh cube(std::move(cube_material), get_rm().new_from<gal::vertex_array>(std::move(cube_vao)));

            constexpr int k = cube_amount_constant;
            std::minstd_rand rng((std::random_device()()));
            std::uniform_real_distribution<float> distr(-1., 1.);

            for(int x = -k; x <= k; x++) {
                for(int y = -k; y <= k; y++) {
                    for(int z = -k; z <= k; z++) {
                        using glm::vec3;
                        vec3 displacement = vec3{x,y,z} == vec3(0) ? vec3(0) : vec3{x,y,z} + rand_displacement_amount * powf((float)distr(rng), 1.5f) * (vec3{x,y,z} == vec3{0} ? vec3{0} : glm::normalize(vec3{distr(rng), distr(rng), distr(rng)}));

                        auto c = node::make(
                            std::format("cube_{},{},{}", x, y, z),
                            cube,
                            glm::scale(glm::translate(glm::mat4(1), displacement), glm::vec3(cube_scale))
                        );


                        if(glm::uvec3(x,y,z) == glm::uvec3(0))
                            c->attach_script(engine::stateless_script::from(get_rm().load<dylib::library>("plugins/scripts/lib/scripts"), "3d_demo.custom_uniform_example"));

                        //artificially increase depth of tree
                        for(int i = 0; i < nodetree_depth; i++) {
                            auto p = node::make(c->name());
                            p->add_child(std::move(c));
                            c = std::move(p);
                        }
                        n.add_child(std::move(c));
                    }
                }
            }

            return std::any(std::monostate());
        },
    };
    constexpr engine::script_vtable cam_script {
        .construct = [](node&, const std::any&) { return std::any(cam_state()); },
        .process = [](node& n, std::any& ss, engine::application_channel_t& app_chan) {

            cam_state& s = *std::any_cast<cam_state>(&ss);
            s.time += app_chan.from_app().delta;

            float distance = cube_amount_constant * camera_relative_distance;
            glm::vec3 pos = distance * glm::vec3{ sin(s.time), sin(s.time), cos(s.time) };

            n.set_transform(glm::inverse(glm::lookAt(pos, glm::vec3(0), glm::vec3(0,1,0))));
        }
    };
    constexpr engine::script_vtable custom_uniform_example_script{
        .construct = [](node&, const std::any&) { return std::any(0.f); },
        .process = [](node& n, std::any& state, engine::application_channel_t& app_chan) {
            EXPECTS(state.type() == typeid(float));
            float& last_delta = *std::any_cast<float>(&state);
            float delta_diff = last_delta - app_chan.from_app().delta;

            n.get<engine::mesh>()
                .primitives()[0].primitive_material
                .get_custom_uniforms()[0].second = delta_diff * 200.f;


            last_delta = app_chan.from_app().delta;
        },
    };
}
namespace demo_gltf {
    static constexpr engine::script_vtable rotate_script {
        .process = [](node& n, std::any&, engine::application_channel_t& c) {
            n.set_transform(glm::rotate(n.transform(), c.from_app().delta * pi / 16, y_axis));
        },
    };
}
namespace demo_collision {
    struct kbdcube_state  {
        bool up = false, down = false, left = false, right = false, fwd = false, bwd = false;
        float move_speed = 1.;
    };

    constexpr engine::script_vtable stillcube_script {
        .construct = [](node& self, const std::any&) {
            self.add_child(node::make("colshape", cube::make_col_shape()));

            self.set_payload(cube::make_mesh());

            return std::any(std::monostate());
        },
        .process = [](node& n, std::any&, engine::application_channel_t& app_chan) {
            n.set_transform(glm::rotate(n.transform(), app_chan.from_app().delta * pi / 8, z_axis + y_axis / 2.f));
        },
    };
    constexpr engine::script_vtable kbdcube_script {
        .construct = [](node& self, const std::any&) {
            auto colshape_node = node::make("colshape", cube::make_col_shape());
            colshape_node->set_collision_behaviour(engine::node_collision_behaviour {
                .passes_events_to_father = true,
            });
            self.add_child(std::move(colshape_node));

            self.set_collision_behaviour(engine::node_collision_behaviour {
                .moves_away_on_collision = true,
            });

            self.set_payload(cube::make_mesh());

            return std::any(demo_collision::kbdcube_state());
        },
        .process = [](node& n, std::any& state, engine::application_channel_t& app_chan) {
            EXPECTS(state.type() == typeid(demo_collision::kbdcube_state));
            demo_collision::kbdcube_state& s = *std::any_cast<demo_collision::kbdcube_state>(&state);

            // handle kbd input
            for(const engine::event_variant_t& event : app_chan.from_app().events) {
                match_variant(event, [&s, &app_chan](const engine::key_event_t& e) {
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
            glm::vec3 move_vec = glm::vec3 {
                (int)s.right - (int)s.left,
                (int)s.up    - (int)s.down,
                (int)s.bwd   - (int)s.fwd,
            } * s.move_speed * app_chan.from_app().delta;
            n.set_transform(glm::translate(n.transform(), move_vec));
        }
    };
}
namespace demo_viewport {
    constexpr engine::script_vtable viewport_cube_script {
        .construct = [](node& self, const std::any& params) {
            auto fbo_texture = std::any_cast<rc<gal::texture>>(params);
            auto shader = engine::get_rm().load<engine::shader>(engine::internal_resource_name_t::simple_3d_shader);
            auto vao = engine::get_rm().new_from(cube::make_vao());

            self.set_payload(engine::mesh(engine::material(shader, fbo_texture), vao));

            return std::any(std::monostate());
        },
        .process = [](node& n, std::any&, engine::application_channel_t& c) {
            c.to_app().clear_color = glm::vec4(0.2, 0.2, 0.2, 1.0);
            n.set_transform(glm::rotate(n.transform(), c.from_app().delta * pi / 16, y_axis));
        },
    };
}

const std::pair<const char*, engine::script_vtable> exported_plugins[] {
    std::pair("imgui_dbgmenu", imgui_dbgmenu::script),
    std::pair("gltf_demo.rotate", demo_gltf::rotate_script),
    std::pair("viewport_demo.viewport_cube", demo_viewport::viewport_cube_script),
    std::pair("texture_demo.imgui_windows", demo_texture::windows_script),
    std::pair("freecam_demo.cam", demo_freecam::cam_script),
    std::pair("freecam_demo.set_shader", demo_freecam::set_shader_script),
    std::pair("collision_demo.stillcube", demo_collision::stillcube_script),
    std::pair("collision_demo.kbdcube", demo_collision::kbdcube_script),
    std::pair("3d_demo.cube_spawner", demo_3d::cube_spawner_script),
    std::pair("3d_demo.cam", demo_3d::cam_script),
    std::pair("3d_demo.custom_uniform_example", demo_3d::custom_uniform_example_script),
};

const std::size_t exported_plugins_size = sizeof(exported_plugins) / sizeof(exported_plugins[0]);
