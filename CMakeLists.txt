cmake_minimum_required(VERSION 3.5)

project(demangle CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(AddCXXCompilerFlags)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-ggdb3 gdb_debugging)
if(gdb_debugging)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
endif()
add_cxx_compiler_flags(-Wall -Wextra -Wshadow -Wstrict-aliasing)

set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_subdirectory(libdemangle)
add_subdirectory(src)
