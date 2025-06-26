# author: Theodore Bardy
#
# Over-the-Air (OTA) CMakeLists.txt file
# This file is part of the Zephyr Demo project from Witekio
# It configures the OTA service for Golioth framework for the Zephyr Demo application
# https://github.com/golioth/golioth-firmware-sdk/tree/v0.17.0/examples/zephyr

# Include OTA source files
file(GLOB_RECURSE SOURCES_TEMP ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
target_sources(app PRIVATE ${SOURCES_TEMP})

# Include OTA header files
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
