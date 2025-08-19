# @file      CMakeLists.txt
# @author    Theodore Bardy
#
# @note      This file is part of Witekio's Zephyr Demo project
# @brief     Root CMake file

# Set minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

# Demo runs on ESP32-S3-DevKitC-1 (v1.0)
set(BOARD
    "esp32s3_devkitc/esp32s3/procpu"
    CACHE STRING "Board for the Zephyr application")

# Set devicetree overlays
set(DTC_OVERLAY_FILE boards/esp32s3_devkitc_procpu.overlay)

# Pull Zephyr build system
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Include subdirectories
include(${CMAKE_CURRENT_LIST_DIR}/src/network/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/src/ota/CMakeLists.txt)

# Define project
project(zephyr-witekio-demo)
target_sources(app PRIVATE src/main.c)

# Install certificate if a hosted Mender server is used
if(CONFIG_MENDER_SERVER_HOST_US OR CONFIG_MENDER_SERVER_HOST_US)
  # Links to certificates used for hosted Mender US
  if(CONFIG_MENDER_SERVER_HOST_US)
    set(PRIMARY_CERTIFICATE_LINK
        "https://docs.mender.io/releases/certs/AmazonRootCA1.der")
    set(PRIMARY_CERTIFICATE_SHA256
        "8ecde6884f3d87b1125ba31ac3fcb13d7016de7f57cc904fe1cb97c6ae98196e")

    set(SECONDARY_CERTIFICATE_LINK
        "https://docs.mender.io/releases/certs/GoogleTrustServicesR4.der")
    set(SECONDARY_CERTIFICATE_SHA256
        "349dfa4058c5e263123b398ae795573c4e1313c83fe68f93556cd5e8031b3c7d")
    # Links to certificates used for hosted Mender EU
  elseif(CONFIG_MENDER_SERVER_HOST_EU)
    set(PRIMARY_CERTIFICATE_LINK
        "https://docs.mender.io/releases/certs/isrgrootx1.der")
    set(PRIMARY_CERTIFICATE_SHA256
        "96bcec06264976f37460779acf28c5a7cfe8a3c0aae11a8ffcee05c0bddf08c6")

    set(SECONDARY_CERTIFICATE_LINK
        "https://docs.mender.io/releases/certs/GoogleTrustServicesR4.der")
    set(SECONDARY_CERTIFICATE_SHA256
        "349dfa4058c5e263123b398ae795573c4e1313c83fe68f93556cd5e8031b3c7d")
  endif()

  set(PRIMARY_CERTIFICATE "${CMAKE_CURRENT_BINARY_DIR}/PrimaryCertificate.cer")
  file(DOWNLOAD ${PRIMARY_CERTIFICATE_LINK} ${PRIMARY_CERTIFICATE}
       EXPECTED_HASH SHA256=${PRIMARY_CERTIFICATE_SHA256})

  set(SECONDARY_CERTIFICATE
      "${CMAKE_CURRENT_BINARY_DIR}/SecondaryCertificate.cer")
  file(DOWNLOAD ${SECONDARY_CERTIFICATE_LINK} ${SECONDARY_CERTIFICATE}
       EXPECTED_HASH SHA256=${SECONDARY_CERTIFICATE_SHA256})
else()
  # This is an impossible configuration, MENDER_SERVER_HOST is a choice
  message(
    WARNING
      "Either CONFIG_MENDER_SERVER_HOST_US, CONFIG_MENDER_SERVER_HOST_EU or CONFIG_MENDER_SERVER_HOST_ON_PREM needs to be selected"
  )
endif()

if(PRIMARY_CERTIFICATE)
  add_custom_target(certificate_one DEPENDS ${PRIMARY_CERTIFICATE})
  add_dependencies(app certificate_one)

  generate_inc_file_for_target(
    certificate_one ${PRIMARY_CERTIFICATE}
    "${ZEPHYR_BINARY_DIR}/include/generated/PrimaryCertificate.cer.inc")
endif()

if(SECONDARY_CERTIFICATE)
  add_custom_target(certificate_two DEPENDS ${SECONDARY_CERTIFICATE})
  add_dependencies(app certificate_two)

  generate_inc_file_for_target(
    certificate_two ${SECONDARY_CERTIFICATE}
    "${ZEPHYR_BINARY_DIR}/include/generated/SecondaryCertificate.cer.inc")
endif()

# Set TLS configuration
file(COPY_FILE "config-tls-mender.h"
     "${ZEPHYR_BINARY_DIR}/include/generated/config-tls-mender.h")
