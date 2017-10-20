// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <boost/format.hpp>

#include <libdemangle/demangle.hpp>
#include <libdemangle/demangle_json.hpp>
#include <libdemangle/demangle_text.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace {

using demangle::JsonOutput;
using json::wrapper::Builder;

class Demangler {
  bool debug = false;
  bool nosym = false;
  bool raw = false;
  bool minimal = false;
  bool batch = false;
  std::unique_ptr<Builder> builder;
  std::unique_ptr<JsonOutput> json_output;
  mutable demangle::TextOutput str;

 public:
  void set_winmatch(bool) {
    str.set_attributes(demangle::TextOutput::undname());
  }
  void set_nosym(bool val) {
    nosym = val;
  }
  void set_debug(bool val) {
    debug = val;
  }
  void set_raw(bool val) {
    raw = val;
  }
  void set_minimal(bool val) {
    minimal = val;
  }
  void set_batch(bool val) {
    batch = val;
  }
  void set_json(bool val) {
    if (val) {
      if (!builder) {
        builder = json::simple_builder();
        json_output = std::unique_ptr<JsonOutput>(new JsonOutput(*builder));
      }
    } else {
      builder.reset();
      json_output.reset();
    }
  }

  bool demangle(std::string const & mangled) const;
  bool operator()(std::string const & mangled) const {
    return demangle(mangled);
  }
};

bool Demangler::demangle(std::string const & mangled) const
{
  try {
    auto t = demangle::visual_studio_demangle(mangled, debug);
    if (builder) {
      auto node = raw ? json_output->raw(*t) :
                  (minimal ? json_output->minimal(*t) : json_output->convert(*t));
      node->add("symbol", mangled);
      node->add("demangled", str.convert(*t));
      std::cout << *node;
      if (batch) {
        std::cout << std::endl;
      }
    } else {
      auto dem = str.convert(*t);
      if (!nosym) {
        std::cout << mangled << " ";
      }
      std::cout << dem << std::endl;
    }
    return true;
  }
  catch (const demangle::Error& e) {
    if (builder) {
      auto node = builder->object();
      node->add("symbol", mangled);
      node->add("error", e.what());
      std::cout << *node;
      if (batch) {
        std::cout << std::endl;
      }
    } else {
      std::cout << "! " <<  mangled << " " << e.what() << std::endl;
    }
    return false;
  }
}

struct Driver {
  bool first;
  bool nofile = false;
  bool json = false;
  bool batch = false;
  Demangler const & demangler;
  Driver(Demangler const & d) : demangler(d) {}
  bool demangle_file(std::istream & file);
  bool demangle(std::string const & sym);
  bool run(std::vector<std::string> const & args);
};

bool Driver::demangle_file(std::istream & file)
{
  bool success = true;
  std::string sym;
  while (file >> sym) {
    success &= demangle(sym);
  }
  return success;
}

bool Driver::demangle(std::string const & sym)
{
  if (json) {
    if (first) {
      first = false;
    } else if (!batch) {
      std::cout << ',';
    }
  }
  return demangler(sym);
}

bool Driver::run(std::vector<std::string> const & args)
{
  first = true;
  if (json && !batch) {
    std::cout << '[';
  }
  bool success = true;
  bool dd = false;
  for (auto & arg : args) {
    if (!dd && arg == "--") {
      // Ignore the first "--" arg
      dd = true;
      continue;
    }
    if (!dd && !nofile && arg == "-") {
      // Handle data from stding
      success &= demangle_file(std::cin);
      continue;
    }
    if (!nofile) {
      // See if arg is valid filename
      boost::filesystem::path path(arg);
      try {
        if (exists(path)) {
          // Handle data from file
          std::ifstream file(path.string());
          success &= demangle_file(file);
          continue;
        }
      } catch (boost::filesystem::filesystem_error &) {
        // If exists() fails, assume its a symbol.  Fall through.
      }
    }
    success &= demangle(arg);
  }
  if (json & !batch) {
    std::cout << ']';
  }
  return success;
}


} // unnamed namespace

int main(int argc, char **argv)
{
  namespace po = boost::program_options;

  po::options_description opt;
  opt.add_options()
    ("help,h",    "Display help")
    ("windows,w", "Try to match undname output as slavishly as possible")
    ("nosym,n",   "Only output the demangled name, not the symbol")
    ("nofile",    "Interpret arguments only as symbols, not at filenames")
    ("debug,d",   "Output demangling debugging spew")
    ("json,j",    "JSON output")
    ("raw",       "Raw JSON output")
    ("minimal",   "Minimal JSON output")
    ("batch",     "JSON objects as newline-separated, rather than in a list")
    ;

  po::options_description hidden;
  hidden.add_options()
    ("args", po::value<std::vector<std::string>>(), "Arguments")
    ;

  po::options_description allopt("Demangler options");
  allopt.add(opt).add(hidden);

  po::positional_options_description p;
  p.add("args", -1);

  bool endswitch = false;
  auto dashdash =
    [&endswitch](std::string const & val) -> std::pair<std::string, std::string> {
      if (endswitch) {
        return std::make_pair("args", val);
      }
      if (val == "--") {
        endswitch = true;
        return std::make_pair("args", val);
      }
      return std::make_pair(std::string(), std::string());
    };

  po::variables_map vm;
  try {
    store(po::command_line_parser(argc, argv)
          .options(allopt)
          .positional(p)
          .extra_parser(dashdash)
          .run(),
          vm);
  } catch (po::error const & err) {
    std::cerr << err.what() << std::endl;
    return EXIT_FAILURE;
  }
  notify(vm);

  if (vm.count("help")) {
    boost::filesystem::path path(argv[0]);
    std::cout
      << "Usage: " << path.filename().string() << " [options] [arguments...]\n\n"
      << ("Demangles mangled symbols.  The arguments are either file names or symbols.\n"
          "The special name \"-\" stands for stdin.  If no arguments are given, the\n"
          "symbols are assumed to come from stdin.  The \"--\" argument causes all\n"
          "arguments after it to be treated as symbols.\n\n")
      << opt;
    return EXIT_FAILURE;
  }

  Demangler demangler;

  if (vm.count("debug")) {
    demangler.set_debug(true);
  }
  if (vm.count("windows")) {
    demangler.set_winmatch(true);
  }
  if (vm.count("nosym")) {
    demangler.set_nosym(true);
  }
  if (vm.count("json")) {
    demangler.set_json(true);
  }
  if (vm.count("raw") && vm.count("minimal")) {
    std::cerr << "Cannot use --raw and --minimal at the same time" << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("raw")) {
    demangler.set_raw(true);
  }
  if (vm.count("minimal")) {
    demangler.set_minimal(true);
  }
  if (vm.count("batch")) {
    demangler.set_batch(true);
  }
  std::vector<std::string> args;
  if (vm.count("args")) {
    args = vm["args"].as<std::vector<std::string>>();
  }
  if (args.empty() && (std::cin.peek() != std::istream::traits_type::eof())) {
    // If there are no arguments, but we have stdin, add stdin
    args.push_back("-");
  }

  // Argument verification
  bool dd = false;
  bool stdin = false;
  auto count = decltype(args)::size_type();
  for (auto & arg : args) {
    if (dd) {
      ++count;
    } else if (arg == "--") {
      dd = true;
    } else if (arg == "-") {
      if (stdin) {
        std::cerr << "The stdin file \"-\" can only be used once" << std::endl;
        return EXIT_FAILURE;
      }
      stdin = true;
      ++count;
    } else {
      ++count;
    }
  }
  if (count == 0) {
    std::cerr << "No symbols or filenames were given" << std::endl;
    return EXIT_FAILURE;
  }

  // Do the demangling
  Driver driver(demangler);
  driver.nofile = vm.count("nofile");
  driver.json = vm.count("json");
  driver.batch = vm.count("batch");
  bool success = driver.run(args);

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
