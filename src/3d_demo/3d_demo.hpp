#ifndef _3D_DEMO_HPP
#define _3D_DEMO_HPP

#include "../menu_demo/menu_demo.hpp"

namespace scene_demos {
    class three_dimensional_demo : public menu_demo {
    public:
        three_dimensional_demo(std::shared_ptr<std::forward_list<const char*>> scene_names);
        three_dimensional_demo(three_dimensional_demo&& o);
        void update(float delta) final;
        const char* get_name() const final;
    };

};

#endif //_3D_DEMO_HPP
