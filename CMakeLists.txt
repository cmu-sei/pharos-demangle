# Pharos Demangler
#
# Copyright 2017-2020 Carnegie Mellon University. All Rights Reserved.
#
# NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
# INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. CARNEGIE MELLON
# UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR
# IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF
# FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS
# OBTAINED FROM USE OF THE MATERIAL. CARNEGIE MELLON UNIVERSITY DOES NOT
# MAKE ANY WARRANTY OF ANY KIND WITH RESPECT TO FREEDOM FROM PATENT,
# TRADEMARK, OR COPYRIGHT INFRINGEMENT.
#
# Released under a BSD-style license, please see license.txt or contact
# permission@sei.cmu.edu for full terms.
#
# [DISTRIBUTION STATEMENT A] This material has been approved for public
# release and unlimited distribution.  Please see Copyright notice for
# non-US Government use and distribution.
#
# DM17-0949

cmake_minimum_required(VERSION 3.5)
project(demangle VERSION 1.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(AddCXXCompilerFlags)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-ggdb3 gdb_debugging)
if(gdb_debugging)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
endif()
add_cxx_compiler_flags(-pedantic -Wall -Wextra -Wshadow -Wstrict-aliasing)

set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(BOOST_ROOT "" CACHE PATH "Where to look for Boost")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(DemangleConfigVersion.cmake
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion)
configure_package_config_file(
  "${CMAKE_SOURCE_DIR}/cmake/DemangleConfig.cmake.in"
  DemangleConfig.cmake
  INSTALL_DESTINATION lib/cmake/demangle
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

add_subdirectory(libdemangle)
add_subdirectory(src)

install(EXPORT DEMANGLE_EXPORT
  FILE DemangleTagets.cmake
  NAMESPACE "Demangle::"
  DESTINATION lib/cmake/demangle)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DemangleConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/DemangleConfigVersion.cmake"
        DESTINATION lib/cmake/demangle)
