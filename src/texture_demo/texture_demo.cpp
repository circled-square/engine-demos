#include "texture_demo.hpp"

#include <glm/glm.hpp>
#include <engine/resources_manager.hpp>
#include <array>

/* This is a fairly simple, lower level demo meant to show direct use of the GAL (Graphics Abstraction Layer) library.
 * All rendering is done on 2 imgui windows; the first simply shows a texture, the second shows the result of
 * rendering to a framebuffer object, using a custom shader. The use of anything under engine:: namespace is avoided
 * wherever possible.
 */

namespace engine_demos {

    engine::scene make_texture_demo() {
        engine::node root("");

        engine::rc<const dylib::library> scripts_plugin = engine::get_rm().load<dylib::library>("plugins/scripts/lib/scripts");

        root.add_child(engine::node("menu", std::monostate(), glm::mat4(1), engine::stateless_script::from(scripts_plugin, "imgui_dbgmenu")));
        root.add_child(engine::node("imgui-tex-node", std::monostate(), glm::mat4(1), engine::stateless_script::from(scripts_plugin, "texture_demo.imgui_windows")));

        return engine::scene("texture demo", std::move(root));
    }
} // namespace engine_demos
