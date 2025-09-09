# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
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

# PUBLIC_INTERFACE
# Telemetry plugin CMake configuration
# This CMakeLists enables building the Telemetry plugin and its implementation.
# It gracefully handles optional external dependencies that may not be present
# in minimal build environments. When SDK packages are available (via
# CMAKE_PREFIX_PATH, or specific *_DIR variables), they are linked in; otherwise
# the build continues with reduced features.

set(PLUGIN_NAME Telemetry)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation)

set(PLUGIN_TELEMETRY_STARTUPORDER "" CACHE STRING "To configure startup order of Telemetry plugin")
set(PLUGIN_T2_PERSISTENT_FOLDER /opt/.t2reportprofiles/ CACHE STRING "Path to t2 persistent folder")
set(PLUGIN_DEFAULT_PROFILES_FILE /etc/t2profiles/default.json CACHE STRING "Path to default t2 profile")

# Core WPEFramework Plugins package (required for all plugins in this repo)
# Keep REQUIRED to fail early if the SDK is truly missing when needed.
find_package(${NAMESPACE}Plugins REQUIRED)

# Optionally enable external EntServicesInfraPlugins backend pieces if available.
option(ENABLE_TELEMETRY_ESI_BACKEND "Enable Telemetry integration with EntServicesInfraPlugins if available" ON)
set(TELEMETRY_HAVE_ESI_BACKEND FALSE)

# Allow module search path extensions for this directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Try to discover additional optional packages
find_package(RFC QUIET)
find_package(IARMBus QUIET)

# RBUS headers are probed manually since some SDKs ship headers without a CMake package
find_path(RBUS_API_HEADER NAMES rbus.h PATH_SUFFIXES rbus)
if (NOT ${RBUS_API_HEADER} STREQUAL "RBUS_API_HEADER-NOTFOUND")
    message(STATUS "Telemetry: Found rbus.h at ${RBUS_API_HEADER}")
    add_definitions(-DHAS_RBUS)
    set(RBUS_API_HEADERS ${RBUS_API_HEADER})
    find_library(RBUS_LIBRARY NAMES rbus)
endif()

set(RBUS_LIBRARIES "")
if (RBUS_LIBRARY AND NOT ${RBUS_LIBRARY} STREQUAL "RBUS_LIBRARY-NOTFOUND")
    set(RBUS_LIBRARIES ${RBUS_LIBRARY})
endif()

# Security util lib (optional)
set(SEC_UTIL_LIBRARIES "")
find_library(SEC_UTIL_LIB NAMES WPEFrameworkSecurityUtil)
if (SEC_UTIL_LIB AND NOT ${SEC_UTIL_LIB} STREQUAL "SEC_UTIL_LIB-NOTFOUND")
    set(SEC_UTIL_LIBRARIES ${SEC_UTIL_LIB})
endif()

# Attempt to find EntServicesInfraPlugins only if the backend is enabled.
# IMPORTANT: This must remain fully optional. Do NOT add REQUIRED here.
if (ENABLE_TELEMETRY_ESI_BACKEND)
    set(TELEMETRY_HAVE_ESI_BACKEND FALSE)
    # Avoid CMake attempting to locate a config package unless present in the toolchain/SDK.
    # We only probe QUIETly; if not found, we proceed without ESI backend.
    find_package(EntServicesInfraPlugins QUIET)
    if (EntServicesInfraPlugins_FOUND)
        message(STATUS "Telemetry: EntServicesInfraPlugins found; enabling ESI backend integration")
        set(TELEMETRY_HAVE_ESI_BACKEND TRUE)
    else()
        message(STATUS "Telemetry: EntServicesInfraPlugins not found; building without ESI backend")
        set(TELEMETRY_HAVE_ESI_BACKEND FALSE)
    endif()
else()
    message(STATUS "Telemetry: ESI backend explicitly disabled by option")
endif()

# Telemetry module (JSON-RPC facade)
add_library(${MODULE_NAME} SHARED
    Telemetry.cpp
    Module.cpp
)

set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

target_include_directories(${MODULE_NAME}
    PRIVATE
        ../helpers
)

target_link_libraries(${MODULE_NAME}
    PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
)

install(TARGETS ${MODULE_NAME}
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins
)

# Implementation library with optional deps
add_library(${PLUGIN_IMPLEMENTATION} SHARED
    TelemetryImplementation.cpp
    Module.cpp
)

set_target_properties(${PLUGIN_IMPLEMENTATION} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

# Unit test mocks (optional)
if (RDK_SERVICE_L2_TEST)
    find_library(TESTMOCKLIB_LIBRARIES NAMES TestMocklib)
    if (TESTMOCKLIB_LIBRARIES)
        message(STATUS "Telemetry: linking mock library ${TESTMOCKLIB_LIBRARIES}")
        target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${TESTMOCKLIB_LIBRARIES})
    else()
        message(WARNING "Telemetry: TestMocklib not found, skipping mock linkage")
    endif()
endif()

# Apply include dirs and link optional libraries when available
target_include_directories(${PLUGIN_IMPLEMENTATION}
    PRIVATE
        ${RFC_INCLUDE_DIRS}
        ${RBUS_API_HEADERS}
        ../helpers
)

target_link_libraries(${PLUGIN_IMPLEMENTATION}
    PRIVATE
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${RFC_LIBRARIES}
        ${RBUS_LIBRARIES}
        ${SEC_UTIL_LIBRARIES}
)

# If ESI backend is available, link it; otherwise build a reduced variant.
if (TELEMETRY_HAVE_ESI_BACKEND)
    target_link_libraries(${PLUGIN_IMPLEMENTATION}
        PRIVATE
            EntServicesInfraPlugins::EntServicesInfraPlugins
    )
    target_compile_definitions(${PLUGIN_IMPLEMENTATION}
        PRIVATE TELEMETRY_HAVE_ESI_BACKEND=1
    )
else()
    target_compile_definitions(${PLUGIN_IMPLEMENTATION}
        PRIVATE TELEMETRY_HAVE_ESI_BACKEND=0
    )
endif()

install(TARGETS ${PLUGIN_IMPLEMENTATION}
    DESTINATION lib/${STORAGE_DIRECTORY}/plugins
)

# Generate plugin config
write_config(${PLUGIN_NAME})
