#include "freecam_demo.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <engine/gltf_loader.hpp>
#include <engine/resources_manager.hpp>
#include <engine/window/window.hpp>
#include <cstring>

namespace scene_demos {
    using namespace glm;
    using namespace engine;

    freecam_demo::freecam_demo(std::shared_ptr<std::forward_list<const char*>> scene_names)
        : menu_demo(std::move(scene_names)), m_current_y_rotation(0.f)
    {
        application_channel().to_app.wants_mouse_cursor_captured = true;

        node halftone_viewport("halftone_vp", engine::viewport(
            get_rm().new_from<shader>(shader::from_file("src/shaders/halftone_postfx.glsl")),
            glm::vec2(1./2.)
        ));

        node transparent_viewport("transparent_vp", engine::viewport(
            get_rm().new_from<shader>(shader::from_file("src/shaders/transparent_postfx.glsl")),
            glm::vec2(1./3.)
        ));
        node camera_node("camera", engine::camera(), glm::translate(glm::mat4(1), glm::vec3(0, 2, 0)));

        halftone_viewport.add_child(std::move(camera_node));
        halftone_viewport.add_child(node(get_rm().get_nodetree_from_gltf("resources/castlebl.glb")));
        transparent_viewport.add_child(std::move(halftone_viewport));
        get_root().add_child(std::move(transparent_viewport));

        freecam_demo::on_mouse_move({0,0}, {0,0}); // TODO: what is this?
    }

    freecam_demo::freecam_demo(freecam_demo &&o)
        : menu_demo(std::move(o)),
          m_current_y_rotation(o.m_current_y_rotation),
          m_keys() {}

    void freecam_demo::update(float delta) {
        constexpr float move_speed = 30.0;

        auto& cam_node = get_node("/transparent_vp/halftone_vp/camera");

        glm::vec3 movement = glm::vec3(
            (m_keys.right?1:0) - (m_keys.left?1:0),
            (m_keys.up?1:0) - (m_keys.down?1:0),
            (m_keys.bwd?1:0) - (m_keys.fwd?1:0)
        );
        movement *= move_speed * delta * (m_keys.go_faster ? 3.0 : 1.0);

        cam_node.transform() = glm::translate(cam_node.transform(), movement);
    }

    const char* freecam_demo::get_name() const { return "freecam demo"; }

    void freecam_demo::on_key_press(int key, int scancode, int action, int mods) {
        using namespace engine::window;
        if (key == key_codes::SPACE) {
            m_keys.up = action != key_action_codes::RELEASE;
        } else if (key == key_codes::LEFT_CONTROL) {
            m_keys.down = action != key_action_codes::RELEASE;
        } else if (key == key_codes::A) {
            m_keys.left = action != key_action_codes::RELEASE;
        } else if (key == key_codes::D) {
            m_keys.right = action != key_action_codes::RELEASE;
        } else if (key == key_codes::W) {
            m_keys.fwd = action != key_action_codes::RELEASE;
        } else if (key == key_codes::S) {
            m_keys.bwd = action != key_action_codes::RELEASE;
        } else if (key == key_codes::LEFT_SHIFT) {
            m_keys.go_faster = action != key_action_codes::RELEASE;
        } else if (key == key_codes::C && action == key_action_codes::RELEASE) {
            //toggle mouse capture
            application_channel().to_app.wants_mouse_cursor_captured =
                !application_channel().from_app.mouse_cursor_is_captured;
        }
    }

    static glm::mat4 to_rotation_mat(glm::mat4 mat) {
        mat[3] = vec4(0);
        return mat;
    }
    static glm::mat4 clear_first_3_rows(glm::mat4 mat) {
        mat[0] = mat[1] = mat[2] = vec4(0);
        return mat;
    }

    void freecam_demo::on_mouse_move(glm::vec2 position, glm::vec2 movement) {
        constexpr float movement_multiplier = 0.002;
        movement *= movement_multiplier;

        float old_rotation = m_current_y_rotation;
        m_current_y_rotation += movement.y;
        m_current_y_rotation = std::clamp(m_current_y_rotation, -glm::pi<float>()/2.f * 0.99f, glm::pi<float>()/2.f * 0.99f);
        movement.y = m_current_y_rotation - old_rotation;

        auto& cam= get_node("/transparent_vp/halftone_vp/camera").transform();


        cam = glm::rotate(glm::mat4(1), -movement.x, glm::vec3(0,1,0)) * to_rotation_mat(cam) + clear_first_3_rows(cam);
        cam = glm::rotate(cam, -movement.y, glm::vec3(1,0,0));
    }
} // scene_demos
