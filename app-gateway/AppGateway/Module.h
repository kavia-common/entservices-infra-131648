#pragma once

// PUBLIC_INTERFACE
/**
 * Module header for AppGateway plugin.
 * Defines constants, versioning, and includes shared headers used across this plugin.
 */
#include <core/Enumerate.h>
#include <core/JSON.h>
#include <plugins/Module.h>

#ifndef APPGATEWAY_VERSION_MAJOR
#define APPGATEWAY_VERSION_MAJOR 1
#endif

#ifndef APPGATEWAY_VERSION_MINOR
#define APPGATEWAY_VERSION_MINOR 0
#endif

#ifndef APPGATEWAY_VERSION_PATCH
#define APPGATEWAY_VERSION_PATCH 0
#endif

#define APPGATEWAY_PLUGIN_NAMESPACE WPEFramework
#define APPGATEWAY_PLUGIN_NAME "AppGateway"

namespace WPEFramework {
namespace Plugin {
    static constexpr uint8_t Major() { return APPGATEWAY_VERSION_MAJOR; }
    static constexpr uint8_t Minor() { return APPGATEWAY_VERSION_MINOR; }
    static constexpr uint8_t Patch() { return APPGATEWAY_VERSION_PATCH; }
} // namespace Plugin
} // namespace WPEFramework
