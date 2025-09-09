# EntServices Infra Plugins

This repository contains a suite of WPEFramework plugins that provide storage, analytics, telemetry, device settings, and more.

## Building

A typical build:

```
cmake -S . -B build
cmake --build build -j
```

If you use SDKs installed in non-default locations, point CMake to them using:

- CMAKE_PREFIX_PATH to include one or more install prefixes that contain package configs under lib/cmake
- Or set variables for specific packages, for example:
  - WPEFramework_DIR=/path/to/wpeframework/lib/cmake/WPEFramework
  - EntServicesInfraPlugins_DIR=/path/to/esi/lib/cmake/EntServicesInfraPlugins

Example:

```
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH="/opt/wpeframework;/opt/esi"
cmake --build build -j
```

### CMake cache options and usage

You can customize builds using cache options and flags passed with -DName=Value. Below is a catalog of the most relevant options and cache variables grouped by purpose, with their defaults.

General usage:
- Enable option: -DOPTION_NAME=ON
- Disable option: -DOPTION_NAME=OFF
- Set string/path: -DVAR_NAME=/path/to/value or -DVAR_NAME="string"
- Inspect cache after configure: cmake -S . -B build -LH

Global build and tooling
- ENABLE_CLANG_TIDY (OFF): Enable clang-tidy static analysis if clang-tidy is available.
- ENABLE_CPPCHECK (OFF): Enable cppcheck static analysis if cppcheck is available.
- COMCAST_CONFIG (ON): Include services.cmake defaults for plugin toggles and feature flags.
- DISABLE_SECURITY_TOKEN (OFF): Define DISABLE_SECURITY_TOKEN for builds that must not use the security token.
- WPEFramework discovery: Set WPEFramework_DIR or include its prefix in CMAKE_PREFIX_PATH. If not found, plugins requiring it will be auto-disabled.
- Packaging (optional): When WPEFRAMEWORK_CREATE_IPKG_TARGETS=ON, the following are consulted:
  - WPEFRAMEWORK_PLUGINS_OPKG_NAME, WPEFRAMEWORK_PLUGINS_OPKG_VERSION, WPEFRAMEWORK_PLUGINS_OPKG_ARCHITECTURE, WPEFRAMEWORK_PLUGINS_OPKG_MAINTAINER, WPEFRAMEWORK_PLUGINS_OPKG_DESCRIPTION, WPEFRAMEWORK_PLUGINS_OPKG_FILE_NAME

Tests and examples/JS assets
- RDK_SERVICE_L2_TEST (OFF): Enable L2 tests.
- RDK_SERVICES_L1_TEST (OFF): Enable L1 tests.
- ENABLE_EXAMPLE_TEST_PLUGINS_AND_JS (ON): Master toggle for example/test plugins and JavaScript assets. When OFF:
  - L1/L2 tests are forced OFF.
  - JS-dependent features like NativeJS are disabled.
  - When ON but Node/NPM/JavaScriptCore are not present, JS features are auto-disabled with a status message.

JavaScript toolchains (auto-detected)
- Node/NPM: CMake probes for node/nodejs and npm on PATH.
- JavaScriptCore: CMake probes for JavaScriptCore.
- Behavior: If toolchains are missing, JS features (e.g., NativeJS) are skipped automatically. To build NativeJS explicitly, ensure these toolchains are available and set -DPLUGIN_NATIVEJS=ON (guarded at the top level).

Services integration (IARM/DBUS and related flags)
- USE_IARM (ON): Enable IARM integration (requires IARMBus). Auto-disabled if not found.
- USE_IARM_BUS (ON): Enable IARM Bus integration. Auto-disabled if not found.
- BUILD_DBUS (OFF unless set): Enable DBUS transport support (defines BUILD_DBUS and IARM_USE_DBUS).
- IARM_USE_DBUS (ON when BUILD_DBUS): Enable IARM over DBUS (set automatically when BUILD_DBUS is ON).
- Additional feature flags (set as boolean cache values when passed to cmake):
  - CONTINUEWATCHING_DISABLE_SECAPI
  - DISABLE_GEOGRAPHY_TIMEZONE
  - BUILD_ENABLE_SYSTIMEMGR_SUPPORT
  - BUILD_ENABLE_THERMAL_PROTECTION
  - BUILD_ENABLE_DEVICE_MANUFACTURER_INFO
  - SUPPRESS_MAINTENANCE
  - BUILD_ENABLE_CLOCK
  - BUILD_ENABLE_EXTENDED_ALL_SEGMENTS_TEXT_PATTERN
  - ENABLE_SYSTEM_GET_STORE_DEMO_LINK
  - BUILD_ENABLE_TELEMETRY_LOGGING
  - BUILD_ENABLE_LINK_LOCALTIME
  - NET_DISABLE_NETSRVMGR_CHECK
  - ENABLE_WHOAMI
  - ENABLE_RFC_MANAGER
  - DISABLE_DCM_TASK
  - BUILD_ENABLE_ERM

Plugin selection (top-level toggles)
These control whether subprojects are added. Defaults come from services.cmake and CMakeLists.txt.
- PLUGIN_OCICONTAINER (OFF)
- PLUGIN_APPGATEWAY (ON)
- PLUGIN_APP2APP_PROVIDER (ON)
- PLUGIN_LAUNCHDELEGATE (ON)
- PLUGIN_WAREHOUSE (ON)
- HAS_API_HDMI_INPUT (ON)
- PLUGIN_COPILOT (OFF)
- PLUGIN_FRAMERATE (ON)
- PLUGIN_STORAGE_MANAGER (ON)
- PLUGIN_DEVICEDIAGNOSTICS (ON)
- PLUGIN_SOUNDPLAYER (OFF)
- PLUGIN_TELEMETRY (ON)
- PLUGIN_LEDCONTROL (ON)
- PLUGIN_CONTINUEWATCHING (ON)
- PLUGIN_NATIVEJS (not defined by option; can be enabled by passing -DPLUGIN_NATIVEJS=ON when toolchains are satisfied)

Per-plugin cache variables
Unless noted, values are STRINGs. Modes are commonly “Off”, “Local”, or “Remote/out-of-process” depending on the plugin.

- UserSettings
  - PLUGIN_USERSETTINGS_AUTOSTART ("true")
  - PLUGIN_USERSETTINGS_STARTUPORDER ("51")

- USBMassStorage
  - PLUGIN_USB_MASS_STORAGE_AUTOSTART ("false")
  - PLUGIN_USB_MASS_STORAGE_STARTUPORDER ("45")

- SharedStorage
  - PLUGIN_SHAREDSTORAGE_MODE ("Off")
  - PLUGIN_SHAREDSTORAGE_STARTUPORDER ("51")

- USBDevice
  - PLUGIN_USBDEVICE_AUTOSTART ("true")
  - PLUGIN_USBDEVICE_STARTUPORDER ("40")

- MessageControl
  - PLUGIN_MESSAGECONTROL_AUTOSTART (true)
  - PLUGIN_MESSAGECONTROL_ABBREVIATED (true)
  - PLUGIN_MESSAGECONTROL_MAX_EXPORTCONNECTIONS (5)
  - PLUGIN_MESSAGECONTROL_REMOTE ("false")
  - PLUGIN_MESSAGECONTROL_PORT ("0")
  - PLUGIN_MESSAGECONTROL_BINDING ("0.0.0.0")

- Monitor (BOOL toggles auto-derived from other plugins; memory limits in bytes)
  - Plugin monitors (BOOL): PLUGIN_MONITOR_OPENCDMI, PLUGIN_MONITOR_WEBKITBROWSER, PLUGIN_MONITOR_WEBKITBROWSER_APPS, PLUGIN_MONITOR_WEBKITBROWSER_RESIDENT_APP, PLUGIN_MONITOR_WEBKITBROWSER_UX, PLUGIN_MONITOR_WEBKITBROWSER_YOUTUBE, PLUGIN_MONITOR_SYSTEMAUDIOPLAYER
  - External plugin monitors (BOOL): PLUGIN_MONITOR_AMAZON, PLUGIN_MONITOR_COBALT, PLUGIN_MONITOR_NETFLIX, PLUGIN_MONITOR_OUTOFPROCESS, PLUGIN_MONITOR_TESTUTILITY
  - Memory limits (STRING): PLUGIN_MONITOR_AMAZON_MEMORYLIMIT, PLUGIN_MONITOR_COBALT_MEMORYLIMIT, PLUGIN_MONITOR_NETFLIX_MEMORYLIMIT, PLUGIN_MONITOR_WEBKITBROWSER_MEMORYLIMIT, PLUGIN_MONITOR_WEBKITBROWSER_APPS_MEMORYLIMIT, PLUGIN_MONITOR_WEBKITBROWSER_RESIDENT_APP_MEMORYLIMIT, PLUGIN_MONITOR_WEBKITBROWSER_UX_MEMORYLIMIT, PLUGIN_MONITOR_WEBKITBROWSER_YOUTUBE_MEMORYLIMIT, PLUGIN_MONITOR_NETWORKMANAGER_MEMORYLIMIT
  - General: PLUGIN_MONITOR_AUTOSTART ("true"), PLUGIN_MONITOR_STARTUPORDER ("")

- WebBridge
  - PLUGIN_WEBBRIDGE_STARTUPORDER ("")

- RuntimeManager
  - PLUGIN_RUNTIME_MANAGER_MODE ("Off")
  - PLUGIN_RUNTIME_MANAGER_AUTOSTART (false)
  - PLUGIN_RUNTIME_MANAGER_STARTUPORDER ("")
  - PLUGIN_RUNTIME_MANAGER_EXTRA_LIBRARIES ("")
  - JSONCPP_INCLUDE_PATH (PATH, only used with L1 tests; default "/usr/include/jsoncpp")

- ResourceManager
  - PLUGIN_RESOURCE_MANAGER_AUTOSTART (true)
  - PLUGIN_RESOURCE_MANAGER_STARTUPORDER ("")
  - PLUGIN_RESOURCE_MANAGER_EXTRA_LIBRARIES ("")

- Analytics
  - PLUGIN_ANALYTICS_STARTUPORDER ("")
  - PLUGIN_ANALYTICS_AUTOSTART ("false")
  - PLUGIN_ANALYTICS_EVENTS_MAP ("")
  - PLUGIN_ANALYTICS_LOGGER_NAME ("Analytics")
  - PLUGIN_ANALYTICS_LOGGER_VERSION ("1.0.4")
  - PLUGIN_ANALYTICS_BACKEND_LIBRARY_NAME ("")

- PersistentStore
  - PLUGIN_PERSISTENTSTORE_MODE ("Off")
  - PLUGIN_PERSISTENTSTORE_PATH ("/opt/secure/persistent/rdkservicestore")
  - PLUGIN_PERSISTENTSTORE_LEGACYPATH ("/opt/persistent/rdkservicestore")
  - PLUGIN_PERSISTENTSTORE_KEY ("")
  - PLUGIN_PERSISTENTSTORE_MAXSIZE ("1000000")
  - PLUGIN_PERSISTENTSTORE_MAXVALUE ("3000")
  - PLUGIN_PERSISTENTSTORE_LIMIT ("10000")
  - PLUGIN_PERSISTENTSTORE_STARTUPORDER ("")

- StorageManager
  - PLUGIN_STORAGE_MANAGER_MODE ("Off")
  - PLUGIN_STORAGE_MANAGER_AUTOSTART (false)
  - PLUGIN_STORAGE_MANAGER_STARTUPORDER ("")
  - PLUGIN_STORAGE_MANAGER_EXTRA_LIBRARIES ("")
  - PLUGIN_STORAGE_MANAGER_PATH ("/opt/persistent/storageManager")

- RDKWindowManager
  - PLUGIN_RDK_WINDOW_MANAGER_MODE ("Off")
  - PLUGIN_RDK_WINDOW_MANAGER_AUTOSTART (false)
  - PLUGIN_RDK_WINDOW_MANAGER_STARTUPORDER ("")
  - PLUGIN_RDK_WINDOW_MANAGER_EXTRA_LIBRARIES ("")

- CloudStore
  - PLUGIN_CLOUDSTORE_MODE ("Off")
  - PLUGIN_CLOUDSTORE_URI ("")
  - PLUGIN_CLOUDSTORE_STARTUPORDER ("")

- OCIContainer
  - PLUGIN_OCICONTAINER_STARTUPORDER ("")

- AppManager
  - PLUGIN_APP_MANAGER_MODE ("Off")
  - PLUGIN_APP_MANAGER_AUTOSTART (false)
  - PLUGIN_APP_MANAGER_STARTUPORDER ("")
  - PLUGIN_APP_MANAGER_EXTRA_LIBRARIES ("")

- PackageManager
  - PLUGIN_PACKAGEMANAGER_MODE ("Local")
  - PLUGIN_PACKAGEMANAGER_AUTOSTART ("false")

- LifecycleManager
  - PLUGIN_LIFECYCLE_MANAGER_MODE ("Local")
  - PLUGIN_LIFECYCLE_MANAGER_AUTOSTART (false)
  - PLUGIN_LIFECYCLE_MANAGER_STARTUPORDER ("")
  - PLUGIN_LIFECYCLE_MANAGER_EXTRA_LIBRARIES ("")

- Telemetry
  - ENABLE_TELEMETRY_ESI_BACKEND (ON) [option]: enable ESI backend when EntServicesInfraPlugins package is available
  - PLUGIN_T2_PERSISTENT_FOLDER ("/opt/.t2reportprofiles/")
  - PLUGIN_DEFAULT_PROFILES_FILE ("/etc/t2profiles/default.json")

- RDKShell
  - PLUGIN_RDKSHELL_AUTOSTART (true)
  - PLUGIN_RDKSHELL_STARTUPORDER ("")
  - PLUGIN_RDKSHELL_READ_MAC_ON_STARTUP (OFF) [option]
  - PLUGIN_HIBERNATESUPPORT (OFF) [option]
  - PLUGIN_HIBERNATE_NATIVE_APPS_ON_SUSPENDED (OFF) [option]
  - RIALTO_FEATURE (BOOL, not a cache option by default but supported): when set, enables Rialto integration

Environment-assisted include paths (optional)
Some plugins accept environment-provided include lists; values are split by spaces and added as include directories.
- RESOURCE_MANAGER_INCLUDES
- STORAGE_MANAGER_INCLUDES
- APP_MANAGER_INCLUDES
- RUNTIME_MANAGER_INCLUDES
- RDK_WINDOW_MANAGER_INCLUDES
- RDKSHELL_INCLUDES
- LIFECYCLE_MANAGER_INCLUDES

Install/layout notes
- NAMESPACE controls the install namespace and is used to compute STORAGE_DIRECTORY (lower-cased NAMESPACE).
- Plugin binaries are installed to lib/${STORAGE_DIRECTORY}/plugins under CMAKE_INSTALL_PREFIX (default: /usr/local).

Examples
- Build with Telemetry ESI backend disabled and CloudStore URI set:
  ```
  cmake -S . -B build \
    -DENABLE_TELEMETRY_ESI_BACKEND=OFF \
    -DPLUGIN_CLOUDSTORE_URI="https://example.com/secure" \
    -DCMAKE_PREFIX_PATH="/opt/wpeframework;/opt/esi"
  cmake --build build -j
  ```

- Disable example/test and JS assets, enable only specific plugins:
  ```
  cmake -S . -B build \
    -DENABLE_EXAMPLE_TEST_PLUGINS_AND_JS=OFF \
    -DPLUGIN_STORAGE_MANAGER=ON \
    -DPLUGIN_PERSISTENTSTORE=ON \
    -DPLUGIN_TELEMETRY=ON
  ```

- Enable RDKShell hibernate features and set startup order:
  ```
  cmake -S . -B build \
    -DPLUGIN_RDKSHELL_AUTOSTART=true \
    -DPLUGIN_RDKSHELL_STARTUPORDER="20" \
    -DPLUGIN_HIBERNATESUPPORT=ON \
    -DPLUGIN_HIBERNATE_NATIVE_APPS_ON_SUSPENDED=ON
  ```

- Turn on static analysis:
  ```
  cmake -S . -B build \
    -DENABLE_CLANG_TIDY=ON \
    -DENABLE_CPPCHECK=ON
  ```

## Telemetry optional backend

The Telemetry plugin can integrate with an external EntServicesInfraPlugins package when available.

- Control with: -DENABLE_TELEMETRY_ESI_BACKEND=ON|OFF (default: ON)
- If enabled but the package is not found in your environment, the build will proceed without the ESI backend and emit a warning.
- To enable the integration, provide the package via CMAKE_PREFIX_PATH or set EntServicesInfraPlugins_DIR.

## Notes

- Some optional features depend on SDK libraries such as RFC, RBUS, IARMBus, or WPEFrameworkSecurityUtil. When available, they will be detected and linked; otherwise the build will continue with those features disabled.

---

## Code style and formatting

### EditorConfig policy

This repository includes a .editorconfig that enforces:
- LF line endings
- UTF-8 encoding
- Consistent indentation (spaces) and size per file type
- Trimming trailing whitespace and ensuring a final newline

Configure your editor to respect .editorconfig (most modern IDEs do this automatically).

### C/C++ formatting (clang-format)

We recommend using clang-format to keep C/C++ sources consistent. If a .clang-format is not present, the tool’s default style (LLVM) is applied by your local installation.

- Install:
  - Linux: via your package manager (e.g., clang-format)
  - macOS: brew install clang-format
  - Windows: via LLVM installer or Visual Studio tooling

- Format in place (all tracked C/C++ files):
  ```
  git ls-files '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx' | xargs -r clang-format -i
  ```

- Check formatting without modifying files (useful for CI and local checks):
  ```
  git ls-files '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx' | xargs -r clang-format --dry-run --Werror
  ```

- IDE integration:
  - VS Code: C/C++ extension supports clang-format (“Format on Save”).
  - CLion/Qt Creator/Visual Studio: built-in clang-format support can be enabled to run on save.

### Optional: pre-commit hook

Using pre-commit is optional but recommended to enforce formatting before commits.

1) Install and enable:
```
pip install pre-commit
pre-commit install
```

2) Example .pre-commit-config.yaml snippet:
```
repos:
  - repo: https://github.com/pre-commit/mirrors-clang-format
    rev: v17.0.0  # Use a version matching your toolchain
    hooks:
      - id: clang-format
        files: "\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$"
```

3) Run manually on demand:
```
pre-commit run -a
```

This ensures staged C/C++ files are formatted consistently across contributors.
