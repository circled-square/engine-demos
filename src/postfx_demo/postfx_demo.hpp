#ifndef POSTFX_DEMO_HPP
#define POSTFX_DEMO_HPP

#include "../menu_demo/menu_demo.hpp"

namespace scene_demos {
    class postfx_demo : public menu_demo {
    public:
        postfx_demo(std::shared_ptr<std::forward_list<const char*>> scene_names);
        void update(float delta) final;
        const char* get_name() const final;
    };
} // scene_demos

#endif //POSTFX_DEMO_HPP
