# Pharos Demangler
#
# Copyright 2017-2018 Carnegie Mellon University. All Rights Reserved.
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

find_package(Boost 1.60.0 REQUIRED)

add_library(libdemangle SHARED demangle.cpp json.cpp demangle_json.cpp
            codes.cpp demangle_text.cpp)

set_target_properties(libdemangle PROPERTIES
  CXX_STANDARD 11
  OUTPUT_NAME demangle)

target_include_directories(libdemangle INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
  PRIVATE ${Boost_INCLUDE_DIR})


target_compile_features(libdemangle
  PUBLIC cxx_alias_templates cxx_strong_enums cxx_defaulted_functions cxx_rvalue_references
  PRIVATE cxx_lambdas)

install(TARGETS libdemangle EXPORT libdemangle
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES demangle.hpp codes.hpp code_data.hpp DESTINATION include/libdemangle)
install(EXPORT libdemangle DESTINATION lib/cmake/libdemangle)
export(TARGETS libdemangle FILE libdemangle.cmake)
