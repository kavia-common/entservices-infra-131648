#pragma once

// PUBLIC_INTERFACE
/**
 * Module header for App2AppProvider plugin.
 * Defines constants, versioning, and includes shared headers used across this plugin.
 */
#include <core/Enumerate.h>
#include <core/JSON.h>
#include <plugins/Module.h>

#ifndef APP2APPPROVIDER_VERSION_MAJOR
#define APP2APPPROVIDER_VERSION_MAJOR 1
#endif

#ifndef APP2APPPROVIDER_VERSION_MINOR
#define APP2APPPROVIDER_VERSION_MINOR 0
#endif

#ifndef APP2APPPROVIDER_VERSION_PATCH
#define APP2APPPROVIDER_VERSION_PATCH 0
#endif

#define APP2APPPROVIDER_PLUGIN_NAMESPACE WPEFramework
#define APP2APPPROVIDER_PLUGIN_NAME "App2AppProvider"

namespace WPEFramework {
namespace Plugin {
    static constexpr uint8_t Major() { return APP2APPPROVIDER_VERSION_MAJOR; }
    static constexpr uint8_t Minor() { return APP2APPPROVIDER_VERSION_MINOR; }
    static constexpr uint8_t Patch() { return APP2APPPROVIDER_VERSION_PATCH; }
} // namespace Plugin
} // namespace WPEFramework
