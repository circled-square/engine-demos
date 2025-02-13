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

    noderef make_imgui_menu_node(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name) {
        rc<const stateless_script> imgui_menu_script = get_rm().new_from(stateless_script {
            .process = [](const noderef& n, std::any& ss, application_channel_t& c) {
                menu_state_t& s = *std::any_cast<menu_state_t>(&ss);

                if(ImGui::Begin("Scene Menu", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
                    ImGui::Text("Active Demo: %s", s.scene_name.c_str());

                    for(const std::string& name : *s.scene_names) {
                        if(ImGui::Button(name.c_str())) {
                            c.to_app().scene_to_change_to = get_rm().get_scene(name);
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

                        float lowest = 1.f / *iterator; // lowest framerate recorded
                        for(; index < s.sorted_delta_times.size()/1000; index++) iterator++;
                        float low_0_1_percent = 1.f / *iterator; // low 0.1%
                        for(; index < s.sorted_delta_times.size()/100; index++) iterator++;
                        float low_1_percent = 1.f / *iterator; // low 1%
                        // median fps is not calculated since it is by far the most expensive metric, and also the least useful
                        // for(; index < m_sorted_delta_times.size()/2; index++) iterator++;
                        // float median = 1.f / *iterator; // median fps
                        float average = 1.f/ (s.delta_time_total / s.delta_time_hist.size()); // average fps

                        ImGui::Text(
                            "average\t%.1f FPS\n"
                            "low 1%%\t%.1f FPS\n"
                            "low 0.1%%\t%.1f FPS\n"
                            "lowest\t%.1f FPS",
                            average, low_1_percent, low_0_1_percent, lowest
                        );

                        ImGui::Text("hist size: %lu", s.delta_time_hist.size());

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
        });

        std::any menu_state = menu_state_t(std::move(scene_names), std::move(scene_name));

        noderef ret("menu-node", null_node_data(), glm::mat4(1), std::move(imgui_menu_script));
        ret->set_script_state(std::move(menu_state));
        return ret;
    }
} // namespace engine_demos
