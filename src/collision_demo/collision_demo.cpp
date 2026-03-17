#include "collision_demo.hpp"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <engine/resources_manager.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/application/window.hpp>
#include <engine/utils/constants.hpp>

// This demo illustrates collisions and different collision behaviours

namespace engine_demos {
    using namespace engine;
    using glm::mat4; using glm::uvec3; using glm::vec3;

    scene make_collision_demo() {
        rc<const dylib::library> scripts_plugin = get_rm().load<dylib::library>("plugins/scripts/lib/scripts");

        node root("");
        {
            root.add_child(node("menu", std::monostate(), glm::mat4(1), stateless_script::from(scripts_plugin, "imgui_dbgmenu")));

            root.add_child(node("camera", camera(), glm::inverse(glm::lookAt(vec3(3,6,6), vec3(0), vec3(0,1,0)))));

            node cone(get_rm().load<nodetree_blueprint>("cone_with_collision.glb"), "cone");
            {
                cone->set_transform(glm::translate(cone->transform(), vec3(2, 0, 0)));
            }
            root.add_child(std::move(cone));

            node stillcube("stillcube", std::monostate(), mat4(1), stateless_script::from(scripts_plugin, "collision_demo.stillcube"));
            root.add_child(std::move(stillcube));

            node kbdcube("kbdcube", std::monostate(), mat4(1), stateless_script::from(scripts_plugin, "collision_demo.kbdcube"));
            root.add_child(std::move(kbdcube));
        }


        return scene("collision demo", std::move(root));
    }
} // namespace engine_demos
