# author: Theodore Bardy
#
# Wi-Fi CMakeLists.txt file
# This file is part of the Zephyr Demo project from Witekio
# It configures the Wi-Fi settings for the Zephyr Demo application

# Include Wi-Fi source files
file(GLOB_RECURSE SOURCES_TEMP ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
target_sources(app PRIVATE ${SOURCES_TEMP})

# Include Wi-Fi header files
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
