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

        auto root = node::make("");
        {
            node::add_child(root, node::make("menu", stateless_script::from(scripts_plugin, "imgui_dbgmenu")));
            node::add_child(root, node::make("camera", camera(), glm::inverse(glm::lookAt(vec3(3,6,6), vec3(0), vec3(0,1,0)))));

            auto cone = node::deep_copy(get_rm().load<nodetree_blueprint>("cone_with_collision.glb"), "cone");
            cone->set_transform(glm::translate(cone->transform(), vec3(2, 0, 0)));
            node::add_child(root, std::move(cone));

            auto stillcube = node::make("stillcube", stateless_script::from(scripts_plugin, "collision_demo.stillcube"));
            node::add_child(root, std::move(stillcube));

            auto kbdcube = node::make("kbdcube", stateless_script::from(scripts_plugin, "collision_demo.kbdcube"));
            node::add_child(root, std::move(kbdcube));
        }


        return scene("collision demo", std::move(root));
    }
} // namespace engine_demos
