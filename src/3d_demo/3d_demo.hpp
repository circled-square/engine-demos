#ifndef _3D_DEMO_HPP
#define _3D_DEMO_HPP

#include <engine/scene.hpp>
#include <memory>
#include <forward_list>

namespace engine_demos {
    engine::scene make_3d_demo(std::shared_ptr<std::forward_list<const char*>> scene_names, const char* scene_name);
};

#endif //_3D_DEMO_HPP
