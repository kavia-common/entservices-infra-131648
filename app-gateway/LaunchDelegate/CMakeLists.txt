# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2023 RDK
#
# Licensed under the Apache License, Version 2.0 (the "License");

cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

project(LaunchDelegate)

set(TARGET LaunchDelegate)

add_library(${TARGET} SHARED
    Module.cpp
    LaunchDelegate.cpp
)

target_include_directories(${TARGET}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET}
    PRIVATE
        WPEFrameworkCore
        WPEFrameworkPlugins
)

set_target_properties(${TARGET} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

install(TARGETS ${TARGET}
    DESTINATION lib/${NAMESPACE}/plugins)

# Generate a minimal configuration file, can be overridden by deployment
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LaunchDelegate.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/LaunchDelegate.config @ONLY)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LaunchDelegate.config
    DESTINATION /etc/${NAMESPACE}/plugins)
