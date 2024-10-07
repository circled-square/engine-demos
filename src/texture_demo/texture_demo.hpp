#ifndef GAL_TEXTURE_DEMO_HPP
#define GAL_TEXTURE_DEMO_HPP

#include "../menu_demo/menu_demo.hpp"

namespace scene_demos {
    class texture_demo : public menu_demo {
        gal::renderer m_renderer;
        gal::vertex_array m_vao;
        gal::shader_program m_shader;
        gal::image m_img;
        gal::texture m_tex;
    public:
        texture_demo(std::shared_ptr<std::forward_list<const char*>> scene_names);
        virtual void render_ui(float frame_time) final;

        const char* get_name() const final;
    };
}

#endif //GAL_TEST_TEXTURE_HPP
