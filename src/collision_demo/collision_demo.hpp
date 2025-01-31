#ifndef COLLISION_DEMO_HPP
#define COLLISION_DEMO_HPP

#include <engine/scene/scene.hpp>
#include <memory>
#include <forward_list>

namespace engine_demos {
    engine::scene make_collision_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name);
};

#endif //COLLISION_DEMO_HPP
