#ifndef GLTF_DEMO_HPP
#define GLTF_DEMO_HPP

#include "../menu_demo/menu_demo.hpp"

namespace scene_demos {
    class gltf_demo : public menu_demo {
    public:
        gltf_demo(std::shared_ptr<std::forward_list<const char*>> scene_names);
        void update(float delta) final;
        const char* get_name() const final;
    };

} // scene_demos

#endif //GLTF_DEMO_HPP
