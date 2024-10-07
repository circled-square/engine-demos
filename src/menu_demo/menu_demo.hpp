#ifndef MENU_DEMO_HPP
#define MENU_DEMO_HPP

#include <engine/scene/scene.hpp>
#include <GAL/renderer/renderer.hpp>

#include <memory>
#include <set>
#include <forward_list>
#include <queue>

#include <glad/glad.h>
#include <imgui.h>

namespace scene_demos {
    class menu_demo : public engine::scene {
        std::shared_ptr<std::forward_list<const char*>> m_scene_names;

        std::queue<float> m_delta_time_hist;
        std::multiset<float, std::greater<float>> m_sorted_delta_times;
        float m_delta_time_total = 0.f; //always contains the sum of all the elements of m_delta_time_hist
        bool m_is_first_frame = true;
    public:
        menu_demo(std::shared_ptr<std::forward_list<const char*>> scene_names);
        menu_demo(menu_demo&& o);
        menu_demo() = delete;
        menu_demo(menu_demo&) = delete;
        menu_demo(const menu_demo&) = delete;
        virtual ~menu_demo() = default;

        virtual void render_ui(float frame_time) override;

        virtual const char* get_name() const override = 0;
    };
};

#endif // MENU_DEMO_HPP
