#ifndef GLTF_DEMO_HPP
#define GLTF_DEMO_HPP

#include <engine/scene/scene.hpp>
#include <memory>
#include <forward_list>

namespace scene_demos {
    engine::scene make_gltf_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name);
} // scene_demos

#endif //GLTF_DEMO_HPP