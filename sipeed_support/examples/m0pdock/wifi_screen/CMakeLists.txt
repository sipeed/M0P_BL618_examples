cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

# Define
sdk_add_compile_definitions(-DCONFIG_CLI_CMD_ENABLE)

file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
target_sources(app PRIVATE ${sources})
target_include_directories(app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/include")

sdk_add_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/exported")
sdk_set_main_file(main.c)

project(wifi_screen)
