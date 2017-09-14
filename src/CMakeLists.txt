find_package(Boost 1.60.0 REQUIRED COMPONENTS filesystem program_options)

add_executable(demangle demangle.cpp json.cpp demangle_json.cpp)
target_link_libraries(demangle libdemangle Boost::filesystem Boost::program_options)
install(TARGETS demangle DESTINATION bin)
