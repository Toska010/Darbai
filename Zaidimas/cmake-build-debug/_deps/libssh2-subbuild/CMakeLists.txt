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

project(libssh2-populate NONE)



include(ExternalProject)
ExternalProject_Add(libssh2-populate
                     "UPDATE_DISCONNECTED" "False" "DOWNLOAD_EXTRACT_TIMESTAMP" "FALSE" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "URL" "https://github.com/libssh2/libssh2/archive/704299e997bf518375dc9222670c57b800ac59e6.tar.gz" "URL_HASH" "SHA256=10c3a28bf3099b2a4a6c21a7f4a180fb1bacf5ee6560ccc5542e62f762b2f4f1" "DOWNLOAD_NO_PROGRESS" "ON" "PATCH_COMMAND" "C:/Program Files/JetBrains/CLion 2024.2.3/bin/cmake/win/x64/bin/cmake.exe" "-DLIBSSH2_DIR=C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/libssh2-src" "-DMODULES_DIR=C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sfml-src/src/SFML/Network/../../../cmake/Modules" "-P" "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sfml-src/tools/libssh2/PatchLibssh2.cmake"
                    SOURCE_DIR          "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/libssh2-src"
                    BINARY_DIR          "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/libssh2-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


