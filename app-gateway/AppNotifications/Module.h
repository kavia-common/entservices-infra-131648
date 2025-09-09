#pragma once

// PUBLIC_INTERFACE
/**
 * Module header for AppNotifications plugin.
 * Defines constants, versioning, and includes shared headers used across this plugin.
 */
#include <core/Enumerate.h>
#include <core/JSON.h>
#include <plugins/Module.h>

#ifndef APPNOTIFICATIONS_VERSION_MAJOR
#define APPNOTIFICATIONS_VERSION_MAJOR 1
#endif

#ifndef APPNOTIFICATIONS_VERSION_MINOR
#define APPNOTIFICATIONS_VERSION_MINOR 0
#endif

#ifndef APPNOTIFICATIONS_VERSION_PATCH
#define APPNOTIFICATIONS_VERSION_PATCH 0
#endif

#define APPNOTIFICATIONS_PLUGIN_NAMESPACE WPEFramework
#define APPNOTIFICATIONS_PLUGIN_NAME "AppNotifications"

namespace WPEFramework {
namespace Plugin {
    static constexpr uint8_t Major() { return APPNOTIFICATIONS_VERSION_MAJOR; }
    static constexpr uint8_t Minor() { return APPNOTIFICATIONS_VERSION_MINOR; }
    static constexpr uint8_t Patch() { return APPNOTIFICATIONS_VERSION_PATCH; }
} // namespace Plugin
} // namespace WPEFramework
