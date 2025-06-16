# author: Theodore Bardy
#
# Root CMakeLists.txt file
# This file is part of the Zephyr Demo project from Witekio
# It configures the Zephyr build system for the demo project

# Set minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

# Demo runs on ESP32-S3-DevKitC-1 (v1.0)
set(BOARD "esp32s3_devkitc/esp32s3/procpu" CACHE STRING "Board for the Zephyr application")

# Set devicetree overlays
set(DTC_OVERLAY_FILE boards/esp32s3_devkitc_procpu.overlay)

# Pull Zephyr build system
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Define project
project(zephyr-witekio-demo)
target_sources(app PRIVATE src/main.c)
