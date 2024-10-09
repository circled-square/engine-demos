#ifndef FREECAM_DEMO_HPP
#define FREECAM_DEMO_HPP

#include "../menu_demo/menu_demo.hpp"

namespace scene_demos {
    engine::scene make_freecam_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name);
}

#endif //FREECAM_DEMO_HPP
