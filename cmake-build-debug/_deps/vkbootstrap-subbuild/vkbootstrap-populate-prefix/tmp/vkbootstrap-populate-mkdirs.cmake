# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-src"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-build"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-subbuild/vkbootstrap-populate-prefix"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-subbuild/vkbootstrap-populate-prefix/tmp"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-subbuild/vkbootstrap-populate-prefix/src/vkbootstrap-populate-stamp"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-subbuild/vkbootstrap-populate-prefix/src"
  "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-subbuild/vkbootstrap-populate-prefix/src/vkbootstrap-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-subbuild/vkbootstrap-populate-prefix/src/vkbootstrap-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/theo/Documents/CPP/vk_engine/cmake-build-debug/_deps/vkbootstrap-subbuild/vkbootstrap-populate-prefix/src/vkbootstrap-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
