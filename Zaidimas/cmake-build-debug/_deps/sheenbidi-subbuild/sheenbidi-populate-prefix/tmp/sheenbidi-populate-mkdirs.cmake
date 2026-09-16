# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-src"
  "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-build"
  "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-subbuild/sheenbidi-populate-prefix"
  "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-subbuild/sheenbidi-populate-prefix/tmp"
  "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-subbuild/sheenbidi-populate-prefix/src/sheenbidi-populate-stamp"
  "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-subbuild/sheenbidi-populate-prefix/src"
  "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-subbuild/sheenbidi-populate-prefix/src/sheenbidi-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-subbuild/sheenbidi-populate-prefix/src/sheenbidi-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/toska/Desktop/PraktikaDarbas/Zaidimas/cmake-build-debug/_deps/sheenbidi-subbuild/sheenbidi-populate-prefix/src/sheenbidi-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
