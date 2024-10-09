#ifndef POSTFX_DEMO_HPP
#define POSTFX_DEMO_HPP

#include <memory>
#include <forward_list>
#include <engine/scene/scene.hpp>

namespace scene_demos {
    engine::scene make_postfx_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name);
} // scene_demos

#endif //POSTFX_DEMO_HPP
