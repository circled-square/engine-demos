#include "gltf_demo.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager.hpp>

namespace scene_demos {
    using namespace glm;

    [[maybe_unused]]
    constexpr vec3 x_axis = vec3(1,0,0), y_axis = vec3(0,1,0), z_axis = vec3(0,0,1);
    [[maybe_unused]]
    constexpr float pi = glm::pi<f32>();


    gltf_demo::gltf_demo(std::shared_ptr<std::forward_list<const char*>> scene_names)
        : menu_demo(std::move(scene_names)) {
        get_root().add_child(engine::node(engine::get_rm().get_nodetree_from_gltf("resources/castlebl.glb"), "castle"));
        get_root().add_child(engine::node("camera", engine::camera(), glm::translate(glm::mat4(1), vec3(0,50,250))));
    }

    void gltf_demo::update(float delta) {
        mat4& model = get_node("/castle").transform();
        model = rotate(model, delta * pi / 16, y_axis);
    }


    const char* gltf_demo::get_name() const {
        return "gltf demo";
    }
} // scene_demos
