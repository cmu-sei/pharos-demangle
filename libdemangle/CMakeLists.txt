find_package(Boost 1.60.0 REQUIRED)
add_library(libdemangle SHARED demangle.cpp)
set_target_properties(libdemangle PROPERTIES OUTPUT_NAME demangle)
target_include_directories(libdemangle INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)


install(TARGETS libdemangle EXPORT libdemangle DESTINATION lib)
install(FILES demangle.hpp DESTINATION include/libdemangle)
install(EXPORT libdemangle DESTINATION lib/cmake/libdemangle)
