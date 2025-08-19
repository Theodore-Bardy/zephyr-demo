# @file      CMakeLists.txt
# @author    Theodore Bardy
#
# @note      This file is part of Witekio's Zephyr Demo project
# @brief     CMake file for Wi-Fi stack

# Include Wi-Fi source files
file(GLOB_RECURSE SOURCES_TEMP ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
target_sources(app PRIVATE ${SOURCES_TEMP})

# Include Wi-Fi header files
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
