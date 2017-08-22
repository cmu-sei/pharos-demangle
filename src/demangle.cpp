#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <boost/format.hpp>

#include <libdemangle/demangle.hpp>
#include <boost/filesystem.hpp>

namespace bfi = boost::filesystem;

class Demangler {
  bool debug = false;
  bool winmatch = false;
  bool nosym = false;

 public:
  void set_winmatch(bool val) {
    winmatch = val;
  }
  void set_nosym(bool val) {
    nosym = val;
  }
  void set_debug(bool val) {
    debug = val;
  }

  bool demangle(std::string const & mangled) const;
  bool operator()(std::string const & mangled) const {
    return demangle(mangled);
  }
};

[[noreturn]] void usage(char const * progname)
{
  bfi::path path(progname);
  auto fname = path.filename().string();
  std::cout << "Usage: " << fname
            << "  [--debug] [--windows] [--nosym] <mangled-symbol>" << std::endl;
  std::cout << "       " << fname
            << "  [--debug] [--windows] [--nosym] <mangled-symbol-file>" << std::endl;
  std::exit(1);
}

bool demangle_file(Demangler const & demangler, std::istream & file)
{
  bool success = true;
  std::string mangled;
  while (file >> mangled) {
    success &= demangler(mangled);
  }
  return success;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    usage(argc ? argv[0] : "demangle");
  }

  Demangler demangler;
  bool endswitch = false;
  std::string name;

  for (int arg = 1; arg < argc; ++arg) {
    std::string argstr(argv[arg]);
    if (argstr[0] == '-' && !endswitch) {
      if (argstr == "-h" || argstr == "--help") {
        usage(argc ? argv[0] : "demangle");
      } else if (argstr == "-d" || argstr == "--debug") {
        demangler.set_debug(true);
      } else if (argstr == "-w" || argstr == "--windows") {
        demangler.set_winmatch(true);
      } else if (argstr == "-n" || argstr == "--nosym") {
        demangler.set_nosym(true);
      } else if (argstr == "--") {
        endswitch = true;
      } else {
        std::cerr << "Unknown option " << argstr << std::endl;
        std::exit(1);
      }
    } else if (!name.empty()) {
      std::cerr << "Too many arguments" << std::endl;
      std::exit(1);
    } else {
      name = std::move(argstr);
    }
  }

  bool failed = false;
  if (name == "-") {
    demangle_file(demangler, std::cin);
  } else {
    bfi::path path(name);
    if (exists(path)) {
      std::ifstream file(path.string());
      demangle_file(demangler, file);
    } else {
      failed = !demangler(name);
    }
  }

  if (failed) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


bool Demangler::demangle(std::string const & mangled) const
{
  try {
    auto t = demangle::visual_studio_demangle(mangled, debug);
    auto dem = t->str(winmatch);
    if (!nosym) {
      std::cout << mangled << " ";
    }
    std::cout << dem << std::endl;
    return true;
  }
  catch (const demangle::Error& e) {
    std::cout << "! " <<  mangled << " " << e.what() << std::endl;
    return false;
  }
}
