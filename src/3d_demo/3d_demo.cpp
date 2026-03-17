#include "3d_demo.hpp"

#include "engine/scene/application_channel.hpp"
#include "engine/scene/renderer/mesh/material.hpp"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <engine/resources_manager.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/utils/format_glm.hpp>
#include <engine/utils/constants.hpp>
#include <variant>
#include <random>

/* This demo demonstrates a few things:
 * - 3d rendering
 * - use of custom texture on a raw-data 3d mesh (the cube)
 * - use of the scene tree
 * - use of script to programmatically generate a scene (container_script)
 * - use of script to manipulate objects' transforms (camera and centre cube)
 * - custom script state for the camera (the time, which would be unnecessary since it can be retrieved through the application_channel_t)
 * - custom uniform passed to the (centre) cube's material (to offset its texture), without having any effect on other instances (other cubes) of the same material (done through its own script)
 */

namespace engine_demos {
    using namespace engine;
    using glm::mat4; using glm::uvec3; using glm::vec3;
    scene make_3d_demo() {
        node root("");

        rc<const dylib::library> scripts_plugin = get_rm().load<dylib::library>("plugins/scripts/lib/scripts");

        root.add_child(node("menu", std::monostate(), glm::mat4(1), stateless_script::from(scripts_plugin, "imgui_dbgmenu")));
        root.add_child(node("cubes_container", std::monostate(), mat4(1), stateless_script::from(scripts_plugin, "3d_demo.cube_spawner")));
        root.add_child(node("camera", camera(), glm::translate(mat4(1), vec3(0,0,4)), stateless_script::from(scripts_plugin, "3d_demo.cam")));

        return scene("3d demo", std::move(root));
    }
} // namespace engine_demos
