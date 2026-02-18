#ifndef SCRIPTS_HPP
#define SCRIPTS_HPP

#include <engine/resources_manager.hpp>
#include <engine/scene/node.hpp>
#include <engine/utils/api_macro.hpp>

// #define IMPORT __attribute((visibility("default")))

ENGINE_EXPORT extern const std::size_t exported_plugins_size;
ENGINE_EXPORT extern const std::pair<const char*, engine::script_vtable> exported_plugins[];


#endif // SCRIPTS_HPP
