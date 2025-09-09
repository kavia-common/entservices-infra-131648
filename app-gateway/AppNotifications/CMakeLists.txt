###
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2025
#
# Licensed under the Apache License, Version 2.0 (the "License");
###

cmake_minimum_required(VERSION 3.3)

project(AppNotifications)

find_package(WPEFramework)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

set(PLUGIN_NAME AppNotifications)

set(HEADERS
    Module.h
    AppNotifications.h
    AppNotificationsTypes.h
)

set(SOURCES
    Module.cpp
    AppNotifications.cpp
    AppNotificationsJsonRpc.cpp
    AppNotificationsImplementation.cpp
)

add_library(${PLUGIN_NAME} SHARED
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(${PLUGIN_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Link against WPEFramework core libraries
target_link_libraries(${PLUGIN_NAME}
    PRIVATE
        WPEFrameworkCore
        WPEFrameworkPlugins
        WPEFrameworkTracing
)

# Set properties and install
set_target_properties(${PLUGIN_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    FRAMEWORK TRUE
)

install(TARGETS ${PLUGIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install plugin config to /etc/WPEFramework/plugins
if(DEFINED CMAKE_INSTALL_FULL_SYSCONFDIR)
    set(SYSCONFDIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}")
else()
    set(SYSCONFDIR "/etc")
endif()

install(FILES
    ${PLUGIN_NAME}.config
    DESTINATION ${SYSCONFDIR}/WPEFramework/plugins)

# Optionally package as part of WPEFramework ipkg if enabled at top-level
if(WPEFRAMEWORK_CREATE_IPKG_TARGETS)
    set(CPACK_DEBIAN_${PLUGIN_NAME}_PACKAGE_NAME "${PLUGIN_NAME}")
endif()
