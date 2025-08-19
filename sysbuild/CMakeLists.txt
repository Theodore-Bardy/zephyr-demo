# @file      CMakeLists.txt
# @author    Theodore Bardy
#
# @note      This file is part of Witekio's Zephyr Demo project
# @brief     CMake file for sysbuild configuration

# Set minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

# This file is based on zephyr/share/sysbuild/template, just setting our default board

# Demo runs on ESP32-S3-DevKitC-1 (v1.0)
set(BOARD "esp32s3_devkitc/esp32s3/procpu" CACHE STRING "Board for the Zephyr application")

# Set sysbuild configuration file
set(SB_CONF_FILE "sysbuild-mcuboot.conf" CACHE STRING "Config for sysbuild")

find_package(Sysbuild REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sysbuild LANGUAGES)
