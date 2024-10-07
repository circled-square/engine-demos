#include "postfx_demo.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <engine/gltf_loader.hpp>
#include <engine/resources_manager.hpp>
#include <GAL/framebuffer.hpp>
#include <cstring>

namespace scene_demos {
    using namespace glm;
    using namespace engine;

    postfx_demo::postfx_demo(std::shared_ptr<std::forward_list<const char*>> scene_names)
        : menu_demo(std::move(scene_names))
    {
        node halftone_vp("halftone-vp", engine::viewport(
            get_rm().new_from<shader>(shader::from_file("src/shaders/halftone_postfx.glsl")),
            glm::vec2(1./2.)
        ));

        node transparent_vp("transparent-vp", engine::viewport(
            get_rm().new_from<shader>(shader::from_file("src/shaders/transparent_postfx.glsl")),
            glm::vec2(1./3.)
        ));

        node cam("camera", camera(), glm::translate(glm::mat4(1), vec3(0,50,250)));

        halftone_vp.add_child(std::move(cam));
        halftone_vp.add_child(engine::node(engine::get_rm().get_nodetree_from_gltf("resources/castlebl.glb"), "castle"));
        transparent_vp.add_child(std::move(halftone_vp));
        get_root().add_child(std::move(transparent_vp));
    }

    void postfx_demo::update(float delta) {
        mat4& model = get_node("/transparent-vp/halftone-vp/castle").transform();
        model = rotate(model, delta * pi<float>() / 16, vec3(0,1,0));
    }

    const char* postfx_demo::get_name() const { return "postfx demo"; }
} // scene_demos
