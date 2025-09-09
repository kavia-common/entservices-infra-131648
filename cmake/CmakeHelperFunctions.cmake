# Placeholder helper functions module
# This file provides minimal no-op stubs to satisfy include() calls when the real helper is not available.
# If the repository provides a full version elsewhere, it will supersede or extend this one.

# PUBLIC_INTERFACE
function(register_service target_name)
    # No-op stub. Real implementation should register service targets with common properties.
    if(NOT DEFINED target_name)
        message(WARNING "register_service called without target_name; ignoring.")
    endif()
endfunction()

# PUBLIC_INTERFACE
function(enable_warnings target_name)
    # No-op stub. Real implementation should set compiler warnings.
    if(TARGET ${target_name})
        # Intentionally left minimal to avoid altering build flags unexpectedly.
    endif()
endfunction()

# PUBLIC_INTERFACE
function(configure_plugin target_name)
    # No-op stub. Real implementation would apply plugin-specific properties.
    if(TARGET ${target_name})
        # nothing
    endif()
endfunction()
