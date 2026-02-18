#include "scripts.hpp"
#include <slogga/log.hpp>
#include <engine/utils/constants.hpp>
#include <engine/scene/node.hpp>
#include <engine/scene/application_channel.hpp>

using namespace engine::constants;

const std::pair<const char*, engine::script_vtable> exported_plugins[] {
    std::pair("rotate", engine::script_vtable {
        .construct = [](const engine::node&) -> std::any {
            slogga::stdout_log("hello from plugin");
            return std::monostate();
        },
        .process = [](const engine::node& n, std::any&, engine::application_channel_t& c) {
            n->set_transform(glm::rotate(n->transform(), c.from_app().delta * pi / 16, y_axis));
        },
    }),
    std::pair("rotate_and_set_clear_color", engine::script_vtable {
        .process = [](const engine::node& n, std::any&, engine::application_channel_t& c) {
            c.to_app().clear_color = glm::vec4(0.2, 0.2, 0.2, 1.0);
            n->set_transform(glm::rotate(n->transform(), c.from_app().delta * pi / 16, y_axis));
        },
    }),
};

const std::size_t exported_plugins_size = sizeof(exported_plugins) / sizeof(exported_plugins[0]);
