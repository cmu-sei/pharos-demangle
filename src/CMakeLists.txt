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

find_package(Boost 1.60.0 REQUIRED COMPONENTS filesystem program_options)

add_executable(demangle demangle.cpp)
target_link_libraries(demangle libdemangle Boost::filesystem Boost::program_options)
install(TARGETS demangle DESTINATION ${CMAKE_INSTALL_BINDIR})

set(POD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/demangle.pod")
set(POD_DEST "${CMAKE_CURRENT_BINARY_DIR}/demangle.1")

find_program(POD2MAN pod2man HINTS "${CMAKE_INSTALL_BINDIR}")

add_custom_command(
  OUTPUT "${POD_DEST}"
  COMMAND ${POD2MAN} --section=1 --release=Pharos --center="Pharos Tools"
          "${POD_SRC}" "${POD_DEST}"
  MAIN_DEPENDENCY "${POD_SRC}")
add_custom_target(demangle_man ALL DEPENDS "${POD_DEST}")
install(
  FILES "${POD_DEST}"
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
