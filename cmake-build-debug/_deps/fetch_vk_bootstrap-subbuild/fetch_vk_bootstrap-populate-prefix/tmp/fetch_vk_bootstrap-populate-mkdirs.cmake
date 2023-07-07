# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-src"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-build"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-subbuild/fetch_vk_bootstrap-populate-prefix"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-subbuild/fetch_vk_bootstrap-populate-prefix/tmp"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-subbuild/fetch_vk_bootstrap-populate-prefix/src/fetch_vk_bootstrap-populate-stamp"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-subbuild/fetch_vk_bootstrap-populate-prefix/src"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-subbuild/fetch_vk_bootstrap-populate-prefix/src/fetch_vk_bootstrap-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-subbuild/fetch_vk_bootstrap-populate-prefix/src/fetch_vk_bootstrap-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/fetch_vk_bootstrap-subbuild/fetch_vk_bootstrap-populate-prefix/src/fetch_vk_bootstrap-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
