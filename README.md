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

## Telemetry optional backend

The Telemetry plugin can integrate with an external EntServicesInfraPlugins package when available.

- Control with: -DENABLE_TELEMETRY_ESI_BACKEND=ON|OFF (default: ON)
- If enabled but the package is not found in your environment, the build will proceed without the ESI backend and emit a warning.
- To enable the integration, provide the package via CMAKE_PREFIX_PATH or set EntServicesInfraPlugins_DIR.

## Notes

- Some optional features depend on SDK libraries such as RFC, RBUS, IARMBus, or WPEFrameworkSecurityUtil. When available, they will be detected and linked; otherwise the build will continue with those features disabled.
