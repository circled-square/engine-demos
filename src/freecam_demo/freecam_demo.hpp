#ifndef FREECAM_DEMO_HPP
#define FREECAM_DEMO_HPP

#include <engine/scene.hpp>
#include <memory>
#include <forward_list>

namespace engine_demos {
    engine::scene make_freecam_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name);
}

#endif //FREECAM_DEMO_HPP
