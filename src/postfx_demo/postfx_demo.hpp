#ifndef POSTFX_DEMO_HPP
#define POSTFX_DEMO_HPP

#include <memory>
#include <forward_list>
#include <engine/scene.hpp>

namespace engine_demos {
    engine::scene make_postfx_demo(std::shared_ptr<std::forward_list<std::string>> scene_names, std::string scene_name);
}

#endif //POSTFX_DEMO_HPP
