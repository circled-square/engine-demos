#ifndef FREECAM_DEMO_HPP
#define FREECAM_DEMO_HPP

#include "../menu_demo/menu_demo.hpp"

namespace scene_demos {
    class freecam_demo : public menu_demo {
        float m_current_y_rotation;
        struct keys_t {
            bool up, down, left, right, fwd, bwd, go_faster;
        };
        keys_t m_keys = {false, false, false, false, false, false};
    public:
        freecam_demo(std::shared_ptr<std::forward_list<const char*>> scene_names);
        freecam_demo(freecam_demo&& o);
        void update(float delta) final;

        const char* get_name() const final;

        virtual void on_key_press(int key, int scancode, int action, int mods) override;
        virtual void on_mouse_move(glm::vec2 position, glm::vec2 movement) override;
    };
} // scene_demos

#endif //FREECAM_DEMO_HPP
