# StaticAnalysis.cmake
#
# Configures static analysis tools when ENABLE_CLANG_TIDY or ENABLE_CPPCHECK options are enabled.
# This module is safe to include multiple times and is a no-op when tools are not requested or not found.
#
# Usage:
#   option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
#   option(ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)
#   include(StaticAnalysis)
#
# Behavior:
#   - When ENABLE_CLANG_TIDY is ON and clang-tidy is found, sets CMAKE_CXX_CLANG_TIDY and CMAKE_C_CLANG_TIDY.
#   - When ENABLE_CPPCHECK is ON and cppcheck is found, sets CMAKE_CXX_CPPCHECK and CMAKE_C_CPPCHECK.
#   - If tool is requested but not found, emits a WARNING and continues without breaking the build.

include_guard(GLOBAL)

# Helper to append an arg only if the value is defined and not empty
function(_sa__append_if_set out_var flag value)
    if(DEFINED value AND NOT "${value}" STREQUAL "")
        set(tmp "${${out_var}}")
        list(APPEND tmp "${flag}" "${value}")
        set(${out_var} "${tmp}" PARENT_SCOPE)
    endif()
endfunction()

# Configure clang-tidy
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-16 clang-tidy-15 clang-tidy-14 clang-tidy-13)
    if(CLANG_TIDY_EXE)
        # Base arguments: point to compile_commands.json of the current build dir
        set(_CLANG_TIDY_ARGS)
        list(APPEND _CLANG_TIDY_ARGS "-p" "${CMAKE_BINARY_DIR}")

        # Respect a project-level .clang-tidy in the source tree automatically.
        # clang-tidy will auto-discover .clang-tidy files, so no need to pass config explicitly.

        # Enable extra header filtering if user provides CLANG_TIDY_HEADER_FILTER via cache/env
        _sa__append_if_set(_CLANG_TIDY_ARGS "--header-filter" "${CLANG_TIDY_HEADER_FILTER}")

        # Compose command lists for C and C++
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" ${_CLANG_TIDY_ARGS})
        set(CMAKE_C_CLANG_TIDY   "${CLANG_TIDY_EXE}" ${_CLANG_TIDY_ARGS})

        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE} ${_CLANG_TIDY_ARGS}")
    else()
        message(WARNING "ENABLE_CLANG_TIDY is ON but clang-tidy was not found on PATH. Static analysis will be skipped.")
    endif()
endif()

# Configure cppcheck
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES cppcheck)
    if(CPPCHECK_EXE)
        set(_CPPCHECK_ARGS)
        # Common enable sets; adjust as needed per project tolerance
        list(APPEND _CPPCHECK_ARGS "--enable=warning,style,performance,portability")
        list(APPEND _CPPCHECK_ARGS "--inline-suppr")
        list(APPEND _CPPCHECK_ARGS "--std=c++17")

        # Use compile_commands for precise analysis if available
        if(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
            list(APPEND _CPPCHECK_ARGS "--project=${CMAKE_BINARY_DIR}/compile_commands.json")
        endif()

        # Honor a suppression list if present at repo root
        if(EXISTS "${CMAKE_SOURCE_DIR}/.cppcheck.suppress")
            list(APPEND _CPPCHECK_ARGS "--suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck.suppress")
        endif()

        # Quiet noisy missing system includes (cppcheck limitation on some platforms)
        list(APPEND _CPPCHECK_ARGS "--suppress=missingIncludeSystem")

        # Compose command lists for C and C++
        set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE}" ${_CPPCHECK_ARGS})
        set(CMAKE_C_CPPCHECK   "${CPPCHECK_EXE}" ${_CPPCHECK_ARGS})

        message(STATUS "cppcheck enabled: ${CPPCHECK_EXE} ${_CPPCHECK_ARGS}")
    else()
        message(WARNING "ENABLE_CPPCHECK is ON but cppcheck was not found on PATH. Static analysis will be skipped.")
    endif()
endif()
