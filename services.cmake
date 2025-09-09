# If not stated otherwise in this file or this component's Licenses.txt file the
# following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# Feature toggles and plugin options
# Optional dependencies must be discovered QUIETly and features should be
# gracefully disabled if a dependency is not found.
#

# Sound player feature toggle (no external dependency declared here)
add_definitions(-DUSE_SOUND_PLAYER)

# IARM / IARMBus feature toggles
# These require the IARMBus package; if not available, they are set OFF and not defined.
option(USE_IARM "Enable IARM integration (requires IARMBus)" ON)
option(USE_IARM_BUS "Enable IARM Bus integration (requires IARMBus)" ON)

# Only attempt to resolve IARMBus when a related feature has been requested
if(USE_IARM OR USE_IARM_BUS)
    # Optional dependency: search quietly
    find_package(IARMBus QUIET)
    if(IARMBus_FOUND)
        if(USE_IARM)
            add_definitions(-DUSE_IARM)
        endif()
        if(USE_IARM_BUS)
            add_definitions(-DUSE_IARM_BUS)
        endif()
        # Legacy/aggregate define used by some components
        add_definitions(-DUSE_IARMBUS)
        message(STATUS "IARMBus found: enabling IARM features (USE_IARM=${USE_IARM}, USE_IARM_BUS=${USE_IARM_BUS}).")
    else()
        if(USE_IARM)
            set(USE_IARM OFF CACHE BOOL "Enable IARM integration (requires IARMBus)" FORCE)
        endif()
        if(USE_IARM_BUS)
            set(USE_IARM_BUS OFF CACHE BOOL "Enable IARM Bus integration (requires IARMBus)" FORCE)
        endif()
        message(STATUS "IARMBus not found: disabling IARM features (USE_IARM, USE_IARM_BUS).")
    endif()
endif()

# Other compile-time feature defines (no explicit external deps declared here)
add_definitions(-DUSE_TR_69)
add_definitions(-DHAS_API_SYSTEM)
add_definitions(-DHAS_API_POWERSTATE)
add_definitions(-DRDK_LOG_MILESTONE)
add_definitions(-DUSE_DS)

# Plugin toggles (some may be auto-disabled at top-level if WPEFramework is missing)
option(PLUGIN_WAREHOUSE "Enable Warehouse plugin" ON)
option(HAS_API_HDMI_INPUT "Enable HDMI Input API" ON)
option(PLUGIN_COPILOT "Enable CoPilot plugin" OFF)
option(PLUGIN_FRAMERATE "Enable Framerate plugin" ON)
option(PLUGIN_STORAGE_MANAGER "Enable StorageManager plugin" ON)
option(PLUGIN_DEVICEDIAGNOSTICS "Enable DeviceDiagnostics plugin" ON)
option(PLUGIN_SOUNDPLAYER "Enable SoundPlayer plugin" OFF)
option(PLUGIN_TELEMETRY "Enable Telemetry plugin" ON)
option(PLUGIN_LEDCONTROL "Enable LEDControl plugin" ON)
option(PLUGIN_CONTINUEWATCHING "Enable ContinueWatching plugin" ON)

# ContinueWatching plugin specific flags
if(PLUGIN_CONTINUEWATCHING)
    # Define plugin macro when enabled
    add_definitions(-DPLUGIN_CONTINUEWATCHING)

    # Optionally disable SECAPI for ContinueWatching if requested by the integrator
    if(CONTINUEWATCHING_DISABLE_SECAPI)
        add_definitions(-DDISABLE_SECAPI)
    endif()
endif()

# Optional/conditional compile defines driven by integrator toggles
if(DISABLE_GEOGRAPHY_TIMEZONE)
    add_definitions(-DDISABLE_GEOGRAPHY_TIMEZONE)
endif()

if(BUILD_ENABLE_SYSTIMEMGR_SUPPORT)
    message(STATUS "Building with SYSTIMEMGR_SUPPORT enabled")
    add_definitions(-DENABLE_SYSTIMEMGR_SUPPORT)
endif()

if(BUILD_DBUS)
    message(STATUS "Building with DBUS transport enabled")
    add_definitions(-DBUILD_DBUS)
    option(BUILD_DBUS "Enable DBUS build integration" ON)
    add_definitions(-DIARM_USE_DBUS)
    option(IARM_USE_DBUS "Enable IARM with DBUS" ON)
endif()

if(BUILD_ENABLE_THERMAL_PROTECTION)
    add_definitions(-DBUILD_ENABLE_THERMAL_PROTECTION)
    add_definitions(-DENABLE_THERMAL_PROTECTION)
endif()

if(BUILD_ENABLE_DEVICE_MANUFACTURER_INFO)
    message(STATUS "Building with device manufacturer info")
    add_definitions(-DENABLE_DEVICE_MANUFACTURER_INFO)
endif()

if(SUPPRESS_MAINTENANCE)
    message(STATUS "Enable SUPPRESS_MAINTENANCE")
    add_definitions(-DSUPPRESS_MAINTENANCE)
endif()

if(BUILD_ENABLE_CLOCK)
    message(STATUS "Building with clock support")
    add_definitions(-DCLOCK_BRIGHTNESS_ENABLED)
endif()

if(BUILD_ENABLE_EXTENDED_ALL_SEGMENTS_TEXT_PATTERN)
    add_definitions(-DUSE_EXTENDED_ALL_SEGMENTS_TEXT_PATTERN)
endif()

if(ENABLE_SYSTEM_GET_STORE_DEMO_LINK)
    message(STATUS "Building with System Service getStoreDemoLink")
    add_definitions(-DENABLE_SYSTEM_GET_STORE_DEMO_LINK)
endif()

if(BUILD_ENABLE_TELEMETRY_LOGGING)
    message(STATUS "Building with telemetry logging")
    add_definitions(-DENABLE_TELEMETRY_LOGGING)
endif()

if(BUILD_ENABLE_LINK_LOCALTIME)
    message(STATUS "Building with link localtime")
    add_definitions(-DENABLE_LINK_LOCALTIME)
endif()

add_definitions(-DENABLE_DEEP_SLEEP)

# Only on specific platforms
if(BUILD_ENABLE_APP_CONTROL_AUDIOPORT_INIT)
    add_definitions(-DAPP_CONTROL_AUDIOPORT_INIT)
endif()

if(NET_DISABLE_NETSRVMGR_CHECK)
    add_definitions(-DNET_DISABLE_NETSRVMGR_CHECK)
endif()

if(ENABLE_WHOAMI)
    message(STATUS "Enable WHOAMI")
    add_definitions(-DENABLE_WHOAMI=ON)
endif()

if(ENABLE_RFC_MANAGER)
    message(STATUS "Using binary for RFC Maintenance task")
    add_definitions(-DENABLE_RFC_MANAGER=ON)
endif()

if(DISABLE_DCM_TASK)
    message(STATUS "Disabling DCM Maintenance task")
    add_definitions(-DDISABLE_DCM_TASK=ON)
endif()

if(BUILD_ENABLE_ERM)
    add_definitions(-DENABLE_ERM)
endif()
