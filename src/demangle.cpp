#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <boost/format.hpp>

#include <libdemangle/demangle.hpp>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: myund [--debug] <mangled-symbol>" << std::endl;
    std::exit(1);
  }

  bool debug = false;
  std::string mangled = std::string(argv[1]);
  if (argc == 3) {
    if (mangled != "-d" && mangled != "--debug") {
      std::cerr << "Second argument was not -d or --debug." << std::endl;
    }
    std::cout << "Enabling debugging" << std::endl;
    debug = true;
    mangled = std::string(argv[2]);
  }

  bool failed = false;
  try {
    auto t = visual_studio_demangle(mangled, debug);
    //t->debug_type(true, 0, "Main");
    std::cout << mangled << " " << t->str(true) << std::endl;
  }
  catch (const DemanglerError& e) {
    failed = true;
    std::cout << mangled << " " << e.what() << std::endl;
  }

  if (failed) std::exit(1);
}
