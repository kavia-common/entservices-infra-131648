#pragma once

// PUBLIC_INTERFACE
/**
 * Module header for LaunchDelegate plugin.
 * Defines constants, versioning, and includes shared headers used across this plugin.
 */
#include <core/Enumerate.h>
#include <core/JSON.h>
#include <plugins/Module.h>

#ifndef LAUNCHDELEGATE_VERSION_MAJOR
#define LAUNCHDELEGATE_VERSION_MAJOR 1
#endif

#ifndef LAUNCHDELEGATE_VERSION_MINOR
#define LAUNCHDELEGATE_VERSION_MINOR 0
#endif

#ifndef LAUNCHDELEGATE_VERSION_PATCH
#define LAUNCHDELEGATE_VERSION_PATCH 0
#endif

#define LAUNCHDELEGATE_PLUGIN_NAMESPACE WPEFramework
#define LAUNCHDELEGATE_PLUGIN_NAME "LaunchDelegate"

namespace WPEFramework {
namespace Plugin {
    static constexpr uint8_t Major() { return LAUNCHDELEGATE_VERSION_MAJOR; }
    static constexpr uint8_t Minor() { return LAUNCHDELEGATE_VERSION_MINOR; }
    static constexpr uint8_t Patch() { return LAUNCHDELEGATE_VERSION_PATCH; }
} // namespace Plugin
} // namespace WPEFramework
