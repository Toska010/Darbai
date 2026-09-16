# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.29.6)

# Reject any attempt to use a toolchain file. We must not use one because
# we could be downloading it here. If the CMAKE_TOOLCHAIN_FILE environment
# variable is set, the cache variable will have been initialized from it.
unset(CMAKE_TOOLCHAIN_FILE CACHE)
unset(ENV{CMAKE_TOOLCHAIN_FILE})

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(mbedtls-populate NONE)



include(ExternalProject)
ExternalProject_Add(mbedtls-populate
                     "UPDATE_DISCONNECTED" "False" "DOWNLOAD_EXTRACT_TIMESTAMP" "FALSE" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "URL" "https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.5/mbedtls-3.6.5.tar.bz2" "URL_HASH" "SHA256=4a11f1777bb95bf4ad96721cac945a26e04bf19f57d905f241fe77ebeddf46d8" "DOWNLOAD_NO_PROGRESS" "ON" "PATCH_COMMAND" "C:/Program Files/JetBrains/CLion 2024.2.3/bin/cmake/win/x64/bin/cmake.exe" "-DMBEDTLS_DIR=C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/mbedtls-src" "-P" "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sfml-src/tools/mbedtls/PatchMbedTLS.cmake"
                    SOURCE_DIR          "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/mbedtls-src"
                    BINARY_DIR          "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/mbedtls-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


