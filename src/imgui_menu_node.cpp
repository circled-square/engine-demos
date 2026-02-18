#include "imgui_menu_node.hpp"

#include <engine/resources_manager.hpp>
#include <imgui.h>
#include <queue>
#include <set>

namespace engine_demos {
    using namespace engine;

    struct menu_state_t {
        std::shared_ptr<std::forward_list<std::string>> scene_names;

        std::queue<float> delta_time_hist;
        std::multiset<float, std::greater<float>> sorted_delta_times;
        float delta_time_total = 0.f; //always contains the sum of all the elements of m_delta_time_hist
        bool is_first_frame = true;

        float max_delta_time_total = 5.f;

        std::string scene_name;

        menu_state_t(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name): scene_names(std::move(scene_names)), scene_name(std::move(scene_name)) {}
        virtual ~menu_state_t() = default;
    };

    node make_imgui_menu_node(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        stateless_script imgui_menu_script { script_vtable {
            .process = [](const node& n, std::any& ss, application_channel_t& c) {
                ImGui::SetCurrentContext(c.from_app().get_current_imgui_context());

                menu_state_t& s = *std::any_cast<menu_state_t>(&ss);

                if(ImGui::Begin("Scene Menu", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
                    ImGui::Text("Active Demo: %s", s.scene_name.c_str());

                    for(const std::string& name : *s.scene_names) {
                        if(ImGui::Button(name.c_str())) {
                            c.to_app().scene_to_change_to = get_rm().load_mut<scene>(name);
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
                        EXPECTS((rc<node_data>)n);
                        rc<node_data> root = n;
                        while(true) {
                            EXPECTS(root);
                            rc<node_data> new_root = root->get_father();
                            if(!new_root)
                                break;
                            else
                                root = std::move(new_root);
                            ENSURES(root);
                        }
                        ENSURES(root);
                        //dfs
                        struct stack_tuple_t {
                            rc<node_data> node;
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

                            if(visit_type == stack_tuple_t::preorder) {
                                if(n->children().empty()) {
                                    // this is a leaf (just print the name)
                                    ImGui::Text("- %s", n_name.c_str());
                                } else {
                                    // this is not a leaf (print the name in a collapsing header and its children under it)
                                    ImGuiTreeNodeFlags flags = n->children().size() >= 5 ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
                                    if(ImGui::CollapsingHeader(n_name.c_str(), flags)) {
                                        ImGui::Indent(8.f);
                                        dfs_stack.push_back({n, stack_tuple_t::postorder});

                                        for(std::int64_t i = n->children().size()-1; i >= 0; i--)
                                            dfs_stack.push_back({ n->children()[i], stack_tuple_t::preorder });
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
        }};

        std::any menu_state = menu_state_t(std::move(scene_names), std::move(scene_name));

        node ret("menu-node", std::monostate(), glm::mat4(1), std::move(imgui_menu_script));
        ret->set_script_state(std::move(menu_state));
        return ret;
    }
} // namespace engine_demos
