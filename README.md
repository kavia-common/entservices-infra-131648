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
