###
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2023 RDK Management
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
###

# Minimum required must be before project()
cmake_minimum_required(VERSION 3.10)

# Declare project so CMAKE_PROJECT_NAME and related variables exist
project(EntServicesInfra LANGUAGES C CXX)

# All packages that did not deliver a CMake Find script (and some deprecated scripts that need to be removed)
# are located in the cmake directory. Include it in the search.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" "${CMAKE_SOURCE_DIR}/cmake/modules" "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Opt-in static analysis tools
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis during build (requires clang-tidy available)" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck static analysis during build (requires cppcheck available)" OFF)

# If requested, wire up static analysis toolchains (sets CMAKE_CXX_CLANG_TIDY / CMAKE_CXX_CPPCHECK)
include(StaticAnalysis)

# Options to control optional components/deps
option(COMCAST_CONFIG "Comcast services configuration" ON)
option(PLUGIN_OCICONTAINER "Include OCIContainer plugin" OFF)
option(DISABLE_SECURITY_TOKEN "Disable security token" OFF)

# Optionally enable building L1/L2 tests (guard missing external deps)
option(RDK_SERVICE_L2_TEST "Enable L2 tests" OFF)
option(RDK_SERVICES_L1_TEST "Enable L1 tests" OFF)

# Try to find WPEFramework but do not fail hard so local dev can configure without SDK
find_package(WPEFramework QUIET)
if(NOT WPEFramework_FOUND)
    message(WARNING "WPEFramework package not found. Set WPEFramework_DIR or CMAKE_PREFIX_PATH to the SDK. Plugins depending on it may fail to build.")
endif()

# Safe lowercase conversion for storage directory; if NAMESPACE is undefined, default to project name
if(NOT DEFINED NAMESPACE)
    set(NAMESPACE "${CMAKE_PROJECT_NAME}")
endif()
string(TOLOWER "${NAMESPACE}" STORAGE_DIRECTORY)

# for writing pc and config files - include if present
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/CmakeHelperFunctions.cmake")
    include(CmakeHelperFunctions)
else()
    message(STATUS "Optional helper CmakeHelperFunctions.cmake not found; proceeding without it.")
endif()

# Load services configuration if requested
if(COMCAST_CONFIG)
    if(EXISTS "${CMAKE_SOURCE_DIR}/services.cmake")
        include(services.cmake)
    else()
        message(WARNING "services.cmake not found; default plugin options may be unset.")
    endif()

    # If WPEFramework is not found, gracefully disable plugins that require it.
    # This prevents configuration failures when SDK paths are not provided.
    if(NOT WPEFramework_FOUND)
        set(_disabled_plugins "")
        foreach(_p IN ITEMS
            PLUGIN_USERSETTINGS
            PLUGIN_USB_MASS_STORAGE
            PLUGIN_OCICONTAINER
            PLUGIN_PERSISTENTSTORE
            PLUGIN_USBDEVICE
            PLUGIN_TELEMETRY
            PLUGIN_MONITOR
            PLUGIN_RDKSHELL
            PLUGIN_RUSTADAPTER
            PLUGIN_MESSAGECONTROL
            PLUGIN_RESOURCEMANAGER
            PLUGIN_ANALYTICS
            PLUGIN_RDK_WINDOW_MANAGER
            PLUGIN_CLOUDSTORE
            PLUGIN_SHAREDSTORAGE
            PLUGIN_WEBBRIDGE
            PLUGIN_LIFECYCLE_MANAGER
            PLUGIN_APPMANAGER
            PLUGIN_RUNTIME_MANAGER
            PLUGIN_STORAGE_MANAGER
            PLUGIN_PACKAGE_MANAGER
            PLUGIN_NATIVEJS
            PLUGIN_APPGATEWAY
            PLUGIN_APP2APP_PROVIDER
            PLUGIN_LAUNCHDELEGATE
        )
            if(DEFINED ${_p} AND ${_p})
                # Mark OFF in cache so subsequent runs retain the setting
                set(${_p} OFF CACHE BOOL "Auto-disabled: requires WPEFramework which was not found" FORCE)
                list(APPEND _disabled_plugins ${_p})
            endif()
        endforeach()

        if(_disabled_plugins)
            list(JOIN _disabled_plugins ", " _disabled_plugins_str)
            message(STATUS "WPEFramework not found. Disabled plugins: ${_disabled_plugins_str}")
        else()
            message(STATUS "WPEFramework not found. No plugins were enabled, nothing to disable.")
        endif()
    endif()
endif()

# Tests
if(RDK_SERVICE_L2_TEST)
    if(EXISTS "${CMAKE_SOURCE_DIR}/Tests/L2Tests/CMakeLists.txt")
        add_subdirectory(Tests/L2Tests)
    else()
        message(STATUS "L2 tests directory not found; skipping.")
    endif()
endif()

if(RDK_SERVICES_L1_TEST)
    if(EXISTS "${CMAKE_SOURCE_DIR}/Tests/L1Tests/CMakeLists.txt")
        # Allow the L1Tests CMakeLists to skip optional deps (Plugins) when absent
        set(ALLOW_OPTIONAL_TEST_DEPS ON CACHE BOOL "Allow skipping optional test dependencies if unavailable")
        add_subdirectory(Tests/L1Tests)
    else()
        message(STATUS "L1 tests directory not found; skipping.")
    endif()
endif()

if(DISABLE_SECURITY_TOKEN)
    add_definitions(-DDISABLE_SECURITY_TOKEN)
endif()

# Conditionally add plugin subdirectories based on options that may be set via services.cmake or cache
if(PLUGIN_USERSETTINGS)
    add_subdirectory(UserSettings)
endif()

if(PLUGIN_USB_MASS_STORAGE)
    add_subdirectory(USBMassStorage)
endif()

if(PLUGIN_OCICONTAINER)
    add_subdirectory(OCIContainer)
endif()

if(PLUGIN_PERSISTENTSTORE)
    add_subdirectory(PersistentStore)
endif()

if(PLUGIN_USBDEVICE)
    add_subdirectory(USBDevice)
endif()

if(PLUGIN_TELEMETRY)
    add_subdirectory(Telemetry)
endif()

if(PLUGIN_MONITOR)
    add_subdirectory(Monitor)
endif()

if(PLUGIN_RDKSHELL)
    add_subdirectory(RDKShell)
endif()

if(PLUGIN_RUSTADAPTER)
    add_subdirectory(RustAdapter)
endif()

if(PLUGIN_MESSAGECONTROL)
    add_subdirectory(MessageControl)
endif()

if(PLUGIN_RESOURCEMANAGER)
    add_subdirectory(ResourceManager)
endif()

if(PLUGIN_ANALYTICS)
    add_subdirectory(Analytics)
endif()

if(PLUGIN_RDK_WINDOW_MANAGER)
    add_subdirectory(RDKWindowManager)
endif()

if(PLUGIN_CLOUDSTORE)
    add_subdirectory(CloudStore)
endif()

if(PLUGIN_SHAREDSTORAGE)
    add_subdirectory(SharedStorage)
endif()

if(PLUGIN_WEBBRIDGE)
    add_subdirectory(WebBridge)
endif()

if(PLUGIN_LIFECYCLE_MANAGER)
    add_subdirectory(LifecycleManager)
endif()

if(PLUGIN_APPMANAGER)
    add_subdirectory(AppManager)
endif()

if(PLUGIN_RUNTIME_MANAGER)
    add_subdirectory(RuntimeManager)
endif()

if(PLUGIN_STORAGE_MANAGER)
    add_subdirectory(StorageManager)
endif()

if(PLUGIN_PACKAGE_MANAGER)
    add_subdirectory(PackageManager)
endif()

if(PLUGIN_NATIVEJS)
    add_subdirectory(NativeJS)
endif()

# New AppGateway plugin integration
option(PLUGIN_APPGATEWAY "Include AppGateway plugin" ON)
if(PLUGIN_APPGATEWAY)
    if(EXISTS "${CMAKE_SOURCE_DIR}/app-gateway/AppGateway/CMakeLists.txt")
        add_subdirectory(app-gateway/AppGateway)
    else()
        message(STATUS "AppGateway plugin directory not found; skipping.")
    endif()
endif()

# New App2AppProvider plugin integration
option(PLUGIN_APP2APP_PROVIDER "Include App2AppProvider plugin" ON)
if(PLUGIN_APP2APP_PROVIDER)
    if(EXISTS "${CMAKE_SOURCE_DIR}/app-gateway/App2AppProvider/CMakeLists.txt")
        add_subdirectory(app-gateway/App2AppProvider)
    else()
        message(STATUS "App2AppProvider plugin directory not found; skipping.")
    endif()
endif()

# New LaunchDelegate plugin integration
option(PLUGIN_LAUNCHDELEGATE "Include LaunchDelegate plugin" ON)
if(PLUGIN_LAUNCHDELEGATE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/app-gateway/LaunchDelegate/CMakeLists.txt")
        add_subdirectory(app-gateway/LaunchDelegate)
    else()
        message(STATUS "LaunchDelegate plugin directory not found; skipping.")
    endif()
endif()

# Package config for ipkg if requested by environment
if(WPEFRAMEWORK_CREATE_IPKG_TARGETS)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_GROUPING IGNORE)

    set(CPACK_DEBIAN_PACKAGE_NAME "${WPEFRAMEWORK_PLUGINS_OPKG_NAME}")
    set(CPACK_DEBIAN_PACKAGE_VERSION "${WPEFRAMEWORK_PLUGINS_OPKG_VERSION}")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${WPEFRAMEWORK_PLUGINS_OPKG_ARCHITECTURE}")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${WPEFRAMEWORK_PLUGINS_OPKG_MAINTAINER}")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${WPEFRAMEWORK_PLUGINS_OPKG_DESCRIPTION}")
    set(CPACK_PACKAGE_FILE_NAME "${WPEFRAMEWORK_PLUGINS_OPKG_FILE_NAME}")

    include(CPack)
endif()
