#include "menu_demo.hpp"
#include <glad/glad.h>
#include <imgui.h>
#include <cstring>

#include <engine/resources_manager.hpp>

namespace scene_demos {
    using namespace engine;

    menu_demo::menu_demo(std::shared_ptr<std::forward_list<const char*>> scene_names)
        : m_scene_names(std::move(scene_names)) {}

    menu_demo::menu_demo(menu_demo&& o)
        : engine::scene(std::move(o)), m_scene_names(std::move(o.m_scene_names)),
        m_delta_time_hist(std::move(o.m_delta_time_hist)), m_delta_time_total(o.m_delta_time_total),
        m_is_first_frame(o.m_is_first_frame)  {}


    constexpr float max_delta_time_total = 5.f;

    void menu_demo::render_ui(float frame_time) {
        if(ImGui::Begin("Scene Menu", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
            ImGui::Text("Active Demo: %s", this->get_name());

            for(const char* name : *m_scene_names) {
                if(ImGui::Button(name)) {
                    application_channel().to_app.scene_to_change_to = get_rm().get_scene(name);
                }
            }

            //TODO: use ImPlot to plot framerate

            float delta = ImGui::GetIO().DeltaTime;
            m_delta_time_hist.push(delta);
            m_sorted_delta_times.insert(delta);
            m_delta_time_total += delta;

            while (m_delta_time_total > max_delta_time_total && m_delta_time_hist.size() > 1) {
                float removed_delta = m_delta_time_hist.front();
                m_delta_time_total -= removed_delta;
                m_sorted_delta_times.erase(m_sorted_delta_times.lower_bound(removed_delta));
                m_delta_time_hist.pop();
            }
            ASSERTS(m_delta_time_hist.size() == m_sorted_delta_times.size());


            ImGui::Text("framerate\t%.1f FPS\n", 1.f/delta);
            if(ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(16.f);

                auto iterator = m_sorted_delta_times.begin();
                size_t index = 0;

                float lowest = 1.f / *iterator; // lowest framerate recorded
                for(; index < m_sorted_delta_times.size()/1000; index++) iterator++;
                float low_0_1_percent = 1.f / *iterator; // low 0.1%
                for(; index < m_sorted_delta_times.size()/100; index++) iterator++;
                float low_1_percent = 1.f / *iterator; // low 1%
                // median fps is not calculated since it is by far the most expensive metric, and also the least useful
                // for(; index < m_sorted_delta_times.size()/2; index++) iterator++;
                // float median = 1.f / *iterator; // median fps
                float average = 1.f/ (m_delta_time_total / m_delta_time_hist.size()); // average fps

                ImGui::Text(
                    "average\t%.1f FPS\n"
                    "low 1%%\t%.1f FPS\n"
                    "low 0.1%%\t%.1f FPS\n"
                    "lowest\t%.1f FPS",
                    average, low_1_percent, low_0_1_percent, lowest
                );

                ImGui::Text("hist size: %lu", m_delta_time_hist.size());

                ImGui::Unindent(16.f);
            }

            if(ImGui::Button("Collect Garbage"))
                get_rm().collect_garbage();

            ImGui::End();
        }

        if(m_is_first_frame) {
            ImGui::SetWindowFocus(nullptr);
            m_is_first_frame = false;
        }
    }
}
