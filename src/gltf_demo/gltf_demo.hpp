#ifndef GLTF_DEMO_HPP
#define GLTF_DEMO_HPP

#include <engine/scene.hpp>
#include <memory>
#include <forward_list>

namespace engine_demos {
    engine::scene make_gltf_demo();

    std::unique_ptr<engine::node> make_gltf_demo_node_tree();
}

#endif //GLTF_DEMO_HPP
